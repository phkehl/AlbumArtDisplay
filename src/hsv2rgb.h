/*!
    \file
    \brief flipflip's Album Art Display:  HSV to RGB conversion (see \ref FF_HSV2RGB)

    - Copyright (c) 2020 Philippe Kehl & flipflip industries (flipflip at oinkzwurgl dot org),
      https://oinkzwurgl.org/projaeggd/u-clox

    \defgroup FF_HSV2RGB HSV2RGB
    \ingroup FF

    This implements hue, saturation and value (HSV) to red, green and blue (RGB) conversion.

    @{
*/
#ifndef __HSV2RGB_H__
#define __HSV2RGB_H__

#include <stdint.h>

//! hue, saturation and value (HSV) to red, green, blue (RGB) conversion
/*!
    \param[in]  H  hue value (scaled 0..255)
    \param[in]  S  saturation value (scaled 0..255)
    \param[in]  V  "brightness" value (scaled 0..255)
    \param[out] R  red (0..255)
    \param[out] G  green (0..255)
    \param[out] B  blue (0..255)
*/
void hsv2rgb(const uint8_t H, const uint8_t S, uint8_t V, uint8_t *R, uint8_t *G, uint8_t *B);


#endif // __HSV2RGB_H__
//@}
// eof
