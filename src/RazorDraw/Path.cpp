
// main header
#include "Path.h"

// deps
#include <Shellapi.h>

// protected
namespace 
{
    // multi byte to wide char:
    WSTRING s2ws(const STRING &str)
    {
        INT size_needed = MICROSOFT::MultiByteToWideChar(CP_UTF8, 0, &str[0], (INT)str.size(), NULL, 0);
        WSTRING wstrTo(size_needed, 0);
        MICROSOFT::MultiByteToWideChar(CP_UTF8, 0, &str[0], (INT)str.size(), &wstrTo[0], size_needed);
        return wstrTo;
    }

    // wide char to multi byte: 
    STRING ws2s(const WSTRING &wstr)
    {
        INT size_needed = MICROSOFT::WideCharToMultiByte(CP_ACP, 0, wstr.c_str(), INT(wstr.length() + 1), 0, 0, 0, 0);
        STRING strTo(size_needed, 0);
        MICROSOFT::WideCharToMultiByte(CP_ACP, 0, wstr.c_str(), INT(wstr.length() + 1), &strTo[0], size_needed, 0, 0);
        // BUG?? APPENDS A \0 at the end... must erase last character sometimes
        strTo.erase(strTo.end() - 1);
        return strTo;
    }
}

// constructor
RAZOR::PATH::PATH(RAZOR::ENGINECORE *MyParent) : ParentEngine(MyParent)
{
    NOCODE;
}

// main methods 
UINT32 RAZOR::PATH::Initialize()
{
    // autocall this
    ConfigureRelToAbs();

    return 0;
}

// must be called to configure relative to absolute translation
void RAZOR::PATH::ConfigureRelToAbs()
{
    // set absolute path
    AbsoluteDirectoryWide = this->GetModulePath();
    AbsoluteDirectoryChar = ws2s(AbsoluteDirectoryWide);

    // check command line
    INT NumArguments = 0;
    LPWSTR *szArgList;
    szArgList = MICROSOFT::CommandLineToArgvW(MICROSOFT::GetCommandLineW(), &NumArguments);
    CommandArguments.resize(NumArguments);

    // save command line arguments
    for (INT i = 0; i<NumArguments; i++)
    {
        CommandArguments[i] = szArgList[i];
        // MessageBoxW(0, szArgList[i], L"Command Line", 0);
    }
}

// Get this modules path
WSTRING RAZOR::PATH::GetModulePath()
{
    // local vars
    const INT PathLength = 1024;
    WCHAR szBasePath[PathLength];
    WSTRING base = L"NULL";
    size_t found;

    // get filename
    if (!MICROSOFT::GetModuleFileNameW(MICROSOFT::GetModuleHandleW(NULL), szBasePath, PathLength))
    {
        return base;
    }
    base = szBasePath;
    // extract path via reverse search
    if ((found = base.rfind(L"\\")) == base.npos)
    {
        return base;
    }
    base.erase(base.begin() + found + 1, base.end());
    return base;
}

STRING RAZOR::PATH::MakeAbsolutePathChar(STRING RelativePathName)
{
    STRING Return = AbsoluteDirectoryChar;
    return Return += RelativePathName;
}

WSTRING RAZOR::PATH::MakeAbsolutePathWide(WSTRING RelativePathName)
{
    WSTRING Return = AbsoluteDirectoryWide;
    return Return += RelativePathName;
}
