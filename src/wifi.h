/*!
    \file
    \brief flipflip's Album Art Display: wifi and network things  (see \ref WIFI)

    - Copyright (c) 2020 Philippe Kehl (flipflip at oinkzwurgl dot org),
      https://oinkzwurgl.org/projaeggd/album-art-display

    \defgroup FF_WIFI WIFI
    \ingroup FF

    @{
*/
#ifndef __WIFI_H__
#define __WIFI_H__

#include <Arduino.h>

//! initialise
void wifiInit(void);

bool wifiIsConnected(void);

bool wifiWaitForConnect(void);

const char *wifiUserAgentStr(void);

#endif // __WIFI_H__
//@}
// eof
