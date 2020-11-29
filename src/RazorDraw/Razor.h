
// header gaurd
#ifndef _RAZOR_ENGINE_CLASSES_H
#define _RAZOR_ENGINE_CLASSES_H

// include main
#include "defines.h"
#include <RazorDraw.hpp>
#include <NovaX.hpp>

// linker
#pragma comment(lib, "legacy_stdio_definitions.lib")

// declare namespace
namespace RAZOR
{
    class ENGINECORE;   // the main engine RAZOR
    class DIRECTX;      // the directx manager
    class DRAW;         // the drawing manager
    class WINDOW;       // the window manager
    class CONSOLE;      // the console manager
    class RESOURCE;     // the resource manager
    class PATH;         // the path manager
    class SCHEDULER;    // the timing manager
    class INPUT;        // the input keyboard/mouse bridge
    class EXCEPTION_INFO; // a wrapper class for SEH
    class SYS_METRIC;   // wrapper for system metrics

    // states for us
    enum RAZOR_ENGINE_STATE
    {
        RAZOR_ENGINE_UNKNOWN = 0,
        RAZOR_ENGINE_INIT,

        // must be last
        RAZOR_ENGINE_AUTO_COUNTER    
    };

    // used to identify what is the current drawing item being used
    enum RAZOR_DRAW_TYPES
    { 
	    DRAW_NONE   = 0,
	    DRAW_SPRITE = 1, 
	    DRAW_LINE   = 2
    };

};

// typedef to void function ptr, and 2 int -> ugly ones
typedef void (*_tFuncPtr)();			
typedef void (*_tFuncPtr2Int)(INT32, INT32); 
typedef void (*_tFuncPtrlpv)(VECTOR<METADATA>);
typedef void (*_tFuncPtrlpv2)(METADATA, VECTOR<METADATA>);

// nice ones based on above
typedef _tFuncPtr        VOID_FUNCTION_POINTER;
typedef _tFuncPtrlpv     VOID_FUNCTION_POINTER_LPV;
typedef _tFuncPtrlpv2    VOID_FUNCTION_POINTER_LPV2;





#endif