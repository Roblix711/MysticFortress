
// main header
#include "Console.h"

// prefs for colors
#define BLACK			0
#define BLUE			1
#define GREEN			2
#define CYAN			3
#define RED				4
#define MAGENTA			5
#define BROWN			6
#define LIGHTGRAY		7
#define DARKGRAY		8
#define LIGHTBLUE		9
#define LIGHTGREEN		10
#define LIGHTCYAN		11
#define LIGHTRED		12
#define LIGHTMAGENTA	13
#define YELLOW			14
#define WHITE			15


// protected
namespace
{
    // sets colors
    static void SetColorAndBackground(INT32 ForgC, INT32 BackC=0)
	{
		MICROSOFT::WORD wColor = ((BackC & 0x0F) << 4) + (ForgC & 0x0F);
		MICROSOFT::SetConsoleTextAttribute(MICROSOFT::GetStdHandle(STD_OUTPUT_HANDLE), wColor);		
	}

    // sets color
    static void SetDebugColor(CONSOLE_DEBUG_LEVEL Level)
    {
        switch(Level)
        {
            case DEBUG_NONE: SetColorAndBackground(LIGHTGRAY); break;
            case DEBUG_INFO: SetColorAndBackground(LIGHTCYAN); break;
            case DEBUG_WARN: SetColorAndBackground(YELLOW); break;
            case DEBUG_FATAL: SetColorAndBackground(RED); break;
            default: break;
        }
    }
}

// set strings
const STRING RAZOR::CONSOLE::DebugLevelToString[]= 
{
    "DEBUG_NONE",
    "DEBUG_INFO",
    "DEBUG_WARN",
    "DEBUG_FATAL"
};

// constructor
RAZOR::CONSOLE::CONSOLE(RAZOR::ENGINECORE *MyParent) : ParentEngine(MyParent), LogCount(0), bConsole(false)
{
    LogBufferLarge = new CHAR[RAZOR_CONSOLE_BUFFER_SIZE*2];
}
// destructor
RAZOR::CONSOLE::~CONSOLE()
{
    delete [] LogBufferLarge;
}

// any other initializations
UINT32 RAZOR::CONSOLE::Initialize(STRING FileName, BOOLEEN bMakeConsole)
{
    // add console first thing because it allows debugging
    if (bMakeConsole == true)
    {
        FILE *junkFile;// = fopen("DebugEngine.txt", "a+");
        MICROSOFT::AllocConsole();
        MICROSOFT::AttachConsole(MICROSOFT::GetCurrentProcessId());
        GLOBAL::freopen_s(&junkFile, "CONIN$",  "r", stdin);
        GLOBAL::freopen_s(&junkFile, "CONOUT$", "wt", stdout);
        GLOBAL::freopen_s(&junkFile, "CONOUT$", "wt", stderr);

        // disable quickedit -> prevents mouse clicks!
        DWORD prev_mode;
        GetConsoleMode(MICROSOFT::GetStdHandle(STD_INPUT_HANDLE), &prev_mode); 
        SetConsoleMode(MICROSOFT::GetStdHandle(STD_INPUT_HANDLE), (prev_mode & ~ENABLE_QUICK_EDIT_MODE) | ENABLE_EXTENDED_FLAGS ); 
    }

    // wraps it up to go
    static RAZOR::ENGINECORE *Parent = this->ParentEngine;
    auto Wr = [](DWORD fdwCtrlType) -> BOOL
    {
        switch( fdwCtrlType ) 
        { 
            case CTRL_C_EVENT: 
            case CTRL_CLOSE_EVENT: 
            case CTRL_BREAK_EVENT: 
            case CTRL_LOGOFF_EVENT: 
            case CTRL_SHUTDOWN_EVENT: 
            default: 
            {
                Parent->bDoExit = true;
                Sleep(30000); // should die in before 10 seconds
                return FALSE; 
            }
        } 
    };

    // set the control handler
    MICROSOFT::SetConsoleCtrlHandler( static_cast<MICROSOFT::PHANDLER_ROUTINE>( Wr ), TRUE );

    // set the console debug filename output
    FileLocation = FileName;
    return 0;
}

// proxy
void RAZOR::CONSOLE::LogV(const CHAR *message, VAR_LIST Args)
{
    RAZOR::CONSOLE::LogV(DEBUG_NONE, message, Args);
}

// a good version of printf that is threadsafe, and appends timestamps!
void RAZOR::CONSOLE::LogV(CONSOLE_DEBUG_LEVEL Level, const CHAR *message, VAR_LIST Args)
{
    // get time and store it when the function was called, not when it was written!
    MICROSOFT::SYSTEMTIME LocalTime;
    MICROSOFT::GetLocalTime(&LocalTime);

    // auto lock
    NOVA::SCOPE_LOCK AutoLock(WriteLock); // threadsafe

    // write to file, and add a timestamp!
    if (Level > DEBUG_NONE)
    {
        SetDebugColor(Level);
        printf("[%s] ", DebugLevelToString[Level].c_str());
    }
    SetDebugColor(DEBUG_NONE);
    GLOBAL::vprintf(message, Args);
    FILE *OutputLog = NULL;
   

    // attempts to force the write
    const UINT32 MAX_WRITE_ATTEMPTS = 16;
    UINT32 Tries = 0;
    while (Tries++ < MAX_WRITE_ATTEMPTS)
    {
        GLOBAL::fopen_s(&OutputLog, FileLocation.c_str(), "a+");
        if (OutputLog != NULL)
        {
            UINT64 Offset = GLOBAL::sprintf_s(LogBufferLarge, RAZOR_CONSOLE_BUFFER_SIZE, "[%d][%d/%d/%d][%02d:%02d:%02d.%03d][%s] ", LogCount++, (UINT)LocalTime.wYear, (UINT)LocalTime.wMonth, (UINT)LocalTime.wDay, (UINT)LocalTime.wHour, (UINT)LocalTime.wMinute, (UINT)LocalTime.wSecond, (UINT)LocalTime.wMilliseconds, DebugLevelToString[Level].c_str());
            GLOBAL::vsprintf_s(LogBufferLarge + Offset, RAZOR_CONSOLE_BUFFER_SIZE, message, Args);
            GLOBAL::fputs(LogBufferLarge, OutputLog);
            MICROSOFT::OutputDebugString(LogBufferLarge);
            GLOBAL::fclose(OutputLog);
            break;
        }
        else
        {
            MICROSOFT::Sleep(1);
        }
    }

}

// proxy 
void RAZOR::CONSOLE::Log(const CHAR *message, ...)
{
    // start variable arguments
    VAR_LIST args;
    va_start(args, message);
    RAZOR::CONSOLE::LogV(DEBUG_NONE, message, args);
    va_end(args);
}

// a good version of printf that is threadsafe, and appends timestamps!
void RAZOR::CONSOLE::Log(CONSOLE_DEBUG_LEVEL Level, const CHAR *message, ...)
{
    // get time and store it when the function was called, not when it was written!
    MICROSOFT::SYSTEMTIME LocalTime;
    MICROSOFT::GetLocalTime(&LocalTime);

    // autolock
    NOVA::SCOPE_LOCK AutoLock(WriteLock); // threadsafe

    // start variable arguments
    VAR_LIST args;
    va_start(args, message);

    // write to file, and add a timestamp!
    if (Level > DEBUG_NONE)
    {
        SetDebugColor(Level);
        printf("[%s] ", DebugLevelToString[Level].c_str());
    }
    SetDebugColor(DEBUG_NONE);
    GLOBAL::vprintf(message, args);
    FILE *OutputLog = NULL;

    // attempts to force the write
    const UINT32 MAX_WRITE_ATTEMPTS = 16;
    UINT32 Tries = 0;
    while (Tries++ < MAX_WRITE_ATTEMPTS)
    {
        GLOBAL::fopen_s(&OutputLog, FileLocation.c_str(), "a+");
        if (OutputLog != NULL)
        {
            UINT64 Offset = GLOBAL::sprintf_s(LogBufferLarge, RAZOR_CONSOLE_BUFFER_SIZE, "[%d][%d/%d/%d][%02d:%02d:%02d.%03d][%s] ", LogCount++, (UINT)LocalTime.wYear, (UINT)LocalTime.wMonth, (UINT)LocalTime.wDay, (UINT)LocalTime.wHour, (UINT)LocalTime.wMinute, (UINT)LocalTime.wSecond, (UINT)LocalTime.wMilliseconds, DebugLevelToString[Level].c_str());
            GLOBAL::vsprintf_s(LogBufferLarge + Offset, RAZOR_CONSOLE_BUFFER_SIZE, message, args);
            GLOBAL::fputs(LogBufferLarge, OutputLog);
            MICROSOFT::OutputDebugString(LogBufferLarge);
            GLOBAL::fclose(OutputLog);
            break;
        }
        else
        {
            MICROSOFT::Sleep(1);
        }
    }
    va_end(args);

}

