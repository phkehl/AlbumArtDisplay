/*!
    \file
    \brief flipflip's Album Art Display: GIFs (see \ref FF_GIFS)

    - Copyright (c) 2020 Philippe Kehl (flipflip at oinkzwurgl dot org),
      https://oinkzwurgl.org/projaeggd/album-art-display

    \defgroup FF_GIFS GIFS
    \ingroup FF

    @{
*/
#ifndef __GIFS_H__
#define __GIFS_H__

#include <Arduino.h>

void gifsInit(void);

const char *gifsGetRandom(void);
const char *gifsGetNext(void);

#endif // __GIFS_H__
//@}
// eof
