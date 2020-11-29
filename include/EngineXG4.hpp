
// header gaurd
#ifndef _ENGINEXG4_GAURD_HPP
#define _ENGINEXG4_GAURD_HPP

// main includes
#include <Types.hpp>

// windows stuff
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

// namespace
namespace ENGINEXG4
{
    // typedefs
    typedef LRESULT (CALLBACK *WINDOWS_CALLBACK) (HWND, UINT, WPARAM, LPARAM);
    
    // forward declare our class
    class APPLICATION;

    // enums 
    namespace ENUMS
    {
        enum KEY_CODES;
        enum MOUSE_CODES;
    };

    // states
    namespace STATE
    {
        struct MOUSE;
        struct KEYBOARD;
        struct XBOX_CONTROLLER;
        struct JOYSTICK_CONTROLLER;
        struct WINDOW;
        struct ENGINE;
    };

};

// main class
class ENGINEXG4::APPLICATION
{
    public:

    // pure virtual methods -> implemented by application
    virtual void OnDraw(UINT64 Timestamp) = 0;
    virtual void OnTick(UINT64 Timestamp) = 0;
    virtual void OnLost() = 0;
    virtual void OnReset() = 0;
    virtual void OnResDest() = 0;
    virtual void OnResInit() = 0;
    virtual void OnDragDrop(const VECTOR<STRING> &Paths) = 0;
    virtual void OnKeyboard(UINT32 KeyCode,      BOOLEEN UpOrDown) = 0;
    virtual void OnMouse   (UINT32 MouseCode,    BOOLEEN UpOrDown) = 0;
    virtual void OnJoystick(UINT32 JoystickCode, BOOLEEN UpOrDown) = 0;
    virtual void OnXBox    (UINT32 XBoxCode,     BOOLEEN UpOrDown) = 0;

    // methods 
    STATE::WINDOW                GetWindowState();
    STATE::MOUSE                 GetMouseState();
    STATE::KEYBOARD              GetKeyboardState();
    STATE::XBOX_CONTROLLER       GetXboxControllerState();
    STATE::JOYSTICK_CONTROLLER   GetJoystickControllerState();
    
    // initialization
    UINT32 Start(HINSTANCE hInstance, WINDOWS_CALLBACK WndProc);

    class ENGINECORE;
    private:
    ENGINECORE* Impl;
};

// holds the mouse state full
struct ENGINEXG4::STATE::MOUSE
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

// all input keyboard stuff
struct ENGINEXG4::STATE::KEYBOARD
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

    // for editor keys
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

// struct for the xbox controler
struct ENGINEXG4::STATE::XBOX_CONTROLLER
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
struct ENGINEXG4::STATE::JOYSTICK_CONTROLLER
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

// key states -> referenced from windows.h
enum ENGINEXG4::ENUMS::KEY_CODES
{
    /* alphabet */    /* number keys */   /* modifiers */                     /* F1-F24 keys */
    KEY_A = 'A',      KEY_1 = '1',        KEY_ALT     = VK_MENU,              KEY_F1  = VK_F1,
    KEY_B = 'B',      KEY_2 = '2',        KEY_SHIFT   = VK_SHIFT,             KEY_F2  = VK_F2,
    KEY_C = 'C',      KEY_3 = '3',        KEY_CONTROL = VK_CONTROL,           KEY_F3  = VK_F3,
    KEY_D = 'D',      KEY_4 = '4',                                            KEY_F4  = VK_F4,
    KEY_E = 'E',      KEY_5 = '5',        /* arrow keys */                    KEY_F5  = VK_F5,
    KEY_F = 'F',      KEY_6 = '6',        KEY_UP      = VK_UP,                KEY_F6  = VK_F6,
    KEY_G = 'G',      KEY_7 = '7',        KEY_DOWN    = VK_DOWN,              KEY_F7  = VK_F7,
    KEY_H = 'H',      KEY_8 = '8',        KEY_LEFT    = VK_LEFT,              KEY_F8  = VK_F8,
    KEY_I = 'I',      KEY_9 = '9',        KEY_RIGHT   = VK_RIGHT,             KEY_F9  = VK_F9,
    KEY_J = 'J',      KEY_0 = '0',                                            KEY_F10 = VK_F10,
    KEY_K = 'K',                          /* editor keys */                   KEY_F11 = VK_F11,
    KEY_L = 'L',                          KEY_SPACE       = VK_SPACE,         KEY_F12 = VK_F12,
    KEY_M = 'M',                          KEY_BACKSPACE   = VK_BACK,          KEY_F13 = VK_F13,
    KEY_N = 'N',                          KEY_ESCAPE      = VK_ESCAPE,        KEY_F14 = VK_F14,
    KEY_O = 'O',                          KEY_ENTER       = VK_RETURN,        KEY_F15 = VK_F15,
    KEY_P = 'P',                          KEY_TAB         = VK_TAB,           KEY_F16 = VK_F16,
    KEY_Q = 'Q',                          KEY_CAPS_LOCK   = VK_CAPITAL,       KEY_F17 = VK_F17,
    KEY_R = 'R',                          KEY_SCROLL_LOCK = VK_SCROLL,        KEY_F18 = VK_F18,
    KEY_S = 'S',                          KEY_NUM_LOCK    = VK_NUMLOCK,       KEY_F19 = VK_F19,
    KEY_T = 'T',                                                              KEY_F20 = VK_F20,
    KEY_U = 'U',                          /* navigation keys */               KEY_F21 = VK_F21,
    KEY_V = 'V',                          KEY_INSERT      = VK_INSERT,        KEY_F22 = VK_F22,
    KEY_W = 'W',                          KEY_DELETE      = VK_DELETE,        KEY_F23 = VK_F23,
    KEY_X = 'X',                          KEY_HOME        = VK_HOME,          KEY_F24 = VK_F24,
    KEY_Y = 'Y',                          KEY_END         = VK_END,
    KEY_Z = 'Z',                          KEY_PAGE_UP     = VK_PRIOR,
                                          KEY_PAGE_DOWN   = VK_NEXT,

};

// mouse states
enum ENGINEXG4::ENUMS::MOUSE_CODES
{
    MOUSE_LEFT          = 0,
    MOUSE_RIGHT         = 1,
    MOUSE_MIDDLE        = 2,
    MOUSE_SCROLL_DOWN   = 3,
    MOUSE_SCROLL_UP     = 4,
    MOUSE_SCROLL_BUTTON = 5,

};




#endif // _ENGINEXG4_GAURD_HPP