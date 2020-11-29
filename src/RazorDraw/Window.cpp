
// main header
#include "Window.h"

// os stuff
#include <Windows.h>
#include <commdlg.h>
#include <algorithm>

// defines for ease
#define RAZOR_ENGINE_WINDOW_MAX_PATH 1024

// constructor
RAZOR::WINDOW::WINDOW(RAZOR::ENGINECORE *MyParent): ParentEngine(MyParent), WindowHandle(NULL), DefaultCursor(NULL), DefaultIcon(NULL), IsSizing(WINDOW_NOT_SIZING), bInit(false)
{
    // class name -> junk
    sprintf_s<64>(WindowClassName, "RazorEngine_WindowClass");

    // load cursor and icon
    DefaultCursor = MICROSOFT::LoadCursor(NULL, IDC_ARROW);
    DefaultIcon = (MICROSOFT::HICON) MICROSOFT::LoadImage 
    (
        NULL,             // hInstance must be NULL when loading from a file
        "icon.ico",       // the icon file name
        IMAGE_ICON,       // specifies that the file is an icon
        0,                // width of the image (we'll specify default later on)
        0,                // height of the image
        LR_LOADFROMFILE | // we want to load a file (as opposed to a resource)
        LR_DEFAULTSIZE |  // default metrics based on the type (IMAGE_ICON, 32x32)
        LR_SHARED         // let the system release the handle when it's no longer used
    );

	// do window class first // CALL BY constuctor
    MICROSOFT::WNDCLASSEX WndClassNew = 
    {   
        sizeof(MICROSOFT::WNDCLASSEX),
        CS_CLASSDC | CS_HREDRAW | CS_VREDRAW, 
        NULL, 0L, 0L, // note here: NULL = wndproc is not specified yet
        ParentEngine->Instance, 
        DefaultIcon, 
        DefaultCursor, 
        NULL, 
        NULL,
        WindowClassName, 
        NULL 
    }; 
    WindowClass = WndClassNew; // transfer
}

// destructor
RAZOR::WINDOW::~WINDOW()
{
    delete ShellIO;
}

// initializes a window, and adds it to self list
UINT32 RAZOR::WINDOW::Initialize( WINDOW_PARAMS &MyWindowParams)
{
    // variable only allows registration once
    static BOOLEEN ClassRegistered = false;
    if (ClassRegistered == false)
    {
        WindowClass.lpfnWndProc = ParentEngine->FunctionLinks.EventMain;
        MICROSOFT::RegisterClassEx(&WindowClass);
        ClassRegistered = true;
    }

    // transfer IN
    WindowParams = MyWindowParams;

    // load from file 
    if (WindowParams.IconResource > 0)
    {
        DefaultIcon = (MICROSOFT::HICON) MICROSOFT::LoadImage 
        (
            NULL,             // hInstance must be NULL when loading from a file
            MAKEINTRESOURCE (WindowParams.IconResource),       // the icon file name
            IMAGE_ICON,       // specifies that the file is an icon
            0,                // width of the image (we'll specify default later on)
            0,                // height of the image
            LR_DEFAULTSIZE |  // default metrics based on the type (IMAGE_ICON, 32x32)
            LR_SHARED         // let the system release the handle when it's no longer used
        );
        WindowClass.hIcon = DefaultIcon;
    }
    
	// create the style of the window based on windowed bool
    INT32 MyWSSTYLE = 0;
    INT32 MyWSEXSTYLE = WS_EX_LEFT | WS_EX_APPWINDOW; // | WS_EX_CLIENTEDGE WS_EX_TOPMOST | 
    
	// set windowed and non windowed styles
    if (WindowParams.bWindowed == true)	
	{
		MyWSSTYLE |= WS_OVERLAPPEDWINDOW;
		MyWSEXSTYLE |= WS_EX_WINDOWEDGE;
	}
    else					
	{
		MyWSSTYLE |= WS_POPUP;
		MyWSEXSTYLE |= WS_EX_PALETTEWINDOW ;  // prevents clicking out!
	}

	// make a generic window so that messages can appear!
    if (WindowParams.ScreenWidth <= 0 || WindowParams.ScreenHeight <= 0)
	{
        ParentEngine->Console->Log(DEBUG_FATAL, "RAZOR::WINDOW::Initialize(): Failed: ScreenWidth <= 0 or ScreenHeight <= 0");
        throw EXCEPTION_RUNTIME("RAZOR::WINDOW::Initialize() Failed: ScreenWidth <= 0 or ScreenHeight <= 0");
	}

	// adjust window area to be exact including menu's and borders and stuff
    RECT WindowDimensions = {0, 0, WindowParams.ScreenWidth+1, WindowParams.ScreenHeight+1}; // why + 1? cus bad windows
	if (WindowParams.bWindowed == true) 
	{
        MICROSOFT::AdjustWindowRectEx(&WindowDimensions, MyWSSTYLE, false, WS_OVERLAPPEDWINDOW);
	}

    // make it 
    SetClientDims(WindowParams.ScreenWidth, WindowParams.ScreenHeight);  
    INT32 WindowWidth  = WindowDimensions.right + 1 - WindowDimensions.left;
    INT32 WindowHeight = WindowDimensions.bottom + 1 - WindowDimensions.top;
	WindowHandle = MICROSOFT::CreateWindowExA(MyWSEXSTYLE, WindowClassName, WindowParams.Name.c_str(), MyWSSTYLE, 100, 100, WindowWidth, WindowHeight, NULL, NULL, ParentEngine->Instance, NULL);

    // some show commands
    MICROSOFT::ShowWindow(WindowHandle, true);
    MICROSOFT::UpdateWindow(WindowHandle);
    bInit = true;

    // check here
    RECT ClientRect;
    GetClientRect(WindowHandle, &ClientRect);
    if ( (ClientRect.right - ClientRect.left) != GetScreenWidth() || (ClientRect.bottom - ClientRect.top) != GetScreenHeight() )
    {
        ParentEngine->Console->Log(DEBUG_WARN, "RAZOR::WINDOW::Initialize() Bad Client Size: [%d %d, %d %d]\n", ClientRect.left, ClientRect.top, ClientRect.right, ClientRect.bottom);
    }

    // setup shell io drags
    ShellIO = new SHELL_IO();

    return 0;
}

// handle actual callback here
void RAZOR::WINDOW::SafeDragDrop(const VECTOR<STRING> &Paths)
{
    if (WindowParams.DragDropCallback != nullptr)
    {
        WindowParams.DragDropCallback(Paths);
    }
}

// set new state
void RAZOR::WINDOW::InnerDragDrop(VECTOR<STRING> *Paths)
{
    // SE filter + backlog
    static RAZOR::ENGINE *ParentEngine = ParentEngine;
    auto ExceptionFilter = [](EXCEPTION_POINTERS *P, DWORD dwExceptionCode, RAZOR::WINDOW* This) -> DWORD
    {
        STRING ExceptionError = ParentEngine->GetExceptionInfo(P, dwExceptionCode);
        ParentEngine->ConsoleLog("RAZOR::WINDOW::SafeDragDrop() Caught SEH Error: %s", ExceptionError.c_str());
        return EXCEPTION_EXECUTE_HANDLER;
    };

    __try
    {
        // might blow up??
        SafeDragDrop(*Paths);
        delete Paths;
    }
    __except(ExceptionFilter(GetExceptionInformation(), GetExceptionCode(), this))
    {
        // do something
    }
}


// safe fork here to prevent loading
void* RAZOR::WINDOW::ThreadDragDrop(METADATA Unknown)
{
    // purge wrapper
    DRAG_DROP_WRAPPER *WrapperPtr = static_cast<DRAG_DROP_WRAPPER*>(Unknown);
    DRAG_DROP_WRAPPER WrapperReal = *WrapperPtr;
    delete WrapperPtr;

    // pass call
    WrapperReal.ParentWindow->InnerDragDrop(WrapperReal.Paths);
    return 0;
}

// handle engine events
void RAZOR::WINDOW::MsgProc(MICROSOFT::HWND hWnd, UINT Msg, MICROSOFT::WPARAM wParam, MICROSOFT::LPARAM lParam)
{
    // hidden state 
    const INT32 MinSizeX = 320;
    const INT32 MinSizeY = 240;
	static BOOLEEN MonitorSize = 0;
	static INT32 LastSizedX = ScreenWidth;
    static INT32 LastSizedY = ScreenHeight;
    BOOLEEN DoResize = false;

    // check the event
	switch (Msg)
	{
        case WM_DROPFILES:
        {
            if (WindowParams.DragDropCallback != nullptr)
            {
                // dump to vector string
                HDROP DestControl = (HDROP) wParam; // FORCE
                CHAR FileName[512] = {0};
		        UINT nFileCount = DragQueryFileA(DestControl, (UINT) INVALID_HANDLE_VALUE, NULL, 0);
		        if(nFileCount > 0)
		        {
                    // allocate some memeory
                    VECTOR<STRING> *Paths = new VECTOR<STRING>; // deletion is handled later
                    Paths->reserve(nFileCount);
                    for(UINT32 i=0; i<nFileCount; ++i)
                    {
                        DragQueryFile(DestControl, i, FileName, 512);
                        Paths->push_back(FileName);
                    }

                    // FORK, and call the callback there
                    NOVA::THREAD_FORK Thread;
                    DRAG_DROP_WRAPPER *Wrapper = new DRAG_DROP_WRAPPER{this, Paths};
		            Thread.Fork(RAZOR::WINDOW::ThreadDragDrop, Wrapper);

                }
            }
            break;
        }

		case WM_ENTERSIZEMOVE:
		{
            // printf("Window WM_ENTERSIZEMOVE\n");
            IsSizing = WINDOW_SIZING; 
		} break;
        
		case WM_EXITSIZEMOVE: // SETS TO 0 then resizes!
		{
            // printf("Window WM_EXITSIZEMOVE");
            IsSizing = WINDOW_NOT_SIZING; 

            // check last size
            if (MonitorSize == true)
            {
                DoResize = true;
                MonitorSize = false;
            }
		} break;
        case WM_GETMINMAXINFO:
        {
            // set sizes
            RECT MaxUnionSize = ParentEngine->AllMonitors.GetLargestUnionSize();
            LPMINMAXINFO lpMMI = (LPMINMAXINFO) lParam;
            lpMMI->ptMinTrackSize.x = MinSizeX;
            lpMMI->ptMinTrackSize.y = MinSizeY;
            lpMMI->ptMaxTrackSize.x = MaxUnionSize.right;
            lpMMI->ptMaxTrackSize.y = MaxUnionSize.bottom;
            break;
        } 
		case WM_SIZE:
		{
			if (wParam == SIZE_RESTORED )
			{
                // printf("Window SIZE_RESTORED: %d %d\n", LOWORD(lParam), HIWORD(lParam));
                if (IsSizing == WINDOW_SIZING)
                {
                    MonitorSize = true;
					LastSizedX = LOWORD(lParam);
					LastSizedY = HIWORD(lParam);
                    DoNextResize(LastSizedX, LastSizedY);
                }
                else
                {
                    Maximized = false;
                    Minimized = false;
                    DoResize = true;
					LastSizedX = LOWORD(lParam);
					LastSizedY = HIWORD(lParam);
                    DoNextResize(LastSizedX, LastSizedY);
                }
			}
			else if (wParam == SIZE_MAXIMIZED) // on maximize 
			{
                // printf("Window SIZE_MAXIMIZED: %d %d\n", LOWORD(lParam), HIWORD(lParam));
                Minimized = false;
                Maximized = true;
                DoResize = true;
				LastSizedX = LOWORD(lParam);
				LastSizedY = HIWORD(lParam);
                DoNextResize(LastSizedX, LastSizedY);
			}
			else if (wParam == SIZE_MINIMIZED) // on minimize 
			{
                // printf("Window SIZE_MINIMIZED: %d %d\n", LOWORD(lParam), HIWORD(lParam));
                Minimized = true;
                Maximized = false;
			}
		} break;
	}

    // check the resize condition
    if (DoResize == true)
    {
        // call 
        DoNextResize(LastSizedX, LastSizedY);
    }

}

// resize the buffer
void RAZOR::WINDOW::DoNextResize(INT32 NewWidth, INT32 NewHeight)
{
    if (bInit == true)
    {
        SetClientDims(NewWidth, NewHeight);

        /*if (NewWidth != ParentEngine->DirectX9->PresParameters.BackBufferWidth || NewHeight != ParentEngine->DirectX9->PresParameters.BackBufferHeight)
        {
            ParentEngine->Console->Log(DEBUG_INFO, "RAZOR::WINDOW::DoNextResize(): Resizing to [%d %d]\n", NewWidth, NewHeight);
	        ParentEngine->DirectX9->PresParameters.BackBufferWidth = NewWidth;
	        ParentEngine->DirectX9->PresParameters.BackBufferHeight = NewHeight;
            ParentEngine->bResetting = true;
        }*/
    }
}

// get stuff
INT32 RAZOR::WINDOW::GetSizingState()    { return IsSizing;     }
INT32 RAZOR::WINDOW::GetMinimized()      { return Minimized;    }
INT32 RAZOR::WINDOW::GetMaximized()      { return Maximized;    }
INT32 RAZOR::WINDOW::GetScreenWidth()    { return ScreenWidth;  }
INT32 RAZOR::WINDOW::GetScreenHeight()   { return ScreenHeight; }

// geta saved file name
INT32 RAZOR::WINDOW::GetDialogSaveFile(STRING &OutputFile, const VECTOR<PAIR<STRING, STRING>> &Types)
{
    // temps
    MICROSOFT::OPENFILENAME Options;
    CHAR szFileName[RAZOR_ENGINE_WINDOW_MAX_PATH] = "";
    MICROSOFT::ZeroMemory(&Options, sizeof(Options));

    // assemble the types
    STRING Filter;
    for(auto Type : Types)
    {
        Filter.append(Type.first + " (*." + Type.second + ")");
        Filter.append(1, '\0');
        Filter.append("*." + Type.second);
        Filter.append(1, '\0');
    }
    Filter.append("All Files (*.*)");
    Filter.append(1, '\0');
    Filter.append("*.*");
    Filter.append(2, '\0');

    // fill out
    Options.lStructSize = sizeof(Options);
    Options.hwndOwner = WindowHandle;
    Options.nFilterIndex = 1;
    Options.lpstrFilter = Filter.data();
    Options.lpstrFile = szFileName;
    Options.nMaxFile = RAZOR_ENGINE_WINDOW_MAX_PATH;
    Options.Flags = OFN_EXPLORER | OFN_PATHMUSTEXIST;  
    Options.lpstrDefExt = (Types.size() > 0) ? Types[0].second.c_str() : "*";

    // check success
    MICROSOFT::BOOL Ret = MICROSOFT::GetSaveFileName(&Options);
    if (Ret > 0)
    {
        OutputFile = szFileName;
        return 1;
    }
    return 0;
}

// open a file
INT32 RAZOR::WINDOW::GetDialogOpenFile(VECTOR<STRING> &OutputFile, const VECTOR<PAIR<STRING, STRING>> &Types)
{
    return 0;
}

// set 
void RAZOR::WINDOW::SetClientDims(INT32 X, INT32 Y)
{
    NOVA::SCOPE_LOCK AutoLock(SizeLock);
    ScreenWidth  = X;
    ScreenHeight = Y;
}