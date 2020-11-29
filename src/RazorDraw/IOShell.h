
// header gaurd
#ifndef _RAZOR_ENGINE_IO_SHELL_H
#define _RAZOR_ENGINE_IO_SHELL_H

// main include
#include <Windows.h>
#include <Ole2.h>
#include <Oleidl.h>
#include <Shellapi.h>

// link with lib
#pragma comment(lib, "Ole32.lib")
#pragma comment(lib, "UUID.lib")

// class to handle IO from shell (clipboard + drag n drop)
class SHELL_IO: public IDropTarget
{
    public:
    LONG volatile m_cRef;

    // constructor
    SHELL_IO();
    ~SHELL_IO();

    // methods inherited
    HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid, LPVOID *ppvObj);
    ULONG STDMETHODCALLTYPE AddRef();
    ULONG STDMETHODCALLTYPE Release();

    // drop stuff
    HRESULT STDMETHODCALLTYPE DragEnter(IDataObject *pDataObj, DWORD grfKeyState, POINTL pt, DWORD *pdwEffect);
    HRESULT STDMETHODCALLTYPE DragLeave();
    HRESULT STDMETHODCALLTYPE DragOver(DWORD  grfKeyState, POINTL pt, DWORD *pdwEffect);
    HRESULT STDMETHODCALLTYPE Drop(IDataObject *pDataObj, DWORD grfKeyState, POINTL pt, DWORD *pdwEffect);


};


#endif // _RAZOR_ENGINE_IO_SHELL_H