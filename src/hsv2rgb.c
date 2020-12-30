/*!
    \file
    \brief flipflip's Album Art Display: HSV to RGB conversion (see \ref FF_HSV2RGB)

    - Copyright (c) 2020 Philippe Kehl & flipflip industries (flipflip at oinkzwurgl dot org),
      https://oinkzwurgl.org/projaeggd/u-clox
*/

#include "hsv2rgb.h"


#define HSV2RGB_METHOD 2

/* ***** HSV to RGB conversion ******************************************************************* */

#if ( (HSV2RGB_METHOD == 1) || (HSV2RGB_METHOD == 2) )
// classic HSV2RGB code (#HSV2RGB_METHOD 1 and 2) à la Wikipedia
#  define HSV2RGB_CLASSIC(_H, _S, _V, _R, _G, _B) \
    const uint8_t __H = _H; \
    const uint8_t __S = _S; \
    const uint8_t __V = _V; \
    const uint32_t s = (6 * (uint32_t)__H) >> 8;               /* the segment 0..5 (360/60 * [0..255] / 256) */ \
    const uint32_t t = (6 * (uint32_t)__H) & 0xff;             /* within the segment 0..255 (360/60 * [0..255] % 256) */ \
    const uint32_t l = ((uint32_t)__V * (255 - (uint32_t)__S)) >> 8; /* lower level */ \
    const uint32_t r = ((uint32_t)__V * (uint32_t)__S * t) >> 16;    /* ramp */ \
    switch (s) \
    { \
        case 0: (_R) = (uint8_t)__V;        (_G) = (uint8_t)(l + r);    (_B) = (uint8_t)l;          break; \
        case 1: (_R) = (uint8_t)(__V - r);  (_G) = (uint8_t)__V;        (_B) = (uint8_t)l;          break; \
        case 2: (_R) = (uint8_t)l;          (_G) = (uint8_t)__V;        (_B) = (uint8_t)(l + r);    break; \
        case 3: (_R) = (uint8_t)l;          (_G) = (uint8_t)(__V - r);  (_B) = (uint8_t)__V;        break; \
        case 4: (_R) = (uint8_t)(l + r);    (_G) = (uint8_t)l;          (_B) = (uint8_t)__V;        break; \
        case 5: (_R) = (uint8_t)__V;        (_G) = (uint8_t)l;          (_B) = (uint8_t)(__V - r);  break; \
    }
#endif

// ***** classic conversion *****
#if (HSV2RGB_METHOD == 1)


void hsv2rgb(const uint8_t H, const uint8_t S, uint8_t V, uint8_t *R, uint8_t *G, uint8_t *B)
{
    HSV2RGB_CLASSIC(H, S, V, *R, *G, *B);
}

// ***** saturation/value dimming *****
#elif (HSV2RGB_METHOD == 2)

// saturation/value lookup table
// Saturation/Value lookup table to compensate for the nonlinearity of human
// vision. Used in the getRGB function on saturation and brightness to make
// dimming look more natural. Exponential function used to create values below
// : x from 0 - 255 : y = round(pow( 2.0, x+64/40.0) - 1)
// From: http://www.kasperkamperman.com/blog/arduino/arduino-programming-hsb-to-rgb/
static const uint8_t skMatrixDimCurve[] = // RAM
{
      0,   1,   1,   2,   2,   2,   2,   2,   2,   3,   3,   3,   3,   3,   3,   3,
      3,   3,   3,   3,   3,   3,   3,   4,   4,   4,   4,   4,   4,   4,   4,   4,
      4,   4,   4,   5,   5,   5,   5,   5,   5,   5,   5,   5,   5,   6,   6,   6,
      6,   6,   6,   6,   6,   7,   7,   7,   7,   7,   7,   7,   8,   8,   8,   8,
      8,   8,   9,   9,   9,   9,   9,   9,  10,  10,  10,  10,  10,  11,  11,  11,
     11,  11,  12,  12,  12,  12,  12,  13,  13,  13,  13,  14,  14,  14,  14,  15,
     15,  15,  16,  16,  16,  16,  17,  17,  17,  18,  18,  18,  19,  19,  19,  20,
     20,  20,  21,  21,  22,  22,  22,  23,  23,  24,  24,  25,  25,  25,  26,  26,
     27,  27,  28,  28,  29,  29,  30,  30,  31,  32,  32,  33,  33,  34,  35,  35,
     36,  36,  37,  38,  38,  39,  40,  40,  41,  42,  43,  43,  44,  45,  46,  47,
     48,  48,  49,  50,  51,  52,  53,  54,  55,  56,  57,  58,  59,  60,  61,  62,
     63,  64,  65,  66,  68,  69,  70,  71,  73,  74,  75,  76,  78,  79,  81,  82,
     83,  85,  86,  88,  90,  91,  93,  94,  96,  98,  99, 101, 103, 105, 107, 109,
    110, 112, 114, 116, 118, 121, 123, 125, 127, 129, 132, 134, 136, 139, 141, 144,
    146, 149, 151, 154, 157, 159, 162, 165, 168, 171, 174, 177, 180, 183, 186, 190,
    193, 196, 200, 203, 207, 211, 214, 218, 222, 226, 230, 234, 238, 242, 248, 255
};


void hsv2rgb(const uint8_t H, const uint8_t S, uint8_t V, uint8_t *R, uint8_t *G, uint8_t *B)
{
    HSV2RGB_CLASSIC(H, 255 - skMatrixDimCurve[255 - S], skMatrixDimCurve[V],
        *R, *G, *B);
}


#else
#  error Illegal value for HSV2RGB_METHOD!
#endif



/* *********************************************************************************************** */

// eof
