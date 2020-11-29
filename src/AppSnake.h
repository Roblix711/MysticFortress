
// header gaurd
#ifndef _APP_SNAKE_H
#define _APP_SNAKE_H

// main engine include
#include <RazorDraw.hpp>
#include <functional>


// main interface class static + singleton
class APP_SNAKE
{
    // statics
    public:
    static RAZOR::ENGINE *RazorEngine;

    // init method
    static void Start(HINSTANCE hInstance, RAZOR::WINDOWS_CALLBACK WndProc);       
    static RAZOR::ENGINE* GetRazorEngine() { return RazorEngine; }

    // imported prototypes
    static void dprintf(const CHAR *message, ...);

    // for engine
    static void Draw();
    static void ResDest();
    static void ResInit();
    static void OnLost();
    static void OnReset();
    static void Tick();
    static void Frame();


    // input callbacks
    static void OnDragDrop(const VECTOR<STRING> &Paths);
    static void GlobalKeyboardAny(UINT32 KeyCode, BOOLEEN State);
    static void MouseLeftUp();
    static void MouseLeftDown();
    static void MouseMiddle();
    static void MouseRightUp();
    static void MouseRightDown();
    static void GlobalScrollDown();
    static void GlobalScrollUp();  

};

#endif // _APP_SNAKE_H
