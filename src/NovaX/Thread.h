
// header gaurd
#ifndef _NOVA_THREAD_H
#define _NOVA_THREAD_H

// include private header
#include "Nova.h"

// enable some debug for us
#ifndef DEBUG_THREADIO 
#define DEBUG_THREADIO 0
#endif

// special member function pointer
//template <class MEMBER>
//typedef void (MEMBER::*FPTR_Int)(int);

/* =================================================================================================================== 
HOW TO USE THIS THREADED WRAPPER

// METHOD 1
SPAWN_THREAD<FPTR_Int, int> NewThread;
for(UINT i=0; i<500; ++i)
NewThread.Fork( ThreadedPrintf, i);


// METHOD 2
THREAD NewThread;
for(UINT i=0; i<500; ++i)
NewThread.Fork<FPTR_Int, int>(ThreadedPrintf, i);

// METHOD 3
THREAD StateThread;
for(UINT i=0; i<1000; ++i)
{
    ThreadHandle = StateThread.MemberFork<RFID_PROTOCOL, FPTR_RFID_Int, int>(this, &RFID_PROTOCOL::StateMachine, i, 0);
}
=================================================================================================================== */

// wrapper for easy threads
class NOVA::THREAD_IMPL
{
protected:
    VOID_FPTR_MT            TargetCallback;    // needs a target function to call
    NOVA::THREAD_FORK       InternalThread; 
    MICROSOFT::HANDLE       ThreadHandle; 
    std::atomic<BOOLEEN>   bExitSignaler;
    std::atomic<BOOLEEN>   bRunningSignaler;
    std::atomic<BOOLEEN>   bInCallback;
    std::atomic<BOOLEEN>   bFinished;
    UINT32                  InternalSeed;
    NOVA::MUTEX_FAST        StateLock;

public:
    VECTOR<METADATA> MetaData;
    UINT32 ConfigSleepTimeUS;
    NOVA::THREAD            *InterfaceParent;

    // constructor destructor 
    THREAD_IMPL(VOID_FPTR_MT InputCallback, NOVA::THREAD *Parent, VECTOR<METADATA> MetaIn = {}, UINT32 SleepTimeMS = 1); // configed to 1 MS, QD is for testing purposes only
    ~THREAD_IMPL();

    // helpers for state model
    void    WaitForStop();
    void    WaitForStart();
    BOOLEEN Start();
    void    Stop();
    BOOLEEN IsRunning();
    BOOLEEN IsExiting();
    BOOLEEN InCallback();
    BOOLEEN IsFinished();
    void    SetRunning(BOOLEEN bNewRunningState);
    void    SetExiting(BOOLEEN bNewExitingState);
    void    SetInCallback(BOOLEEN bNewCallbackState);
    void    SetFinished(BOOLEEN bNewFinishedState);
    void    SetSeed(UINT32 X);

    // statics
    static void* ThreadMain(void* Unknown);

};

// implements a "signal" 
class NOVA::SIGNAL_IMPL
{
    public:
    CONDITION_VARIABLE      EventData;
    CRITICAL_SECTION        EventLock;
    std::atomic<BOOLEEN>   bNewData;
    std::atomic<BOOLEEN>   bExitCondition;

    // constructor
    SIGNAL_IMPL();
    ~SIGNAL_IMPL();

    // methods
    void WaitForTrigger(BOOLEEN bNew);
    void Trigger();
    void Terminate();
};


// implementes a bidirectional signal
class NOVA::RACESYNC_IMPL
{
    private:
    HANDLE AccessSemaphore;
    NOVA::MUTEX_FAST ThreadAccess;
    std::atomic<UINT32> JoinCount;
    const UINT32 MaxCount;

    // included multiprong signal
    CONDITION_VARIABLE          EventData;
    VECTOR<CRITICAL_SECTION>    EventLock;
    std::atomic<BOOLEEN>       bSyncReady;
    std::atomic<BOOLEEN>       bExitCondition;

    // constructor
    public:
    RACESYNC_IMPL(UINT32 MaxCount);
    ~RACESYNC_IMPL();

    // methods
    void Sync();
    void Terminate();
};


#endif
