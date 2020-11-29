
// header gaurd
#ifndef _RAZOR_ENGINE_DIRECTX_H
#define _RAZOR_ENGINE_DIRECTX_H

// if in debug mode, enable some specials
#if defined(DEBUG)|defined(_DEBUG)
    #define D3D_DEBUG_INFO
    #pragma comment(lib,"d3dx9d.lib")
#else
    #pragma comment(lib,"d3dx9.lib")
#endif

// main directx
#pragma comment(lib, "dxerr.lib")
#pragma comment(lib, "d3d9.lib")
#pragma comment(lib, "dxguid.lib")
#pragma comment(lib, "dsound.lib")

// directx related
#include <d3d9.h>
#include <D3dx9core.h>
#include <DxErr.h>

// headers
#include "Engine.h"


struct CUSTOMVERTEX
{
	FLOAT x,y,z;
    DWORD color;
};


// directx object, plugs into the engine object
class RAZOR::DIRECTX
{
    public:
    RAZOR::ENGINECORE *ParentEngine;

	// setup variables
	DIRECTX9::LPDIRECT3D9                   D3D;	        // Used to create the D3DDevice
	DIRECTX9::LPDIRECT3DDEVICE9             Device;	        // Our rendering device
	DIRECTX9::D3DDISPLAYMODE                DisplayMode;    // holds display mode info
	DIRECTX9::D3DPRESENT_PARAMETERS         PresParameters; // Holds rendering preferences 
    DIRECTX9::D3DFORMAT                     DisplayFormat;  // holds adapter display format
    DIRECTX9::LPDIRECT3DVERTEXBUFFER9       VertexBuffer;
    DIRECTX9::LPDIRECT3DSWAPCHAIN9          SwapChain;

	// object targets
	DIRECTX9::LPDIRECT3DSURFACE9            DrawSurface;    // neede internally to draw on
	DIRECTX9::LPD3DXFONT                    DefaultFont;    // just a regular font for display
	DIRECTX9::LPD3DXSPRITE                  Sprite;         // MASTER SPRITE... only need 1
	DIRECTX9::LPD3DXLINE				    Line;	        // MASTER LINE... only neeed 1
    std::atomic<MICROSOFT::HRESULT>        LastCooperativeLevel;

    // constructor
    DIRECTX(RAZOR::ENGINECORE *MyParent);
    ~DIRECTX();
    UINT32 Initialize(RAZOR::WINDOW &TargetWindow);

    // methods
    UINT32 Render(VOID_FUNCTION_POINTER ApplicationDrawCallback);
    UINT32 MessagePump();
    UINT32 InvalidateDeviceObjects();
    UINT32 RestoreDeviceObjects();
    UINT32 SetSamplerStates();
    UINT32 SetRenderStates();
    UINT32 SetRenderStates(UINT32 Type, UINT32 Value);

};


#endif