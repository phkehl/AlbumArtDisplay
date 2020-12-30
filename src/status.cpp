/*!
    \file
    \brief flipflip's Album Art Display: system status (noises and LED) (see \ref FF_STATUS)

    - Copyright (c) 2018 Philippe Kehl (flipflip at oinkzwurgl dot org),
      https://oinkzwurgl.org/projaeggd/album-art-display
*/

#include <Ticker.h>

#include "stuff.h"
#include "config.h"

#include "rtttl.h"
#include "debug.h"
#include "status.h"

static Ticker sLedTicker;

static Ticker sToneTicker;
static int sMelodyIx;
static const int16_t *skMelody;
static int16_t sMelodyDur;
static bool sToneIsPlaying;

static uint8_t sLedPeriod;
static uint8_t sLedNumBlink;

#define LED_ON  (CONFIG_STATUS_LED_POL == LOW ? LOW : HIGH)
#define LED_OFF (CONFIG_STATUS_LED_POL == LOW ? HIGH : LOW)


#if defined(ESP32)
#  define LEDC_TONE_CHANNEL 1
#  define LEDC_TONE_FREQ    8000
#  define LEDC_TONE_RES     8
#  define noTone(pin) ledcWriteTone(LEDC_TONE_CHANNEL, 0)
#  define tone(pin, freq, dur) ledcWriteTone(LEDC_TONE_CHANNEL, freq);
#endif // ESP32


static void sStatusLedTimerFunc(void)
{
    static uint32_t tick = 0; // 10 Hz
    if ( (sLedPeriod != 0) && (sLedNumBlink != 0) )
    {
        const uint8_t phase = tick % sLedPeriod;
        if ( phase < (2 * sLedNumBlink) )
        {
            digitalWrite(CONFIG_STATUS_LED_PIN, ((phase % 2) == 0) ? LED_ON : LED_OFF);
        }
    }
    tick++;

}

void statusLed(const STATUS_LED_t status)
{
    digitalWrite(CONFIG_STATUS_LED_PIN, LED_OFF);
    
    switch (status)
    {
        case STATUS_LED_NONE:
            DEBUG("status: led none");
            sLedPeriod = 0;
            sLedNumBlink = 0;
            break;
        case STATUS_LED_HEARTBEAT:
            DEBUG("status: led heartbeat");
            sLedPeriod = 20;
            sLedNumBlink    = 2;
            break;
        case STATUS_LED_OFFLINE:
            DEBUG("status: led offline");
            sLedPeriod = 20;
            sLedNumBlink    = 1;
            break;
        case STATUS_LED_FAIL:
            DEBUG("status: led fail");
            sLedPeriod = 20;
            sLedNumBlink    = 5;
            break;
        case STATUS_LED_UPDATE:
            DEBUG("status: led update");
            sLedPeriod = 2;
            sLedNumBlink    = 1;
            break;
    }
}

void sStatusMelodyTicker(void)
{
    // next note
    if (sMelodyDur == 0)
    {
#if defined(ESP32)
        noTone();
#endif
        const int16_t note = skMelody[sMelodyIx];
        if (note == RTTTL_NOTE_END)
        {
            sToneTicker.detach();
            sToneIsPlaying = false;
        }
        else
        {
            const int16_t duration = skMelody[sMelodyIx + 1];
            sMelodyDur = (duration / 10) - 1; // ticker is [10ms]
            if (note != RTTTL_NOTE_PAUSE)
            {
                tone(CONFIG_STATUS_TONE_PIN, note, duration);
            }
            sMelodyIx += 2;
        }
    }
    // still playing
    else
    {
        sMelodyDur--;
    }
}

void sStatusPlayMelody(const int16_t *melody)
{
    sToneTicker.detach();
    sMelodyIx = 0;
    skMelody = melody;
    sMelodyDur = 0;
    sToneIsPlaying = true;
    sToneTicker.attach_ms(10, sStatusMelodyTicker);
}

bool statusTonePlaying(void)
{
    return sToneIsPlaying;
}

void statusToneStop(void)
{
    sToneTicker.detach();
    noTone(CONFIG_STATUS_TONE_PIN);
    sToneIsPlaying = false;
}

#define TONE(note, dur) RTTTL_NOTE_ ## note, dur
#define TONE_END RTTTL_NOTE_END
#define TONE_PAUSE RTTTL_NOTE_PAUSE

void statusNoise(const STATUS_NOISE_t noise)
{
    if ( statusTonePlaying() )
    {
        return;
    }

    switch (noise)
    {
        case STATUS_NOISE_ABORT:
        {
            DEBUG("status: noise abort");
            static const int16_t skNoiseAbort[] =
            {
                TONE(A5, 30), TONE(PAUSE, 20), TONE(G5, 60), TONE_END
            };
            sStatusPlayMelody(skNoiseAbort);
            break;
        }
        case STATUS_NOISE_FAIL:
        {
            DEBUG("status: noise fail");
            static const int16_t skNoiseFail[] =
            {
                TONE(A5, 30), TONE(PAUSE, 20), TONE(G5, 60), TONE(PAUSE, 20), TONE(F5, 100), TONE_END
            };
            sStatusPlayMelody(skNoiseFail);
            break;
        }
        case STATUS_NOISE_ONLINE:
        {
            DEBUG("status: noise online");
            static const int16_t skNoiseOnline[] =
            {
                TONE(D6, 30), TONE(PAUSE, 20), TONE(E6, 60), TONE_END
            };
            sStatusPlayMelody(skNoiseOnline);
            break;
        }
        case STATUS_NOISE_OTHER:
        {
            DEBUG("status: noise other");
            static const int16_t skNoiseOther[] =
            {
                TONE(C6, 30), TONE_END
            };
            sStatusPlayMelody(skNoiseOther);
            break;
        }
        case STATUS_NOISE_ERROR:
        {
            DEBUG("status: noise error");
            static const int16_t skNoiseError[] =
            {
                TONE(C4, 200), TONE(PAUSE, 50), TONE(C4, 200), TONE_END
            };
            sStatusPlayMelody(skNoiseError);
            break;
        }
        case STATUS_NOISE_TICK:
        {
            DEBUG("status: noise tick");
            static const int16_t skNoiseTick[] =
            {
                TONE(C8, 40), TONE(PAUSE, 30), TONE(C7, 40), TONE_END
            };
            sStatusPlayMelody(skNoiseTick);
            break;
        }
        case STATUS_NOISE_BOMB:
        {
            DEBUG("status: noise bomb");
            static const int16_t skNoiseBomb[] =
            {
                TONE(C8, 40), TONE(PAUSE, 300),
                TONE(C8, 40), TONE(PAUSE, 300),
                TONE(C8, 40), TONE(PAUSE, 300),
                TONE(C8, 40), TONE(PAUSE, 300),
                TONE(C8, 40), TONE(PAUSE, 300),
                TONE(C8, 40), TONE(PAUSE, 300),
                TONE(C4, 400), TONE_END
            };
            sStatusPlayMelody(skNoiseBomb);
            break;
        }
        case STATUS_NOISE_FFI:
        {
            DEBUG("status: noise ffi");
            static const int16_t skNoiseFfi[] = // ..-. ..-. .. 
            {
                TONE(G5, 100), TONE(PAUSE, 50),
                TONE(G5, 100), TONE(PAUSE, 50),
                TONE(G5, 250), TONE(PAUSE, 50),
                TONE(G5, 100), TONE(PAUSE, 250),

                TONE(G5, 100), TONE(PAUSE, 50),
                TONE(G5, 100), TONE(PAUSE, 50),
                TONE(G5, 250), TONE(PAUSE, 50),
                TONE(G5, 100), TONE(PAUSE, 250),

                TONE(G5, 100), TONE(PAUSE, 50),
                TONE(G5, 100), TONE(PAUSE, 50),

                TONE_END
            };
            sStatusPlayMelody(skNoiseFfi);
            break;
        }
    }
}

#define TONE_MELODY_N 100

void statusMelody(const char *name)
{
    // get RTTTL string
    const char *pkRtttl = NULL;
    if (strcmp_PP(name, PSTR("random")) == 0)
    {
        pkRtttl = rtttlBuiltinMelodyRandom();
    }
    else
    {
        pkRtttl = rtttlBuiltinMelody(name);
    }

    if (pkRtttl == NULL)
    {
        ERROR("status: no melody (%s)", name);
        return;
    }

    // convert to tone/duration pairs
    static int16_t melody[(TONE_MELODY_N * 2) + 1];
    rtttlMelody(pkRtttl, melody, (TONE_MELODY_N * 2) + 1);

    // play
    DEBUG("status: melody (%s)", name);
    statusToneStop();
    sStatusPlayMelody(melody);
}

void statusInit(void)
{
    DEBUG("status: init (ledPin=" STRINGIFY(CONFIG_STATUS_LED_PIN) ", ledPol=" STRINGIFY(CONFIG_STATUS_LED_POL)
        ", tonePin=" STRINGIFY(CONFIG_STATUS_TONE_PIN) ")");

    pinMode(CONFIG_STATUS_LED_PIN, OUTPUT);
    digitalWrite(CONFIG_STATUS_LED_PIN, LED_OFF);

    pinMode(CONFIG_STATUS_TONE_PIN, OUTPUT);
    digitalWrite(CONFIG_STATUS_TONE_PIN, LOW);

    sLedTicker.attach_ms(100, sStatusLedTimerFunc);

    statusLed(STATUS_LED_NONE);

#if defined(ESP32)
    ledcSetup(LEDC_TONE_CHANNEL, LEDC_TONE_FREQ, LEDC_TONE_RES);
    ledcAttachPin(CONFIG_STATUS_TONE_PIN, LEDC_TONE_CHANNEL);
    ledcWriteTone(LEDC_TONE_CHANNEL, 0);
#endif
}

// eof
