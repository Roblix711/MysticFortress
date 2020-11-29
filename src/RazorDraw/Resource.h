
// header gaurd
#ifndef _RAZOR_ENGINE_RESOURCE_H
#define _RAZOR_ENGINE_RESOURCE_H

// headers
#include "Engine.h"

// channel shortcuts for readibility
enum DATA_CHANNEL
{
	CHANNEL_RED   = 0,
	CHANNEL_GREEN = 1,
	CHANNEL_BLUE  = 2,
    CHANNEL_ALPHA = 3,
};	

// struct for holding video resources
struct RESOURCE_VIDEO 
{
    // directx connections
    VECTOR<DIRECTX9::LPDIRECT3DSURFACE9>  Data;     // SYSTEM MEMORY
	       DIRECTX9::LPDIRECT3DSURFACE9   DataVid;  // VIDEO MEMORY
    VECTOR<DIRECTX9::D3DLOCKED_RECT>      LockRect; // for locking

    // description
	UINT32 Width, Height;
	UINT32 Frames;
	BOOLEEN bIsValid;
    BOOLEEN bIsLocked;

    // private
    UINT32 Index; // auto set
    UINT32 Pitch; // auto set
    VECTOR<BYTE*> LockStart;
    

    // constructor
    RESOURCE_VIDEO() : bIsValid(false), bIsLocked(true), Width(0), Height(0), Frames(0), DataVid(NULL)
    {
        NOCODE; 
    }

    // methods
    typedef RAZOR::RAZOR_COLOR RAZOR_COLOR;
    void Unlock();
    void Lock();
    RAZOR_COLOR GetPixel       (UINT32 x, UINT32 y, UINT32 Frame);
    BYTE        GetPixelChannel(UINT32 x, UINT32 y, UINT32 Frame, UINT8 Channel);
    void        PutPixel       (UINT32 x, UINT32 y, UINT32 Frame, RAZOR_COLOR Color);
    void        PutPixelChannel(UINT32 x, UINT32 y, UINT32 Frame, UINT8 Channel, BYTE Value);
};

// states for a resource
enum RESOURCE_STATE
{
    RESOURCE_STATE_UNINITIALIZED = 0,
    RESOURCE_STATE_DELETED,
    RESOURCE_STATE_INITIALIZED,
    RESOURCE_STATE_RESET,
};


// bitmap structure
struct RESOURCE_BITMAP
{
    // directx connections
	DIRECTX9::LPDIRECT3DTEXTURE9 VidBmp;
	DIRECTX9::LPDIRECT3DTEXTURE9 SysBmp;

    // description
    UINT32 State;
    STRING strFileName;
	UINT32 Width, Height;
    BOOLEEN SystemManaged;
    
    // constructor
    RESOURCE_BITMAP(): State(RESOURCE_STATE_UNINITIALIZED), VidBmp(NULL), SysBmp(NULL), Width(0), Height(0), SystemManaged(false)
    {
        NOCODE;
    }

};

// this is a helper class
class RAZOR::RESOURCE
{
    public:
    RAZOR::ENGINECORE *ParentEngine;
    NOVA::MUTEX_FAST    AddResourceMutex;

    // vector of all resources 
    VECTOR<RESOURCE_VIDEO*>  VideoArray;
    VECTOR<RESOURCE_BITMAP> BitmapArray;
    //RESOURCE_BITMAP *pPixel;    // alias
   // RESOURCE_BITMAP *pScreen;   // alias

    // constructor
    RESOURCE(RAZOR::ENGINECORE *MyParent);
    ~RESOURCE();
    UINT32 Initialize();

    RESOURCE_BITMAP* GetScreenBitmap();
    RESOURCE_BITMAP* GetPixelBitmap();

    // methods
    UINT32 CreateVideo(UINT32 Width, UINT32 Height, UINT32 Frames);
    UINT32 CreateBitmap(UINT32 Width, UINT32 Height, BOOLEEN bFill);
    void   CreateBitmap(RESOURCE_BITMAP &NewBitmap, UINT32 Width, UINT32 Height, BOOLEEN bFill);

    UINT32 OnLostDevice();
    UINT32 OnResetDevice();
    UINT32 ResetBitmap(RESOURCE_BITMAP &TargetBitmap);
};

#endif