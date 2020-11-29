
// main headers
#include "DX9MAT.h"

// defines for ease
#ifndef DEBUG
#define ASSERT(x) if (!(x)) return;
#else
#define ASSERT(x) assert((x))
#endif

// constructor sets up link, and inits DX9 resources
DX9MAT::DX9MAT(RAZOR::ENGINE* R, UINT32 W, UINT32 H, METADATA M): RazorEngine(R), Width(W), Height(H), MetaData(M)
{
    // make the razor engine resource
    ResouceSurfaceID = RazorEngine->VideoCreate(Width, Height, 2);
    ResouceTextureID = RazorEngine->SpriteCreate(Width, Height);
    TempFrame = MAT(Height, Width, CV_8UC4);
}

// make a dx9mat from image loc
DX9MAT::DX9MAT(RAZOR::ENGINE* R, STRING Location, METADATA M): RazorEngine(R), MetaData(M)
{
    // get the image first, then make the resources from it
    TempFrame = cv::imread(Location, cv::IMREAD_UNCHANGED);
    Width  = TempFrame.size().width;
    Height = TempFrame.size().height;
    if (TempFrame.channels() == 3)
    {
        cv::cvtColor(TempFrame, TempFrame, cv::COLOR_BGR2BGRA);
    }
    ResouceSurfaceID = RazorEngine->VideoCreate(Width, Height, 2);
    ResouceTextureID = RazorEngine->SpriteCreate(Width, Height);
    UpdateTexture(TempFrame);
}

DX9MAT::~DX9MAT()
{
    // todo: cleanup from razor
}

// lock and unlock of resources
void DX9MAT::Lock()
{
    RazorEngine->VideoLock(ResouceSurfaceID);
}
void DX9MAT::Unlock()
{
    RazorEngine->VideoUnlock(ResouceSurfaceID);
}

// does a transfer of pixels from the opencv mat to the DX surface, ready for drawing after this.
void DX9MAT::UpdateSurface(const MAT &Src)
{
    // sanity checks
    ASSERT(Src.size().width <= Width);
    ASSERT(Src.size().height <= Height);
    ASSERT(Src.channels() == 4); // must be XRGB

    // transfer row by row
    Lock();
    BYTE *StartMatLoc = Src.data;
    for(UINT32 y=0; y<Src.size().height; ++y)
    {
        // get the starting row loc
        BYTE *TargetLoc = RazorEngine->VideoDeref(ResouceSurfaceID, 0, y, 0); 

        // block transfer
        memcpy( TargetLoc,  StartMatLoc, Src.size().width*sizeof(BYTE)*4 );
        StartMatLoc += Width*sizeof(BYTE)*4;
    }
    Unlock();

    // update the gpu one
    RazorEngine->VideoUpdate(ResouceSurfaceID, 0);
}

// updates the texture (for alpha blending)
void DX9MAT::UpdateTexture(const MAT &Src)
{
    UpdateSurface(Src);

    // update the gpu one
    RazorEngine->SpriteUpdate(ResouceTextureID, ResouceSurfaceID, 0);
}

// call this after UpdateSurface
void DX9MAT::Draw(INT32 X, INT32 Y, DOUBLE Sx, DOUBLE Sy)
{
    RazorEngine->VideoDraw(ResouceSurfaceID, X, Y, 0, Sx, Sy);
}

// call this after UpdateSurface
void DX9MAT::DrawMat(INT32 X, INT32 Y, DOUBLE Sx, DOUBLE Sy)
{
    RazorEngine->VideoDraw(ResouceSurfaceID, X, Y, 0, Sx, Sy);
}
// call this after UpdateSurface
void DX9MAT::DrawMat(const MAT &Ref, INT32 X, INT32 Y, DOUBLE Sx, DOUBLE Sy)
{
    ASSERT(Ref.channels() == 3); 
    MAT TempRGBA; cv::cvtColor(Ref, TempRGBA, CV_RGB2RGBA);
    UpdateSurface(TempRGBA);
    DrawMat(X, Y, Sx, Sy);
}

// call this after UpdateSurface
void DX9MAT::DrawMatAlpha(const MAT &Ref, INT32 X, INT32 Y, DOUBLE Sx, DOUBLE Sy, RAZOR::RAZOR_COLOR ColorMix)
{
    ASSERT(Ref.channels() == 4); 
    UpdateTexture(Ref);
    DrawMatAlpha(X, Y, Sx, Sy, ColorMix);
}

// call this after UpdateSurface
void DX9MAT::DrawMatAlpha(INT32 X, INT32 Y, DOUBLE Sx, DOUBLE Sy, RAZOR::RAZOR_COLOR ColorMix)
{
    RazorEngine->SpriteDraw(ResouceTextureID, X, Y, Sx, Sy, ColorMix);
}

// call this after UpdateSurface
void DX9MAT::DrawMatAlphaRot(INT32 X, INT32 Y, DOUBLE Sx, DOUBLE Sy, FLOAT CenterXNorm, FLOAT CenterYNorm, FLOAT Rotation, RAZOR::RAZOR_COLOR ColorMix)
{
    RazorEngine->SpriteDrawRotated(ResouceTextureID, X, Y, Sx, Sy, CenterXNorm, CenterYNorm, Rotation, ColorMix);
}
