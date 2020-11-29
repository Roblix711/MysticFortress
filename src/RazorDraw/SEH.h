
// header gaurd
#ifndef _RAZOR_ENGINE_SEH_H
#define _RAZOR_ENGINE_SEH_H

// headers
#include "Razor.h"

// deps
#include <Windows.h>
#include <eh.h>
#include <Psapi.h>

// linker
#pragma comment(lib, "psapi.lib")

// wrapper class to get information about SEH -> credit to Alessandro Jacopson: 
// see https://stackoverflow.com/questions/3523716/is-there-a-function-to-convert-exception-pointers-struct-to-a-string
class RAZOR::EXCEPTION_INFO
{
    public:
    typedef unsigned int exception_code_t;
    static const char* opDescription( const ULONG opcode );
    static const char* seDescription( const exception_code_t& code );
    static STRING GetExceptionInfo( struct _EXCEPTION_POINTERS* ep, bool has_exception_code = false, exception_code_t code = 0  );

    static DWORD ExceptionFilter(EXCEPTION_POINTERS *P, DWORD dwExceptionCode);

};


#endif