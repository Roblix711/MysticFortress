
// main header
#include "Input.h"

// other stuff for joystick
#include <commctrl.h>
#include <basetsd.h>
#include <dinput.h>
#include <dinputd.h>
#include <assert.h>
#include <oleauto.h>
#include <shellapi.h>

// remove warnings in release
#ifndef _DEBUG
#pragma warning( disable : 4996 ) // disable deprecated warning 
#endif

// for HID mouse 
#ifndef HID_USAGE_PAGE_GENERIC
#define HID_USAGE_PAGE_GENERIC         ((USHORT) 0x01)
#endif
#ifndef HID_USAGE_GENERIC_MOUSE
#define HID_USAGE_GENERIC_MOUSE        ((USHORT) 0x02)
#endif

// junks
#define SAFE_DELETE(p)  { if(p) { delete (p);     (p)=NULL; } }
#define SAFE_RELEASE(p) { if(p) { (p)->Release(); (p)=NULL; } }

// helper function -> STOLEN from Windowsx.h
inline INT GetXLParam(MICROSOFT::LPARAM LP) { return ((int)(short)LOWORD(LP)); }
inline INT GetYLParam(MICROSOFT::LPARAM LP) { return ((int)(short)HIWORD(LP)); }

// static proxies
static RAZOR::INPUT* StaticInput = NULL;

// constructor
RAZOR::INPUT::INPUT(RAZOR::ENGINECORE *MyParent) : ParentEngine(MyParent)
{
    // clear these large structs
    memset(&MouseState,    0, sizeof(MouseState));
    memset(&KeyboardState, 0, sizeof(KeyboardState));
}

RAZOR::INPUT::~INPUT()
{

}

// main methods
UINT32 RAZOR::INPUT::Initialize(RAZOR::INPUT_CALLBACKS MyEvents)
{
    // transfer callbacks
    Events = MyEvents; 
    return 0;
}

// gets xbox state
BOOLEEN RAZOR::INPUT::GetXboxState(UINT32 Index, XBOX_CONTROLLER_STATE &State)
{
    // Zeroise the state
    MICROSOFT::ZeroMemory(&XControllerState, sizeof(XINPUT_STATE));

    // Get the state
    DWORD Result = XInputGetState(Index, &XControllerState);
    if (Result == ERROR_SUCCESS)
    {
        // transfer 
        State.ButtonA = (XControllerState.Gamepad.wButtons & XINPUT_GAMEPAD_A) > 0;
        State.ButtonB = (XControllerState.Gamepad.wButtons & XINPUT_GAMEPAD_B) > 0;
        State.ButtonX = (XControllerState.Gamepad.wButtons & XINPUT_GAMEPAD_X) > 0;
        State.ButtonY = (XControllerState.Gamepad.wButtons & XINPUT_GAMEPAD_Y) > 0;
        State.ButtonStart  = (XControllerState.Gamepad.wButtons & XINPUT_GAMEPAD_START) > 0;
        State.ButtonBack  = (XControllerState.Gamepad.wButtons & XINPUT_GAMEPAD_BACK) > 0;
        State.ButtonLeft  = (XControllerState.Gamepad.wButtons & XINPUT_GAMEPAD_DPAD_LEFT) > 0;
        State.ButtonRight = (XControllerState.Gamepad.wButtons & XINPUT_GAMEPAD_DPAD_RIGHT) > 0;
        State.ButtonUp    = (XControllerState.Gamepad.wButtons & XINPUT_GAMEPAD_DPAD_UP) > 0;
        State.ButtonDown  = (XControllerState.Gamepad.wButtons & XINPUT_GAMEPAD_DPAD_DOWN) > 0;
        State.ButtonLeftShoulder  = (XControllerState.Gamepad.wButtons & XINPUT_GAMEPAD_LEFT_SHOULDER) > 0;
        State.ButtonRightShoulder = (XControllerState.Gamepad.wButtons & XINPUT_GAMEPAD_RIGHT_SHOULDER) > 0;
        State.ButtonLeftThumb  = (XControllerState.Gamepad.wButtons & XINPUT_GAMEPAD_LEFT_THUMB) > 0;
        State.ButtonRightThumb = (XControllerState.Gamepad.wButtons & XINPUT_GAMEPAD_RIGHT_THUMB) > 0;

        // ANALOGS
        State.LeftThumbX = XControllerState.Gamepad.sThumbLX;
        State.LeftThumbY = XControllerState.Gamepad.sThumbLY;
        State.RightThumbX = XControllerState.Gamepad.sThumbRX;
        State.RightThumbY = XControllerState.Gamepad.sThumbRY;
        State.LeftTrigger = XControllerState.Gamepad.bLeftTrigger;
        State.RightTrigger = XControllerState.Gamepad.bRightTrigger;
        return true;
    }
    return false;
}

// handle engine events
void RAZOR::INPUT::MsgProc(MICROSOFT::HWND hWnd, UINT Msg, MICROSOFT::WPARAM wParam, MICROSOFT::LPARAM lParam)
{
    
    // process INPUT events
	switch (Msg)
	{
		// process mouse stuff
		case WM_MOUSEMOVE: 
		{ 
			MouseState.Position.X = GetXLParam(lParam);
			MouseState.Position.Y = GetYLParam(lParam);
		} break;

		case WM_MOUSEWHEEL: 
		{ 
            INT Diff = (CHAR)HIWORD(wParam)/120;
			MouseState.Position.S += Diff; 
            if (Events.MouseScrollAny != nullptr) Events.MouseScrollAny();
			     if (Diff ==  1)	{ if (Events.MouseScrollUp   != nullptr) Events.MouseScrollUp();   } // mouse up EVENT 
			else if (Diff == -1)    { if (Events.MouseScrollDown != nullptr) Events.MouseScrollDown(); } // mouse down EVENT
		} break;

		// process left and right mouse buttons up/down
		case WM_RBUTTONDOWN:	MouseState.Buttons.R = true;   if (Events.MouseRightAny  != nullptr)  Events.MouseRightAny();  if (Events.MouseRightDown   != nullptr) Events.MouseRightDown();  break; 
		case WM_RBUTTONUP:		MouseState.Buttons.R = false;  if (Events.MouseRightAny  != nullptr)  Events.MouseRightAny();  if (Events.MouseRightUp     != nullptr) Events.MouseRightUp();    break; 
		case WM_LBUTTONDOWN:	MouseState.Buttons.L = true;   if (Events.MouseLeftAny   != nullptr)  Events.MouseLeftAny();   if (Events.MouseLeftDown    != nullptr) Events.MouseLeftDown();   break; 
		case WM_LBUTTONUP:		MouseState.Buttons.L = false;  if (Events.MouseLeftAny   != nullptr)  Events.MouseLeftAny();   if (Events.MouseLeftUp      != nullptr) Events.MouseLeftUp();     break; 
		case WM_MBUTTONDOWN:	MouseState.Buttons.M = true;   if (Events.MouseMiddleAny != nullptr)  Events.MouseMiddleAny(); if (Events.MouseMiddleDown  != nullptr) Events.MouseMiddleDown(); break; 
		case WM_MBUTTONUP:		MouseState.Buttons.M = false;  if (Events.MouseMiddleAny != nullptr)  Events.MouseMiddleAny(); if (Events.MouseMiddleUp    != nullptr) Events.MouseMiddleUp();   break; 
	
        // special for alt key
        case WM_SYSKEYDOWN:
        case WM_SYSKEYUP:
        {
            BOOLEEN UpOrDown = (Msg == WM_SYSKEYDOWN ? true : false);
            switch (wParam)
            {
                case VK_MENU:  KeyboardState.CSA.Alt                = UpOrDown; break;
	            case VK_SHIFT:  KeyboardState.CSA.Shift             = UpOrDown; break;
                case VK_CONTROL:KeyboardState.CSA.Control           = UpOrDown; break;
			    case VK_UP:     KeyboardState.Arrows.Up             = UpOrDown; break;
                case VK_DOWN:   KeyboardState.Arrows.Down           = UpOrDown; break;
                case VK_LEFT:   KeyboardState.Arrows.Left           = UpOrDown; break;
                case VK_RIGHT:  KeyboardState.Arrows.Right          = UpOrDown; break;
                case VK_SPACE:  KeyboardState.Edit.Space            = UpOrDown; break; 
                case VK_BACK:   KeyboardState.Edit.Backspace        = UpOrDown; break; 
                case VK_ESCAPE: KeyboardState.Edit.Esc              = UpOrDown; break;
                case VK_RETURN: KeyboardState.Edit.Enter            = UpOrDown; break;
                case VK_TAB:    KeyboardState.Edit.Tab              = UpOrDown; break;
                case VK_INSERT: KeyboardState.Navigation.Insert     = UpOrDown; break;
                case VK_DELETE: KeyboardState.Navigation.Delete     = UpOrDown; break;
                case VK_HOME:   KeyboardState.Navigation.Home       = UpOrDown; break;
                case VK_END:    KeyboardState.Navigation.End        = UpOrDown; break;
                case VK_PRIOR:  KeyboardState.Navigation.PageUp     = UpOrDown; break;
                case VK_NEXT:   KeyboardState.Navigation.PageDown   = UpOrDown; break;
                case VK_CAPITAL: KeyboardState.Locks.CapsLock       = UpOrDown; break;
                case VK_SCROLL:  KeyboardState.Locks.ScrollLock     = UpOrDown; break;
                case VK_NUMLOCK: KeyboardState.Locks.NumLock        = UpOrDown; break;

                // for F keys, compute it 
                case VK_F1:  case VK_F2:  case VK_F3:  case VK_F4:  case VK_F5:  case VK_F6:
                case VK_F7:  case VK_F8:  case VK_F9:  case VK_F10: case VK_F11: case VK_F12:
                case VK_F13: case VK_F14: case VK_F15: case VK_F16: case VK_F17: case VK_F18: 
                case VK_F19: case VK_F20: case VK_F21: case VK_F22: case VK_F23: case VK_F24:
                {
                    KeyboardState.FKeys.F[ wParam - VK_F1 ] = UpOrDown;
                    break;
                } 
                default: break;
            }

            // do callbacks
            if(Events.KeyboardAny != nullptr)
            {
                Events.KeyboardAny(wParam, UpOrDown);
            }

            break;
        }

        // key up
        case WM_KEYDOWN:
		case WM_KEYUP:
        {
            BOOLEEN UpOrDown = (Msg == WM_KEYDOWN ? true : false);
		    switch (wParam) 
		    { 
                case VK_MENU:  KeyboardState.CSA.Alt                = UpOrDown; break;
	            case VK_SHIFT:  KeyboardState.CSA.Shift             = UpOrDown; break;
                case VK_CONTROL:KeyboardState.CSA.Control           = UpOrDown; break;
			    case VK_UP:     KeyboardState.Arrows.Up             = UpOrDown; break;
                case VK_DOWN:   KeyboardState.Arrows.Down           = UpOrDown; break;
                case VK_LEFT:   KeyboardState.Arrows.Left           = UpOrDown; break;
                case VK_RIGHT:  KeyboardState.Arrows.Right          = UpOrDown; break;
                case VK_SPACE:  KeyboardState.Edit.Space            = UpOrDown; break; 
                case VK_BACK:   KeyboardState.Edit.Backspace        = UpOrDown; break; 
                case VK_ESCAPE: KeyboardState.Edit.Esc              = UpOrDown; break;
                case VK_RETURN: KeyboardState.Edit.Enter            = UpOrDown; break;
                case VK_TAB:    KeyboardState.Edit.Tab              = UpOrDown; break;
                case VK_INSERT: KeyboardState.Navigation.Insert     = UpOrDown; break;
                case VK_DELETE: KeyboardState.Navigation.Delete     = UpOrDown; break;
                case VK_HOME:   KeyboardState.Navigation.Home       = UpOrDown; break;
                case VK_END:    KeyboardState.Navigation.End        = UpOrDown; break;
                case VK_PRIOR:  KeyboardState.Navigation.PageUp     = UpOrDown; break;
                case VK_NEXT:   KeyboardState.Navigation.PageDown   = UpOrDown; break;
                case VK_CAPITAL: KeyboardState.Locks.CapsLock       = UpOrDown; break;
                case VK_SCROLL:  KeyboardState.Locks.ScrollLock     = UpOrDown; break;
                case VK_NUMLOCK: KeyboardState.Locks.NumLock        = UpOrDown; break;

                // for F keys, compute it 
                case VK_F1:  case VK_F2:  case VK_F3:  case VK_F4:  case VK_F5:  case VK_F6:
                case VK_F7:  case VK_F8:  case VK_F9:  case VK_F10: case VK_F11: case VK_F12:
                case VK_F13: case VK_F14: case VK_F15: case VK_F16: case VK_F17: case VK_F18: 
                case VK_F19: case VK_F20: case VK_F21: case VK_F22: case VK_F23: case VK_F24:
                {
                    KeyboardState.FKeys.F[ wParam - VK_F1 ] = UpOrDown;
                    break;
                } 
                default: break;
            }

            // do callbacks
            if(Events.KeyboardAny != nullptr)
            {
                Events.KeyboardAny(wParam, UpOrDown);
            }
            break;
        }

	    case WM_CHAR: // wParam has the keycode // add to the keyboardbuffer!
	    {
		    //ParentEngine->Console->Log(DEBUG_WARN, "pressed = %d\n", wParam);
		    //AddToKeyboardBuffer(wParam);
		    //EngineXEmulator.alltextbox[EngineXEmulator.topTextBox]->TickerLastPosition = EngineX.TickModCount;
            if(Events.KeyboardAny != nullptr)
            {
                //Events.KeyboardAny(wParam, true);
            }
            break;
	    } 
        default: break;
    }
	

}


// mouse input stuff
RAZOR::INPUT_MOUSE_STATE                    RAZOR::INPUT::GetMouseState()       {   return MouseState;             }
RAZOR::INPUT_MOUSE_STATE::MOUSE_POSITION    RAZOR::INPUT::GetMousePosition()    {   return MouseState.Position;    }
RAZOR::INPUT_MOUSE_STATE::MOUSE_BUTTONS     RAZOR::INPUT::GetMouseButtons()     {   return MouseState.Buttons;     }

// mouse input stuff
INT32 RAZOR::INPUT::GetMouseX() { return MouseState.Position.X; }
INT32 RAZOR::INPUT::GetMouseY() { return MouseState.Position.Y; }
INT32 RAZOR::INPUT::GetMouseS() { return MouseState.Position.S; }
INT32 RAZOR::INPUT::GetMouseM() { return MouseState.Buttons.M; }
INT32 RAZOR::INPUT::GetMouseL() { return MouseState.Buttons.L; }
INT32 RAZOR::INPUT::GetMouseR() { return MouseState.Buttons.R; }

// keyboard input stuff
RAZOR::INPUT_KEYBOARD_STATE                           RAZOR::INPUT::GetKeyboardState()    { return KeyboardState; }
RAZOR::INPUT_KEYBOARD_STATE::STATE_CONTROL_SHIFT_ALT  RAZOR::INPUT::GetKeyboardStateCSA() { return KeyboardState.CSA; }





