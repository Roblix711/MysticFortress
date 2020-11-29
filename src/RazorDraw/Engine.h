
// header gaurd
#ifndef _RAZOR_ENGINE_ENGINE_H
#define _RAZOR_ENGINE_ENGINE_H

// headers
#include "Razor.h"
#include "DirectX.h"
#include "Console.h"
#include "Window.h"
#include "Path.h"
#include "Scheduler.h"
#include "Input.h"
#include "Draw.h"
#include "Resource.h"
#include "SEH.h"
#include "SysMetric.h"

// get all monitors, take the biggest x, biggest y, thats our buffer size!
// THANKS https://stackoverflow.com/questions/18112616/how-do-i-get-the-dimensions-rect-of-all-the-screens-in-win32-api
struct MONITOR_RECT
{
    VECTOR<RECT>   rcMonitors;
    RECT           rcCombined;

    // constructor
    MONITOR_RECT();

    // methods
    static BOOL CALLBACK MonitorEnum(HMONITOR hMon, HDC hdc, LPRECT lprcMonitor, LPARAM pData);
    RECT GetLargestUnionSize();
};


// master class does dirty work
class RAZOR::ENGINECORE
{
    private:
    std::atomic<UINT>    State;

    public:
    MICROSOFT::HINSTANCE    Instance;       // Our instance
    RAZOR::FUNCTION_LINKS   FunctionLinks;  // the passed function callbacks
    std::atomic<BOOLEEN>   bDoExit;
    std::atomic<BOOLEEN>   bResetting;
    NOVA::MEASUREFPS<32>    LoopTimer;
    NOVA::MEASUREFPS<4>     FPSTimer;
    MONITOR_RECT            AllMonitors;

    // direct modules
	RAZOR::DIRECTX	 *DirectX9;		// Direct3D collections
	RAZOR::WINDOW	 *Window;	    // Windows interface 
    RAZOR::CONSOLE	 *Console;	    // Console interface 
    RAZOR::RESOURCE	 *Resources;	// Resource manager  
    RAZOR::PATH	     *Path;	        // Path interface  
    RAZOR::SCHEDULER *Scheduler;	// Time Callback Scheduler class    
    RAZOR::INPUT     *Input;        // Input interface
    RAZOR::DRAW      *Draw;         // Drawing tool

    // constructor
    ENGINECORE(MICROSOFT::HINSTANCE MyInstance);
    ~ENGINECORE();
    void InnerDestructor();
    void InnerDestructorSEH();

    // public interface
    public:
    UINT32 Initialize(RAZOR::FUNCTION_LINKS InputLinks, RAZOR::WINDOW_PARAMS WindowParams, RAZOR::INPUT_CALLBACKS MyEvents = RAZOR::INPUT_CALLBACKS() );
    void MsgProc(MICROSOFT::HWND hWnd, UINT Msg, MICROSOFT::WPARAM wParam, MICROSOFT::LPARAM lParam);
    void MainLoop();
    HINSTANCE GetInstance();

    MAP<STRING, VECTOR<DOUBLE>> GetSystemMetrics();
    MAP<STRING, VECTOR<DOUBLE>> GetSystemMetricsAverage();


    // special
    STRING GetExceptionInfo( EXCEPTION_POINTERS* ep, RAZOR::EXCEPTION_INFO::exception_code_t code = 0);
};


#endif