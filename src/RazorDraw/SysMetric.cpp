
// main include
#include "SysMetric.h"

// deps
#include <Windows.h>
#include <Psapi.h>
#include <Pdh.h>


// gets the memory footprint (3 values)
VECTOR<DOUBLE> RAZOR::SYS_METRIC::GetMemoryFootprint()
{
    MEMORYSTATUSEX memInfo;
    memInfo.dwLength = sizeof(MEMORYSTATUSEX);
    MICROSOFT::GlobalMemoryStatusEx(&memInfo);
    DWORDLONG totalVirtualMem = memInfo.ullTotalPageFile;
    DWORDLONG virtualMemUsed = memInfo.ullTotalPageFile - memInfo.ullAvailPageFile;

    // virtual memory used by process
    PROCESS_MEMORY_COUNTERS_EX pmc;
    MICROSOFT::GetProcessMemoryInfo(GetCurrentProcess(), (PROCESS_MEMORY_COUNTERS*) &pmc, sizeof(pmc));
    SIZE_T virtualMemUsedByMe = pmc.PrivateUsage;

    // get totals
    DWORDLONG totalPhysMem = memInfo.ullTotalPhys; 
    DWORDLONG physMemUsed = memInfo.ullTotalPhys - memInfo.ullAvailPhys;
    SIZE_T physMemUsedByMe = pmc.WorkingSetSize;  

    // give back
    VECTOR<DOUBLE> Output;
    Output.push_back(double(((unsigned long long) virtualMemUsedByMe) >> 10)/1024.0);
    Output.push_back(double(((unsigned long long) physMemUsed) >> 10)/1024.0);
    Output.push_back(double(((unsigned long long) totalPhysMem) >> 10)/1024.0);
    return Output;
}

// gets the cpu footprint (2 values)
VECTOR<DOUBLE> RAZOR::SYS_METRIC::GetCPUFootprint()
{
    // get output
    VECTOR<DOUBLE> Output;

    // for total cpu
    {
        // crap for cpu usage
        static MICROSOFT::PDH_HQUERY   cpuQuery;
        static MICROSOFT::PDH_HCOUNTER cpuTotal;
        static BOOLEAN init = false;
        if (init == false)
        {
            PdhOpenQuery(NULL, NULL, &cpuQuery);
            PdhAddCounter(cpuQuery, "\\Processor(_Total)\\% Processor Time", NULL, &cpuTotal);
            PdhCollectQueryData(cpuQuery);
            init = true;
        }

        // crap for cpu usage
        MICROSOFT::PDH_FMT_COUNTERVALUE counterVal;
        MICROSOFT::PdhCollectQueryData(cpuQuery);
        MICROSOFT::PdhGetFormattedCounterValue(cpuTotal, PDH_FMT_DOUBLE, NULL, &counterVal);
        Output.push_back(counterVal.doubleValue/100.0);
    }

    // for process cpu
    {
        // junk for process cpu
        static BOOLEAN Init = false;
        static ULARGE_INTEGER lastCPU, lastSysCPU, lastUserCPU;
        static INT32 numProcessors;
        static HANDLE self;

        if (Init == false)
        {
            SYSTEM_INFO sysInfo;
            FILETIME ftime, fsys, fuser;
            MICROSOFT::GetSystemInfo(&sysInfo);
            numProcessors = sysInfo.dwNumberOfProcessors;
            MICROSOFT::GetSystemTimeAsFileTime(&ftime);
            GLOBAL::memcpy(&lastCPU, &ftime, sizeof(FILETIME));
            self = GetCurrentProcess();
            MICROSOFT::GetProcessTimes(self, &ftime, &ftime, &fsys, &fuser);
            GLOBAL::memcpy(&lastSysCPU, &fsys, sizeof(FILETIME));
            GLOBAL::memcpy(&lastUserCPU, &fuser, sizeof(FILETIME));
            Init = true;
        }

        FILETIME ftime, fsys, fuser;
        ULARGE_INTEGER now, sys, user;
        DOUBLE percent;

        MICROSOFT::GetSystemTimeAsFileTime(&ftime);
        GLOBAL::memcpy(&now, &ftime, sizeof(FILETIME));
        MICROSOFT::GetProcessTimes(self, &ftime, &ftime, &fsys, &fuser);
        GLOBAL::memcpy(&sys, &fsys, sizeof(FILETIME));
        GLOBAL::memcpy(&user, &fuser, sizeof(FILETIME));
        percent = (sys.QuadPart - lastSysCPU.QuadPart) + (user.QuadPart - lastUserCPU.QuadPart);
        percent /= (now.QuadPart - lastCPU.QuadPart);
        percent /= numProcessors;
        lastCPU = now;
        lastUserCPU = user;
        lastSysCPU = sys;

        // checks for initials
        if (isnan(percent) || !isfinite(percent))
        {
            percent = 0.0; // sometimes numProcessors = 0, causing bad
        }

        // push it in
        Output.push_back(percent);

    }
    return Output;
}
