/*!
    \file
    \brief flipflip's Album Art Display: debugging output (see \ref FF_DEBUG)

    - Copyright (c) 2020 Philippe Kehl (flipflip at oinkzwurgl dot org),
      https://oinkzwurgl.org/projaeggd/album-art-display
    
    \defgroup FF_DEBUG DEBUG
    \ingroup FF

    @{
*/
#ifndef __DEBUG_H__
#define __DEBUG_H__

#include <Arduino.h>

//! initialise debugging output
void debugInit(void);

//! debug monitor function type
typedef void (*DEBUG_MON_FUNC_t)(void);

//! register monitor function
void debugRegisterMon(DEBUG_MON_FUNC_t monFunc);

//! print an error message \hideinitializer
#define ERROR(fmt, ...)   Serial.printf_P(PSTR("%09.3f E: " fmt "\r\n"), millis() * 1e-3, ## __VA_ARGS__)

//! print a warning message \hideinitializer
#define WARNING(fmt, ...) Serial.printf_P(PSTR("%09.3f W: " fmt "\r\n"), millis() * 1e-3, ## __VA_ARGS__)

//! print a notice \hideinitiali.printf_P
#define NOTICE(fmt, ...)  Serial.printf_P(PSTR("%09.3f N: " fmt "\r\n"), millis() * 1e-3, ## __VA_ARGS__)

//! print a normal message \hideinitializer
#define PRINT(fmt, ...)   Serial.printf_P(PSTR("%09.3f P: " fmt "\r\n"), millis() * 1e-3, ## __VA_ARGS__)

//! print a debug message \hideinitializer
#define DEBUG(fmt, ...)   Serial.printf_P(PSTR("%09.3f D: " fmt "\r\n"), millis() * 1e-3, ## __VA_ARGS__)

//! hex dump data
void HEXDUMP(const void *pkData, int size);

//! flush all debug output \hideinitializer
#define FLUSH()  Serial.flush()

#endif // __DEBUG_H__
//@}
// eof
