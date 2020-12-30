/*!
    \file
    \brief flipflip's Album Art Display: wifi and network things (see \ref FF_WIFI)

    - Copyright (c) 2018 Philippe Kehl (flipflip at oinkzwurgl dot org),
      https://oinkzwurgl.org/projaeggd/album-art-display
*/

#if defined(ESP8266)
#  include <ESP8266WiFi.h>
#  include <ESP8266WiFiMulti.h>
#  include <ESP8266HTTPClient.h>
#  include <WiFiClient.h>
#  include <WiFiClientSecure.h>
#elif defined(ESP32)
#  include <WiFi.h>
#  include <WiFiMulti.h>
#endif

#include "stuff.h"
#include "config.h"
#include "secrets.h"

#include "debug.h"
#include "status.h"

#include "wifi.h"

#if defined(ESP8266)
static ESP8266WiFiMulti wifiMulti;
#elif defined(ESP32)
static WiFiMulti wifiMulti;
#endif
static char sClientName[8];
static char sUserAgent[100];

static const char *sWifiWlStatusStr(const wl_status_t status)
{
    switch (status)
    {
        case WL_NO_SHIELD:       return PSTR("NO_SHIELD");
        case WL_IDLE_STATUS:     return PSTR("IDLE_STATUS");
        case WL_NO_SSID_AVAIL:   return PSTR("NO_SSID_AVAIL");
        case WL_SCAN_COMPLETED:  return PSTR("CAN_COMPLETED");
        case WL_CONNECTED:       return PSTR("CONNECTED");
        case WL_CONNECT_FAILED:  return PSTR("CONNECT_FAILED");
        case WL_CONNECTION_LOST: return PSTR("CONNECTION_LOST");
        case WL_DISCONNECTED:    return PSTR("DISCONNECTED");
        default:                 return PSTR("???");
    }
}


void sWifiMon(void)
{
    DEBUG("mon: wifi: status=%s, ssid=%s, rssi=%d, client=%s",
        sWifiWlStatusStr(WiFi.status()), WiFi.SSID().c_str(), WiFi.RSSI(), sClientName);
    DEBUG("mon: wifi: ip=%s, mask=%s, gw=%s, dns=%s",
        WiFi.localIP().toString().c_str(),
        WiFi.subnetMask().toString().c_str(),
        WiFi.gatewayIP().toString().c_str(),
        WiFi.dnsIP().toString().c_str());
}


void wifiInit(void)
{
    WiFi.mode(WIFI_STA);
    
    snprintf(sClientName, sizeof(sClientName), "%06x",
#if defined(ESP8266)
         (uint32_t)ESP.getChipId() & 0x00ffffff);
#elif defined (ESP32)
         (uint32_t)ESP.getEfuseMac() & 0x00ffffff);
#endif
    
    char staName[33];
    snprintf(staName, NUMOF(staName), PSTR("tschenggins-laempli-%s"), sClientName);

#if defined(ESP8266)
    WiFi.hostname(staName);
#elif defined (ESP32)
    WiFi.setHostname(staName);
#endif

    DEBUG("wifi: init (staName=%s, staMac=%s, ssid=" SECRET_WIFI_SSID ", ssid2="
#ifdef SECRET_WIFI2_SSID
        SECRET_WIFI2_SSID
#else
        "n/a"
#endif
        ", ssid3="
#ifdef SECRET_WIFI3_SSID
        SECRET_WIFI3_SSID
#else
        "n/a"
#endif
        ")", staName, WiFi.macAddress().c_str());
    debugRegisterMon(sWifiMon);
    
    //WiFi.setAutoConnect(true);
    //WiFi.setAutoReconnect(true);
    //WiFi.begin();

    WiFi.setAutoConnect(false);
    WiFi.setAutoReconnect(false);

    wifiMulti.addAP(SECRET_WIFI_SSID, SECRET_WIFI_PASS);
#if defined(SECRET_WIFI2_SSID) && defined(SECRET_WIFI2_PASS)
    wifiMulti.addAP(SECRET_WIFI2_SSID, SECRET_WIFI2_PASS);
#endif
#if defined(SECRET_WIFI3_SSID) && defined(SECRET_WIFI3_PASS)
    wifiMulti.addAP(SECRET_WIFI3_SSID, SECRET_WIFI3_PASS);
#endif

    snprintf(sUserAgent, NUMOF(sUserAgent), PSTR("album-art-display/" CONFIG_SOFTWARE_VERSION
        " (" CONFIG_VERSION_GIT_HASH "; " CONFIG_PLATFORM_NAME "; %s; " CONFIG_VERSION_YYYYMMDD 
        "; " CONFIG_VERSION_HHMMSS ")"), sClientName);
    DEBUG("wifi: http user agent=%s", sUserAgent);
}


static wl_status_t sWifiStatus = (wl_status_t)254; 

bool wifiIsConnected(void)
{
#if defined(ESP8266)
    const wl_status_t status = wifiMulti.run();
#elif defined(ESP32)
    const wl_status_t status = (wl_status_t)wifiMulti.run();
#endif
    return status == WL_CONNECTED;
}

bool wifiWaitForConnect(void)
{
#if defined(ESP8266)
    const wl_status_t status = wifiMulti.run();
#elif defined(ESP32)
    const wl_status_t status = (wl_status_t)wifiMulti.run();
#endif
    if (status != sWifiStatus)
    {
        DEBUG("wifi: status %s -> %s", sWifiWlStatusStr(sWifiStatus), sWifiWlStatusStr(status));
        sWifiStatus = status;
        switch (status)
        {
            case WL_CONNECTED:
                statusNoise(STATUS_NOISE_ONLINE);
                break;
            case WL_DISCONNECTED:
            case WL_NO_SSID_AVAIL:
            case WL_CONNECT_FAILED:
            case WL_CONNECTION_LOST:
                statusNoise(STATUS_NOISE_ABORT);
                statusLed(STATUS_LED_OFFLINE);
                break;
            default:
                statusLed(STATUS_LED_OFFLINE);
                break;
        }
    }
    return status == WL_CONNECTED;
}

const char *wifiUserAgentStr(void)
{
    return sUserAgent;
}

// eof
