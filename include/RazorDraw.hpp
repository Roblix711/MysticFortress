
// header gaurd
#ifndef _RAZERDRAW_PUBLIC_GAURD_H
#define _RAZERDRAW_PUBLIC_GAURD_H

// typedefs and standard types
#include <Types.hpp>

// windows stuff
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <Windows.h>

// add to our namespace
namespace RAZOR
{
    // typedefs for function pointers
    typedef LRESULT (CALLBACK *WINDOWS_CALLBACK) (HWND, UINT, WPARAM, LPARAM);
    typedef void (*VOID_FUNCTION_POINTER)();	
    typedef void (*VOID_FUNCTION_POINTER_UINT32_BOOLEEN)(UINT32, BOOLEEN);	
    typedef void (*VOID_FUNCTION_POINTER_VECTOR_STRING)(const VECTOR<STRING>&);
    typedef UINT32   RAZOR_COLOR; // XRGB
    typedef va_list VAR_LIST;

    // struct for the xbox controler
    struct XBOX_CONTROLLER_STATE
    {
        BOOLEEN ButtonUp;
        BOOLEEN ButtonDown;
        BOOLEEN ButtonLeft;
        BOOLEEN ButtonRight;
        BOOLEEN ButtonStart;
        BOOLEEN ButtonBack;
        BOOLEEN ButtonLeftThumb;
        BOOLEEN ButtonRightThumb;
        BOOLEEN ButtonLeftShoulder;
        BOOLEEN ButtonRightShoulder;
        BOOLEEN ButtonA;
        BOOLEEN ButtonB;
        BOOLEEN ButtonX;
        BOOLEEN ButtonY;
        INT32 LeftTrigger;
        INT32 RightTrigger;
        INT32 LeftThumbX;
        INT32 LeftThumbY;
        INT32 RightThumbX;
        INT32 RightThumbY;
    };

    // struct for dxinput8 joysticks
    struct JOYSTICK_CONTROLLER_STATE
    {
        INT32 AxisX;
        INT32 AxisY;
        INT32 AxisZ;
        INT32 RotX;
        INT32 RotY;
        INT32 RotZ;
        VECTOR<BOOLEEN> Buttons;    // supports: 16
        VECTOR<INT32> Sliders;      // supports: 2
        VECTOR<INT32> Others;       // supports: 4
    };


    // holds the mouse state full
    struct INPUT_MOUSE_STATE
    {
        // holds the x,y state + scroll
        struct MOUSE_POSITION
        {
            INT32 X;     // x position
            INT32 Y;     // y position
            INT32 S;     // scroll position
        };

        // holds the button state
        struct MOUSE_BUTTONS
        {
            INT32 M;     // middle state
            INT32 L;     // left state
            INT32 R;     // right state
        };

        // assemble into one
        MOUSE_POSITION Position;
        MOUSE_BUTTONS  Buttons;
    };

    // fill this shit out for callback events
    struct INPUT_CALLBACKS
    {
        // input transitions
        VOID_FUNCTION_POINTER MouseLeftAny;
        VOID_FUNCTION_POINTER MouseLeftDown;
        VOID_FUNCTION_POINTER MouseLeftUp;
        VOID_FUNCTION_POINTER MouseRightAny;
        VOID_FUNCTION_POINTER MouseRightDown;
        VOID_FUNCTION_POINTER MouseRightUp;
        VOID_FUNCTION_POINTER MouseMiddleAny;
        VOID_FUNCTION_POINTER MouseMiddleDown;
        VOID_FUNCTION_POINTER MouseMiddleUp;
        VOID_FUNCTION_POINTER MouseScrollAny;
        VOID_FUNCTION_POINTER MouseScrollDown;
        VOID_FUNCTION_POINTER MouseScrollUp;

        // special callback for keyboards
        VOID_FUNCTION_POINTER_UINT32_BOOLEEN KeyboardAny;

        // default -> no callbacks
        INPUT_CALLBACKS() : 
            MouseLeftAny(nullptr),   MouseLeftDown(nullptr),   MouseLeftUp(nullptr),
            MouseRightAny(nullptr),  MouseRightDown(nullptr),  MouseRightUp(nullptr),
            MouseMiddleAny(nullptr), MouseMiddleDown(nullptr), MouseMiddleUp(nullptr),
            MouseScrollAny(nullptr), MouseScrollDown(nullptr), MouseScrollUp(nullptr),
            KeyboardAny(nullptr)
        {
            // NOCODE;
        }
    };

    // all input keyboard stuff
    struct INPUT_KEYBOARD_STATE
    {
        // f1 THROUGH f12
        struct STATE_FKEYS
        {
            BOOLEEN F[24];
        };

        // for control shift alt
        struct STATE_CONTROL_SHIFT_ALT
        {
            BOOLEEN Control;
            BOOLEEN Alt;
            BOOLEEN Shift;
        };

        // for control shift alt
        struct STATE_ARROWS
        {
            BOOLEEN Left;
            BOOLEEN Right;
            BOOLEEN Up;
            BOOLEEN Down;
        };

        // for lock keys
        struct STATE_LOCKS
        {
            BOOLEEN CapsLock;
            BOOLEEN NumLock;
            BOOLEEN ScrollLock;
        };

        // for navigation keys
        struct STATE_NAVIGATION
        {
            BOOLEEN Insert;
            BOOLEEN Delete;
            BOOLEEN Home;
            BOOLEEN End;
            BOOLEEN PageUp;
            BOOLEEN PageDown;
        };

        // for lock keys
        struct STATE_EDITOR
        {
            BOOLEEN Space;
            BOOLEEN Backspace;
            BOOLEEN Esc;
            BOOLEEN Enter;
            BOOLEEN Tab;
        };

        // for less used 
        struct STATE_WINDOWS
        {
            BOOLEEN Print;
            BOOLEEN Pause;
            BOOLEEN Windows;
            BOOLEEN AltMenu;
        };

        // assemble into one
        STATE_CONTROL_SHIFT_ALT     CSA;
        STATE_ARROWS                Arrows;
        STATE_NAVIGATION            Navigation;
        STATE_EDITOR                Edit;
        STATE_LOCKS                 Locks;  
        STATE_WINDOWS               Winkeys;
        STATE_FKEYS                 FKeys;
    };

    // the required functions
    struct FUNCTION_LINKS
    {
	    VOID_FUNCTION_POINTER  DrawFunc;
        VOID_FUNCTION_POINTER  TickFunc;
        VOID_FUNCTION_POINTER  FrameFunc;
	    VOID_FUNCTION_POINTER  ResInitFunc;
	    VOID_FUNCTION_POINTER  ResDestFunc;
        VOID_FUNCTION_POINTER  OnLostFunc;
	    VOID_FUNCTION_POINTER  OnResetFunc;
        WINDOWS_CALLBACK       EventMain;
    };

    // what kind of window, etc..
    struct WINDOW_PARAMS
    {
        STRING Name;
        UINT32 ScreenWidth;
        UINT32 ScreenHeight;
        UINT32 DesiredFPS;  // draw    frames per second
        UINT32 DesiredLPS;  // logical frames per second
        BOOLEEN bWindowed;
        BOOLEEN bVerticalSync; 
        UINT32 IconResource;
        BOOLEEN bConsole;   // enable the console?
        VOID_FUNCTION_POINTER_VECTOR_STRING DragDropCallback;
    };

    // transformed colorized
    struct DRAW_VERTEX_2D
    { 
        FLOAT x, y;                 // X,y points on screen
        FLOAT z, rhw;               // set z=0.0f, rhw = 1.0f
        RAZOR::RAZOR_COLOR color;   // the color
    };

    // public api for tracker
    class ENGINECORE;
    class ENGINE
    {
        public:
        typedef void* METADATA;

        // constructor destructor
        ENGINE(HINSTANCE MyInstance);
        ~ENGINE();

        // imports from engine
        UINT32 EngineInitialize(VECTOR<METADATA> Metadata);
        void EngineStop();
        void EngineMainLoop();
        void EngineMsgProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
        void ConsoleLog(const CHAR *message, ...);
        void ConsoleLogV(const CHAR *message, VAR_LIST Args);
        STRING GetExceptionInfo(EXCEPTION_POINTERS *pointers, DWORD dwException);
        HINSTANCE GetInstance();

        // imports from path
        STRING PathMakeAbsolute(const STRING &RelativePathName);
        WSTRING PathMakeAbsolute(const WSTRING &RelativePathName);

        // imports from draw
        UINT32 DrawSetRenderState(UINT32 Type, UINT32 Value);
        void DrawSetRenderState();
        void DrawClearScreen(UINT32 Color);
        void DrawPixel(INT32 X, INT32 Y, UINT32 Color);
        void DrawCircle(INT32 X, INT32 Y, INT32 Radius, UINT32 Color, BOOLEEN Fill);
        void DrawLine(INT32 X1, INT32 Y1, INT32 X2, INT32 Y2, UINT32 Color, FLOAT Thickness = 0.0f);
        void DrawRect(INT32 X1, INT32 Y1, INT32 X2, INT32 Y2, UINT32 Color, BOOLEEN Fill, INT32 Thickness = 0, BOOLEEN Rounded = false);
        void DrawSetLinePrefs(FLOAT Width = 1.0, BOOLEEN Anti = false, FLOAT Scale = 1.0, UINT32 Pattern = UINT32(-1));
        void DrawTriangleFan(VECTOR<DRAW_VERTEX_2D> Points, UINT32 Color);
        void DrawTriangleList(VECTOR<DRAW_VERTEX_2D> Points, UINT32 Color);

        // for the "UINT32 Flags" use | combinations of the following: 
        // DT_TOP,  DT_LEFT,  DT_RIGHT,  DT_BOTTOM,  DT_CENTER,  DT_VCENTER
	    RECT DrawTexts(const STRING &Font, UINT32 Size, const CHAR *message, INT32 x, INT32 y, INT32 x2, INT32 y2, RAZOR_COLOR color, UINT32 Flags, BOOLEEN Clip, BOOLEEN Calculate); // for clip
        RECT DrawTexts(const STRING &Font, UINT32 Size, const CHAR *message, INT32 x, INT32 y,                     RAZOR_COLOR color, UINT32 Flags, BOOLEEN Calculate); // for free
        RECT DrawTextsF(const STRING &Font, UINT32 Size,                      INT32 x, INT32 y,                     RAZOR_COLOR color, UINT32 Flags, BOOLEEN Calculate, const CHAR *message, ...); // for printfs
        RECT DrawTextsF(                                                      INT32 x, INT32 y,                     RAZOR_COLOR color, UINT32 Flags, BOOLEEN Calculate, const CHAR *message, ...); // no font specified
        RECT DrawTextsR(const STRING &Font, UINT32 Size,                      INT32 x, INT32 y, FLOAT Rotation,     RAZOR_COLOR color, UINT32 Flags, BOOLEEN Calculate, const CHAR *message, ...); // for rotations
        RECT DrawTextsR(                                                      INT32 x, INT32 y, FLOAT Rotation,     RAZOR_COLOR color, UINT32 Flags, BOOLEEN Calculate, const CHAR *message, ...); // for rotations


        // for color gen
        inline static RAZOR_COLOR DrawMakeColor(INT32 r, INT32 g, INT32 b, INT32 a) 
        {
	        return  BYTE(b) + (BYTE(g)<<8) + (BYTE(r)<<16) + (BYTE(a)<<24);
        }

        // imports from resource
        UINT32 VideoCreate(UINT32 Width, UINT32 Height, UINT32 Frames);
        void   VideoLock(UINT32 ResouceEngineID);
        void   VideoUnlock(UINT32 ResouceEngineID);
        BYTE*  VideoDeref(UINT32 ResouceEngineID, UINT32 X, UINT32 Y, UINT32 F);
        void   VideoUpdate(UINT32 ResouceEngineID, UINT32 F);
        void   VideoDraw(UINT32 ResouceEngineID, INT32 X, INT32 Y, UINT32 F, DOUBLE Sx, DOUBLE Sy);

        // imports from sprites
        void   SpriteDrawRotated(UINT32 ResourceEngineID, INT32 X, INT32 Y, DOUBLE Sx, DOUBLE Sy, FLOAT Cx, FLOAT Cy, FLOAT Rotation, RAZOR_COLOR ColorMix);
        void   SpriteDraw(UINT32 ResourceEngineID, INT32 X, INT32 Y, DOUBLE Sx, DOUBLE Sy, RAZOR_COLOR ColorMix);
        UINT32 SpriteCreate(UINT32 Width, UINT32 Height);
        void   SpriteUpdate(UINT32 TextureID, UINT32 SurfaceID, UINT32 F);

        // imports from keyboard input
        INPUT_KEYBOARD_STATE                          GetKeyboardState();
        INPUT_KEYBOARD_STATE::STATE_CONTROL_SHIFT_ALT GetKeyboardStateCSA();

        // imports from mouse input
        INPUT_MOUSE_STATE                   GetMouseState();
        INPUT_MOUSE_STATE::MOUSE_POSITION   GetMousePosition();
        INPUT_MOUSE_STATE::MOUSE_BUTTONS    GetMouseButtons();
        INT32 GetMouseX();
        INT32 GetMouseY();
        INT32 GetMouseS();
        INT32 GetMouseM();
        INT32 GetMouseL();
        INT32 GetMouseR();

        // imports from xbox/joystick input
        BOOLEEN GetXboxState    (UINT32 Index, XBOX_CONTROLLER_STATE     &State);
        BOOLEEN GetJoystickState(UINT32 Index, JOYSTICK_CONTROLLER_STATE &State);

        // imports from window
        RECT  GetScreenExtent();
        INT32 GetScreenWidth();
        INT32 GetScreenHeight();
        INT32 GetDialogSaveFile(STRING         &OutputFile,  const VECTOR<PAIR<STRING, STRING>> &Types); // single
        INT32 GetDialogOpenFile(VECTOR<STRING> &OutputFiles, const VECTOR<PAIR<STRING, STRING>> &Types); // multi

        // imports from resources
        void StopAll();
        void StartAll();

        // imports from sysmetric
        MAP<STRING, VECTOR<DOUBLE>> GetSystemMetrics();
        MAP<STRING, VECTOR<DOUBLE>> GetSystemMetricsAverage();

        // details hidden
        private:
        ENGINECORE* _Impl;
    };

};


#endif // _RAZERDRAW_PUBLIC_GAURD_H