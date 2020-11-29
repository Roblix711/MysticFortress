
// our header
#include "Timer.h"
#include <ctime>

// typedefs for ease
typedef BOOL MSBOOL;

// constructor 
NOVA::TIMER::TIMER(): _Impl(new NOVA::TIMER_IMPL())
{
    // NOCODE
}

// destructor
NOVA::TIMER::~TIMER()
{
    delete _Impl;
}

// timer functions, Tick starts, Tock Stops -> PROXY
void   NOVA::TIMER::Tick()                      {         _Impl->Tick();                 }
DOUBLE NOVA::TIMER::Tock(BOOLEEN bAutoTick)     {  return _Impl->Tock(bAutoTick);        }
STRING NOVA::TIMER::_GetTimestampString()       {  return _Impl->_GetTimestampString();   }
UINT64 NOVA::TIMER::_GetTimestampUINT64()       {  return _Impl->_GetTimestampUINT64();   }


// static versions
STRING NOVA::TIMER::GetTimestampString()        {  return NOVA::TIMER_IMPL::GetTimestampString();   }
UINT64 NOVA::TIMER::GetTimestampUINT64()        {  return NOVA::TIMER_IMPL::GetTimestampUINT64();   }
STRING NOVA::TIMER::GetTimestampStringFormatted(STRING Format) {  return NOVA::TIMER_IMPL::GetTimestampStringFormatted(Format);   }



// default constructor checks if we can do HPC
NOVA::TIMER_IMPL::TIMER_IMPL()
{
    // assign to a windows spec BOOLEAN
    MICROSOFT::MSBOOL Success = MICROSOFT::QueryPerformanceFrequency( (MICROSOFT::LARGE_INTEGER*) &HDFrequency); // GET FREQ
    bHDClock = Success ? TRUE : FALSE;

    // v2.0 editions
    Tick(); // must tick -> prevents negative times returned by Tock() if Tick() was never called -> could mess up
}

// Starts timer
void NOVA::TIMER_IMPL::Tick()
{
    // get the calculated time difference in ms
    if (bHDClock == FALSE)
    {
	    LastTime = MICROSOFT::GetTickCount();
    }
    else
    {
	    MICROSOFT::QueryPerformanceCounter( (MICROSOFT::LARGE_INTEGER*) &HDLastTime);
    }
}

// Stops timer, and retuns difference in ms
DOUBLE NOVA::TIMER_IMPL::Tock(BOOLEEN bAutoTick)
{
    // get the calculated time difference in ms
    DOUBLE CalculatedDifference;
    if (bHDClock == FALSE)
    {
	    CurrentTime = MICROSOFT::GetTickCount();
	    CalculatedDifference = (CurrentTime - LastTime);
	    LastTime = CurrentTime;
        return CalculatedDifference;
    }
    else
    {
        MICROSOFT::MSBOOL Success = MICROSOFT::QueryPerformanceCounter( (MICROSOFT::LARGE_INTEGER*) &HDCurrentTime);
        //if (Success == FALSE) throw NOVA::EXCEPTION("Timer Failed");
        HDDifference = HDCurrentTime; // dont merge statements!
	    HDDifference -= HDLastTime;
        DOUBLE Diff32 = DOUBLE(HDDifference*1.0);
        DOUBLE Freq32 = DOUBLE(HDFrequency*1.0);
        if (bAutoTick == true) HDLastTime = HDCurrentTime; // only reset base if we explicitly said so
        CalculatedDifference = Diff32/Freq32;
        return 1000.0*CalculatedDifference;
    }

}

// unsigned int64 in microseconds as string
STRING NOVA::TIMER_IMPL::_GetTimestampString()
{
    CHAR Temp[64] = {0};
    GLOBAL::sprintf_s<64>(Temp, "%I64d", _GetTimestampUINT64());
    STRING Output = Temp;
    return Temp;
}

// unsigned int64 in microseconds
UINT64 NOVA::TIMER_IMPL::_GetTimestampUINT64()
{
    Tick();
    return ((MICROSOFT::LARGE_INTEGER*) &HDLastTime)->QuadPart*1000000/(HDFrequency);
}


// gets timestamp as a UINT64 -> uint64_T
UINT64 NOVA::TIMER_IMPL::GetTimestampUINT64() 
{
    SYSTEMTIME myTime;    GetLocalTime(&myTime);
    FILETIME myTimeFile;  SystemTimeToFileTime(&myTime, &myTimeFile);
    ULARGE_INTEGER SecondCount;
    SecondCount.LowPart = myTimeFile.dwLowDateTime;
    SecondCount.HighPart = myTimeFile.dwHighDateTime;
    return SecondCount.QuadPart;
}

// gets timestamp (in usec) as string
STRING NOVA::TIMER_IMPL::GetTimestampString() 
{
    char Temp[64] = {0};
    SYSTEMTIME myTime;    GetLocalTime(&myTime);
    FILETIME myTimeFile;  SystemTimeToFileTime(&myTime, &myTimeFile);
    ULARGE_INTEGER SecondCount;
    SecondCount.LowPart = myTimeFile.dwLowDateTime;
    SecondCount.HighPart = myTimeFile.dwHighDateTime;
    sprintf_s(Temp,  "%llu", SecondCount.QuadPart);
    std::string Output = Temp;
    return Output;
}

// gets the timestamp as a formatted string
STRING NOVA::TIMER_IMPL::GetTimestampStringFormatted(STRING Format)
{
    time_t rawtime;
    tm timeinfo;
    char buffer[64];
    char timestr[64];
    time (&rawtime);
    localtime_s(&timeinfo, &rawtime);

    // use any strftime format spec here
    strftime(timestr, sizeof(timestr), Format.c_str(), &timeinfo); // NO timezone
    sprintf_s(buffer, "%s", timestr);
    STRING stdBuffer(buffer);
    return stdBuffer;
}
