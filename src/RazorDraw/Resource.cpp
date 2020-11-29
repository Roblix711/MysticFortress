
// main header
#include "Resource.h"

// protected
namespace 
{
    using RAZOR::RAZOR_COLOR;

    // just a crap function to fill texture in with white only.....
    VOID WINAPI WhiteColorFill (D3DXVECTOR4* pOut, const D3DXVECTOR2* pTexCoord, const D3DXVECTOR2* pTexelSize, LPVOID pData)
    {
	    UNREFERENCED_PARAMETER(pData);
	    UNREFERENCED_PARAMETER(pTexelSize);
	    UNREFERENCED_PARAMETER(pTexCoord);
        *pOut = D3DXVECTOR4(255, 255, 255, 255);
    }
}

// constructor
RAZOR::RESOURCE::RESOURCE(RAZOR::ENGINECORE *MyParent) : ParentEngine(MyParent)
{
    // allocate a bit
    VideoArray.reserve(32);
    BitmapArray.reserve(32);
}

// CLEANUP
RAZOR::RESOURCE::~RESOURCE()
{
    // kill 
    for(UINT32 i=0; i<BitmapArray.size(); ++i)
    {
        BitmapArray[i].VidBmp->Release();
        BitmapArray[i].SysBmp->Release();
    }

    for(UINT32 i=0; i<VideoArray.size(); ++i)
    {
        delete VideoArray[i];
    }

}

// makes a new video resource
UINT32 RAZOR::RESOURCE::CreateVideo(UINT32 Width, UINT32 Height, UINT32 Frames)
{
    // threadsafe
    NOVA::SCOPE_LOCK AutoLock(AddResourceMutex);

    // make the DX9 resource
    UINT32 SystemMemoryErrors = 0;
    UINT32 NewIndex = UINT32(-1);
    RESOURCE_VIDEO *NewResource = new RESOURCE_VIDEO;
	MICROSOFT::HRESULT hr = ParentEngine->DirectX9->Device->CreateOffscreenPlainSurface(Width+1, Height+1, D3DFMT_A8R8G8B8, D3DPOOL_DEFAULT, &NewResource->DataVid, NULL);
    if (!FAILED(hr)) 
    {
        // make multiple surfaces
        NewResource->Data.resize(Frames);
        for(UINT32 t=0; t<Frames; ++t)
        {
            // attempt each surface one by one
			MICROSOFT::HRESULT hr = ParentEngine->DirectX9->Device->CreateOffscreenPlainSurface(Width+1, Height+1, D3DFMT_A8R8G8B8, D3DPOOL_SYSTEMMEM, &NewResource->Data[t], NULL);
            if (FAILED(hr)) 
            {
                // undo the created crap
                for(UINT32 K=0; K<t; ++K)
                    NewResource->Data[t]->Release();
                SystemMemoryErrors++;
                break;   
            }
        }

        // if we get here, that means theres no errors
        if (SystemMemoryErrors == 0)
        {
            // configure the rest of the struct
		    NewResource->bIsValid = true; // set true when we change its data.
            NewResource->bIsLocked = true; // don't draw yet
		    NewResource->Height = Height;
		    NewResource->Width  = Width;
		    NewResource->Frames = Frames;
            //NewResource.LockRect.resize(Frames); // resize occurs ON Lock()

            // add to the total list
            VideoArray.push_back(NewResource);
            NewIndex                   = VideoArray.size()-1;
            VideoArray[NewIndex]->Index = NewIndex; 
        }
        else
        {
            // if we have errors, destroy the video memory one
            NewResource->DataVid->Release();
            ParentEngine->Console->Log(DEBUG_FATAL, "RAZOR::RESOURCE::CreateVideo() Failed CreateOffscreenPlainSurface(): D3DPOOL_SYSTEMMEM -> MICROSOFT::HRESULT hr bad = %d\n", hr);
        }
    }
    else
    {
        // log it
        ParentEngine->Console->Log(DEBUG_FATAL, "RAZOR::RESOURCE::CreateVideo() Failed CreateOffscreenPlainSurface(): D3DPOOL_DEFAULT -> MICROSOFT::HRESULT hr bad = %d\n", hr);
        // SystemMemoryErrors++; // implicit 
    }


    // return the index, or -1 for error
    return NewIndex;
}


// makes a bitmap inplace
void RAZOR::RESOURCE::CreateBitmap(RESOURCE_BITMAP &NewBitmap, UINT32 Width, UINT32 Height, BOOLEEN bFill)
{	
    // threadsafe
    NOVA::SCOPE_LOCK AutoLock(AddResourceMutex);

	// set to null
	MICROSOFT::HRESULT res1 = S_OK, res2 = S_OK;
	res2 = ParentEngine->DirectX9->Device->CreateTexture(Width, Height, 1, D3DUSAGE_DYNAMIC, D3DFMT_A8R8G8B8, D3DPOOL_SYSTEMMEM, &NewBitmap.SysBmp, NULL);
	res1 = ParentEngine->DirectX9->Device->CreateTexture(Width, Height, 1, D3DUSAGE_DYNAMIC, D3DFMT_A8R8G8B8, D3DPOOL_DEFAULT,   &NewBitmap.VidBmp, NULL);

    // check result codes
	if (res1 == S_OK && res2 == S_OK)
	{
        // fill out the rest of the struct
		NewBitmap.Width  = Width;
		NewBitmap.Height = Height;
		NewBitmap.State = RESOURCE_STATE_INITIALIZED;
		NewBitmap.strFileName.clear();
        
        // check fill params
		if (bFill == true)                           
		{
			//if (target->SysBmp != NULL) 	
			D3DXFillTexture(NewBitmap.SysBmp, WhiteColorFill, NULL);
			D3DXFillTexture(NewBitmap.VidBmp, WhiteColorFill, NULL);
		}

	}
    else // release partner
    {
        if (res2 == S_OK ) { NewBitmap.SysBmp->Release(); ParentEngine->Console->Log(DEBUG_FATAL, "RAZOR::RESOURCE::CreateBitmap() Failed CreateTexture(): D3DPOOL_DEFAULT   -> MICROSOFT::HRESULT hr bad = %d\n", res1); }
        if (res1 == S_OK ) { NewBitmap.VidBmp->Release(); ParentEngine->Console->Log(DEBUG_FATAL, "RAZOR::RESOURCE::CreateBitmap() Failed CreateTexture(): D3DPOOL_SYSTEMMEM -> MICROSOFT::HRESULT hr bad = %d\n", res2); }
    }

}

// makes a bitmap 
UINT32 RAZOR::RESOURCE::CreateBitmap(UINT32 Width, UINT32 Height, BOOLEEN bFill)
{	
    // threadsafe
    NOVA::SCOPE_LOCK AutoLock(AddResourceMutex);

    // make the DX9 resource
    RESOURCE_BITMAP NewBitmap;
    UINT32 RetIndex = -1;

	// set to null
	MICROSOFT::HRESULT res1 = S_OK, res2 = S_OK;
	res2 = ParentEngine->DirectX9->Device->CreateTexture(Width, Height, 1, D3DUSAGE_DYNAMIC, D3DFMT_A8R8G8B8, D3DPOOL_SYSTEMMEM, &NewBitmap.SysBmp, NULL);
	res1 = ParentEngine->DirectX9->Device->CreateTexture(Width, Height, 1, D3DUSAGE_DYNAMIC, D3DFMT_A8R8G8B8, D3DPOOL_DEFAULT,   &NewBitmap.VidBmp, NULL);

    // check result codes
	if (res1 == S_OK && res2 == S_OK)
	{
        // fill out the rest of the struct
		NewBitmap.Width  = Width;
		NewBitmap.Height = Height;
		NewBitmap.State = RESOURCE_STATE_INITIALIZED;
		NewBitmap.strFileName.clear();
        
        // check fill params
		if (bFill == true)                           
		{
			//if (target->SysBmp != NULL) 	
			D3DXFillTexture(NewBitmap.SysBmp, WhiteColorFill, NULL);
			D3DXFillTexture(NewBitmap.VidBmp, WhiteColorFill, NULL);
		}

        // add to resources
        RetIndex = BitmapArray.size();
        BitmapArray.push_back(NewBitmap);
	}
    else // release partner
    {
        if (res2 == S_OK ) { NewBitmap.SysBmp->Release(); ParentEngine->Console->Log(DEBUG_FATAL, "RAZOR::RESOURCE::CreateBitmap() Failed CreateTexture(): D3DPOOL_DEFAULT   -> MICROSOFT::HRESULT hr bad = %d\n", res1); }
        if (res1 == S_OK ) { NewBitmap.VidBmp->Release(); ParentEngine->Console->Log(DEBUG_FATAL, "RAZOR::RESOURCE::CreateBitmap() Failed CreateTexture(): D3DPOOL_SYSTEMMEM -> MICROSOFT::HRESULT hr bad = %d\n", res2); }
    }

    return RetIndex;
}

// called when reset
UINT32 RAZOR::RESOURCE::OnLostDevice()
{
    // threadsafe
    NOVA::SCOPE_LOCK AutoLock(AddResourceMutex);

	// clear all of the bitmaps
	for(UINT32 i=0; i<BitmapArray.size(); ++i)
	{
        if (BitmapArray[i].State == RESOURCE_STATE_INITIALIZED)
        if (BitmapArray[i].VidBmp != nullptr)     
		{
            // release video memory
			BitmapArray[i].State = RESOURCE_STATE_RESET;
			BitmapArray[i].VidBmp->Release();
			BitmapArray[i].VidBmp = nullptr;

			// dont touch the system versions
			//BitmapArray[i]->SysBmp->Release();
			//BitmapArray[i]->SysBmp = NULL;
		}
	}

	// clear all of the special video bitmaps
	for(UINT32 i=0; i<VideoArray.size(); ++i)
	{
        
		if (VideoArray[i]->DataVid != nullptr)     
		{
			VideoArray[i]->DataVid->Release();
			VideoArray[i]->bIsValid = false;
            VideoArray[i]->DataVid = nullptr;
		}
	}


    return 0;
}

UINT32 RAZOR::RESOURCE::ResetBitmap(RESOURCE_BITMAP &TargetBitmap)
{
    if (TargetBitmap.SystemManaged == false)
    if (TargetBitmap.State == RESOURCE_STATE_RESET)
    {
        if (TargetBitmap.strFileName == "")
        {
            HRESULT CreateResult = ParentEngine->DirectX9->Device->CreateTexture(TargetBitmap.Width, TargetBitmap.Height, 1, D3DUSAGE_DYNAMIC, D3DFMT_A8R8G8B8, D3DPOOL_DEFAULT,   &TargetBitmap.VidBmp, NULL);
	        if (CreateResult != D3D_OK)  
            {
                ParentEngine->Console->Log(DEBUG_FATAL,"RAZOR::RESOURCE::ResetBitmap(): CreateTexture() Failed: %d != D3D_OK[%d]\n", CreateResult, D3D_OK);
                TargetBitmap.State = RESOURCE_STATE_DELETED;
            }
            else 		        
            {
                TargetBitmap.State = RESOURCE_STATE_INITIALIZED;
            }
        }
        else
        {
	        // only reload valid strings or resoource numbers
            D3DXIMAGE_INFO ImageInfo;
            STRING RetPath = ParentEngine->Path->MakeAbsolutePathChar(TargetBitmap.strFileName);
	        HRESULT CreateResult = D3DXCreateTextureFromFileExA(ParentEngine->DirectX9->Device, RetPath.c_str(), 0, 0, 1, 0, 
	           D3DFMT_A8R8G8B8, D3DPOOL_DEFAULT, D3DX_FILTER_NONE, D3DX_DEFAULT, 
	           0xFFFF00FF, &ImageInfo, NULL, &(TargetBitmap.VidBmp)); // magenta is the colorkey!

	        if (CreateResult != D3D_OK)  
            {
                ParentEngine->Console->Log(DEBUG_FATAL, "RAZOR::RESOURCE::ResetBitmap:\n%s = %d != D3D_OK[%d]\n", RetPath.c_str(), CreateResult, D3D_OK);
                TargetBitmap.State = RESOURCE_STATE_DELETED;
            }
            else 		        
            {
                TargetBitmap.State = RESOURCE_STATE_INITIALIZED;
            }
        }
    }
    return 0;
}


// reset the resources
UINT32 RAZOR::RESOURCE::OnResetDevice()
{
    // reset everything
    for(auto x = BitmapArray.begin(); x != BitmapArray.end(); ++x)
    {
        ResetBitmap(*x);
    }

    // reset special bitmaps -> system managed flag
    CreateBitmap(*GetPixelBitmap(),      1, 1, true);
    CreateBitmap(*GetScreenBitmap(), 512, 512, true);


	// reset all of the special video bitmaps
	for(UINT32 i=0; i<VideoArray.size(); ++i)
	{ 
		// check against 0 because it signals that this is a purged resource
		if (VideoArray[i]->Width != 0 && VideoArray[i]->Height != 0)
		{
			HRESULT hr = ParentEngine->DirectX9->Device->CreateOffscreenPlainSurface(VideoArray[i]->Width+1, VideoArray[i]->Height+1, D3DFMT_A8R8G8B8, D3DPOOL_DEFAULT, &VideoArray[i]->DataVid, NULL);
			if (FAILED(hr)) 
			{
				VideoArray[i]->bIsValid = false;
			}
			else
			{
				VideoArray[i]->bIsValid = true;
			}
		}
	}

    return 0;
}
RESOURCE_BITMAP* RAZOR::RESOURCE::GetScreenBitmap()
{
    return &BitmapArray[1]; 
}
RESOURCE_BITMAP* RAZOR::RESOURCE::GetPixelBitmap()
{
    return &BitmapArray[0]; 
}

// main methods
UINT32 RAZOR::RESOURCE::Initialize()
{
    // make some prelim resouces
    //assert(BitmapArray.size() == 0);
	CreateBitmap(  1,   1, true); GetPixelBitmap()->SystemManaged = true;
	CreateBitmap(512, 512, true); GetScreenBitmap()->SystemManaged = true;

    return 0;
}

// unlock all surfaces
void RESOURCE_VIDEO::Unlock()
{
    for(UINT32 t=0; t < Frames; ++t)
    {
        Data[t]->UnlockRect();
    }
    LockRect.clear();
    LockStart.clear();
    bIsLocked = false;
}

// self lock whole structure 
void RESOURCE_VIDEO::Lock()
{
    bIsLocked = true;
    if (LockRect.size() == 0)
    {
        LockStart.resize( Frames );
        LockRect.resize( Frames );
    }
    else
    {
        printf("RESOURCE_VIDEO::Lock() Failed: Double Lock Encountered!!\n");
        DebugBreak();
        return; // can't double lock
    }

    // lock ALL individual frames
    for(UINT32 t = 0; t < Frames; ++t)
    {
        top:
        MICROSOFT::HRESULT Success = Data[t]->LockRect( &LockRect[t], NULL, D3DLOCK_DISCARD | D3DLOCK_DONOTWAIT);
        if (Success != D3D_OK)
        {
            printf("RESOURCE_VIDEO::Lock() Failed: Can't lock a frame...%d\n Retrying\n", t);
            DebugBreak();
            goto top;
        }
        // indicate the start of the lock position
        LockStart[t] = static_cast<BYTE*> (LockRect[t].pBits);
    }

    // set pitch -> should be uniform across all frames t
    Pitch = LockRect[0].Pitch;

}

// uses the pitch/channels to figure out where the data is
inline unsigned int Deref(unsigned int x, unsigned int y, unsigned int Pitch, unsigned int Channel) // changes an [x,y,c] -> [t] ... assumes XRGB
{
	return (x << 2) + (Pitch*y) + (3 - Channel - 1);
}

// pixel functions -> only in lock mode
RAZOR_COLOR RESOURCE_VIDEO::GetPixel(UINT32 x, UINT32 y, UINT32 Frame)
{
    return static_cast<RAZOR_COLOR>(LockStart[Frame][ Deref(x, y, Pitch, 0)  ]);
}

// pixel functions -> only in lock mode
BYTE RESOURCE_VIDEO::GetPixelChannel(UINT32 x, UINT32 y, UINT32 Frame, UINT8 Channel)
{
    return LockStart[Frame][ Deref(x, y, Pitch, Channel)  ];
}

// write to all channels same time -> only in lock mode
void RESOURCE_VIDEO::PutPixel(UINT32 x, UINT32 y, UINT32 Frame, RAZOR_COLOR Color)
{
    LockStart[Frame][ Deref(x, y, Pitch, CHANNEL_RED)    ] = BYTE(  Color & (255)              );
    LockStart[Frame][ Deref(x, y, Pitch, CHANNEL_GREEN)  ] = BYTE( (Color & (255 << 8))  >> 8  );
    LockStart[Frame][ Deref(x, y, Pitch, CHANNEL_BLUE)   ] = BYTE( (Color & (255 << 16)) >> 16 );
    LockStart[Frame][ Deref(x, y, Pitch, CHANNEL_RED)  +3] = BYTE( (Color & (255 << 24)) >> 24 );
}

// write to all channels same time -> only in lock mode
void RESOURCE_VIDEO::PutPixelChannel(UINT32 x, UINT32 y, UINT32 Frame, UINT8 Channel, BYTE Value)
{
    LockStart[Frame][ Deref(x, y, Pitch, Channel)    ] = Value;
}