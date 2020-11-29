
// main header
#include "Engine.h"
#include <algorithm>

// protected
namespace 
{
    // for colors ARGB888 only
    using RAZOR::RAZOR_COLOR;
    const RAZOR_COLOR cOpaque  = (255 << 24);
    const RAZOR_COLOR cBlack   = (255 << 24);
}

BOOL CALLBACK MONITOR_RECT::MonitorEnum(HMONITOR hMon, HDC hdc, LPRECT lprcMonitor, LPARAM pData)
{
    MONITOR_RECT* pThis = reinterpret_cast<MONITOR_RECT*>(pData);
    pThis->rcMonitors.push_back(*lprcMonitor);
    UnionRect(&pThis->rcCombined, &pThis->rcCombined, lprcMonitor);
    return TRUE;
}

MONITOR_RECT::MONITOR_RECT()
{
    SetRectEmpty(&rcCombined);
    EnumDisplayMonitors(0, 0, MonitorEnum, (LPARAM)this);
}

RECT MONITOR_RECT::GetLargestUnionSize()
{
    // find maxX, maxY
    MONITOR_RECT AllMonitors;
    RECT MaxUnion = {0,0,0,0};
    MaxUnion.right  = std::numeric_limits<UINT32>::min();
    MaxUnion.bottom = std::numeric_limits<UINT32>::min();
    for(auto &MoniorRect: AllMonitors.rcMonitors)
    {
        MaxUnion.right = std::max<UINT32>(MaxUnion.right, MoniorRect.right-MoniorRect.left);
        MaxUnion.bottom = std::max<UINT32>(MaxUnion.bottom, MoniorRect.bottom-MoniorRect.top);
    }
    return MaxUnion;
}

// constructor
RAZOR::ENGINECORE::ENGINECORE(MICROSOFT::HINSTANCE MyInstance) :
     State(RAZOR_ENGINE_UNKNOWN), 
     bDoExit(false),
     bResetting(false),
     Instance(MyInstance), 
     Path(nullptr), 
     Window(nullptr), 
     DirectX9(nullptr), 
     Console(nullptr),
     Scheduler(nullptr),
     Input(nullptr),
     Resources(nullptr),
     Draw(nullptr)
{
    // initialize our objects
    Path       = new NOTHROW RAZOR::PATH(this);
    Window     = new NOTHROW RAZOR::WINDOW(this);
    DirectX9   = new NOTHROW RAZOR::DIRECTX(this);
    Console    = new NOTHROW RAZOR::CONSOLE(this);
    Scheduler  = new NOTHROW RAZOR::SCHEDULER(this);
    Input      = new NOTHROW RAZOR::INPUT(this);
    Resources  = new NOTHROW RAZOR::RESOURCE(this);
    Draw       = new NOTHROW RAZOR::DRAW(this);

    //printf("CORE INIT(): %d %d %d %d %d %d %d %d\n", Path, Window, DirectX9, Console, Scheduler, Input, Resources, Draw);

    // find maxX, maxY
    
    UINT32 MaxX = std::numeric_limits<UINT32>::min();
    UINT32 MaxY = std::numeric_limits<UINT32>::min();
    UINT i=0;
    for(auto &MoniorRect: AllMonitors.rcMonitors)
    {
        MaxX = std::max<UINT32>(MaxX, MoniorRect.right-MoniorRect.left);
        MaxY = std::max<UINT32>(MaxY, MoniorRect.bottom-MoniorRect.top);
        //ParentEngine->Console->Log(DEBUG_INFO, "Sizes[%d] = %d %d %d %d\n", i++, MoniorRect.left, MoniorRect.top, MoniorRect.right, MoniorRect.bottom);
    }
    //ParentEngine->Console->Log(DEBUG_INFO, "Sizes = %d %d\n", MaxX, MaxY);


}

void RAZOR::ENGINECORE::InnerDestructor()
{
    try
    {
        // this stops all threads!
        delete Scheduler;

        // call the end 
        FunctionLinks.ResDestFunc();

        // delete rest of things
        delete Input;
        delete Draw;
        delete Resources;
        delete DirectX9;
        delete Window;
        delete Path;
        delete Console;
    }
    catch( const EXCEPTION& e)
    {
        // make a note of it
        Console->Log(DEBUG_FATAL, "RAZOR::~ENGINECORE() Exception Caught: %s", e.what());

        // rethrow downstairs
        //throw RUNTIME_EXCEPTION(e.what()); // recast
    }
}


// just a wrapper
void RAZOR::ENGINECORE::InnerDestructorSEH()
{
    __try
    {
        InnerDestructor();
    }
    __except(RAZOR::EXCEPTION_INFO::ExceptionFilter(GetExceptionInformation(), GetExceptionCode()))
    {
        // set this pipeblock to DESTROYED!!
        //This->SetDestroyed();
    }
}

// kill all
RAZOR::ENGINECORE::~ENGINECORE()
{
    InnerDestructorSEH();
}

// initializes stuff
UINT32 RAZOR::ENGINECORE::Initialize(FUNCTION_LINKS InputLinks, WINDOW_PARAMS WindowParams, INPUT_CALLBACKS MyEvents)
{
    // save it 
    FunctionLinks = InputLinks;

    // safe
    try
    {
        // initialize core modules
        Path->Initialize();
        Console->Initialize(Path->MakeAbsolutePathChar("DebugLog.txt"), WindowParams.bConsole);
        Window->Initialize(WindowParams);
        DirectX9->Initialize(*Window);
        Input->Initialize( MyEvents );
        Resources->Initialize();
        Draw->Initialize();
        Scheduler->Initialize(InputLinks, WindowParams);   

        // check all things
        #ifdef _DEBUG
        Console->Log(DEBUG_INFO, "RazorEngine[debug] Version %s: Startup Successful\n", RAZOR_VERSION);
        #else
        Console->Log(DEBUG_INFO, "RazorEngine[release] Version %s: Startup Successful\n", RAZOR_VERSION);
        #endif 

	    // call application resources init function
        FPSTimer.Tick();
	    if (FunctionLinks.ResInitFunc != NULL) FunctionLinks.ResInitFunc();
         
    }
    catch( const EXCEPTION& e)
    {
        // make a note of it
        Console->Log(DEBUG_FATAL, "RAZOR::ENGINECORE::Initialize() Exception Caught: %s", e.what());

        // rethrow downstairs
        Sleep(10000);
        throw EXCEPTION_RUNTIME(e.what()); // recast
    }

    return 0;
}

// must be called by the application if you want to receive events
void RAZOR::ENGINECORE::MsgProc(MICROSOFT::HWND hWnd, UINT Msg, MICROSOFT::WPARAM wParam, MICROSOFT::LPARAM lParam)
{
    // pass events down
    Input->MsgProc(hWnd, Msg, wParam, lParam);
    Window->MsgProc(hWnd, Msg, wParam, lParam);
}

// get hinstance
HINSTANCE RAZOR::ENGINECORE::GetInstance()
{
    return Instance;
}

// an easy call for main loop start
void RAZOR::ENGINECORE::MainLoop()
{
    // global resources
    MICROSOFT::MSG Msg;
    MICROSOFT::ZeroMemory(&Msg, sizeof(Msg));
    

    // Main message loop:
    LoopTimer.Tick();
    while (FOREVER) // MICROSOFT::GetMessage(&Msg, NULL, 0, 0)
    {
        // read safetly
        BOOLEEN bExit = bDoExit;
        if (bExit == true) break;
        else
        {
            // process all messages 
            while (MICROSOFT::PeekMessageA(&Msg, NULL, 0U, 0U, PM_REMOVE) && (bResetting == false) )
            {
                MICROSOFT::TranslateMessage(&Msg);
                MICROSOFT::DispatchMessage(&Msg);
            }
        }

        // update fps
        LoopTimer.Tock();
        Sleep(0);

        // pass to DX9
        DirectX9->MessagePump();

    }
}


// gets all metrics available
MAP<STRING, VECTOR<DOUBLE>> RAZOR::ENGINECORE::GetSystemMetrics()
{
    MAP<STRING, VECTOR<DOUBLE>> Output;
    Output["Memory"] =  RAZOR::SYS_METRIC::GetMemoryFootprint();
    Output["CPU"]    =  RAZOR::SYS_METRIC::GetCPUFootprint();
    Output["Loop"]   =  {FPSTimer.GetTime(), LoopTimer.GetTime()};
    return Output;
}

// gets all metrics available
MAP<STRING, VECTOR<DOUBLE>> RAZOR::ENGINECORE::GetSystemMetricsAverage()
{
    // local here only
    static const DOUBLE NUM_SAMPLES = 64.0;
    static BOOLEEN FirstTime = false;
    static MAP<STRING, VECTOR<DOUBLE>> Average;
    if (FirstTime == false)
    {
        FirstTime = true;
        Average = GetSystemMetrics();
    }

    // get new average
    MAP<STRING, VECTOR<DOUBLE>> Raw = GetSystemMetrics();
    for(auto it = Average.begin(); it != Average.end(); ++it)
    {
        for(UINT32 i=0; i<it->second.size(); ++i)
        {
            DOUBLE NewAvg = it->second[i];
            it->second[i] -= NewAvg / NUM_SAMPLES;
            it->second[i] += Raw[it->first][i] / NUM_SAMPLES;
        }
    }

    return Average;
}


// call static 
STRING RAZOR::ENGINECORE::GetExceptionInfo( EXCEPTION_POINTERS* P, RAZOR::EXCEPTION_INFO::exception_code_t code)
{
    return RAZOR::EXCEPTION_INFO::GetExceptionInfo(P, true, code);
}


// implementation constructor
RAZOR::ENGINE::ENGINE(MICROSOFT::HINSTANCE MyInstance): _Impl(new RAZOR::ENGINECORE(MyInstance))
{
    // NOCODE;
}
// destructor 
RAZOR::ENGINE::~ENGINE()
{
    delete _Impl;
}

// unload into 
UINT32 RAZOR::ENGINE::EngineInitialize(VECTOR<METADATA> Metadata)
{
    // unload by index
    FUNCTION_LINKS InputLinks       = *static_cast<FUNCTION_LINKS*>( Metadata[0] );
    WINDOW_PARAMS WindowParams      = *static_cast<WINDOW_PARAMS*>( Metadata[1] );
    INPUT_CALLBACKS InputCallback   = *static_cast<INPUT_CALLBACKS*>( Metadata[2] );

    // proxy
    return _Impl->Initialize(InputLinks, WindowParams, InputCallback);
}

// shut down everything
void RAZOR::ENGINE::EngineStop()
{
    _Impl->bDoExit = true;
} 

// loop function
void RAZOR::ENGINE::EngineMainLoop()
{
    _Impl->MainLoop();
}

// implements message process handling
void RAZOR::ENGINE::EngineMsgProc(MICROSOFT::HWND hWnd, UINT message, MICROSOFT::WPARAM wParam, MICROSOFT::LPARAM lParam)
{
    _Impl->MsgProc(hWnd, message, wParam, lParam);
}

// gets the internal instance
HINSTANCE RAZOR::ENGINE::GetInstance()
{
    return _Impl->GetInstance();
}

STRING RAZOR::ENGINE::PathMakeAbsolute(const STRING &RelativePathName)
{
    return _Impl->Path->MakeAbsolutePathChar(RelativePathName);
}
WSTRING RAZOR::ENGINE::PathMakeAbsolute(const WSTRING &RelativePathName)
{
    return _Impl->Path->MakeAbsolutePathWide(RelativePathName);
}

// implements console logs
void RAZOR::ENGINE::ConsoleLog(const CHAR *message, ...)
{
    // start variable arguments
    VAR_LIST args;
    va_start(args, message);
    _Impl->Console->LogV(message, args);
    va_end(args);
}
// implements console logs
void RAZOR::ENGINE::ConsoleLogV(const CHAR *message, VAR_LIST Args)
{
    _Impl->Console->LogV(message, Args);
}
// proxy back
STRING RAZOR::ENGINE::GetExceptionInfo(EXCEPTION_POINTERS *pointers, DWORD dwException)
{
    return _Impl->GetExceptionInfo(pointers, dwException);
}

// others
void RAZOR::ENGINE::StopAll()
{
    _Impl->Scheduler->StopAll();
}
void RAZOR::ENGINE::StartAll()
{
    _Impl->Scheduler->StartAll();
}

// get all metrics at once
MAP<STRING, VECTOR<DOUBLE>> RAZOR::ENGINE::GetSystemMetrics()        { return _Impl->GetSystemMetrics();        }
MAP<STRING, VECTOR<DOUBLE>> RAZOR::ENGINE::GetSystemMetricsAverage() { return _Impl->GetSystemMetricsAverage(); }

// imports from mouse input
RAZOR::INPUT_MOUSE_STATE                 RAZOR::ENGINE::GetMouseState()       {   return _Impl->Input->GetMouseState();       }
RAZOR::INPUT_MOUSE_STATE::MOUSE_POSITION RAZOR::ENGINE::GetMousePosition()    {   return _Impl->Input->GetMousePosition();    }
RAZOR::INPUT_MOUSE_STATE::MOUSE_BUTTONS  RAZOR::ENGINE::GetMouseButtons()     {   return _Impl->Input->GetMouseButtons();     }

// imports from mouse input
INT32 RAZOR::ENGINE::GetMouseX(){   return _Impl->Input->GetMouseX();  }
INT32 RAZOR::ENGINE::GetMouseY(){   return _Impl->Input->GetMouseY();  }
INT32 RAZOR::ENGINE::GetMouseS(){   return _Impl->Input->GetMouseS();  }
INT32 RAZOR::ENGINE::GetMouseM(){   return _Impl->Input->GetMouseM();  }
INT32 RAZOR::ENGINE::GetMouseL(){   return _Impl->Input->GetMouseL();  }
INT32 RAZOR::ENGINE::GetMouseR(){   return _Impl->Input->GetMouseR();  }

// imports from keyboard input 
RAZOR::INPUT_KEYBOARD_STATE                           RAZOR::ENGINE::GetKeyboardState()     { return _Impl->Input->GetKeyboardState(); }
RAZOR::INPUT_KEYBOARD_STATE::STATE_CONTROL_SHIFT_ALT  RAZOR::ENGINE::GetKeyboardStateCSA()  { return _Impl->Input->GetKeyboardStateCSA(); }

// imports from window
RECT  RAZOR::ENGINE::GetScreenExtent() { return {0, 0, GetScreenWidth(), GetScreenHeight()}; };
INT32 RAZOR::ENGINE::GetScreenWidth()  { return _Impl->Window->GetScreenWidth();  }
INT32 RAZOR::ENGINE::GetScreenHeight() { return _Impl->Window->GetScreenHeight();  }

INT32 RAZOR::ENGINE::GetDialogSaveFile(STRING         &OutputFile,  const VECTOR<PAIR<STRING, STRING>> &Types) 
{
    return _Impl->Window->GetDialogSaveFile(OutputFile, Types); 
}
INT32 RAZOR::ENGINE::GetDialogOpenFile(VECTOR<STRING> &OutputFiles, const VECTOR<PAIR<STRING, STRING>> &Types) { return _Impl->Window->GetDialogOpenFile(OutputFiles, Types); }


// gets xinput controller
BOOLEEN RAZOR::ENGINE::GetXboxState(UINT32 Index, XBOX_CONTROLLER_STATE &State) 
{ 
    return _Impl->Input->GetXboxState(Index, State); 
}

// gets dxinput8 controller
BOOLEEN RAZOR::ENGINE::GetJoystickState(UINT32 Index, JOYSTICK_CONTROLLER_STATE &State)
{
    return false; // _Impl->Input->GetJoystickState(Index, State);
}


// imports from resource
UINT32 RAZOR::ENGINE::VideoCreate(UINT32 Width, UINT32 Height, UINT32 Frames)
{
    return _Impl->Resources->CreateVideo(Width, Height, Frames);
}

void RAZOR::ENGINE::VideoLock(UINT32 ResouceEngineID)
{
    _Impl->Resources->VideoArray[ResouceEngineID]->Lock();
}

void RAZOR::ENGINE::VideoUnlock(UINT32 ResouceEngineID)
{
    _Impl->Resources->VideoArray[ResouceEngineID]->Unlock();
}

// uses the pitch/channels to figure out where the data is
inline UINT32 RawVideoDeref(UINT32 x, UINT32 y, UINT32 Pitch, UINT32 Channel) // changes an [x,y,c] -> [t] ... assumes XRGB
{
	return (x << 2) + (Pitch*y) + (3 - Channel - 1);
}

BYTE* RAZOR::ENGINE::VideoDeref(UINT32 ResouceEngineID, UINT32 X, UINT32 Y, UINT32 F)
{
    return &_Impl->Resources->VideoArray[ResouceEngineID]->LockStart[F][ RawVideoDeref(X, Y, _Impl->Resources->VideoArray[ResouceEngineID]->Pitch, CHANNEL_BLUE) ];
}

void RAZOR::ENGINE::VideoUpdate(UINT32 ResouceEngineID, UINT32 F)
{
    _Impl->Draw->UpdateVideo(ResouceEngineID, 0);
}

void RAZOR::ENGINE::VideoDraw(UINT32 ResouceEngineID, INT32 X, INT32 Y, UINT32 F, DOUBLE Sx, DOUBLE Sy)
{
    _Impl->Draw->Video(ResouceEngineID, X, Y, F, Sx, Sy, false);
}

UINT32 RAZOR::ENGINE::SpriteCreate(UINT32 Width, UINT32 Height)
{
    return _Impl->Resources->CreateBitmap(Width, Height, true);
}
void RAZOR::ENGINE::SpriteDraw(UINT32 ResourceEngineID, INT32 X, INT32 Y, DOUBLE Sx, DOUBLE Sy, RAZOR_COLOR ColorMix)
{
    _Impl->Draw->Sprite(ResourceEngineID, X, Y, Sx, Sy, ColorMix);
}
void RAZOR::ENGINE::SpriteDrawRotated(UINT32 ResourceEngineID, INT32 X, INT32 Y, DOUBLE Sx, DOUBLE Sy, FLOAT Cx, FLOAT Cy, FLOAT Rotation, RAZOR_COLOR ColorMix)
{
    _Impl->Draw->SpriteRotated(ResourceEngineID, X, Y, Sx, Sy, Cx, Cy, Rotation, ColorMix);
}

void RAZOR::ENGINE::SpriteUpdate(UINT32 TextureID, UINT32 SurfaceID, UINT32 F)
{
    _Impl->Draw->UpdateSprite(TextureID, SurfaceID, F);
}

// imports from draw
UINT32 RAZOR::ENGINE::DrawSetRenderState(UINT32 Type, UINT32 Value)
{
    return _Impl->DirectX9->SetRenderStates(Type, Value);
}
void RAZOR::ENGINE::DrawSetRenderState()
{
    _Impl->DirectX9->SetRenderStates();
}

void RAZOR::ENGINE::DrawClearScreen(UINT32 Color)
{
    _Impl->Draw->ClearScreen(Color);
}
void RAZOR::ENGINE::DrawPixel(INT32 X, INT32 Y, UINT32 Color)
{
    _Impl->Draw->PixelRaw(X, Y, Color);
}
void RAZOR::ENGINE::DrawLine(INT32 X1, INT32 Y1, INT32 X2, INT32 Y2, UINT32 Color, FLOAT Thickness)
{
    if (Thickness == 0.0) _Impl->Draw->Line(X1, Y1, X2, Y2, Color);
    else                  _Impl->Draw->LineThick(X1, Y1, X2, Y2, Thickness, Color);
}

void RAZOR::ENGINE::DrawSetLinePrefs(FLOAT Width, BOOLEEN Anti, FLOAT Scale, UINT32 Pattern)
{
    _Impl->Draw->SetLinePrefs(Width, Anti, Scale, Pattern);
}

void RAZOR::ENGINE::DrawTriangleFan(VECTOR<RAZOR::DRAW_VERTEX_2D> Points, UINT32 Color)
{
    _Impl->Draw->TriangleFanFilled(Points, Color);
}
void RAZOR::ENGINE::DrawTriangleList(VECTOR<RAZOR::DRAW_VERTEX_2D> Points, UINT32 Color)
{
    _Impl->Draw->TriangleListFilled(Points, Color);
}

void RAZOR::ENGINE::DrawRect(INT32 X1, INT32 Y1, INT32 X2, INT32 Y2, UINT32 Color, BOOLEEN Fill, INT32 Thickness, BOOLEEN Rounded)
{   
    if (Thickness == 0)
    {
        if (Fill) _Impl->Draw->RectFill(X1, Y1, X2, Y2, Color);
        else      _Impl->Draw->Rect(X1, Y1, X2, Y2, Color);
    }
    else
    {
        if (Rounded == true) 
        {
             if (Fill) _Impl->Draw->RectFill(X1, Y1, X2, Y2, Color);
            _Impl->Draw->RectFillCurve(std::min(X1,X2), std::min(Y1,Y2), std::max(X1,X2), std::max(Y1,Y2), Thickness, Color);
        }
        else
        {
            _Impl->Draw->RectFillThick(std::min(X1,X2), std::min(Y1,Y2), std::max(X1,X2), std::max(Y1,Y2), Thickness, Color);
        }

        //else      _Impl->Draw->RectFillCurve(std::min(X1,X2), std::min(Y1,Y2), std::max(X1,X2), std::max(Y1,Y2), 1, Color);
    }
}

void RAZOR::ENGINE::DrawCircle(INT32 X, INT32 Y, INT32 Radius, UINT32 Color, BOOLEEN Fill)
{
    if (Fill) _Impl->Draw->CircleFill(X, Y, Radius, Color);
    else      _Impl->Draw->Circle(X, Y, Radius, Color);
}
// with font, message specified, with clip size
RECT RAZOR::ENGINE::DrawTexts(const STRING &Font, UINT32 Size, const CHAR *message, INT32 x, INT32 y, INT32 x2, INT32 y2, RAZOR_COLOR color, UINT32 Flags, BOOLEEN Clip, BOOLEEN Calculate)
{
    return _Impl->Draw->Text(Font, Size, message, x, y, x2, y2, color, Flags, Clip, Calculate);
}
// with font, no clipp size
RECT RAZOR::ENGINE::DrawTexts(const STRING &Font, UINT32 Size, const CHAR *message, INT32 x, INT32 y, RAZOR_COLOR color, UINT32 Flags, BOOLEEN Calculate)
{
    return _Impl->Draw->Text(Font, Size, message, x, y, color, Flags, Calculate);
}
// with font, printf style, no clip
RECT RAZOR::ENGINE::DrawTextsF(const STRING &Font, UINT32 Size, INT32 x, INT32 y, RAZOR_COLOR color, UINT32 Flags, BOOLEEN Calculate, const CHAR *message, ...)
{
    // start variable arguments
    RAZOR::VAR_LIST args;
    va_start(args, message);
    RECT Bounds =  _Impl->Draw->Text(Font, Size, x, y, color, Flags, Calculate, message, args);
    va_end(args);
    return Bounds;
}
// withut font, printf style, no clip
RECT RAZOR::ENGINE::DrawTextsF( INT32 x, INT32 y, RAZOR_COLOR color, UINT32 Flags, BOOLEEN Calculate, const CHAR *message, ...)
{
    // start variable arguments
    RAZOR::VAR_LIST args;
    va_start(args, message);
    RECT Bounds =  _Impl->Draw->Text( x, y, color, Flags, Calculate, message, args);
    va_end(args);
    return Bounds;
}

// with font, printf style, no clip
RECT RAZOR::ENGINE::DrawTextsR(const STRING &Font, UINT32 Size, INT32 x, INT32 y, FLOAT Rotation, RAZOR_COLOR color, UINT32 Flags, BOOLEEN Calculate, const CHAR *message, ...)
{
    // start variable arguments
    RAZOR::VAR_LIST args;
    va_start(args, message);
    RECT Bounds =  _Impl->Draw->TextRotated(Font, Size, x, y, Rotation, color, Flags, Calculate, message, args);
    va_end(args);
    return Bounds;
}