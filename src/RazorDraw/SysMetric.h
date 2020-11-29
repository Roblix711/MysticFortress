
// header gaurd
#ifndef _RAZOR_ENGINE_SYSMETRIC_H
#define _RAZOR_ENGINE_SYSMETRIC_H

// headers
#include "Razor.h"

// linker
#pragma comment(lib, "psapi.lib")
#pragma comment(lib, "pdh.lib")

// class for metrics
class RAZOR::SYS_METRIC
{
    public:

    // get metrics
    static VECTOR<DOUBLE> GetMemoryFootprint(); 
    static VECTOR<DOUBLE> GetCPUFootprint();
};

#endif