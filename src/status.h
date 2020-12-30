/*!
    \file
    \brief flipflip's Album Art Display: system status (noises and LED) (see \ref FF_STATUS)

    - Copyright (c) 2020 Philippe Kehl (flipflip at oinkzwurgl dot org),
      https://oinkzwurgl.org/projaeggd/album-art-display

    \defgroup FF_STATUS STATUS
    \ingroup FF

    @{
*/
#ifndef __STATUS_H__
#define __STATUS_H__

#include <Arduino.h>

//! initialise
void statusInit(void);

//! status display modes
typedef enum STATUS_LED_e
{
    STATUS_LED_NONE,       //!< LED off                                                    --------------------
    STATUS_LED_HEARTBEAT,  //!< LED doing a heart beat style blinking (2 seconds period)   *-*-----------------
    STATUS_LED_OFFLINE,    //!< one blink every two seconds                                *-------------------
    STATUS_LED_FAIL,       //!< bursts of five fast blinks every two seconds               *-*-*-*-*-----------
    STATUS_LED_UPDATE,     //!< fast blinking while things are updating (or so)            *-*-*-*-*-*-*-*-*-*-

} STATUS_LED_t;

void statusLed(const STATUS_LED_t status);

typedef enum STATUS_NOISE_e
{
    STATUS_NOISE_ABORT,
    STATUS_NOISE_FAIL,
    STATUS_NOISE_ONLINE,
    STATUS_NOISE_OTHER,
    STATUS_NOISE_TICK,
    STATUS_NOISE_ERROR,
    STATUS_NOISE_BOMB,
    STATUS_NOISE_FFI,
} STATUS_NOISE_t;

void statusNoise(const STATUS_NOISE_t noise);

void statusMelody(const char *name);

void statusToneStop(void);

bool statusTonePlaying(void);

#endif // __STATUS_H__
//@}
// eof
