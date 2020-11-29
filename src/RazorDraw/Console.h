
// header gaurd
#ifndef _RAZOR_ENGINE_CONSOLE_H
#define _RAZOR_ENGINE_CONSOLE_H

// headers
#include "Engine.h"

// defines
#define RAZOR_CONSOLE_BUFFER_SIZE   1024

// internal messages
enum CONSOLE_DEBUG_LEVEL
{
    DEBUG_NONE = 0, // ALWAYS appears
    DEBUG_INFO,     // appears as informational only
    DEBUG_WARN,     // appears as warning
    DEBUG_FATAL     // appears as fatal
};


// this is a helper class
class RAZOR::CONSOLE
{
    public:
    RAZOR::ENGINECORE *ParentEngine;
    NOVA::MUTEX_FAST WriteLock;
    STRING  FileLocation;
    CHAR *LogBufferLarge;
    UINT32 LogCount;
    BOOLEEN bConsole;

    // stringified
    static const STRING DebugLevelToString[];

    // constructor
    CONSOLE(RAZOR::ENGINECORE *MyParent);
    ~CONSOLE();

    // main methods
    UINT32 Initialize(STRING FileName, BOOLEEN MakeConsole);
    void Log(const CHAR *message, ...);
    void LogV(const CHAR *message, VAR_LIST Args);
    void Log(CONSOLE_DEBUG_LEVEL Level, const CHAR *message, ...);
    void LogV(CONSOLE_DEBUG_LEVEL Level,const CHAR *message, VAR_LIST Args);

};



#endif