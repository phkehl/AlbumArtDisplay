/*!
    \file
    \brief flipflip's Album Art Display: LED display (see \ref FF_DISPLAY)

    - Copyright (c) 2020 Philippe Kehl (flipflip at oinkzwurgl dot org),
      https://oinkzwurgl.org/projaeggd/album-art-display

    \defgroup FF_DISPLAY DISPLAY
    \ingroup FF

    @{
*/
#ifndef __DISPLAY_H__
#define __DISPLAY_H__

#include <Arduino.h>

#include "leddisplay.h"

//! initialise
void displayInit(void);

void displayNoise(const bool enable);

void displayNyan(const bool enable);

bool displayCoverArt(const char *playerId);

void displayTest(void);

void displayRGBerset(const bool enable);

void displayGif(const char *file);
bool displayGifOk(void);

#endif // __DISPLAY_H__
//@}
// eof
