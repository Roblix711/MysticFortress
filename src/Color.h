
// header gaurd
#ifndef _ENGINEXG3_COLOR_H
#define _ENGINEXG3_COLOR_H

// defines
#include <RazorDraw.hpp>

// main colors
extern const RAZOR::RAZOR_COLOR cOpaque;
extern const RAZOR::RAZOR_COLOR cBlack;
extern const RAZOR::RAZOR_COLOR cRed;
extern const RAZOR::RAZOR_COLOR cGreen;
extern const RAZOR::RAZOR_COLOR cBlue;
extern const RAZOR::RAZOR_COLOR cWhite;
extern const RAZOR::RAZOR_COLOR cCyan;
extern const RAZOR::RAZOR_COLOR cTeal;
extern const RAZOR::RAZOR_COLOR cMagenta;
extern const RAZOR::RAZOR_COLOR cPink;
extern const RAZOR::RAZOR_COLOR cYellow;

// medium
extern const RAZOR::RAZOR_COLOR cDRed;
extern const RAZOR::RAZOR_COLOR cDGreen;
extern const RAZOR::RAZOR_COLOR cDBlue;
extern const RAZOR::RAZOR_COLOR cGray;

// very dark
extern const RAZOR::RAZOR_COLOR cVDRed;
extern const RAZOR::RAZOR_COLOR cVDGreen;
extern const RAZOR::RAZOR_COLOR cVDBlue;
extern const RAZOR::RAZOR_COLOR cDGray;

// very very dark
extern const RAZOR::RAZOR_COLOR cVVDRed;
extern const RAZOR::RAZOR_COLOR cVVDGreen;
extern const RAZOR::RAZOR_COLOR cVVDBlue;
extern const RAZOR::RAZOR_COLOR cVDGray;

// light
extern const RAZOR::RAZOR_COLOR cLRed;
extern const RAZOR::RAZOR_COLOR cLGreen;
extern const RAZOR::RAZOR_COLOR cLBlue;
extern const RAZOR::RAZOR_COLOR cLGray;

// very light
extern const RAZOR::RAZOR_COLOR cVLRed;
extern const RAZOR::RAZOR_COLOR cVLGreen;
extern const RAZOR::RAZOR_COLOR cVLBlue;
extern const RAZOR::RAZOR_COLOR cVLGray;


// definitions for ease
RAZOR::RAZOR_COLOR ColorMakeRGBA(INT32 r, INT32 g, INT32 b, INT32 a = 255);   // RGBA colors
RAZOR::RAZOR_COLOR ColorMakeGray(INT32 r, INT32 a = 255);                     // grayscale
RAZOR::RAZOR_COLOR ColorAdjustAlpha(RAZOR::RAZOR_COLOR C, INT32 a = 255);     // adjust to a new alpha
RAZOR::RAZOR_COLOR ColorErase(RAZOR::RAZOR_COLOR C);     // the inverse
RAZOR::RAZOR_COLOR ColorBasicPallete(UINT32 Index);
RAZOR::RAZOR_COLOR ColorMakeFromHSV(FLOAT H, FLOAT S, FLOAT V, FLOAT A = 1.0);

void RGBtoHSV(float fR, float fG, float fB, float& fH, float& fS, float& fV);
void HSVtoRGB(float& fR, float& fG, float& fB, float fH, float fS, float fV);


#endif