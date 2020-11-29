
// header gaurd
#ifndef _RAZOR_ENGINE_INPUT_H
#define _RAZOR_ENGINE_INPUT_H

// headers
#include "Engine.h"

// deps
#include <Windows.h>
#include <XInput.h>
#include <dinput.h>
#include <wbemidl.h>
#include <dinput.h>
#include <xaudio2.h>

// linker
#pragma comment(lib, "dinput8.lib")
#pragma comment(lib, "xinput.lib")


// holds all of the "input" related information, including callbacks
class RAZOR::INPUT
{
    public:
    RAZOR::ENGINECORE *ParentEngine;
    RAZOR::INPUT_CALLBACKS Events;
    INPUT_MOUSE_STATE       MouseState;    // mouse input variables
    INPUT_KEYBOARD_STATE    KeyboardState; // keyboard variables
    private:

    // xbox controller vars
    XINPUT_STATE XControllerState;
    INT32 XControllerNum = 0;
    
    public:
    // constructor
    INPUT(RAZOR::ENGINECORE *MyParent);
    ~INPUT();
    UINT32 Initialize(RAZOR::INPUT_CALLBACKS MyEvents);

    // methods
    void MsgProc(MICROSOFT::HWND hWnd, UINT Msg, MICROSOFT::WPARAM wParam, MICROSOFT::LPARAM lParam);

    // get states
    RAZOR::INPUT_MOUSE_STATE                 GetMouseState();
    RAZOR::INPUT_MOUSE_STATE::MOUSE_POSITION GetMousePosition();
    RAZOR::INPUT_MOUSE_STATE::MOUSE_BUTTONS  GetMouseButtons();
    INT32 GetMouseX();
    INT32 GetMouseY();
    INT32 GetMouseS();
    INT32 GetMouseM();
    INT32 GetMouseL();
    INT32 GetMouseR();

    // keyboard states
    RAZOR::INPUT_KEYBOARD_STATE                           GetKeyboardState();
    RAZOR::INPUT_KEYBOARD_STATE::STATE_CONTROL_SHIFT_ALT  GetKeyboardStateCSA();
    
    // xbox controllers
    BOOLEEN GetXboxState(UINT32 Index, XBOX_CONTROLLER_STATE &State);
  
};



#endif