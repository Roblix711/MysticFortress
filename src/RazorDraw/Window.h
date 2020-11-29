
// header gaurd
#ifndef _RAZOR_ENGINE_WINDOW_H
#define _RAZOR_ENGINE_WINDOW_H

// headers
#include "Engine.h"
#include "IOShell.h"

// enum for state
enum WINDOW_SIZING_STATE
{
    WINDOW_NOT_SIZING = 0,
    WINDOW_SIZING     = 1
};

// handle forking with this
struct DRAG_DROP_WRAPPER
{
    RAZOR::WINDOW *ParentWindow;
    VECTOR<STRING> *Paths;
};


// window class keeps makes new windows to draw on
class RAZOR::WINDOW
{
    public:
    RAZOR::ENGINECORE *ParentEngine;
    RAZOR::WINDOW_PARAMS WindowParams;

    // microsoft junk
    MICROSOFT::HWND         WindowHandle;
    MICROSOFT::WNDCLASSEX   WindowClass;
    MICROSOFT::HCURSOR      DefaultCursor;
    MICROSOFT::HICON        DefaultIcon;
    CHAR                    WindowClassName[64];

    private:
    // state
    INT32 IsSizing;
    INT32 Minimized;
    INT32 Maximized;
    INT32 ScreenWidth;
    INT32 ScreenHeight;
    BOOLEEN bInit;
    SHELL_IO *ShellIO;
    NOVA::MUTEX_FAST SizeLock;

    public:
    // constructor
    WINDOW(RAZOR::ENGINECORE *MyParent);
    ~WINDOW();
    UINT32 Initialize(RAZOR::WINDOW_PARAMS &MyWindowParams);

    // methods
    void MsgProc(MICROSOFT::HWND hWnd, UINT Msg, MICROSOFT::WPARAM wParam, MICROSOFT::LPARAM lParam);
    void DoNextResize(INT32 NewWidth, INT32 NewHeight);
    INT32 GetSizingState();
    INT32 GetMinimized();
    INT32 GetMaximized();
    INT32 GetScreenWidth();
    INT32 GetScreenHeight();
    INT32 GetDialogSaveFile(STRING         &OutputFile, const VECTOR<PAIR<STRING, STRING>> &Types);
    INT32 GetDialogOpenFile(VECTOR<STRING> &OutputFile, const VECTOR<PAIR<STRING, STRING>> &Types);
    void SetClientDims(INT32 X, INT32 Y);

    // for threads
    static void* ThreadDragDrop(METADATA Unknown);
    void InnerDragDrop(VECTOR<STRING> *Paths);
    void SafeDragDrop(const VECTOR<STRING> &Paths);
};

#endif