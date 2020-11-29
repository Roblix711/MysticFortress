
// header gaurd
#ifndef _RAZOR_ENGINE_DRAW_H
#define _RAZOR_ENGINE_DRAW_H

// headers
#include "Engine.h"


    

// this is a helper class
class RAZOR::DRAW
{
    public:
    typedef RAZOR::RAZOR_COLOR RAZOR_COLOR;

    RAZOR::ENGINECORE *ParentEngine;
    std::atomic<INT32> LastDrawType;
    CHAR *TextBufferLarge;
    MAP< PAIR<UINT32, STRING>, DIRECTX9::LPD3DXFONT> FontMap;
    static const DWORD RAZER_DRAW_VERTEX_2D_FVF = D3DFVF_XYZRHW | D3DFVF_DIFFUSE;

    // constructor
    DRAW(RAZOR::ENGINECORE *MyParent);
    ~DRAW();
    UINT32 Initialize();

    // drawing commands
    void SetLinePrefs(FLOAT Width, BOOLEEN Anti, FLOAT Scale, UINT32 Pattern);
    void SetDrawType(INT32 Type);
    void ClearScreen(RAZOR_COLOR Color);
    void Circle(INT32 x0, INT32 y0, INT32 radius, RAZOR_COLOR color);
    void CircleFill(INT32 x0, INT32 y0, INT32 radius, RAZOR_COLOR color);
    void RectFillCurve(INT32 x1, INT32 y1, INT32 x2, INT32 y2, INT32 radius, RAZOR_COLOR color);
    void RectFillThick(INT32 x1, INT32 y1, INT32 x2, INT32 y2, INT32 radius, RAZOR_COLOR color);
    void Line(INT32 x1, INT32 y1, INT32 x2, INT32 y2, RAZOR_COLOR Color);
    void LineThick(INT32 x1, INT32 y1, INT32 x2, INT32 y2, FLOAT Thick, RAZOR_COLOR color);
    void Rect(INT32 x1, INT32 y1, INT32 x2, INT32 y2, RAZOR_COLOR Color);
    void RectFill(INT32 x1, INT32 y1, INT32 x2, INT32 y2, RAZOR_COLOR Color);
    void TriangleFanFilled(const VECTOR<DRAW_VERTEX_2D> &Points, UINT32 Color);
    void TriangleListFilled(const VECTOR<DRAW_VERTEX_2D> &Points, UINT32 Color);
    void PixelRaw(INT32 x, INT32 y, RAZOR_COLOR Color);
    void Pixel(INT32 x, INT32 y, RAZOR_COLOR Color);
    void UpdateVideo(UINT32 ResourceIndex, UINT32 FrameNumber);
    void Video(UINT32 ResourceIndex, INT32 x, INT32 y, UINT32 FrameNumber, DOUBLE ScaleX, DOUBLE ScaleY, BOOLEEN bUpdateFromSystemMemory = true);
    void Sprite(UINT32 ResourceIndex, INT32 x, INT32 y, DOUBLE ScaleX, DOUBLE ScaleY, RAZOR_COLOR ColorMix);
    void SpriteRotated(UINT32 ResourceIndex, INT32 x, INT32 y, DOUBLE ScaleX, DOUBLE ScaleY, FLOAT Cx, FLOAT Cy, FLOAT Rotation, RAZOR_COLOR ColorMix);
    void UpdateSprite(UINT32 TextureID, UINT32 SurfaceID, UINT32 Frame);
	RECT Text(const STRING &Font, UINT32 Size, const CHAR *message, INT32 x, INT32 y, INT32 x2, INT32 y2, RAZOR_COLOR color, UINT32 Flags, BOOLEEN Clip, BOOLEEN Calculate);
    RECT Text(const STRING &Font, UINT32 Size, const CHAR *message, INT32 x, INT32 y,                     RAZOR_COLOR color, UINT32 Flags,               BOOLEEN Calculate);
    RECT Text(const STRING &Font, UINT32 Size,                      INT32 x, INT32 y,                     RAZOR_COLOR color, UINT32 Flags,               BOOLEEN Calculate, const CHAR *message, ...);
    RECT Text(                                                      INT32 x, INT32 y,                     RAZOR_COLOR color, UINT32 Flags,               BOOLEEN Calculate, const CHAR *message, ...);
    RECT Text(const STRING &Font, UINT32 Size,                      INT32 x, INT32 y,                     RAZOR_COLOR color, UINT32 Flags,               BOOLEEN Calculate, const CHAR *message, VAR_LIST Args);
    RECT Text(                                                      INT32 x, INT32 y,                     RAZOR_COLOR color, UINT32 Flags,               BOOLEEN Calculate, const CHAR *message, VAR_LIST Args);
    RECT TextRotated(const STRING &Font, UINT32 Size, const CHAR *message, INT32 x, INT32 y, INT32 x2, INT32 y2, FLOAT Rotation, RAZOR_COLOR color, UINT32 Flags, BOOLEEN Clip, BOOLEEN Calculate);
    RECT TextRotated(const STRING &Font, UINT32 Size,                      INT32 x, INT32 y,                     FLOAT Rotation, RAZOR_COLOR color, UINT32 Flags,               BOOLEEN Calculate, const CHAR *message, VAR_LIST Args);
    RECT TextRotated(                                                      INT32 x, INT32 y,                     FLOAT Rotation, RAZOR_COLOR color, UINT32 Flags,               BOOLEEN Calculate, const CHAR *message, VAR_LIST Args);

    // methods
    static RAZOR_COLOR MakeColor32(BYTE r, BYTE g, BYTE b, BYTE a);
    static BYTE GetChannelColor(RAZOR_COLOR Color, UINT8 Channel);

};

#endif