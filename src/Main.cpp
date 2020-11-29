
// our main header
#include "Main.h"
#include "AppSnake.h"
#include <ctime>

// event processor
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    // pass call onwards
    APP_SNAKE::GetRazorEngine()->EngineMsgProc(hWnd, message, wParam, lParam);

    // main switch
    switch (message)
    {
        case WM_DESTROY:
        {
            APP_SNAKE::GetRazorEngine()->EngineStop();
            PostQuitMessage(0);
            break;
        }
        default: break; 
    }

    // disable alt / F10 processing
    if (wParam == SC_KEYMENU && (lParam>>16) <= 0 ) return 0;

    // return default
    return DefWindowProc(hWnd, message, wParam, lParam);
}

// main functon
int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE, LPTSTR, int)
{
    // start application
    srand(time(0));
    APP_SNAKE::Start(hInstance, WndProc);

    // close
    return 0;
}