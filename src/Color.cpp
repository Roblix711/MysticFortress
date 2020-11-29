
// our main header
#include "Color.h"


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

// medium
const RAZOR_COLOR cDRed     = (cBlack + (127<<16));
const RAZOR_COLOR cDGreen   = (cBlack + (127<<8));
const RAZOR_COLOR cDBlue    = (cBlack + 127);
const RAZOR_COLOR cGray   = (cDRed | cDGreen | cDBlue);

// very dark
const RAZOR_COLOR cVDRed     = (cBlack + (32<<16));
const RAZOR_COLOR cVDGreen   = (cBlack + (32<<8));
const RAZOR_COLOR cVDBlue    = (cBlack + 32);
const RAZOR_COLOR cDGray   = (cVDRed | cVDGreen | cVDBlue);

// very very dark
const RAZOR_COLOR cVVDRed     = (cBlack + (8<<16));
const RAZOR_COLOR cVVDGreen   = (cBlack + (8<<8));
const RAZOR_COLOR cVVDBlue    = (cBlack + 8);
const RAZOR_COLOR cVDGray   = (cVVDRed | cVVDGreen | cVVDBlue);

// light
const RAZOR_COLOR cLRed     = (cBlack + (196<<16));
const RAZOR_COLOR cLGreen   = (cBlack + (196<<8));
const RAZOR_COLOR cLBlue    = (cBlack + 196);
const RAZOR_COLOR cLGray   = (cLRed | cLGreen | cLBlue);

// very light
const RAZOR_COLOR cVLRed     = (cBlack + (224<<16));
const RAZOR_COLOR cVLGreen   = (cBlack + (224<<8));
const RAZOR_COLOR cVLBlue    = (cBlack + 224);
const RAZOR_COLOR cVLGray   = (cVLRed | cVLGreen | cVLBlue);

// definitions for ease
RAZOR_COLOR ColorMakeRGBA(INT32 r, INT32 g, INT32 b, INT32 a) 
{
	return  BYTE(b) + (BYTE(g)<<8) + (BYTE(r)<<16) + (BYTE(a)<<24);
}

// grayscale 
RAZOR_COLOR ColorMakeGray(INT32 r, INT32 a) 
{
	return  ColorMakeRGBA(r, r, r, a);
}

// adjust to a new alpha
RAZOR::RAZOR_COLOR ColorAdjustAlpha(RAZOR::RAZOR_COLOR C, INT32 a)
{
    return (C & ((BYTE(1)<<24)-1)) | (BYTE(a)<<24);
}

// erase color
RAZOR::RAZOR_COLOR ColorErase(RAZOR::RAZOR_COLOR C)
{
    return ColorAdjustAlpha(C, 255-(C >> 24));
}

RAZOR::RAZOR_COLOR ColorBasicPallete(UINT32 Index)
{
    switch(Index)
    {
        case 0: return cDRed;
        case 1: return cDGreen;
        case 2: return cDBlue;
        default: return cWhite;

    };

    /*RAZOR_COLOR Out = BYTE(255)<<24;
    INT32 NumBases = 1 + rand()%2;
    for(INT32 i=0; i<NumBases; ++i)
    {
        UINT32 Scaler = 1 + rand()%((256-32)/32);
        Out |= (31 * Scaler) << (i*8);
    }
    return Out;*/
}



/*! \brief Convert RGB to HSV color space
  
  Converts a given set of RGB values `r', `g', `b' into HSV
  coordinates. The input RGB values are in the range [0, 1], and the
  output HSV values are in the ranges h = [0, 360], and s, v = [0,
  1], respectively.
  
  \param fR Red component, used as input, range: [0, 1]
  \param fG Green component, used as input, range: [0, 1]
  \param fB Blue component, used as input, range: [0, 1]
  \param fH Hue component, used as output, range: [0, 360]
  \param fS Hue component, used as output, range: [0, 1]
  \param fV Hue component, used as output, range: [0, 1]
  
*/
void RGBtoHSV(float fR, float fG, float fB, float& fH, float& fS, float& fV) 
{
  float fCMax = std::max(std::max(fR, fG), fB);
  float fCMin = std::min(std::min(fR, fG), fB);
  float fDelta = fCMax - fCMin;
  
  if(fDelta > 0) {
    if(fCMax == fR) {
      fH = 60 * (fmod(((fG - fB) / fDelta), 6));
    } else if(fCMax == fG) {
      fH = 60 * (((fB - fR) / fDelta) + 2);
    } else if(fCMax == fB) {
      fH = 60 * (((fR - fG) / fDelta) + 4);
    }
    
    if(fCMax > 0) {
      fS = fDelta / fCMax;
    } else {
      fS = 0;
    }
    
    fV = fCMax;
  } else {
    fH = 0;
    fS = 0;
    fV = fCMax;
  }
  
  if(fH < 0) {
    fH = 360 + fH;
  }
}


/*! \brief Convert HSV to RGB color space
  
  Converts a given set of HSV values `h', `s', `v' into RGB
  coordinates. The output RGB values are in the range [0, 1], and
  the input HSV values are in the ranges h = [0, 360], and s, v =
  [0, 1], respectively.
  
  \param fR Red component, used as output, range: [0, 1]
  \param fG Green component, used as output, range: [0, 1]
  \param fB Blue component, used as output, range: [0, 1]
  \param fH Hue component, used as input, range: [0, 360]
  \param fS Hue component, used as input, range: [0, 1]
  \param fV Hue component, used as input, range: [0, 1]
  
*/
void HSVtoRGB(float& fR, float& fG, float& fB, float fH, float fS, float fV) 
{
  float fC = fV * fS; // Chroma
  float fHPrime = fmod(fH / 60.0, 6);
  float fX = fC * (1 - fabs(fmod(fHPrime, 2) - 1));
  float fM = fV - fC;
  
  if(0 <= fHPrime && fHPrime < 1) {
    fR = fC;
    fG = fX;
    fB = 0;
  } else if(1 <= fHPrime && fHPrime < 2) {
    fR = fX;
    fG = fC;
    fB = 0;
  } else if(2 <= fHPrime && fHPrime < 3) {
    fR = 0;
    fG = fC;
    fB = fX;
  } else if(3 <= fHPrime && fHPrime < 4) {
    fR = 0;
    fG = fX;
    fB = fC;
  } else if(4 <= fHPrime && fHPrime < 5) {
    fR = fX;
    fG = 0;
    fB = fC;
  } else if(5 <= fHPrime && fHPrime < 6) {
    fR = fC;
    fG = 0;
    fB = fX;
  } else {
    fR = 0;
    fG = 0;
    fB = 0;
  }
  
  fR += fM;
  fG += fM;
  fB += fM;
}


// create HSV color
RAZOR::RAZOR_COLOR ColorMakeFromHSV(FLOAT H, FLOAT S, FLOAT V, FLOAT A)
{
    FLOAT R,G,B;
    HSVtoRGB(R, G, B, H, S, V);
    return ColorMakeRGBA(R*255, B*255, G*255, A*255);
}