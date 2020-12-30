// RTTTL melodies stuff, see rtttl.c for credits and copyrights

#ifndef __RTTTL_H__
#define __RTTTL_H__

#include <Arduino.h>

#ifdef __cplusplus
extern "C" {
#endif

// takes a https://en.wikipedia.org/wiki/Ring_Tone_Transfer_Language melody,
// (s.a. http://merwin.bespin.org/t4a/specs/nokia_rtttl.txt)
// and fills in a list of frequency and duration pairs
void rtttlMelody(const char *melodyStr, int16_t *pFreqDur, const int nFreqDur);

const char *rtttlBuiltinMelody(const char *name);

const char *rtttlBuiltinMelodyRandom(void);

// tone frequencies, from Arduino's "toneMelody" example's pitches.h
#define RTTTL_NOTE_B0  31
#define RTTTL_NOTE_C1  33
#define RTTTL_NOTE_CS1 35
#define RTTTL_NOTE_D1  37
#define RTTTL_NOTE_DS1 39
#define RTTTL_NOTE_E1  41
#define RTTTL_NOTE_F1  44
#define RTTTL_NOTE_FS1 46
#define RTTTL_NOTE_G1  49
#define RTTTL_NOTE_GS1 52
#define RTTTL_NOTE_A1  55
#define RTTTL_NOTE_AS1 58
#define RTTTL_NOTE_B1  62
#define RTTTL_NOTE_C2  65
#define RTTTL_NOTE_CS2 69
#define RTTTL_NOTE_D2  73
#define RTTTL_NOTE_DS2 78
#define RTTTL_NOTE_E2  82
#define RTTTL_NOTE_F2  87
#define RTTTL_NOTE_FS2 93
#define RTTTL_NOTE_G2  98
#define RTTTL_NOTE_GS2 104
#define RTTTL_NOTE_A2  110
#define RTTTL_NOTE_AS2 117
#define RTTTL_NOTE_B2  123
#define RTTTL_NOTE_C3  131
#define RTTTL_NOTE_CS3 139
#define RTTTL_NOTE_D3  147
#define RTTTL_NOTE_DS3 156
#define RTTTL_NOTE_E3  165
#define RTTTL_NOTE_F3  175
#define RTTTL_NOTE_FS3 185
#define RTTTL_NOTE_G3  196
#define RTTTL_NOTE_GS3 208
#define RTTTL_NOTE_A3  220
#define RTTTL_NOTE_AS3 233
#define RTTTL_NOTE_B3  247
#define RTTTL_NOTE_C4  262
#define RTTTL_NOTE_CS4 277
#define RTTTL_NOTE_D4  294
#define RTTTL_NOTE_DS4 311
#define RTTTL_NOTE_E4  330
#define RTTTL_NOTE_F4  349
#define RTTTL_NOTE_FS4 370
#define RTTTL_NOTE_G4  392
#define RTTTL_NOTE_GS4 415
#define RTTTL_NOTE_A4  440
#define RTTTL_NOTE_AS4 466
#define RTTTL_NOTE_B4  494
#define RTTTL_NOTE_C5  523
#define RTTTL_NOTE_CS5 554
#define RTTTL_NOTE_D5  587
#define RTTTL_NOTE_DS5 622
#define RTTTL_NOTE_E5  659
#define RTTTL_NOTE_F5  698
#define RTTTL_NOTE_FS5 740
#define RTTTL_NOTE_G5  784
#define RTTTL_NOTE_GS5 831
#define RTTTL_NOTE_A5  880
#define RTTTL_NOTE_AS5 932
#define RTTTL_NOTE_B5  988
#define RTTTL_NOTE_C6  1047
#define RTTTL_NOTE_CS6 1109
#define RTTTL_NOTE_D6  1175
#define RTTTL_NOTE_DS6 1245
#define RTTTL_NOTE_E6  1319
#define RTTTL_NOTE_F6  1397
#define RTTTL_NOTE_FS6 1480
#define RTTTL_NOTE_G6  1568
#define RTTTL_NOTE_GS6 1661
#define RTTTL_NOTE_A6  1760
#define RTTTL_NOTE_AS6 1865
#define RTTTL_NOTE_B6  1976
#define RTTTL_NOTE_C7  2093
#define RTTTL_NOTE_CS7 2217
#define RTTTL_NOTE_D7  2349
#define RTTTL_NOTE_DS7 2489
#define RTTTL_NOTE_E7  2637
#define RTTTL_NOTE_F7  2794
#define RTTTL_NOTE_FS7 2960
#define RTTTL_NOTE_G7  3136
#define RTTTL_NOTE_GS7 3322
#define RTTTL_NOTE_A7  3520
#define RTTTL_NOTE_AS7 3729
#define RTTTL_NOTE_B7  3951
#define RTTTL_NOTE_C8  4186
#define RTTTL_NOTE_CS8 4435
#define RTTTL_NOTE_D8  4699
#define RTTTL_NOTE_DS8 4978

#define RTTTL_NOTE_PAUSE  1
#define RTTTL_NOTE_END    0


#ifdef __cplusplus
}
#endif

#endif
