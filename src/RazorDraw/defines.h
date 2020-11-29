
// header gaurd
#ifndef _HEADER_GAURD_DEFINES_H
#define _HEADER_GAURD_DEFINES_H

// some shortcut crap
#define STRINGIFY(x) #x
#define EXPAND(x) STRINGIFY(x)

// RAZOR defines
#define RAZOR_VERSION "1.18"

// api defines 
#define WIN32_LEAN_AND_MEAN 
#define DIRECTINPUT_VERSION 0x0800

// if in debug mode, enable some specials
#if defined(DEBUG)|defined(_DEBUG)
    #define NOTHROW  //(std::nothrow)
#else
    #define NOTHROW (std::nothrow)

    // disable some crappy warnings
    #pragma warning( disable : 4101) // local variable is unferenced
    #pragma warning( disable : 4189) // local variable is initialized but not referenced
    #pragma warning( disable : 4100) // unreferenced formal parameter
    #pragma warning( disable : 4244) // conversion from double to int
    #pragma warning( disable : 4267) // 'size_t' to 'int', possible loss of data
    #pragma warning( disable : 4127) // conditional expression is constant: while(FOREVER)
    #pragma warning( disable : 4239) // warning C4239: nonstandard extension used : 'argument'

#endif


// readablity defines
#define FOREVER true
#define NOCODE
#define GLOBAL
#define MICROSOFT
#define DIRECTX9
#define NOARG
#define NOCODE

// min/max -> put an x infront so it doesn't conflict with anything
#define xMIN(a,b) (((a)<(b))?(a):(b))
#define xMAX(a,b) (((a)>(b))?(a):(b))

#endif
