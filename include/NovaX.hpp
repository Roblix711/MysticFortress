
// header gaurd
#ifndef _NOVA_PUBLIC_H
#define _NOVA_PUBLIC_H

// windows stuff
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <Windows.h>

// typedefs 
#include <Types.hpp>

// wrap it up to go
namespace NOVA
{
    // template niceness
    template <typename T> class MODULUS;
    template<UINT32 W> class MEASUREFPS;
    template<class T> class  SHARED;
    template<class T> class  SHARED_EXT;
    template<class T> class  ACCESS_BUFFER;

    // exports
    class MUTEX_GENERIC;
    class MUTEX_FAST;
    class MUTEX;
    class SCOPE_LOCK;

    // implementations 
    class THREAD_IMPL;
    class TIMER_IMPL;
    class SIGNAL_IMPL;
    class RACESYNC_IMPL;

    // V1.0 interface
    class THREAD;
    class TIMER;
    class SIGNAL;
    class RACESYNC;
    class THREAD_FORK;

};

// more typdefs for ease
typedef void(*VOID_FPTR_MT)(NOVA::THREAD*, VECTOR<METADATA>);

// abtract class
class NOVA::MUTEX_GENERIC
{
    public:
    virtual void Lock()   = 0;
    virtual void Unlock() = 0;
};

// mutexes
class NOVA::MUTEX_FAST : public NOVA::MUTEX_GENERIC
{
    public:
    CRITICAL_SECTION CriticalSection;

    // constructor destructor
    MUTEX_FAST(DWORD SpinCountMax = 4000);
    ~MUTEX_FAST();

    // some functions generic to mutexes
    void Lock();
    void Unlock();
};

// a generic mutex class
class NOVA::MUTEX : public NOVA::MUTEX_GENERIC
{
    public:
    HANDLE InternalMutex;

    // constructor destructor
    MUTEX();
    ~MUTEX();

    // some functions generic to mutexes
    void Lock();
    void Unlock();
};

// config for thread
struct THREAD_CONFIG
{
    VOID_FPTR_MT InputCallback;
    UINT32 SleepTimeMS;
    VECTOR<METADATA> MetaIn;
    THREAD_CONFIG(VOID_FPTR_MT C, UINT32 S = 1, VECTOR<METADATA> M = {}): InputCallback(C), MetaIn(M), SleepTimeMS(S)
    {}
};

// public api for THREAD
class NOVA::THREAD
{
    public:
    // extra data
    METADATA SpecialData;

    // constructor destructor
    THREAD(VECTOR<METADATA> MetadataArray);
    ~THREAD();

    // helpers for state model
    void WaitForStop();
    void WaitForStart();
    BOOLEEN Start();
    void Stop();
    BOOLEEN IsRunning();
    BOOLEEN IsExiting();
    BOOLEEN InCallback();
    BOOLEEN IsFinished();
    void SetRunning(BOOLEEN bNewRunningState);
    void SetExiting(BOOLEEN bNewExitingState);
    void SetInCallback(BOOLEEN bNewCallbackState);
    void SetFinished(BOOLEEN bNewFinishedState);
    void SetSeed(UINT32 X);

    // details hidden
    private:
    NOVA::THREAD_IMPL* _Impl;
};

// public api for SIGNAL
class NOVA::SIGNAL
{
    public:
    // constructor
    SIGNAL();
    ~SIGNAL();

    // methods
    void WaitForTrigger(BOOLEEN bNew);
    void Trigger();
    void Terminate();

    // details hidden
    private:
    NOVA::SIGNAL_IMPL* _Impl;
};


// public race syncronizer
class NOVA::RACESYNC
{
    public:
    // constructor
    RACESYNC(UINT32 J);
    ~RACESYNC();

    // methods
    void Sync();
    void Terminate();

    // details hidden
    private:
    NOVA::RACESYNC_IMPL* _Impl;
};

// public api for THREAD
class NOVA::TIMER
{
    public:
    TIMER();
    ~TIMER();

    // timer functions, Tick starts, Tock Stops
    void   Tick();
    DOUBLE Tock(BOOLEEN bAutoTick = true);
    STRING _GetTimestampString();
    UINT64 _GetTimestampUINT64();

    // static versions
    static STRING GetTimestampString();
    static UINT64 GetTimestampUINT64();
    static STRING GetTimestampStringFormatted(STRING Format = "%Y-%m-%d %H:%M:%S");

    // details hidden
    private:
    NOVA::TIMER_IMPL* _Impl;
};

// helps with "enclosing"
class NOVA::SCOPE_LOCK
{
	public:
	NOVA::MUTEX_FAST& Target;
    SCOPE_LOCK(NOVA::MUTEX_FAST& Src);
	~SCOPE_LOCK();
};


// buffered shared variable -> seperates read/writes
template<class T>
class NOVA::ACCESS_BUFFER
{
    public:
    T                       UnsharedData;
    std::atomic<T>         SharedData;
    std::atomic<BOOLEEN>   bNewData;

	// default constructors
	ACCESS_BUFFER<T>() :		UnsharedData()      { bNewData = false; }
	ACCESS_BUFFER<T>(T& Pass) : UnsharedData(Pass)  { bNewData = false; }
	ACCESS_BUFFER<T>(T  Pass) : UnsharedData(Pass)  { bNewData = false; }

	// proxy to virtuals
	inline ACCESS_BUFFER<T>& operator= (T& In)                  { return ApplyWriteOperator(In); }
	inline operator T()                                         { return ApplyReadOperator(); }
    inline virtual ACCESS_BUFFER<T>& Sync()                     
    { 
        // try and get the newest
        if (bNewData)  {bNewData = false; UnsharedData = SharedData;} 
        return *this; 
    }

    // overload as needed
protected:
	inline virtual ACCESS_BUFFER<T>& ApplyWriteOperator(T& In)      { SharedData = In; bNewData = true; return *this; }  // write the passed raw value
	inline virtual T                 ApplyReadOperator()            { return  UnsharedData; }    // do a transfer
    
};



// only for opencv
#ifdef __OPENCV_ALL_HPP__

// specialized "clone" for when the input is CV::MAT, since matA=matB doesn't do a deep copy
template<> NOVA::SHARED<cv::Mat>& NOVA::SHARED<cv::Mat>::ApplyWriteOperator(cv::Mat& In)
{
    Container = In.clone();
    return *this;
}
template<> cv::Mat NOVA::SHARED<cv::Mat>::ApplyReadOperator()
{
    return Container.clone();
}

#endif


// measures fps -> using Tick();
template<UINT32 W>
class NOVA::MEASUREFPS
{
    public:

    MEASUREFPS(): Index(0), Total(0.0)
    {
        InternalTimer.Tick();
        for(UINT32 i=0; i<W; ++i) WindowTime[i] = 0.0;
    }
    void Tick()
    {
         InternalTimer.Tick();
    }
    DOUBLE Tock()
    {
        DOUBLE Sample = InternalTimer.Tock();
        DOUBLE Average = AddToFPSAverage(Sample);
        DOUBLE NewFPS = 1000.0/Average;
        CurrentFPS = NewFPS;
        CurrentAvgTime = Average;
        return NewFPS;
    }
    // threadsafe read
    DOUBLE GetFPS()
    {
        return CurrentFPS;
    }
    DOUBLE GetTime()
    {
        return CurrentAvgTime;
    }
    // internal working mechanism
    private:
    DOUBLE                  WindowTime[W];
    UINT32                  Index;
    DOUBLE                  Total;
    NOVA::TIMER             InternalTimer;
    std::atomic<DOUBLE>    CurrentFPS;
    std::atomic<DOUBLE>    CurrentAvgTime;

    // internally add to the average
    DOUBLE AddToFPSAverage(DOUBLE value)
    {
        Total += value;
        Total -= WindowTime[Index];
        WindowTime[Index] = value;
        Index++;
        Index %= W;
        return Total/(1.0*W);
    }

};


// wrapper used to pass these objects to a new thread so it looks more pretty
class NOVA::THREAD_FORK
{
    public:
    
    // special constructor and destructor
	THREAD_FORK(){}
    ~THREAD_FORK(){}

    // forward declare some stuff
    template <              class CALLBACKFUNC, class DATATYPE> class SPAWN_THREAD;
    template <class MEMBER, class CALLBACKFUNC, class DATATYPE> class MEMBER_THREAD;


    // spawns a thread with the callback function, passing the data parameters
    template <class CALLBACKFUNC, class DATATYPE>
    inline HANDLE Fork(CALLBACKFUNC ForkedFunction, DATATYPE ForkedData, DWORD SleepMs = 0)
    {
        SPAWN_THREAD<CALLBACKFUNC, DATATYPE> NewThread;
        return NewThread.Fork(ForkedFunction, ForkedData, SleepMs);
    }

    template <class MEMBER, class CALLBACKFUNC, class DATATYPE>
    inline HANDLE MemberFork(MEMBER *This, CALLBACKFUNC ForkedFunction, DATATYPE ForkedData, DWORD SleepMs = 0)
    {
        MEMBER_THREAD<MEMBER, CALLBACKFUNC, DATATYPE> NewThread;
        return NewThread.MemberFork(This, ForkedFunction, ForkedData, SleepMs);
    }

    // hide the rest of the stuff
    private:

    // internal thread that spawns
    template <class CALLBACKFUNC, class DATATYPE>
    class SPAWN_THREAD
    {
        public:
        
        // special constructor and destructor
        SPAWN_THREAD()  {}
        ~SPAWN_THREAD() {}

        // internal wrapper class
        template <class CALLBACKFUNC, class DATATYPE>
        struct STATIC_DATA_WRAPPER
        {
            CALLBACKFUNC CallbackFunction;
            DATATYPE     CallbackData;
        };

        // spawns a thread with the callback function, passing the data parameters
        HANDLE Fork(CALLBACKFUNC ForkedFunction, DATATYPE ForkedData, DWORD SleepMs = 0)
        {
            // make a wrapper for the function call type
            STATIC_DATA_WRAPPER<CALLBACKFUNC, DATATYPE> *MyWrapper = new STATIC_DATA_WRAPPER<CALLBACKFUNC, DATATYPE>;
            MyWrapper->CallbackFunction = ForkedFunction;
            MyWrapper->CallbackData     = ForkedData;

            // do the call, sleep, and return the handle
            if (SleepMs > 0) Sleep(SleepMs);
	        return CreateThread(NULL, 0, &StaticThreadBranch, (LPVOID*) MyWrapper, 0, NULL);
        }

        // spawns a thread with the callback function, passing the data parameters -> overloaded without sleep
        HANDLE Fork(CALLBACKFUNC ForkedFunction, DATATYPE ForkedData)
        {
            // make a wrapper for the function call type
            STATIC_DATA_WRAPPER<CALLBACKFUNC, DATATYPE> *MyWrapper = new STATIC_DATA_WRAPPER<CALLBACKFUNC, DATATYPE>;
            MyWrapper->CallbackFunction = ForkedFunction;
            MyWrapper->CallbackData     = ForkedData;

            // return the handle
	        return CreateThread(NULL, 0, &StaticThreadBranch, (LPVOID*) MyWrapper, 0, NULL);
        }

        // hide the rest of the stuff
        private:

        // small function that branches off
        static DWORD WINAPI StaticThreadBranch(LPVOID lpParam) 
        {
            // get our data wrapper -> cast to type
            STATIC_DATA_WRAPPER<CALLBACKFUNC, DATATYPE> *MyPassedWrapper = (STATIC_DATA_WRAPPER<CALLBACKFUNC, DATATYPE>*) lpParam;

            // call our outside function 
            MyPassedWrapper->CallbackFunction(MyPassedWrapper->CallbackData);

            // delete memory here after we're done with it
            delete MyPassedWrapper;

            return 0;
        }
    };


    // internal thread that spawns
    template <class MEMBER, class CALLBACKFUNC, class DATATYPE>
    class MEMBER_THREAD
    {
        public:
        
        // special constructor and destructor
        MEMBER_THREAD() {}
        ~MEMBER_THREAD() {}

       // internal wrapper class
        template <class MEMBER, class CALLBACKFUNC, class DATATYPE>
        struct STATIC_DATA_WRAPPER
        {
            CALLBACKFUNC  CallbackFunction;
            DATATYPE      CallbackData;
            MEMBER       *CallbackParent;
        };

        // spawns a thread with the callback function, passing the data parameters
        HANDLE MemberFork(MEMBER *This, CALLBACKFUNC ForkedFunction, DATATYPE ForkedData, DWORD SleepMs = 0)
        {
            // make a wrapper for the function call type
            STATIC_DATA_WRAPPER<MEMBER, CALLBACKFUNC, DATATYPE> *MyWrapper = new STATIC_DATA_WRAPPER<MEMBER, CALLBACKFUNC, DATATYPE>;
            MyWrapper->CallbackFunction = ForkedFunction;
            MyWrapper->CallbackData     = ForkedData;
            MyWrapper->CallbackParent   = This;

            // do the call, sleep, and return the handle
            if (SleepMs > 0) Sleep(SleepMs);
	        return CreateThread(NULL, 0, &StaticThreadBranch, (LPVOID*) MyWrapper, 0, NULL);
        }

        // hide the rest of the stuff
        private:

        // small function that branches off
        static DWORD WINAPI StaticThreadBranch(LPVOID lpParam) 
        {
            // get our data wrapper -> cast to type
            STATIC_DATA_WRAPPER<MEMBER, CALLBACKFUNC, DATATYPE> *MyPassedWrapper = (STATIC_DATA_WRAPPER<MEMBER, CALLBACKFUNC, DATATYPE>*) lpParam;

            // call our outside function 
            //MyPassedWrapper->CallbackFunction
            CALLBACKFUNC Test = (MyPassedWrapper->CallbackFunction);
            ((MyPassedWrapper->CallbackParent)->*Test) (MyPassedWrapper->CallbackData);

            // delete memory here after we're done with it
            delete MyPassedWrapper;

            return 0;
        }

    };

};


#endif // _NOVA_PUBLIC_H