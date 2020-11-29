
// main header
#include "SEH.h"

const char* RAZOR::EXCEPTION_INFO::opDescription( const ULONG opcode )
{
    switch( opcode ) 
    {
        case 0: return "read";
        case 1: return "write";
        case 8: return "user-mode data execution prevention (DEP) violation";
        default: return "unknown";
    }
}
const char* RAZOR::EXCEPTION_INFO::seDescription( const RAZOR::EXCEPTION_INFO::exception_code_t& code )
{ 
    switch( code ) 
    {
        case EXCEPTION_ACCESS_VIOLATION:         return "EXCEPTION_ACCESS_VIOLATION"         ;
        case EXCEPTION_ARRAY_BOUNDS_EXCEEDED:    return "EXCEPTION_ARRAY_BOUNDS_EXCEEDED"    ;
        case EXCEPTION_BREAKPOINT:               return "EXCEPTION_BREAKPOINT"               ;
        case EXCEPTION_DATATYPE_MISALIGNMENT:    return "EXCEPTION_DATATYPE_MISALIGNMENT"    ;
        case EXCEPTION_FLT_DENORMAL_OPERAND:     return "EXCEPTION_FLT_DENORMAL_OPERAND"     ;
        case EXCEPTION_FLT_DIVIDE_BY_ZERO:       return "EXCEPTION_FLT_DIVIDE_BY_ZERO"       ;
        case EXCEPTION_FLT_INEXACT_RESULT:       return "EXCEPTION_FLT_INEXACT_RESULT"       ;
        case EXCEPTION_FLT_INVALID_OPERATION:    return "EXCEPTION_FLT_INVALID_OPERATION"    ;
        case EXCEPTION_FLT_OVERFLOW:             return "EXCEPTION_FLT_OVERFLOW"             ;
        case EXCEPTION_FLT_STACK_CHECK:          return "EXCEPTION_FLT_STACK_CHECK"          ;
        case EXCEPTION_FLT_UNDERFLOW:            return "EXCEPTION_FLT_UNDERFLOW"            ;
        case EXCEPTION_ILLEGAL_INSTRUCTION:      return "EXCEPTION_ILLEGAL_INSTRUCTION"      ;
        case EXCEPTION_IN_PAGE_ERROR:            return "EXCEPTION_IN_PAGE_ERROR"            ;
        case EXCEPTION_INT_DIVIDE_BY_ZERO:       return "EXCEPTION_INT_DIVIDE_BY_ZERO"       ;
        case EXCEPTION_INT_OVERFLOW:             return "EXCEPTION_INT_OVERFLOW"             ;
        case EXCEPTION_INVALID_DISPOSITION:      return "EXCEPTION_INVALID_DISPOSITION"      ;
        case EXCEPTION_NONCONTINUABLE_EXCEPTION: return "EXCEPTION_NONCONTINUABLE_EXCEPTION" ;
        case EXCEPTION_PRIV_INSTRUCTION:         return "EXCEPTION_PRIV_INSTRUCTION"         ;
        case EXCEPTION_SINGLE_STEP:              return "EXCEPTION_SINGLE_STEP"              ;
        case EXCEPTION_STACK_OVERFLOW:           return "EXCEPTION_STACK_OVERFLOW"           ;
        default: return "UNKNOWN EXCEPTION" ;
    }
}

STRING RAZOR::EXCEPTION_INFO::GetExceptionInfo( EXCEPTION_POINTERS* ep, bool has_exception_code, RAZOR::EXCEPTION_INFO::exception_code_t code)
{
    HMODULE hm;
    MODULEINFO mi;
    CHAR fn[MAX_PATH];
    OSTRINGSTREAM oss;

    // get info
    MICROSOFT::GetModuleHandleEx( GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS, static_cast<LPCTSTR>(ep->ExceptionRecord->ExceptionAddress), &hm );
    MICROSOFT::GetModuleInformation( MICROSOFT::GetCurrentProcess(), hm, &mi, sizeof(mi) );
    MICROSOFT::GetModuleFileNameExA( MICROSOFT::GetCurrentProcess(), hm, fn, MAX_PATH );

    // write
    oss << (has_exception_code?seDescription( code ):"") << " address [0x" << std::hex << ep->ExceptionRecord->ExceptionAddress << std::dec << "] inside [" << fn << "] at base address [0x" << std::hex << mi.lpBaseOfDll << "]\n"; 

    if ( has_exception_code && (code == EXCEPTION_ACCESS_VIOLATION || code == EXCEPTION_IN_PAGE_ERROR ) ) 
    {
        oss << "Invalid operation: " << opDescription(ep->ExceptionRecord->ExceptionInformation[0]) << " at address [0x" << std::hex << ep->ExceptionRecord->ExceptionInformation[1] << std::dec << "]\n";
    }

    if ( has_exception_code && code == EXCEPTION_IN_PAGE_ERROR ) 
    {
        oss << "Underlying NTSTATUS code that resulted in the exception " << ep->ExceptionRecord->ExceptionInformation[2] << "\n";
    }

    return oss.str();
}

DWORD RAZOR::EXCEPTION_INFO::ExceptionFilter(EXCEPTION_POINTERS *P, DWORD dwExceptionCode)
{
    STRING ExceptionError = GetExceptionInfo(P, dwExceptionCode);
    printf("RAZOR::EXCEPTION_INFO::ExceptionFilter() Caught SEH: %s", ExceptionError.c_str());
    return EXCEPTION_EXECUTE_HANDLER;
}