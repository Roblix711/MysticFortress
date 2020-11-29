
// main header
#include "DirectX.h"

// backbuffer count
#define RAZOR_DIRECTX9_BACKBUFFER_COUNT  2

// constructor
RAZOR::DIRECTX::DIRECTX(RAZOR::ENGINECORE *MyParent): ParentEngine(MyParent), LastCooperativeLevel(D3D_OK),
    D3D(nullptr), Device(nullptr), DrawSurface(nullptr), DefaultFont(nullptr), Sprite(nullptr), Line(nullptr), SwapChain(nullptr), VertexBuffer(nullptr)
{
    // zero memory then put some parameters in
	MICROSOFT::ZeroMemory(&DisplayMode,    sizeof(D3DDISPLAYMODE));
	MICROSOFT::ZeroMemory(&PresParameters, sizeof(D3DPRESENT_PARAMETERS));
	MICROSOFT::ZeroMemory(&DisplayFormat,  sizeof(D3DFORMAT));

	// Create the D3D object, which is needed to create the D3DDevice.
	D3D = DIRECTX9::Direct3DCreate9(D3D_SDK_VERSION);
	if (D3D == nullptr) 
    {
        ParentEngine->Console->Log(DEBUG_FATAL, "RAZOR::DIRECTX::DIRECTX() Direct3DCreate9() Failed: Unable to initialize DX9 object");
        throw EXCEPTION_RUNTIME("RAZOR::DIRECTX::DIRECTX() Direct3DCreate9() Failed: Unable to initialize DX9 object");
    }
        
	// Get the current desktop display mode
	if (FAILED( D3D->GetAdapterDisplayMode(D3DADAPTER_DEFAULT, &DisplayMode))) 
    {
        ParentEngine->Console->Log(DEBUG_FATAL, "RAZOR::DIRECTX::DIRECTX() GetAdapterDisplayMode() Failed: Unable to get display modes");
        throw EXCEPTION_RUNTIME("RAZOR::DIRECTX::DIRECTX() GetAdapterDisplayMode() Failed: Unable to get display modes");
    }
	DisplayFormat = D3DFMT_A8R8G8B8; // DisplayMode.Format; // most likely D3DFMT_A8R8G8B8

}

// destructor releases
RAZOR::DIRECTX::~DIRECTX()
{
    // release
    //SwapChain->Release();
    DefaultFont->Release();
    Sprite->Release();
    Line->Release();
    Device->Release();
}

// initializes the directx
UINT32 RAZOR::DIRECTX::Initialize(RAZOR::WINDOW &TargetWindow)
{
    // get the buffer size new!
    RECT MaxUnionSize = ParentEngine->AllMonitors.GetLargestUnionSize();

	// Create D3D Device, try software
    PresParameters.Windowed         = TargetWindow.WindowParams.bWindowed;
    PresParameters.SwapEffect       = D3DSWAPEFFECT_COPY;
	PresParameters.hDeviceWindow    = TargetWindow.WindowHandle;
	PresParameters.BackBufferFormat = DisplayFormat;
	PresParameters.BackBufferCount  = 1;//RAZOR_DIRECTX9_BACKBUFFER_COUNT; 
	PresParameters.BackBufferWidth  = MaxUnionSize.right; //TargetWindow.WindowParams.ScreenWidth;
	PresParameters.BackBufferHeight = MaxUnionSize.bottom; //TargetWindow.WindowParams.ScreenHeight;
    PresParameters.EnableAutoDepthStencil = TRUE;
    PresParameters.AutoDepthStencilFormat = D3DFMT_D16;


    // try anti aliasing
    MICROSOFT::HRESULT MultiTest = D3D->CheckDeviceMultiSampleType( D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, D3DFMT_A8R8G8B8, TRUE, D3DMULTISAMPLE_NONMASKABLE, NULL );
    if( SUCCEEDED(MultiTest) )
    {
	    PresParameters.MultiSampleType = D3DMULTISAMPLE_NONMASKABLE;
	    PresParameters.MultiSampleQuality = 7;
        goto below;
    }
    else
    {
        below:
	    PresParameters.MultiSampleType = D3DMULTISAMPLE_NONE;
	    PresParameters.MultiSampleQuality = 0;
    }

    // buffer type & vsync option
	PresParameters.Flags = D3DPRESENTFLAG_VIDEO | D3DPRESENTFLAG_LOCKABLE_BACKBUFFER; // We want to be able to directly write to backbuffer memory
	if (ParentEngine->Window->WindowParams.bVerticalSync == true)	PresParameters.PresentationInterval = D3DPRESENT_INTERVAL_ONE;
	else				                                            PresParameters.PresentationInterval = D3DPRESENT_INTERVAL_IMMEDIATE;

    // after presentation preferences, do the device
    HRESULT Result1 = D3D->CreateDevice(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, TargetWindow.WindowHandle, D3DCREATE_HARDWARE_VERTEXPROCESSING | D3DCREATE_MULTITHREADED, &PresParameters, &Device);
	if (FAILED(Result1)) 
	{
        // try windowed
		PresParameters.Windowed = true;
		if (FAILED(D3D->CreateDevice(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, TargetWindow.WindowHandle, D3DCREATE_HARDWARE_VERTEXPROCESSING | D3DCREATE_MULTITHREADED, &PresParameters, &Device))) 
        {
            // try software vertex processing
		    if (FAILED(D3D->CreateDevice(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, TargetWindow.WindowHandle, D3DCREATE_SOFTWARE_VERTEXPROCESSING | D3DCREATE_MULTITHREADED, &PresParameters, &Device))) 
            {
                if (FAILED(D3D->CreateDevice(D3DADAPTER_DEFAULT, D3DDEVTYPE_SW, TargetWindow.WindowHandle, D3DCREATE_SOFTWARE_VERTEXPROCESSING | D3DCREATE_MULTITHREADED, &PresParameters, &Device))) 
			        throw EXCEPTION_RUNTIME("RAZOR::DIRECTX::DIRECTX() CreateDevice() Failed: Full/Windowed, Software/Hardware Vertex");
            }
        }
	}
    TargetWindow.SetClientDims(TargetWindow.WindowParams.ScreenWidth, TargetWindow.WindowParams.ScreenHeight);

    // create line, sprite and font
	HRESULT ResultFont   = DIRECTX9::D3DXCreateFont(Device, 12, 0, FW_NORMAL, 0, FALSE, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLEARTYPE_QUALITY, FIXED_PITCH | FF_MODERN, TEXT("FixedSys"),  &DefaultFont);
	HRESULT ResultSprite = DIRECTX9::D3DXCreateSprite(Device, &Sprite);
	HRESULT ResultLine   = DIRECTX9::D3DXCreateLine(Device, &Line);
    //HRESULT ResultSwap   = Device->CreateAdditionalSwapChain(&PresParameters, &SwapChain); // don't need
    
    // other resources / states
    Device->CreateVertexBuffer(5*sizeof(CUSTOMVERTEX), D3DUSAGE_DYNAMIC, D3DFVF_XYZ | D3DFVF_DIFFUSE, D3DPOOL_DEFAULT, &VertexBuffer, NULL);
    SetRenderStates();
    SetSamplerStates();

    // check
    if (ResultFont   != D3D_OK) ParentEngine->Console->Log(DEBUG_FATAL, "RAZOR::DIRECTX::Initialize(): D3DXCreateFont() Failed ret = %d\n", ResultFont);
    if (ResultSprite != D3D_OK) ParentEngine->Console->Log(DEBUG_FATAL, "RAZOR::DIRECTX::Initialize(): D3DXCreateSprite() Failed ret = %d\n", ResultSprite);
    if (ResultLine   != D3D_OK) ParentEngine->Console->Log(DEBUG_FATAL, "RAZOR::DIRECTX::Initialize(): D3DXCreateLine() Failed ret = %d\n", ResultLine);
    //if (ResultSwap   != D3D_OK) ParentEngine->Console->Log(DEBUG_FATAL, "RAZOR::DIRECTX::Initialize(): CreateAdditionalSwapChain() Failed ret = %d\n", ResultSwap);

    // okay done
    return 0;

}

// for custom render states
UINT32 RAZOR::DIRECTX::SetRenderStates(UINT32 Type, UINT32 Value)
{
    ParentEngine->Draw->SetDrawType(DRAW_NONE);
    HRESULT Res = Device->SetRenderState( D3DRENDERSTATETYPE(Type), Value);
    return Res;
}


// restore samplerstates
UINT32 RAZOR::DIRECTX::SetSamplerStates()
{
    DIRECTX9::D3DCAPS9 TestPrimitive; 
    Device->GetDeviceCaps(&TestPrimitive);
    Device->SetSamplerState(0, D3DSAMP_MAXANISOTROPY, TestPrimitive.MaxAnisotropy);
    Device->SetSamplerState(1, D3DSAMP_MAXANISOTROPY, TestPrimitive.MaxAnisotropy);
    Device->SetSamplerState(2, D3DSAMP_MAXANISOTROPY, TestPrimitive.MaxAnisotropy);
    Device->SetSamplerState(0, D3DSAMP_MINFILTER, D3DTEXF_ANISOTROPIC);
    Device->SetSamplerState(0, D3DSAMP_MAGFILTER, D3DTEXF_POINT);
    Device->SetSamplerState(0, D3DSAMP_MIPFILTER, D3DTEXF_LINEAR );
    Device->SetSamplerState(1, D3DSAMP_MINFILTER, D3DTEXF_ANISOTROPIC);
    Device->SetSamplerState(1, D3DSAMP_MAGFILTER, D3DTEXF_POINT);
    Device->SetSamplerState(1, D3DSAMP_MIPFILTER, D3DTEXF_LINEAR);
    Device->SetSamplerState(2, D3DSAMP_MINFILTER, D3DTEXF_ANISOTROPIC);
    Device->SetSamplerState(2, D3DSAMP_MAGFILTER, D3DTEXF_POINT );
    Device->SetSamplerState(2, D3DSAMP_MIPFILTER, D3DTEXF_LINEAR);
    return 0;
}

// restore renderstates
UINT32 RAZOR::DIRECTX::SetRenderStates()
{
	// filtering stuff
    ParentEngine->Draw->SetDrawType(DRAW_NONE);
    Device->SetRenderState(D3DRS_ZENABLE, FALSE );
    Device->SetRenderState(D3DRS_LIGHTING, FALSE );
    Device->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);
    Device->SetRenderState(D3DRS_SEPARATEALPHABLENDENABLE, TRUE );
    Device->SetRenderState(D3DRS_FILLMODE, D3DFILL_SOLID);
    Device->SetRenderState(D3DRS_BLENDOP, D3DBLENDOP_ADD);
    Device->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);
    Device->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
    Device->SetRenderState(D3DRS_DIFFUSEMATERIALSOURCE, D3DMCS_COLOR1 );
    Device->SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE); // allow backfacing

    // validate:
    /*static BOOLEEN Once = false;
    if (Once == false) // omit opening bad
    {
        Once = true;
        DWORD Passes = 1;
        HRESULT Valid = Device->ValidateDevice(&Passes);
        if (Valid != D3D_OK) ParentEngine->Console->Log(DEBUG_WARN, "RAZOR::DIRECTX::SetRenderStates(): Call to ValidateDevice() failed (0x%x != D3D_OK) %s -> %s\n", Valid, DXGetErrorString(Valid), DXGetErrorDescription(Valid));
    }*/

    // set the vertex buffer
    return 0;
}

// destroys surfaces and stuffs during a reset 
UINT32 RAZOR::DIRECTX::InvalidateDeviceObjects()
{
    // call the appropriate onlost
    if (ParentEngine->FunctionLinks.OnLostFunc != nullptr) 
        ParentEngine->FunctionLinks.OnLostFunc();
    ParentEngine->Resources->OnLostDevice();

    //if (DirectX9->DrawSurface  != NULL) DirectX9->DrawSurface->Release();	     // DON'T PUT THIS HERE.... ENGINEX 2016 RL 
    if (VertexBuffer != nullptr) VertexBuffer->Release();
    if (DefaultFont  != nullptr) DefaultFont->OnLostDevice();   
    if (Sprite       != nullptr) Sprite->OnLostDevice();		
    if (Line         != nullptr) Line->OnLostDevice();		

    return 0;
}

// remake resouces
UINT32 RAZOR::DIRECTX::RestoreDeviceObjects()
{
    // recreate this
    SetRenderStates();
    SetSamplerStates();
    Device->CreateVertexBuffer(5*sizeof(CUSTOMVERTEX), D3DUSAGE_DYNAMIC, D3DFVF_XYZ | D3DFVF_DIFFUSE, D3DPOOL_DEFAULT, &VertexBuffer, NULL);

    // these managed ones have auto reloads
    DefaultFont->OnResetDevice();   
    Sprite->OnResetDevice();
    Line->OnResetDevice();
    ParentEngine->Resources->OnResetDevice();
    if (ParentEngine->FunctionLinks.OnResetFunc != nullptr) 
        ParentEngine->FunctionLinks.OnResetFunc();

    return 0;
}

// okay this is the main loop from message pump -> must be same thread as message pump!
UINT32 RAZOR::DIRECTX::MessagePump()
{
    // set shared var
    /*LastCooperativeLevel = Device->TestCooperativeLevel();
    if (LastCooperativeLevel != D3D_OK)
    {
        DebugBreak();
    }*/

    // resets -> must be in the thread that created DX device
    if (ParentEngine->bResetting == true)
    {
        // stop all tasks
        ParentEngine->Scheduler->StopAll();

        // call to destroy 
        InvalidateDeviceObjects();

        UINT32 ResetCounter = 0;
	    TryAgain:  	
	    MICROSOFT::HRESULT HR = Device->Reset(&PresParameters);
	    if( HR != D3D_OK )
	    {
            ParentEngine->Console->Log(DEBUG_FATAL, "RAZOR::DIRECTX::MessagePump(): Call to Reset() failed (ret != D3D_OK) [%dx%d, %d] (0x%x != 0x%x): %s -> %s\n", 
		        PresParameters.BackBufferWidth, 
                PresParameters.BackBufferHeight, 
                PresParameters.Windowed, HR, S_OK, DXGetErrorString(HR), DXGetErrorDescription(HR));
		    ResetCounter++;
            if (ResetCounter > 20) 
            {
                ResetCounter = 0;
                //INT Result = MessageBox(NULL, "FATAL: Cannot reset device!", "Reset Failed", MB_OKCANCEL);
                //if (Result == 0) 
                    return 1;
            }
		    Sleep(10);
		    goto TryAgain;
	    }
	    else			
	    {
		    ParentEngine->Console->Log(DEBUG_INFO, "RAZOR::DIRECTX::MessagePump(): Device Properly Reset to [%dx%d, %d] \n",
                PresParameters.BackBufferWidth, 
                PresParameters.BackBufferHeight, 
                PresParameters.Windowed);
		    ParentEngine->Draw->ClearScreen(0);
            ParentEngine->Window->SetClientDims(PresParameters.BackBufferWidth, PresParameters.BackBufferHeight);
            RestoreDeviceObjects();
            ParentEngine->bResetting = false;
	    }

        // start all tasks
        ParentEngine->Scheduler->StartAll();

    }
    return 0;
}


// must be called by scheduler only
UINT32 RAZOR::DIRECTX::Render(VOID_FUNCTION_POINTER ApplicationDrawCallback)
{
    MICROSOFT::HRESULT hr = Device->TestCooperativeLevel();//LastCooperativeLevel; 
	if(hr == D3DERR_DEVICELOST)
	{
		ParentEngine->Console->Log(DEBUG_INFO, "RAZOR::DIRECTX::Render(): Device Lost\n");
		Sleep(100); // just wait
		return 1;
	}
	else if (hr == D3DERR_DEVICENOTRESET)
	{
        ParentEngine->Console->Log(DEBUG_INFO, "RAZOR::DIRECTX::Render(): Device not Reset\n");
		// if we didnt do a reset, but the device needs one anyways, force it
		if (ParentEngine->bResetting == false)
		{
			ParentEngine->bResetting = true;
		}
        Sleep(100); // just wait
        return 1;
	}
	else if (hr == D3D_OK) // ITS GOOD
	{
	    // start device scene
        ParentEngine->FPSTimer.Tock();
        Device->BeginScene();

	    // Get backbuffer and render to it
        static UINT32 BufferIndex = 0;
	    if (SUCCEEDED(Device->GetBackBuffer(0, 0, D3DBACKBUFFER_TYPE_MONO, &DrawSurface)))
	    {
            // start with none
            if (ParentEngine->Draw->LastDrawType != DRAW_NONE)
            ParentEngine->Console->Log(DEBUG_WARN, "RAZOR::DIRECTX::Render(): Detected rendering functions outside of Draw()!!\n");
            ParentEngine->Draw->SetDrawType(DRAW_NONE);

		    //DrawCount = 0; // reset for us
		    if (ApplicationDrawCallback != NULL) ApplicationDrawCallback();         // our procedure!

		    // release all
		    ParentEngine->Draw->SetDrawType(DRAW_NONE);

		    // always returns the ref count
		    DrawSurface->Release(); 

	        // close scene
            BufferIndex++;
	        Device->EndScene();
	    }
	    else
	    {
            ParentEngine->Console->Log(DEBUG_FATAL, "RAZOR::DIRECTX::Render(): Could not get the backbuffer!\n");
	    }

		// Present the backbuffer contents to the display
        const RECT SrcRect = {0, 0, ParentEngine->Window->GetScreenWidth(),ParentEngine->Window->GetScreenHeight()};
		HRESULT Pr = Device->Present(&SrcRect, &SrcRect, NULL, NULL);
        if (Pr != D3D_OK)
        {
            ParentEngine->Console->Log(DEBUG_WARN, "RAZOR::DIRECTX::Render(): Present Failed!! HR 0x%x != D3D_OK, Message = %s -> %s\n", Pr, DXGetErrorString(Pr), DXGetErrorDescription(Pr));
        }
    }
    else // some other return? 
    {
        ParentEngine->Console->Log(DEBUG_FATAL, "RAZOR::DIRECTX::Render(): TestCooperativeLevel() returned something else %d!\n", hr);
    }



    // meh
    return 0;
}