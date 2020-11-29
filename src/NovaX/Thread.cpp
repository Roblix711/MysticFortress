
// main header
#include "Thread.h"

// headers
#include <time.h>
#include <iostream>

// constructor destructor
NOVA::MUTEX::MUTEX()
{
    InternalMutex = MICROSOFT::CreateMutex(NULL, FALSE, NULL);
}
NOVA::MUTEX::~MUTEX()
{
    MICROSOFT::CloseHandle(InternalMutex);
}

// some functions generic to mutexes
void NOVA::MUTEX::Lock()
{
    MICROSOFT::DWORD dwWaitResult = MICROSOFT::WaitForSingleObject(InternalMutex, INFINITE);
}
void NOVA::MUTEX::Unlock()
{
    MICROSOFT::ReleaseMutex(InternalMutex);
}

// critical section -> FAST MUTEX
NOVA::MUTEX_FAST::MUTEX_FAST(MICROSOFT::DWORD SpinCountMax)
{
    MICROSOFT::InitializeCriticalSectionAndSpinCount(&CriticalSection, SpinCountMax);
}
NOVA::MUTEX_FAST::~MUTEX_FAST()
{
    MICROSOFT::DeleteCriticalSection(&CriticalSection);
}

// some functions generic to mutexes
void NOVA::MUTEX_FAST::Lock()
{
    MICROSOFT::EnterCriticalSection(&CriticalSection);
}
void NOVA::MUTEX_FAST::Unlock()
{
    MICROSOFT::LeaveCriticalSection(&CriticalSection);
}
// open lock
NOVA::SCOPE_LOCK::SCOPE_LOCK(NOVA::MUTEX_FAST& Src) : Target(Src) 
{ 
    Target.Lock();    
}
// close lock
NOVA::SCOPE_LOCK::~SCOPE_LOCK()                                   
{ 
    Target.Unlock();  
}

// interface constructor 
NOVA::THREAD::THREAD(VECTOR<METADATA> MetadataArray)
{
    THREAD_CONFIG InternalConfig = *static_cast<THREAD_CONFIG*>(MetadataArray[0]);
    _Impl = new NOVA::THREAD_IMPL(InternalConfig.InputCallback, this, InternalConfig.MetaIn, InternalConfig.SleepTimeMS);

    // check v2.0 interface
    if (MetadataArray.size() >= 2)
    {
        // ignore warnings here
        UINT32 NewSeed = reinterpret_cast<UINT32>(static_cast<UINT32*>(MetadataArray[1]));
        _Impl->SetSeed(NewSeed);
    }

}

// interface destructor
NOVA::THREAD::~THREAD()
{
    delete _Impl;
}

// helpers for state model
void    NOVA::THREAD::WaitForStop()                                 { _Impl->WaitForStop(); }
void    NOVA::THREAD::WaitForStart()                                { _Impl->WaitForStart(); }
BOOLEEN NOVA::THREAD::Start()                                       { return _Impl->Start(); }
void    NOVA::THREAD::Stop()                                        { _Impl->Stop(); }
BOOLEEN NOVA::THREAD::IsRunning()                                   { return _Impl->IsRunning(); }
BOOLEEN NOVA::THREAD::IsExiting()                                   { return _Impl->IsExiting(); }
BOOLEEN NOVA::THREAD::InCallback()                                  { return _Impl->InCallback(); }
BOOLEEN NOVA::THREAD::IsFinished()                                  { return _Impl->IsFinished(); }
void    NOVA::THREAD::SetRunning(BOOLEEN bNewRunningState)          { _Impl->SetRunning(bNewRunningState); }
void    NOVA::THREAD::SetExiting(BOOLEEN bNewExitingState)          { _Impl->SetExiting(bNewExitingState); }
void    NOVA::THREAD::SetInCallback(BOOLEEN bNewCallbackState)      { _Impl->SetInCallback(bNewCallbackState); }
void    NOVA::THREAD::SetFinished(BOOLEEN bNewFinishedState)        { _Impl->SetFinished(bNewFinishedState); }
void    NOVA::THREAD::SetSeed(UINT32 Newseed)                       { _Impl->SetSeed(Newseed); }


// constructor // , QDParams Params
NOVA::THREAD_IMPL::THREAD_IMPL(VOID_FPTR_MT InputCallback, NOVA::THREAD *Parent, VECTOR<METADATA> MetaIn, UINT32 SleepTimeUS) : 
    TargetCallback(InputCallback), 
    ConfigSleepTimeUS(SleepTimeUS),  //, SleepParams(Params)
    InterfaceParent(Parent)
{
    // these are init manually because of shared type
    bInCallback = false;
    bRunningSignaler = false;
    bExitSignaler = false;
    bFinished = true;
    MetaData = MetaIn;

    // sets a seed
    //InternalSeed = time(0);
}

// destructor
NOVA::THREAD_IMPL::~THREAD_IMPL()
{
    Stop();
}

// restart / start the thread
bool NOVA::THREAD_IMPL::Start()
{
    NOVA::SCOPE_LOCK MyLock(StateLock);

    // only make new thread if we're not started
    if (IsRunning() == false)
    {
        // set exiting to false, and run thread
        SetExiting(false);
        SetFinished(false);
        ThreadHandle = InternalThread.Fork(NOVA::THREAD_IMPL::ThreadMain, this);
        if (ThreadHandle == NULL)  return false;
        else
        {
            WaitForStart();
            return true;
        }

    }
    else // can't stop second time
    {
        //if (DEBUG_THREADIO) std::cerr << "Warnin: Multiple Start() Detected" << std::endl;
        return false;
    }
}

// blocking call to stop thread
void NOVA::THREAD_IMPL::WaitForStop()
{
    // wait for signal that we've stopped
    if (DEBUG_THREADIO) std::cerr << "Waiting for Exit" << std::endl;
    while (IsRunning() == true || IsFinished() == false)
    {
        MICROSOFT::Sleep(10);
        //usleep(SleepParams.D1);
    }
    if (DEBUG_THREADIO) std::cerr << "            OK" << std::endl;

    // small sleep to ensure that thread closed
    MICROSOFT::Sleep(1);
}

// blocking call to stop thread
void NOVA::THREAD_IMPL::WaitForStart()
{
    // wait for signal that we've stopped
    if (DEBUG_THREADIO) std::cerr << "Waiting for Start" << std::endl;
    while (IsRunning() == false && IsFinished() == false)
    {
        //usleep(SleepParams.D2);
        MICROSOFT::Sleep(10);
    }
    if (DEBUG_THREADIO) std::cerr << "            OK" << std::endl;
}


// blocking call to stop thread -> do not call this from the callback -> else deadlock
void NOVA::THREAD_IMPL::Stop()
{
    NOVA::SCOPE_LOCK MyLock(StateLock);
    if (IsRunning() == true)
    {
        // signal to stop
        SetExiting(true);
        WaitForStop();
        MICROSOFT::CloseHandle(ThreadHandle);
    }

    // legacy
    //if (InCallback())
    //    std::cerr << "Warnin: Stop() -> Thread SIGNALED to stop instead" << std::endl;
}

// our target thread function
void* NOVA::THREAD_IMPL::ThreadMain(void* Unknown)
{
    // get ptr back to self
    NOVA::THREAD_IMPL *THIS = (NOVA::THREAD_IMPL*) Unknown;

    // special sleep
    //usleep(THIS->SleepParams.D3);

    // signal self that we're done
    THIS->SetRunning(true);

    // go infinite loop here
    while (true)
    {
        // check exit condition
        if (THIS->IsExiting() == true)
        {
            THIS->SetExiting(false);
            THIS->SetFinished(true);
            break;
        }
        else
        {
            // send message
            THIS->SetInCallback(true);
            THIS->TargetCallback(THIS->InterfaceParent, THIS->MetaData);
            THIS->SetInCallback(false);

            // sleeps some amount
            if (THIS->ConfigSleepTimeUS > 0)
            {
                MICROSOFT::Sleep(THIS->ConfigSleepTimeUS);
            }
        }
    }

    // signal self that we're done
    THIS->SetRunning(false);

    // thread done
    return 0;
}

// check if we're running
BOOLEEN NOVA::THREAD_IMPL::IsRunning()
{
    return bRunningSignaler;
}

// reutnrs true if its signaled to exit the thread
BOOLEEN NOVA::THREAD_IMPL::IsExiting()
{
    return bExitSignaler;
}
// returns true if we're in the callback function
BOOLEEN NOVA::THREAD_IMPL::InCallback()
{
    return bInCallback;
}

// returns true if we finished a round of executions (start -> end)
BOOLEEN NOVA::THREAD_IMPL::IsFinished()
{
    return bFinished;
}

// sets the running state
void NOVA::THREAD_IMPL::SetRunning(BOOLEEN bNewRunningState)
{
    // signal self that we're done
    //if (DEBUG_THREADIO) std::cerr << "Set Running = " << bNewRunningState << std::endl;
    bRunningSignaler = bNewRunningState;
}

// sets the exiting state
void NOVA::THREAD_IMPL::SetExiting(BOOLEEN bNewExitingState)
{
    // clear exit signaler
    //if (DEBUG_THREADIO) std::cerr << "Set Exiting = " << bNewExitingState << std::endl;
    bExitSignaler = bNewExitingState;
}

// sets the callback state
void NOVA::THREAD_IMPL::SetInCallback(BOOLEEN bNewCallbackState)
{
    // signal self that we're done
    bInCallback = bNewCallbackState;
}

// sets the running state
void NOVA::THREAD_IMPL::SetFinished(BOOLEEN bNewFinishedState)
{
    // signal self that we're done
    //if (DEBUG_THREADIO) std::cerr << "Set Finished = " << bNewFinishedState << std::endl;
    bFinished = bNewFinishedState;
}

// changes seed on thread
void NOVA::THREAD_IMPL::SetSeed(UINT32 X)
{
    GLOBAL::srand(X);
}

// constructor 
NOVA::SIGNAL::SIGNAL(): _Impl(new NOVA::SIGNAL_IMPL)
{

}
// destructor 
NOVA::SIGNAL::~SIGNAL()
{
    delete _Impl;
}

// signal trigger / wait
void NOVA::SIGNAL::WaitForTrigger(BOOLEEN bNew)
{
    _Impl->WaitForTrigger(bNew);
}
// signal trigger / wait
void NOVA::SIGNAL::Trigger()
{
    _Impl->Trigger();
}

// terminates and throws waiting thread into exception
void NOVA::SIGNAL::Terminate()
{
    _Impl->Terminate();
}

// constructr
NOVA::SIGNAL_IMPL::SIGNAL_IMPL()
{
    // init sync
    bNewData = false;        
    MICROSOFT::InitializeConditionVariable(&EventData);
    MICROSOFT::InitializeCriticalSection(&EventLock);
}

// destructor
NOVA::SIGNAL_IMPL::~SIGNAL_IMPL()
{
    // might put something here...
}

// wakes the shit up
void NOVA::SIGNAL_IMPL::Trigger()
{
    bNewData = true;
    MICROSOFT::WakeConditionVariable(&EventData);
}

// terminates and throws waiting thread into exception
void NOVA::SIGNAL_IMPL::Terminate()
{
    MICROSOFT::WakeConditionVariable(&EventData);
}

// waits for input data
void NOVA::SIGNAL_IMPL::WaitForTrigger(BOOLEEN bNew)
{
    if (bNew) bNewData = false;

    MICROSOFT::EnterCriticalSection (&EventLock);
    while( bNewData == false && bExitCondition == false)
    {
        MICROSOFT::SleepConditionVariableCS(&EventData, &EventLock, INFINITE);
    }
    MICROSOFT::LeaveCriticalSection(&EventLock);

    // special throw
    if (bExitCondition == true)
    {
        throw EXCEPTION_RUNTIME("NOVA::SIGNAL_IMPL::Sync() Terminated!");
    }
}

// constructor 
NOVA::RACESYNC::RACESYNC(UINT32 J): _Impl(new NOVA::RACESYNC_IMPL(J))
{
    // NOCODE
}

// destructor 
NOVA::RACESYNC::~RACESYNC()
{
    delete _Impl;
}

// proxy
void NOVA::RACESYNC::Sync()
{
    _Impl->Sync();
}
void NOVA::RACESYNC::Terminate()
{
    _Impl->Terminate();
}

// constructor
NOVA::RACESYNC_IMPL::RACESYNC_IMPL(UINT32 J): MaxCount(J)
{
    JoinCount = 0; // just in case
    bSyncReady = false;    
    AccessSemaphore = MICROSOFT::CreateSemaphoreA(NULL, MaxCount, MaxCount, NULL);

    // lock up the mutex first!
    bExitCondition = false;

    // make syncs
    EventLock.resize(MaxCount);
    for(UINT32 i=0; i<MaxCount; ++i)
    {
        MICROSOFT::InitializeCriticalSection(&EventLock[i]);
    }
    MICROSOFT::InitializeConditionVariable(&EventData);

}
// destructor close handles
NOVA::RACESYNC_IMPL::~RACESYNC_IMPL()
{
    MICROSOFT::CloseHandle(AccessSemaphore);
}

// causes a "throw" in all threads to terminate right away
void NOVA::RACESYNC_IMPL::Terminate()
{
    // enter the lobby
    ThreadAccess.Lock();
    bExitCondition = true;
    MICROSOFT::WakeAllConditionVariable(&EventData);
    while (JoinCount > 0) { MICROSOFT::Sleep(0); }
    ThreadAccess.Unlock();
}

// sync all threads here, 
void NOVA::RACESYNC_IMPL::Sync()
{
    // enter the lobby
    ThreadAccess.Lock();
    DWORD WaitResult = MICROSOFT::WaitForSingleObject(AccessSemaphore, INFINITE);
    switch(WaitResult)
    {
        case WAIT_OBJECT_0:
        {
            // lock up
            UINT32 MyIndex = JoinCount;

            // everyone waits 
            if (MyIndex < (MaxCount-1))
            {
                MICROSOFT::EnterCriticalSection (&EventLock[MyIndex]);
                JoinCount += 1;
                ThreadAccess.Unlock();
                while( bSyncReady == false && bExitCondition == false)
                {
                    MICROSOFT::SleepConditionVariableCS(&EventData, &EventLock[MyIndex], INFINITE);
                }

                // special check for termination
                if (bExitCondition == true)
                {
                    MICROSOFT::LeaveCriticalSection(&EventLock[MyIndex]);
                    MICROSOFT::ReleaseSemaphore(AccessSemaphore, 1, NULL);
                    JoinCount -= 1;
                    throw EXCEPTION_RUNTIME("NOVA::RACESYNC_IMPL::Sync() Terminated!");
                }
                else
                {
                    JoinCount -= 1;
                    MICROSOFT::LeaveCriticalSection(&EventLock[MyIndex]);
                }
            } 
            else // EXCEPT! the last guy, he triggers everyone!!
            {
                // sync everything
                bSyncReady = true;
                
                // finally release semaphore ALL -> 1 for each thread
                while (JoinCount > 0) 
                { 
                    MICROSOFT::WakeAllConditionVariable(&EventData); 
                    MICROSOFT::Sleep(0); 
                }
                MICROSOFT::ReleaseSemaphore(AccessSemaphore, MaxCount, NULL);
                bSyncReady = false;
                ThreadAccess.Unlock();
            }

        } break;
        default: throw EXCEPTION_RUNTIME("NOVA::RACESYNC_IMPL::Sync() Unable to attain semaphore lock!");
    }
}