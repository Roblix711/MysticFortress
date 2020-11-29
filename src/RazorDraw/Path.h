
// header gaurd
#ifndef _RAZOR_ENGINE_PATH_H
#define _RAZOR_ENGINE_PATH_H

// headers
#include "Engine.h"

// included libs
#pragma comment(lib, "Shell32.lib")

// this is a helper class
class RAZOR::PATH
{
    public:
    RAZOR::ENGINECORE *ParentEngine;

    // absolute path
    STRING  AbsoluteDirectoryChar;
    WSTRING AbsoluteDirectoryWide;
    VECTOR<WSTRING> CommandArguments;     // command line arguments
    

    // special path functions
    void ConfigureRelToAbs();
    WSTRING GetModulePath();
    STRING MakeAbsolutePathChar(STRING RelativePathName);
    WSTRING MakeAbsolutePathWide(WSTRING RelativePathName);

    // constructor
    PATH(RAZOR::ENGINECORE *MyParent);

    // main methods
    UINT32 Initialize();
};

#endif