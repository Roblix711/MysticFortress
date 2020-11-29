
// main header
#include "Draw.h"
#include <math.h>
#include <algorithm>

// typedefs for ease
#define RAZOR_DRAW_MIN_FONT_SIZE  8
#define RAZOR_DRAW_MAX_FONT_SIZE  25
#define RAZOR_DRAW_TEXT_BUFFER 4096
#define RAZOR_DRAW_MAX_CIRCLE_RES 256

// protected
namespace 
{
    // for colors ARGB888 only
    using RAZOR::RAZOR_COLOR;
    const RAZOR_COLOR cOpaque  = (255 << 24);
    const RAZOR_COLOR cBlack   = (255 << 24);
    const RAZOR_COLOR cRed     = (cBlack + (255<<16));
    const RAZOR_COLOR cGreen   = (cBlack + (255<<8));
    const RAZOR_COLOR cBlue    = (cBlack + 255);
    const RAZOR_COLOR cWhite   = (cRed | cGreen | cBlue);
    const RAZOR_COLOR cCyan    = (cGreen | cBlue);
    const RAZOR_COLOR cTeal    = (cGreen | cBlue);
    const RAZOR_COLOR cMagenta = (cRed | cBlue);
    const RAZOR_COLOR cPink    = (cRed | cBlue);
    const RAZOR_COLOR cYellow  = (cRed | cGreen);

    // round functions for int/double
    DOUBLE round1(DOUBLE r)
    {
        return (r > 0.0) ? floor(r + 0.5) : ceil(r - 0.5);
    }
    INT roundi(DOUBLE r)
    {
	    return INT(round1(r));
    }

    // HELPER
    bool InsideRect(INT32 x1, INT32 y1, INT32 x2, INT32 y2, INT32 mx, INT32 my) 
    {
	    return ( (mx >= x1) && (mx < x2) && (my >= y1) && (my < y2) );
    }

    // HELPER
    bool IntersectionRect(MICROSOFT::RECT *Src, MICROSOFT::RECT *Dest, MICROSOFT::RECT *Result)
    {
	    // calculate the boundaries
	    int Xr = xMAX(Src->left,   Dest->left);
	    int Wr = xMIN(Src->right,  Dest->right);
	    int Yr = xMAX(Src->top,    Dest->top);
	    int Hr = xMIN(Src->bottom, Dest->bottom);

	    // make sure that we dont have inverse intersections
	    if (Wr > Xr && Hr > Yr)
	    {
		    Result->left   = (Xr);
		    Result->right  = (Wr+1);
		    Result->top    = (Yr);
		    Result->bottom = (Hr+1);
		    return true;
	    }
	    return false;
    }

    bool IntersectionRectScale(MICROSOFT::RECT *Src, MICROSOFT::RECT *Dest, MICROSOFT::RECT *Result, INT32 x, INT32 y, DOUBLE ScaleX, DOUBLE ScaleY)
    {
	    // calculate the boundaries
	    int Xr = xMAX(Src->left,   Dest->left);
	    int Wr = xMIN(Src->right,  Dest->right);
	    int Yr = xMAX(Src->top,    Dest->top);
	    int Hr = xMIN(Src->bottom, Dest->bottom);

	    // make sure that we dont have inverse intersections
	    if (Wr > Xr && Hr > Yr)
	    {
		    Result->left   = roundi((Xr - x)/ScaleX);
		    Result->right  = roundi((Wr - x)/ScaleX);
		    Result->top    = roundi((Yr - y)/ScaleY);
		    Result->bottom = roundi((Hr - y)/ScaleY);
		    return true;
	    }
	    return false;
    }
}

// constructor
RAZOR::DRAW::DRAW(RAZOR::ENGINECORE *MyParent) : ParentEngine(MyParent), LastDrawType(DRAW_NONE)
{
    // nocode
    TextBufferLarge = new char[RAZOR_DRAW_TEXT_BUFFER];
}
RAZOR::DRAW::~DRAW()
{
    // release
    for(auto it = FontMap.begin(); it != FontMap.end(); ++it)
    {
        it->second->Release();
    }

    delete [] TextBufferLarge;
}

// makes some fonts
UINT32 RAZOR::DRAW::Initialize()
{
    for(UINT32 i=RAZOR_DRAW_MIN_FONT_SIZE; i<RAZOR_DRAW_MAX_FONT_SIZE; ++i)
    {
	    DIRECTX9::D3DXCreateFont(ParentEngine->DirectX9->Device, i, 0, FW_NORMAL, 0, FALSE, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, DEFAULT_QUALITY, FIXED_PITCH | FF_MODERN, TEXT("Cambria"),  &FontMap[ {i, "Cambria"} ]);
    }
    SetLinePrefs(1.0, false, 1.0, DWORD(-1));

    return 0;
}

// changes "tool" 
void RAZOR::DRAW::SetDrawType(INT32 Type)
{
	if (LastDrawType == Type)
	{
		// do nothing... last draw is the same as current, were in clear for accelerated draw
	}
	else
	{
        // end old type
        INT32 LDT = LastDrawType;
        switch(LDT)
        {
            case DRAW_NONE:  break;
            case DRAW_SPRITE: 
            {
                ParentEngine->DirectX9->Sprite->End(); 
                break;
            }
            case DRAW_LINE:
            {
                ParentEngine->DirectX9->Line->End(); 
                break;
            }
            default:
            {
                ParentEngine->Console->Log(DEBUG_FATAL, "RAZOR::DRAW::SetDrawType(): Unknown Previous TYPE! = %d", LDT);
                break;
            }
        }

        // start new type
        switch(Type)
        {
            case DRAW_NONE:  break;
            case DRAW_SPRITE: 
            {
                ParentEngine->DirectX9->Sprite->Begin(D3DXSPRITE_ALPHABLEND | D3DXSPRITE_DO_NOT_ADDREF_TEXTURE ); 
                break;
            }
            case DRAW_LINE:
            {             
                ParentEngine->DirectX9->Line->Begin();
                break;
            }
            default:
            {
                ParentEngine->Console->Log(DEBUG_FATAL, "RAZOR::DRAW::SetDrawType(): Unknown New TYPE! = %d", Type);
                break;
            }
        }

		// assign new type
		LastDrawType = Type;
	}
}

// basic clear screen
void RAZOR::DRAW::ClearScreen(RAZOR_COLOR Color) // no alpha here either...
{
     ParentEngine->DirectX9->Device->Clear(0, NULL, D3DCLEAR_TARGET, Color, 1.0f, 0);
}

// fastest circle
void RAZOR::DRAW::LineThick(INT32 x1, INT32 y1, INT32 x2, INT32 y2, FLOAT Thick, RAZOR_COLOR color)
{
    SetDrawType(DRAW_NONE);
    DRAW_VERTEX_2D verts[4];
    DOUBLE Angle = atan2(y2-y1, x2-x1);
    verts[0].x = x1 + Thick/2.0*cos(Angle+D3DX_PI/2.0);
    verts[0].y = y1 + Thick/2.0*sin(Angle+D3DX_PI/2.0);
    verts[1].x = x1 - Thick/2.0*cos(Angle+D3DX_PI/2.0);
    verts[1].y = y1 - Thick/2.0*sin(Angle+D3DX_PI/2.0);
    verts[2].x = x2 + Thick/2.0*cos(Angle-D3DX_PI/2.0);
    verts[2].y = y2 + Thick/2.0*sin(Angle-D3DX_PI/2.0);
    verts[3].x = x2 - Thick/2.0*cos(Angle-D3DX_PI/2.0);
    verts[3].y = y2 - Thick/2.0*sin(Angle-D3DX_PI/2.0);

    // set remaining
    for (int i = 0; i < 4; i++)
    {
        verts[i].z = 0.0;
        verts[i].rhw = 1.0;
        verts[i].color = color;
    }
    // draw it
    ParentEngine->DirectX9->Device->SetFVF(RAZER_DRAW_VERTEX_2D_FVF);
    ParentEngine->DirectX9->Device->DrawPrimitiveUP(D3DPT_TRIANGLEFAN, 2, &verts, sizeof(DRAW_VERTEX_2D));

}


void RAZOR::DRAW::Line(INT32 x1, INT32 y1, INT32 x2, INT32 y2, RAZOR_COLOR Color)
{
	SetDrawType(DRAW_LINE);
    D3DXVECTOR2 pos[2] = { {FLOAT(x1), FLOAT(y1)}, {FLOAT(x2), FLOAT(y2)} };
    if (x1 == x2 && y1 == y2) return; // UNKNOWN -> SEE BUG report below!
    ParentEngine->DirectX9->Line->Draw(&pos[0], 2, Color); 
}

/* =======================================================
for(UINT i=0; i<80; ++i)
{
    RazorEngine->DrawSetLinePrefs( 10 );
    RazorEngine->DrawLine(50, 51, 52, 53, cRed);
    RazorEngine->DrawSetLinePrefs();
    RazorEngine->DrawLine(50, 51, 52, 53, cRed);
    RazorEngine->DrawPixel(50, 51, cWhite);
    RazorEngine->DrawLine(60, 30, 60, 30, cWhite);
}

THIS CAUSES PROBLEMS WITH DEVICE STATE LOSS:
GOES STRAIGHT TO D3D_DEVICENOTRESET, SKIPS D3D_DEVICELOST

// NOTES:
// bug appears on Win10, RX580, DX9.0C June 2010, under VC++ 2015
// bug is not reproducable on NVIDIA GTX1060, Win10
// bug is not reproducable on Win7

// ======================================================= */

// set the line preferences
void RAZOR::DRAW::SetLinePrefs(FLOAT Width, BOOLEEN Anti, FLOAT Scale, UINT32 Pattern)
{   
    SetDrawType(DRAW_NONE);
    ParentEngine->DirectX9->Line->SetWidth(Width);
    ParentEngine->DirectX9->Line->SetAntialias(Anti);
    ParentEngine->DirectX9->Line->SetPatternScale(Scale);
    ParentEngine->DirectX9->Line->SetPattern(Pattern);
}

// this is the fast version 
void RAZOR::DRAW::PixelRaw(INT32 x, INT32 y, RAZOR_COLOR Color)
{
	SetDrawType(DRAW_SPRITE);
    D3DXMATRIX TransMatrix;
    D3DXMatrixTranslation(&TransMatrix, FLOAT(x), FLOAT(y), FLOAT(0.0f));
    MICROSOFT::HRESULT ResultTransform = ParentEngine->DirectX9->Sprite->SetTransform(&TransMatrix);
    ParentEngine->DirectX9->Sprite->Draw(ParentEngine->Resources->GetPixelBitmap()->VidBmp, NULL, NULL, NULL, Color);
}

// this is the safe version that checks the bounds 
inline void RAZOR::DRAW::Pixel(INT32 x, INT32 y, RAZOR_COLOR Color)
{
	if (InsideRect(0, 0, ParentEngine->Window->GetScreenWidth(), ParentEngine->Window->GetScreenHeight(), x, y) == true) 
        PixelRaw(x, y, Color);
}

// standard rectangle no fill, do it with fast lines
void RAZOR::DRAW::Rect(INT32 x1, INT32 y1, INT32 x2, INT32 y2, RAZOR_COLOR Color)
{
	SetDrawType(DRAW_LINE);
    D3DXVECTOR2 pos[5] = 
    {   
        {FLOAT(x1), FLOAT(y1)}, 
        {FLOAT(x1), FLOAT(y2)}, 
        {FLOAT(x2), FLOAT(y2)},
        {FLOAT(x2), FLOAT(y1)}, 
        {FLOAT(x1), FLOAT(y1)}, 
    };
    if (x1 == x2 && y1 == y2) return; // UNKNOWN -> SEE BUG report below!
    ParentEngine->DirectX9->Line->Draw(&pos[0], 5, Color); 

	/*tDrawType(DRAW_LINE);
	Line(x1, y1, x2,   y1,   Color);
	Line(x1, y1, x1,   y2,   Color);
	Line(x1, y2, x2+1, y2,   Color);
	Line(x2, y1, x2,   y2+1, Color);*/
}

// makes a color -> static
RAZOR_COLOR RAZOR::DRAW::MakeColor32(BYTE r, BYTE g, BYTE b, BYTE a)
{
    return  (BYTE(b) + (BYTE(g)<<8) + (BYTE(r)<<16) + (BYTE(a)<<24));
}
/*
void RAZOR::DRAW::Ellipse(INT32 x0, INT32 y0, DOUBLE rad_1, DOUBLE rad_2, DOUBLE alpha, long int color)
{
	if (rad_1 <= 0.0 || rad_2 <= 0.0) return;
	if (rad_1 > ParentEngine->Window->ScreenWidth || rad_2 > ParentEngine->Window->ScreenWidth) return;

	if (x0 < -xMAX(rad_1, rad_2) || x0 > ParentEngine->Window->ScreenWidth + xMAX(rad_1, rad_2)) return;
	if (y0 < -xMAX(rad_1, rad_2) || y0 > ParentEngine->Window->ScreenHeight + xMAX(rad_1, rad_2)) return;

	// draw by lines
	double CosAlpha = cos(alpha);
	double SinAlpha = sin(alpha);
	int LastX = int(rad_1 * CosAlpha);
	int LastY = int(rad_1 * SinAlpha);
	double K = 16.0*(1.0-1.0/rad_1-1.0/rad_2)/(rad_1+rad_2); // adjust for smaller circles
	if (K <= 0.0) return;
	for(double theta = K; theta <= PI+K; theta += K)
	{
		int X = int(rad_1 * cos(theta) * CosAlpha - rad_2 * sin(theta) * SinAlpha);
		int Y = int(rad_1 * cos(theta) * SinAlpha + rad_2 * sin(theta) * CosAlpha);
		Line(x0 + X,  y0 + Y, x0 + LastX, y0 + LastY, color);
		Line(x0 - X,  y0 - Y, x0 - LastX, y0 - LastY, color);
		LastX = X;
		LastY = Y;
	}

}*/

// just 4 rectangles
void RAZOR::DRAW::RectFillThick(INT32 x1, INT32 y1, INT32 x2, INT32 y2, INT32 radius, RAZOR_COLOR color)
{
    INT32 HalfRad = lround(radius / 2.0);
    RectFill(x1 - HalfRad, y1 - HalfRad, x2 + HalfRad, y1 + HalfRad, color);
    RectFill(x1 - HalfRad, y2 - HalfRad, x2 + HalfRad, y2 + HalfRad, color);
    RectFill(x1 - HalfRad, y1 + HalfRad, x1 + HalfRad, y2 - HalfRad, color);
    RectFill(x2 - HalfRad, y1 + HalfRad, x2 + HalfRad, y2 - HalfRad, color);
}


void RAZOR::DRAW::RectFillCurve(INT32 x1, INT32 y1, INT32 x2, INT32 y2, INT32 radius, RAZOR_COLOR color)
{
    INT32 HalfRad = lround(radius / 2.0);
    SetDrawType(DRAW_NONE);
    DRAW_VERTEX_2D TopLeft[RAZOR_DRAW_MAX_CIRCLE_RES+1];
    DRAW_VERTEX_2D TopRight[RAZOR_DRAW_MAX_CIRCLE_RES+1];
    DRAW_VERTEX_2D BottomLeft[RAZOR_DRAW_MAX_CIRCLE_RES+1];
    DRAW_VERTEX_2D BottomRight[RAZOR_DRAW_MAX_CIRCLE_RES+1];
    INT32 QuarterResolution = std::min(RAZOR_DRAW_MAX_CIRCLE_RES,  std::max(33, INT32(2.0*D3DX_PI*radius/5.0) )) / 4; // 10 PIXELS

    // top left
    TopLeft[0].x = x1  + HalfRad;
    TopLeft[0].y = y1  + HalfRad;
    TopLeft[0].z = 0;
    TopLeft[0].rhw = 1;
    TopLeft[0].color = color;
    for (INT32 i = 0; i <= QuarterResolution; i++)
    {
        TopLeft[i+1].x = x1 + HalfRad - (HalfRad*2.0+1)*cos(D3DX_PI*i/2.0f/QuarterResolution);
        TopLeft[i+1].y = y1 + HalfRad - (HalfRad*2.0+1)*sin(D3DX_PI*i/2.0f/QuarterResolution);
        TopLeft[i+1].z = 0;
        TopLeft[i+1].rhw = 1;
        TopLeft[i+1].color = color;
    }
    // top right
    TopRight[0].x = x2 - HalfRad;
    TopRight[0].y = y1 + HalfRad;
    TopRight[0].z = 0;
    TopRight[0].rhw = 1;
    TopRight[0].color = color;
    for (INT32 i = 0; i <= QuarterResolution; i++)
    {
        TopRight[i+1].x = x2 - HalfRad + (HalfRad*2.0)*cos(D3DX_PI*i/2.0f/QuarterResolution);
        TopRight[i+1].y = y1 + HalfRad - (HalfRad*2.0+1)*sin(D3DX_PI*i/2.0f/QuarterResolution);
        TopRight[i+1].z = 0;
        TopRight[i+1].rhw = 1;
        TopRight[i+1].color = color;
    }

    // bottom left
    BottomLeft[0].x = x1 + HalfRad;
    BottomLeft[0].y = y2 - HalfRad;
    BottomLeft[0].z = 0;
    BottomLeft[0].rhw = 1;
    BottomLeft[0].color = color;
    for (INT32 i = 0; i <= QuarterResolution; i++)
    {
        BottomLeft[i+1].x = x1 + HalfRad - (HalfRad*2.0+1)*cos(D3DX_PI*i/2.0f/QuarterResolution);
        BottomLeft[i+1].y = y2 - HalfRad + (HalfRad*2.0)*sin(D3DX_PI*i/2.0f/QuarterResolution);
        BottomLeft[i+1].z = 0;
        BottomLeft[i+1].rhw = 1;
        BottomLeft[i+1].color = color;
    }

    // bottom right
    BottomRight[0].x = x2 - HalfRad;
    BottomRight[0].y = y2 - HalfRad;
    BottomRight[0].z = 0;
    BottomRight[0].rhw = 1;
    BottomRight[0].color = color;
    for (INT32 i = 0; i <= QuarterResolution; i++)
    {
        BottomRight[i+1].x = x2 - HalfRad + (HalfRad*2.0)*cos(D3DX_PI*i/2.0f/QuarterResolution);
        BottomRight[i+1].y = y2 - HalfRad + (HalfRad*2.0)*sin(D3DX_PI*i/2.0f/QuarterResolution);
        BottomRight[i+1].z = 0;
        BottomRight[i+1].rhw = 1;
        BottomRight[i+1].color = color;
    }


    // draw all 
    ParentEngine->DirectX9->Device->SetFVF(RAZER_DRAW_VERTEX_2D_FVF);
    ParentEngine->DirectX9->Device->DrawPrimitiveUP(D3DPT_TRIANGLEFAN, QuarterResolution, &TopLeft, sizeof(DRAW_VERTEX_2D));
    ParentEngine->DirectX9->Device->DrawPrimitiveUP(D3DPT_TRIANGLEFAN, QuarterResolution, &TopRight, sizeof(DRAW_VERTEX_2D));
    ParentEngine->DirectX9->Device->DrawPrimitiveUP(D3DPT_TRIANGLEFAN, QuarterResolution, &BottomLeft, sizeof(DRAW_VERTEX_2D));
    ParentEngine->DirectX9->Device->DrawPrimitiveUP(D3DPT_TRIANGLEFAN, QuarterResolution, &BottomRight, sizeof(DRAW_VERTEX_2D));


    // draw borders
    
    RectFill(x1 + HalfRad, y1 - HalfRad, x2 - HalfRad, y1 + HalfRad, color);
    RectFill(x1 + HalfRad, y2 - HalfRad, x2 - HalfRad, y2 + HalfRad, color);
    RectFill(x1 - HalfRad, y1 + HalfRad, x1 + HalfRad, y2 - HalfRad, color);
    RectFill(x2 - HalfRad, y1 + HalfRad, x2 + HalfRad, y2 - HalfRad, color);
}

// fastest circle
void RAZOR::DRAW::CircleFill(INT32 x0, INT32 y0, INT32 radius, RAZOR_COLOR color)
{
    // constant for circle num points
    INT32 CircleResolution = std::min(RAZOR_DRAW_MAX_CIRCLE_RES,  std::max(33, INT32(2.0*D3DX_PI*radius/5.0) )); // 10 PIXELS

    SetDrawType(DRAW_NONE);
    DRAW_VERTEX_2D verts[RAZOR_DRAW_MAX_CIRCLE_RES+1];

    for (INT32 i = 0; i < CircleResolution+1; i++)
    {
        verts[i].x = x0 + radius*cos(D3DX_PI*i*2.0f/CircleResolution);
        verts[i].y = y0 + radius*sin(D3DX_PI*i*2.0f/CircleResolution);
        verts[i].z = 0;
        verts[i].rhw = 1;
        verts[i].color = color;
    }
    ParentEngine->DirectX9->Device->SetFVF(RAZER_DRAW_VERTEX_2D_FVF);
    ParentEngine->DirectX9->Device->DrawPrimitiveUP(D3DPT_TRIANGLEFAN, CircleResolution-1, &verts, sizeof(DRAW_VERTEX_2D));
}

// fastest circle method
void RAZOR::DRAW::Circle(INT32 x0, INT32 y0, INT32 radius, RAZOR_COLOR color)
{
	int f = 1 - radius;
	int ddF_x = 1;
	int ddF_y = -2 * radius;
	int x = 0;
	int y = radius;

	if (radius < 0.10) return;
	if (radius > ParentEngine->Window->GetScreenWidth()*2) return;
	if (x0 < -radius || x0 > ParentEngine->Window->GetScreenWidth() + radius) return;
	if (y0 < -radius || y0 > ParentEngine->Window->GetScreenHeight() + radius) return;
	
	Pixel(x0, y0 + radius, color);
	Pixel(x0, y0 - radius, color);
	Pixel(x0 + radius, y0, color);
	Pixel(x0 - radius, y0, color);

	while(x < y)
	{
		// ddF_x == 2 * x + 1;
		// ddF_y == -2 * y;
		// f == x*x + y*y - radius*radius + 2*x - y + 1;
		if(f >= 0) 
		{
			y--;
			ddF_y += 2;
			f += ddF_y;
		}
		x++;
		ddF_x += 2;
		f += ddF_x;    
		Pixel(x0 + x, y0 + y, color);
		Pixel(x0 - x, y0 + y, color);
		Pixel(x0 + x, y0 - y, color);
		Pixel(x0 - x, y0 - y, color);
		Pixel(x0 + y, y0 + x, color);
		Pixel(x0 - y, y0 + x, color);
		Pixel(x0 + y, y0 - x, color);
		Pixel(x0 - y, y0 - x, color);
	}
}

// raw draw
void RAZOR::DRAW::TriangleFanFilled(const VECTOR<DRAW_VERTEX_2D> &Points, UINT32 Color)
{
    SetDrawType(DRAW_NONE);
    HRESULT VFVResult = ParentEngine->DirectX9->Device->SetFVF(RAZER_DRAW_VERTEX_2D_FVF);
    HRESULT DrawResult = ParentEngine->DirectX9->Device->DrawPrimitiveUP(D3DPT_TRIANGLEFAN, Points.size()-2, Points.data(), sizeof(DRAW_VERTEX_2D));
}

// raw draw
void RAZOR::DRAW::TriangleListFilled(const VECTOR<DRAW_VERTEX_2D> &Points, UINT32 Color)
{
    SetDrawType(DRAW_NONE);
    HRESULT VFVResult = ParentEngine->DirectX9->Device->SetFVF(RAZER_DRAW_VERTEX_2D_FVF);
    HRESULT DrawResult = ParentEngine->DirectX9->Device->DrawPrimitiveUP(D3DPT_TRIANGLELIST, Points.size()/3, Points.data(), sizeof(DRAW_VERTEX_2D));
}

// colored fill
void RAZOR::DRAW::RectFill(INT32 x1, INT32 y1, INT32 x2, INT32 y2, RAZOR_COLOR Color)
{
    SetDrawType(DRAW_NONE);
    DRAW_VERTEX_2D verts[4] = 
    {
        {x1, y1, 0.0, 1.0, Color},
        {x2, y1, 0.0, 1.0, Color},
        {x2, y2, 0.0, 1.0, Color},
        {x1, y2, 0.0, 1.0, Color},
    };
    // draw it
    HRESULT VFVResult = ParentEngine->DirectX9->Device->SetFVF(RAZER_DRAW_VERTEX_2D_FVF);
    HRESULT DrawResult = ParentEngine->DirectX9->Device->DrawPrimitiveUP(D3DPT_TRIANGLEFAN, 2, &verts, sizeof(DRAW_VERTEX_2D));

    // debugs
    if (VFVResult != D3D_OK) ParentEngine->Console->Log(DEBUG_WARN, "RAZOR::DRAW::RectFill(): Call to SetFVF() failed (0x%x != D3D_OK) %s -> %s\n", VFVResult, DXGetErrorString(VFVResult), DXGetErrorDescription(VFVResult));
    if (DrawResult != D3D_OK) ParentEngine->Console->Log(DEBUG_WARN, "RAZOR::DRAW::RectFill(): Call to DrawPrimitiveUP() failed (0x%x != D3D_OK) %s -> %s\n", DrawResult, DXGetErrorString(DrawResult), DXGetErrorDescription(DrawResult));


}

// update the surface
void RAZOR::DRAW::UpdateVideo(UINT32 ResourceIndex, UINT32 FrameNumber)
{
    // sanity checks when we get the video
    if (ResourceIndex == UINT32(-1)) return;
    RESOURCE_VIDEO *Video = ParentEngine->Resources->VideoArray[ResourceIndex];
    if (UINT32(FrameNumber) >= Video->Frames) return;

    // call the update surface command -> downloads the picture into video memory
    MICROSOFT::HRESULT UpdateSuccess = ParentEngine->DirectX9->Device->UpdateSurface(Video->Data[FrameNumber], NULL, Video->DataVid, NULL);
}

// draws the surface // 
void RAZOR::DRAW::Video(UINT32 ResourceIndex, INT32 x, INT32 y, UINT32 FrameNumber, DOUBLE ScaleX, DOUBLE ScaleY, BOOLEEN bUpdateFromSystemMemory)
{
    // sanity checks when we get the video
    if (ResourceIndex == UINT32(-1) ) return;
    RESOURCE_VIDEO *Video = ParentEngine->Resources->VideoArray[ResourceIndex];
	if ( UINT32(FrameNumber) >= Video->Frames) return;

    // only for valid init videos
	if (Video->bIsValid == true)
	{
		// predefs -> might change
		//int SW = ParentEngine->Window->ScreenWidth;
		//int SH = ParentEngine->Window->ScreenHeight;
		MICROSOFT::D3DSURFACE_DESC OutDesc;
		ParentEngine->DirectX9->DrawSurface->GetDesc(&OutDesc);
		INT SW = OutDesc.Width;
		INT SH = OutDesc.Height;

		// compute the coordinates, and watch the boundaries
		MICROSOFT::RECT SrcRect  = {x, y, x + roundi(Video->Width*ScaleX), y + roundi(Video->Height*ScaleY) };
		MICROSOFT::RECT DestRect = {0, 0, SW, SH};
		MICROSOFT::RECT InterRect;

		// display the video only if onscreen
		if ( IntersectionRectScale(&SrcRect, &DestRect, &InterRect, x, y, ScaleX, ScaleY) == true )
		{
			// this fixes a z-order problem (otherwise, objects draw OOR)
			SetDrawType(DRAW_NONE);

			// clip off the bad destination sections
			SrcRect.left   = xMAX(0, SrcRect.left);
			SrcRect.top    = xMAX(0, SrcRect.top);
			SrcRect.right  = xMIN(SW, SrcRect.right);
			SrcRect.bottom = xMIN(SH, SrcRect.bottom);

			// skip zero area
			if (SrcRect.left   - SrcRect.right    == 0) return;
			if (SrcRect.top    - SrcRect.bottom   == 0) return;
			if (InterRect.left - InterRect.right  == 0) return;
			if (InterRect.top  - InterRect.bottom == 0) return;

			// some stupid adjustments cus stretchrect doesnt keep the same number of pixels (area) depending on the ratio
			if (SrcRect.right   - SrcRect.left  != INT(Video->Width))  
			{ 
				InterRect.right  += 1;
				//SrcRect.right  += 1;
			}
			if (SrcRect.bottom  - SrcRect.top   != INT(Video->Height)) 
			{
				InterRect.bottom += 1;
				//SrcRect.bottom += 1;
			}
			//SrcRect.right  += 1;
			//SrcRect.bottom += 1;

			// clip off the bad destination sections
			SrcRect.left   = xMAX(0, SrcRect.left);
			SrcRect.top    = xMAX(0, SrcRect.top);
			SrcRect.right  = xMIN(SW, SrcRect.right);
			SrcRect.bottom = xMIN(SH, SrcRect.bottom);
			InterRect.left    = xMAX(0, InterRect.left );
			InterRect.top     = xMAX(0, InterRect.top  );
			InterRect.right   = xMIN(INT(Video->Width+1), InterRect.right );
			InterRect.bottom  = xMIN(INT(Video->Height+1), InterRect.bottom );

			// render from memory to vid, then from vid to streched dest
			// include a final check... this *USUALLY* prevents race conditions
			if (Video->Data.size() > 0 && Video->DataVid != NULL)
			{
                if (Video->bIsLocked == true)
                {
                    return; // prevent update surface if locked..
                }
                else
                {
                    // give it a try.... sometimes datavid can change???
                    try
                    {
                        if (bUpdateFromSystemMemory == true)
                        {
				            MICROSOFT::HRESULT UpdateSuccess = ParentEngine->DirectX9->Device->UpdateSurface(Video->Data[FrameNumber], NULL, Video->DataVid, NULL);
				            if (UpdateSuccess == D3D_OK)
                            {
                                MICROSOFT::HRESULT StretchResult = ParentEngine->DirectX9->Device->StretchRect(Video->DataVid, &InterRect, ParentEngine->DirectX9->DrawSurface, &SrcRect, ( (ScaleX+0.001) >= 1.0 ) ? D3DTEXF_POINT : D3DTEXF_LINEAR ); //
                                if (StretchResult == D3D_OK) 
                                {
                                    NOCODE; 
                                }
                                else
                                {
                                    ParentEngine->Console->Log(DEBUG_FATAL, "RAZOR::DRAW::Video(): StretchRect() (bUpdateFromSystemMemory=true) failed: %x != D3D_OK[%x]\n", StretchResult, D3D_OK);
                                }
                            }
                            else
                            {
                                ParentEngine->Console->Log(DEBUG_FATAL, "RAZOR::DRAW::Video(): UpdateSurface() (bUpdateFromSystemMemory=true) failed: %x != D3D_OK[%x]\n", UpdateSuccess, D3D_OK);
                            }
                        }
                        else
                        {
                            MICROSOFT::HRESULT StretchResult = ParentEngine->DirectX9->Device->StretchRect(Video->DataVid, &InterRect, ParentEngine->DirectX9->DrawSurface, &SrcRect, ((ScaleX + 0.001) >= 1.0) ? D3DTEXF_POINT : D3DTEXF_LINEAR); //
                            if (StretchResult == D3D_OK)
                            {
                                NOCODE;
                            }
                            else
                            {
                                ParentEngine->Console->Log(DEBUG_FATAL, "RAZOR::DRAW::Video(): StretchRect() (bUpdateFromSystemMemory=false) failed: %x != D3D_OK[%x]\n", StretchResult, D3D_OK);
                            }
                        }
                    }
                    catch(...)
                    {
                        ParentEngine->Console->Log(DEBUG_FATAL, "RAZOR::DRAW::Video(): Caught unhandled exception!!!");
                    }
                }
            }
			//DrawCount++;
		}
	}
}


// DRAW EngineX.Sprite colored tinted! w/ ALHPA 
void RAZOR::DRAW::Sprite(UINT32 ResourceIndex, INT32 x, INT32 y, DOUBLE ScaleX, DOUBLE ScaleY, RAZOR_COLOR ColorMix)
{
	SetDrawType(DRAW_SPRITE);
    RESOURCE_BITMAP* sprite = &this->ParentEngine->Resources->BitmapArray[ResourceIndex];
	RECT srcRect={0, 0, sprite->Width, sprite->Height};
    D3DXMATRIX scaleMatrix;
    D3DXMATRIX TransMatrix;
    D3DXMatrixScaling(&scaleMatrix, ScaleX, ScaleY, 1.0f);
    D3DXMatrixTranslation(&TransMatrix, FLOAT(x), FLOAT(y), FLOAT(0.0f)); 
    D3DXMATRIX FinalMatrix = scaleMatrix * TransMatrix;

    // do sprite render with rotation
    ParentEngine->DirectX9->Sprite->SetTransform(&FinalMatrix);
	ParentEngine->DirectX9->Sprite->Draw(sprite->VidBmp, &srcRect, NULL, NULL, ColorMix);
}

// DRAW EngineX.Sprite colored tinted! w/ ALHPA 
void RAZOR::DRAW::SpriteRotated(UINT32 ResourceIndex, INT32 x, INT32 y, DOUBLE ScaleX, DOUBLE ScaleY, FLOAT Cx, FLOAT Cy, FLOAT Rotation, RAZOR_COLOR ColorMix)
{
	SetDrawType(DRAW_SPRITE);
    RESOURCE_BITMAP* sprite = &this->ParentEngine->Resources->BitmapArray[ResourceIndex];
	RECT srcRect={0, 0, sprite->Width, sprite->Height};
    D3DXMATRIX scaleMatrix;
    D3DXMATRIX TransMatrix;
    D3DXMATRIX PreTransMatrix;
    D3DXMATRIX RotMatrix;
    D3DXMatrixScaling(&scaleMatrix, ScaleX, ScaleY, 1.0f);
    D3DXMatrixRotationZ(&RotMatrix, Rotation);
    D3DXMatrixTranslation(&PreTransMatrix, FLOAT(- Cx*sprite->Width), FLOAT(- Cy*sprite->Height), FLOAT(0.0f)); 
    D3DXMatrixTranslation(&TransMatrix, FLOAT(x + Cx*sprite->Width*ScaleX), FLOAT(y + Cy*sprite->Height*ScaleY), FLOAT(0.0f)); 
    D3DXMATRIX FinalMatrix = PreTransMatrix * RotMatrix * scaleMatrix * TransMatrix;

    // do sprite render with rotation
    ParentEngine->DirectX9->Sprite->SetTransform(&FinalMatrix);
	ParentEngine->DirectX9->Sprite->Draw(sprite->VidBmp, &srcRect, NULL, NULL, ColorMix);
}


// update from surface
void RAZOR::DRAW::UpdateSprite(UINT32 TextureID, UINT32 SurfaceID, UINT32 Frame)
{
    //NOVA::TIMER UpTime; UpTime.Tick();
    LPDIRECT3DSURFACE9 TextureSurface = NULL; // destination texture -> under surface
    LPDIRECT3DSURFACE9 RenderTarget = ParentEngine->Resources->VideoArray[SurfaceID]->Data[Frame];
    LPDIRECT3DTEXTURE9 SpriteTexture = ParentEngine->Resources->BitmapArray[TextureID].VidBmp;
    // update surface from texture
    SpriteTexture->GetSurfaceLevel(0, &TextureSurface);
    
    D3DXLoadSurfaceFromSurface (TextureSurface, NULL,NULL,RenderTarget,NULL,NULL,D3DX_DEFAULT,0);
    
    //printf("UpdateSprite took [%d %d] %lf ms\n", ParentEngine->Resources->BitmapArray[TextureID].Width, ParentEngine->Resources->BitmapArray[TextureID].Height, UpTime.Tock());
    TextureSurface->Release();

}

RECT RAZOR::DRAW::TextRotated(const STRING &Font, UINT32 Size, const CHAR *message, INT32 x, INT32 y, INT32 x2, INT32 y2, FLOAT Rotation, RAZOR_COLOR color, UINT32 Flags, BOOLEEN Clip, BOOLEEN Calculate)
{
	if (message == NULL) return {0, 0, 0, 0}; // fast break
    if (Size > 65536) return {0, 0, 0, 0};
	SetDrawType(DRAW_SPRITE);
    RECT FontPosition = {x, y, x2, y2};

    // check existance
    DIRECTX9::LPD3DXFONT TargetFont = ParentEngine->DirectX9->DefaultFont;
    if (Font != "") // use default
    {
        if( FontMap.find( {Size, Font} ) == FontMap.end() )
        {
            //ParentEngine->Console->Log(DEBUG_INFO, "RAZOR::DRAW::Text() Creating Font: '%s', size %d\n", Font.c_str(), Size);
            HRESULT FontCreated = DIRECTX9::D3DXCreateFontA(ParentEngine->DirectX9->Device, Size, 0, FW_NORMAL, 0, FALSE, DEFAULT_CHARSET, OUT_TT_PRECIS, PROOF_QUALITY, FIXED_PITCH | FF_MODERN, Font.c_str(),  &FontMap[ {Size, Font} ]);
            if (FontCreated != D3D_OK)
            {
                ParentEngine->Console->Log(DEBUG_WARN, "RAZOR::DRAW::TextRotated(): Cannot create font: size=%d\n", Size);
                return {0, 0, 0, 0};
            }
        }
        TargetFont = FontMap[ {Size, Font} ];
    }
    // add clip
    if (Clip == false) Flags |= DT_NOCLIP;

    // check clip settings
    if (Calculate == true)
    {
        RECT OutRect = {0, 0, 0, 0};
        TargetFont->DrawText(ParentEngine->DirectX9->Sprite, message, -1, &OutRect, Flags | DT_CALCRECT, color );
        return OutRect;
    }
    else
    {
        static FLOAT Ticker = 0.0;
        RECT OutRect = {0, 0, 0, 0};
        D3DXMATRIX TransMatrix;
        D3DXMATRIX RotMatrix;
        D3DXMATRIX FinalMatrix;
        D3DXMatrixTranslation(&TransMatrix, FLOAT(x), FLOAT(y), FLOAT(0.0f));
        D3DXMatrixRotationZ(&RotMatrix, Rotation);
        FinalMatrix = RotMatrix * TransMatrix;
        ParentEngine->DirectX9->Sprite->SetTransform(&FinalMatrix);
        
        TargetFont->DrawTextA(ParentEngine->DirectX9->Sprite, message, -1, &OutRect, Flags, color );
        return FontPosition;
    }

}

// Text rotated
RECT RAZOR::DRAW::TextRotated(const STRING &Font, UINT32 Size, INT32 x, INT32 y, FLOAT Rotation, RAZOR_COLOR color, UINT32 Flags, BOOLEEN Calculate, const CHAR *message, VAR_LIST Args)
{
    GLOBAL::vsprintf_s(TextBufferLarge, RAZOR_CONSOLE_BUFFER_SIZE, message, Args);
    return TextRotated(Font, Size, TextBufferLarge, x, y, x+10, y+10, Rotation, color, Flags, false, Calculate);
}

// Text rotated
RECT RAZOR::DRAW::TextRotated(INT32 x, INT32 y, FLOAT Rotation, RAZOR_COLOR color, UINT32 Flags, BOOLEEN Calculate, const CHAR *message, VAR_LIST Args)
{
    GLOBAL::vsprintf_s(TextBufferLarge, RAZOR_CONSOLE_BUFFER_SIZE, message, Args);
    return TextRotated("FixedSys", 8, TextBufferLarge, x, y, x+10, y+10, Rotation, color, Flags, false, Calculate);
}

// text
RECT RAZOR::DRAW::Text(const STRING &Font, UINT32 Size, const CHAR *message, INT32 x, INT32 y, INT32 x2, INT32 y2, RAZOR_COLOR color, UINT32 Flags, BOOLEEN Clip, BOOLEEN Calculate)
{
	if (message == NULL) return {0, 0, 0, 0}; // fast break
    if (Size > 65536) return {0, 0, 0, 0};
	SetDrawType(DRAW_SPRITE);
    RECT FontPosition = {x, y, x2, y2};

    // check existance
    DIRECTX9::LPD3DXFONT TargetFont = ParentEngine->DirectX9->DefaultFont;
    if (Font != "") // use default
    {
        if( FontMap.find( {Size, Font} ) == FontMap.end() )
        {
            //ParentEngine->Console->Log(DEBUG_INFO, "RAZOR::DRAW::Text() Creating Font: '%s', size %d\n", Font.c_str(), Size);
            HRESULT FontCreated = DIRECTX9::D3DXCreateFontA(ParentEngine->DirectX9->Device, Size, 0, FW_NORMAL, 0, FALSE, DEFAULT_CHARSET, OUT_TT_PRECIS, PROOF_QUALITY, FIXED_PITCH | FF_MODERN, Font.c_str(),  &FontMap[ {Size, Font} ]);
            if (FontCreated != D3D_OK)
            {
                ParentEngine->Console->Log(DEBUG_WARN, "RAZOR::DRAW::Text(): Cannot create font: size=%d\n", Size);
                return {0, 0, 0, 0};
            }
        }

        TargetFont = FontMap[ {Size, Font} ];
    }
    // add clip
    if (Clip == false) Flags |= DT_NOCLIP;

    // check clip settings
    if (Calculate == true)
    {
        RECT OutRect = {0, 0, 0, 0};
        TargetFont->DrawText(ParentEngine->DirectX9->Sprite, message, -1, &OutRect, Flags | DT_CALCRECT, color );
        return OutRect;
    }
    else
    {
        static FLOAT Ticker = 0.0;
        RECT OutRect = {0, 0, 0, 0};
        D3DXMATRIX TransMatrix;
        D3DXMatrixTranslation(&TransMatrix, FLOAT(x), FLOAT(y), FLOAT(0.0f));
        ParentEngine->DirectX9->Sprite->SetTransform(&TransMatrix);
        
        TargetFont->DrawTextA(ParentEngine->DirectX9->Sprite, message, -1, &OutRect, Flags, color );
        return FontPosition;
    }

}

// overloaded without secondary x y
RECT RAZOR::DRAW::Text(const STRING &Font, UINT32 Size, const CHAR *message, INT32 x, INT32 y, RAZOR_COLOR color, UINT32 Flags, BOOLEEN Calculate)
{
    return Text(Font, Size, message, x, y, x+10, y+10, color, Flags, false, Calculate);
}

// printf type
RECT RAZOR::DRAW::Text(const STRING &Font, UINT32 Size, INT32 x, INT32 y, RAZOR_COLOR color, UINT32 Flags, BOOLEEN Calculate, const CHAR *message, ...)
{
    // start variable arguments
    RAZOR::VAR_LIST args;
    va_start(args, message);
    RECT Bounds =  Text(Font, Size, x, y, color, Flags, Calculate, message, args);
    va_end(args);
    return Bounds;
}

// no font specified
RECT RAZOR::DRAW::Text( INT32 x, INT32 y, RAZOR_COLOR color, UINT32 Flags, BOOLEEN Calculate, const CHAR *message, ...)
{
    // start variable arguments
    RAZOR::VAR_LIST args;
    va_start(args, message);
    RECT Bounds =  Text(x, y, color, Flags, Calculate, message, args);
    va_end(args);
    return Bounds;
}

// printf type
RECT RAZOR::DRAW::Text(const STRING &Font, UINT32 Size, INT32 x, INT32 y, RAZOR_COLOR color, UINT32 Flags, BOOLEEN Calculate, const CHAR *message, VAR_LIST Args)
{
    GLOBAL::vsprintf_s(TextBufferLarge, RAZOR_CONSOLE_BUFFER_SIZE, message, Args);
    return Text(Font, Size, TextBufferLarge, x, y, color, Flags, Calculate);
}

// no font specified
RECT RAZOR::DRAW::Text( INT32 x, INT32 y, RAZOR_COLOR color, UINT32 Flags, BOOLEEN Calculate, const CHAR *message, VAR_LIST Args)
{
    GLOBAL::vsprintf_s(TextBufferLarge, RAZOR_CONSOLE_BUFFER_SIZE, message, Args);
    return Text("FixedSys", 8, TextBufferLarge, x, y, color, Flags, Calculate);
}
