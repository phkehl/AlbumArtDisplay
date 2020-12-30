/*!
    \file
    \brief flipflip's Album Art Display: documentation (see \ref mainpage)

    - Copyright (c) 2020 Philippe Kehl (flipflip at oinkzwurgl dot org),
      https://oinkzwurgl.org/projaeggd/album-art-display
*/

#define DO_TESTS 0

/* ********************************************************************************************** */

#include <Arduino.h>
#if !defined(ESP32)
#  error Unsupported target!
#endif

#include <core_version.h>

#include "src/stuff.h"
#include "src/config.h"
#include "src/debug.h"
#include "src/status.h"
#include "src/display.h"
#include "src/wifi.h"
#include "src/gifs.h"
#include "src/lms.h"

/* ********************************************************************************************** */

void setup()
{
    // Initialise stuff
    debugInit();
    statusInit();
    displayInit();
    randomSeed(ESP.getCycleCount());

    // Say hello
    NOTICE("--------------------------------------------------------------------------------------------");
    NOTICE("Album Art Display " CONFIG_SOFTWARE_VERSION
        " (" CONFIG_VERSION_GIT_HASH ", " CONFIG_PLATFORM_NAME ", " CONFIG_VERSION_YYYYMMDD " " CONFIG_VERSION_HHMMSS ")");
    NOTICE("Copyright (c) 2020 Philippe Kehl & flipflip industries <flipflip at oinkzwurgl dot org>");
    NOTICE("Parts copyright by others. See source code.");
    NOTICE("https://oinkzwurgl.org/projaeggd/album-art-display");
    NOTICE("--------------------------------------------------------------------------------------------");
#if defined(ESP32)
    DEBUG("ESP32: rev=0x%02x, cpu=%uMHz", ESP.getChipRevision(), ESP.getCpuFreqMHz());
    DEBUG("ESP32: %s, Arduino %s", ESP.getSdkVersion(), ARDUINO_ESP32_RELEASE);
    DEBUG("ESP32: flash: size=%u (%uKiB)", ESP.getFlashChipSize(), ESP.getFlashChipSize() >> 10);
    DEBUG("ESP32: heap: minFree=%u (%u), maxBlock=%u (%u), free=%u (%u)",
        heap_caps_get_minimum_free_size(MALLOC_CAP_DEFAULT), ESP.getMinFreeHeap(),
        heap_caps_get_largest_free_block(MALLOC_CAP_DEFAULT), ESP.getMaxAllocHeap(),
        heap_caps_get_free_size(MALLOC_CAP_DEFAULT), ESP.getFreeHeap());
#endif
    DEBUG("GCC: " __VERSION__); // /*", FreeRTOS " tskKERNEL_VERSION_NUMBER */ 
    //DEBUG("Arduino: sketch=%u (%uKiB), free=%u (%uKiB)",
    //    ESP.getSketchSize(), ESP.getSketchSize() >> 10,
    //    ESP.getFreeSketchSpace(), ESP.getFreeSketchSpace() >> 10);

    // Initialise more things
    wifiInit();
    lmsInit();

#if DO_TESTS == 0
    displayNoise(true);
#endif

    gifsInit();

#if 0
    while (true)
    {
        const uint32_t t0 = millis();
        const char *gif = gifsGetNext();
        DEBUG("next is: %s", gif);
        displayGif(gif);
        while ((millis() - t0) < 5000)
        {
            DEBUG("playing...");
            delay(1000);
        }
    }
#endif

    NOTICE("Here we go...");
    statusLed(STATUS_LED_OFFLINE);
    statusNoise(STATUS_NOISE_FFI);
    delay(1000);

    pinMode(CONFIG_BUTTON_PIN, INPUT_PULLUP);
}

typedef enum LOOP_e
{
    LOOP_NOISE, LOOP_GIF, LOOP_COVER

} LOOP_t;

void loop()
{
#if DO_TESTS == 0
    static int numFail;
    displayNoise(true);
    LOOP_t loopMode = LOOP_NOISE;

    // Wait for WiFi connection, play noise until online
    if (!wifiIsConnected())
    {
        PRINT("Waiting for wifi...");
        while (!wifiWaitForConnect())
        {
            delay(100);
        }
        return;
    }
    statusLed(STATUS_LED_UPDATE);

    if (!lmsConnect())
    {
        delay(1000);
        return;
    }

    static String coverArtPlayerId;
    LMS_STATE_t state = LMS_STATE_STOPPED;
    uint32_t lastGifChange;
    bool doGetCoverArt = false;
    bool doChangeGif = false;
    while (state != LMS_STATE_FAIL)
    {
        // Changes in LMS state can change the display
        const LMS_STATE_t newState = lmsLoop(coverArtPlayerId);
        if (state != newState)
        {
            DEBUG("State change: %d -> %d", state, newState);
        }
        switch (newState)
        {
            case LMS_STATE_FAIL:
                break;
            case LMS_STATE_STOPPED:
                if (state != LMS_STATE_STOPPED)
                {
                    PRINT("Now stopped");
                    doChangeGif = true;
                }
                break;
            case LMS_STATE_PLAYING:
                if (state != LMS_STATE_PLAYING)
                {
                    PRINT("Now playing");
                }
                if (coverArtPlayerId.length() > 0)
                {
                    doGetCoverArt = true;
                }
                break;

        }
        state = newState;

        // Button press can change the display
        static uint32_t buttonDown;
        if (digitalRead(CONFIG_BUTTON_PIN) == 0)
        {
            if (buttonDown == 0)
            {
                buttonDown = millis();
            }
        }
        else
        {
            if ( (buttonDown != 0) && ((millis() - buttonDown) > 100) )
            {
                buttonDown = 0;
                statusNoise(STATUS_NOISE_TICK);
                doChangeGif = true;
            }
        }

        // Time can change the display
        switch (loopMode)
        {
            case LOOP_GIF:
                if ( (millis() - lastGifChange) > 60000 )
                {
                    doChangeGif = true;
                }
                break;
            case LOOP_COVER:
                break;
            case LOOP_NOISE:
                break;
        }

        // Change display
        if (doChangeGif)
        {
            const int r = random(10);
            PRINT("New gif (%d)", r);
            if (r < 1)
            {
                displayRGBerset(true);
            }
            else if (r < 2)
            {
                displayNyan(true);
            }
            else
            {
                displayGif(gifsGetRandom());
            }
            doChangeGif = false;
            lastGifChange = millis();
            loopMode = LOOP_GIF;
        }
        if (doGetCoverArt)
        {
            PRINT("new coverart");
            if (!displayCoverArt(coverArtPlayerId.c_str()))
            {
                statusNoise(STATUS_NOISE_FAIL);
            }
            doGetCoverArt = false;
            loopMode = LOOP_COVER;
        }

        // Wait a bit...
        delay(99);

    } // while connected to LMS CLI...

    ERROR("No longer connected...");
    displayNoise(true);
    statusLed(STATUS_LED_FAIL);
    statusNoise(STATUS_NOISE_ERROR);
    delay(5000);

    numFail++;
    if (numFail >= 5)
    {
        statusNoise(STATUS_NOISE_BOMB);
        delay(1000);
        ESP.restart();
    }
#else

    displayTest();

#endif
}