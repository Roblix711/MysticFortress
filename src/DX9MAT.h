
// header gaurd
#ifndef _ENGINEXG3_DX9MAT_H
#define _ENGINEXG3_DX9MAT_H

// dependencies
#include <RazorDraw.hpp>
#include <opencv2/opencv.hpp>

// main libs
#ifdef _DEBUG
#pragma comment(lib, "opencv_world310d.lib")
#else
#pragma comment(lib, "opencv_world310.lib")
#endif

// typdefs for eas
typedef cv::Mat  MAT;
typedef void*    METADATA;

// this is a wrapper around DX9 and cv::Mat to allow fast drawing
class DX9MAT
{
    public: 
    UINT32          ResouceSurfaceID;
    UINT32          ResouceTextureID;
    RAZOR::ENGINE*  RazorEngine;
    MAT             TempFrame;
    UINT32          Width;
    UINT32          Height;
    METADATA        MetaData;

    // constructor sets up link, and inits DX9 resources
    DX9MAT(RAZOR::ENGINE* R, UINT32 Width, UINT32 Height, METADATA M = nullptr);
    DX9MAT(RAZOR::ENGINE* R, STRING ImageLoc, METADATA M = nullptr);
    ~DX9MAT();

    // does a transfer of pixels from the opencv mat to the DX surface, ready for drawing after this.
    void UpdateSurface(const MAT &Src);
    void UpdateTexture(const MAT &Src);

    // call this after UpdateSurface
    void Draw(INT32 X, INT32 Y, DOUBLE Sx = 1.0, DOUBLE Sy = 1.0);
    void DrawMat(const MAT &Ref, INT32 X, INT32 Y, DOUBLE Sx = 1.0, DOUBLE Sy = 1.0);
    void DrawMat(                INT32 X, INT32 Y, DOUBLE Sx = 1.0, DOUBLE Sy = 1.0);
    void DrawMatAlpha(const MAT &Ref, INT32 X, INT32 Y, DOUBLE Sx, DOUBLE Sy, RAZOR::RAZOR_COLOR ColorMix);
    void DrawMatAlpha(                INT32 X, INT32 Y, DOUBLE Sx, DOUBLE Sy, RAZOR::RAZOR_COLOR ColorMix);
    void DrawMatAlphaRot(INT32 X, INT32 Y, DOUBLE Sx, DOUBLE Sy, FLOAT CenterXNorm, FLOAT CenterYNorm, FLOAT Rotation, RAZOR::RAZOR_COLOR ColorMix);

    private:
    // lock and unlock of resources
    void Lock();
    void Unlock();


};

#endif