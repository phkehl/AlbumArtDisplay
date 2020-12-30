/*!
    \file
    \brief flipflip's Album Art Display: LMS CLI (see \ref LMS)

    - Copyright (c) 2020 Philippe Kehl (flipflip at oinkzwurgl dot org),
      https://oinkzwurgl.org/projaeggd/album-art-display

    \defgroup FF_LMS LMS
    \ingroup FF

    @{
*/
#ifndef __LMS_H__
#define __LMS_H__

#include <Arduino.h>

//! initialise
void lmsInit(void);

typedef enum LMS_STATE_e
{
    LMS_STATE_STOPPED,
    LMS_STATE_PLAYING,
    LMS_STATE_FAIL,

} LMS_STATE_t;

bool lmsConnect(void);

LMS_STATE_t lmsLoop(String &coverArtPlayerId);

//LMS_STATE_t checkLmsCli();


#endif // __LMS_H__
//@}
// eof
