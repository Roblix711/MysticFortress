
// header gaurd
#ifndef _NOVA_TIMER_H
#define _NOVA_TIMER_H

// include private header
#include "Nova.h"

// High Def Counter
class NOVA::TIMER_IMPL
{
    private:
    BOOLEEN bHDClock;	        // Set to 1 for high precision clocking....
	INT64 HDFrequency;          // holds the number of counts per second
	INT64 HDCurrentTime;        // holds current QPC count
	INT64 HDLastTime;           // holds last QPC count
	INT64 HDDifference;         // Difference in ms from last to current
	DWORD      CurrentTime;		// current tick count from the CPU
	DWORD      LastTime;		// last tick count	from the CPU

    public:
    TIMER_IMPL(); 

    // timer functions, Tick starts, Tock Stops
    void   Tick();
    DOUBLE Tock(BOOLEEN bAutoTick = true);
    STRING _GetTimestampString();
    UINT64 _GetTimestampUINT64();

    // static versions
    static STRING GetTimestampString();
    static UINT64 GetTimestampUINT64();
    static STRING GetTimestampStringFormatted(STRING Format = "%Y-%m-%d %H:%M:%S");

};


#endif //_NOVA_TIMER_H