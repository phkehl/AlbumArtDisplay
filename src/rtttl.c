#include <pgmspace.h>

#include "stuff.h"
#include "rtttl.h"



// based on:
// - https://github.com/esp8266/Arduino/blob/master/cores/esp8266/Tone.cpp
// - hw_timer.c from the SDK
// - Arduino ESP8266 code
// - http://domoticx.com/arduino-melodie-afspelen-rtttl/ (melodies)
// see below for details

//-------------------------------------------------------------------------------------------------------------

/*
http://merwin.bespin.org/t4a/specifications.html / http://merwin.bespin.org/t4a/specs/nokia_rtttl.txt:
RTTTL Format Specifications

RTTTL (RingTone Text Transfer Language) is the primary format used to distribute
ringtones for Nokia phones. An RTTTL file is a text file, containing the
ringtone name, a control section and a section containing a comma separated
sequence of ring tone commands. White space must be ignored by any reader
application.

Example:
Simpsons:d=4,o=5,b=160:32p,c.6,e6,f#6,8a6,g.6,e6,c6,8a,8f#,8f#,8f#,2g

This file describes a ringtone whose name is 'Simpsons'. The control section
sets the beats per minute at 160, the default note length as 4, and the default
scale as Octave 5.
<RTX file> := <name> ":" [<control section>] ":" <tone-commands>

	<name> := <char> ; maximum name length 10 characters

	<control-section> := <control-pair> ["," <control-section>]

		<control-pair> := <control-name> ["="] <control-value>

		<control-name> := "o" | "d" | "b"
		; Valid in control section: o=default scale, d=default duration, b=default beats per minute.
		; if not specified, defaults are 4=duration, 6=scale, 63=beats-per-minute
		; any unknown control-names must be ignored

		<tone-commands> := <tone-command> ["," <tone-commands>]

		<tone-command> :=<note> | <control-pair>

		<note> := [<duration>] <note> [<scale>] [<special-duration>] <delimiter>

			<duration> := "1" | "2" | "4" | "8" | "16" | "32"
			; duration is divider of full note duration, eg. 4 represents a quarter note

			<note> := "P" | "C" | "C#" | "D" | "D#" | "E" | "F" | "F#" | "G" | "G#" | "A" | "A#" | "B"

			<scale> :="4" | "5" | "6" | "7"
			; Note that octave 4: A=440Hz, 5: A=880Hz, 6: A=1.76 kHz, 7: A=3.52 kHz
			; The lowest note on the Nokia 61xx is A4, the highest is B7

			<special-duration> := "." ; Dotted note

; End of specification


*/

//-------------------------------------------------------------------------------------------------------------

// from http://domoticx.com/arduino-melodieafspelen-rtttl
const char skRtttlMelody01[] PROGMEM = "TheSimpsons:d=4,o=5,b=160:c.6,e6,f#6,8a6,g.6,e6,c6,8a,8f#,8f#,8f#,2g,8p,8p,8f#,8f#,8f#,8g,a#.,8c6,8c6,8c6,c6";
const char skRtttlMelody02[] PROGMEM = "Indiana:d=4,o=5,b=250:e,8p,8f,8g,8p,1c6,8p.,d,8p,8e,1f,p.,g,8p,8a,8b,8p,1f6,p,a,8p,8b,2c6,2d6,2e6,e,8p,8f,8g,8p,1c6,p,d6,8p,8e6,1f.6,g,8p,8g,e.6,8p,d6,8p,8g,e.6,8p,d6,8p,8g,f.6,8p,e6,8p,8d6,2c6";
const char skRtttlMelody03[] PROGMEM = "IndianaShort:d=4,o=5,b=250:e,8p,8f,8g,8p,1c6,8p.,d,8p,8e,1f,p.";
const char skRtttlMelody04[] PROGMEM = "TakeOnMe:d=4,o=4,b=160:8f#5,8f#5,8f#5,8d5,8p,8b,8p,8e5,8p,8e5,8p,8e5,8g#5,8g#5,8a5,8b5,8a5,8a5,8a5,8e5,8p,8d5,8p,8f#5,8p,8f#5,8p,8f#5,8e5,8e5,8f#5,8e5,8f#5,8f#5,8f#5,8d5,8p,8b,8p,8e5,8p,8e5,8p,8e5,8g#5,8g#5,8a5,8b5,8a5,8a5,8a5,8e5,8p,8d5,8p,8f#5,8p,8f#5,8p,8f#5,8e5,8e5";
const char skRtttlMelody05[] PROGMEM = "Entertainer:d=4,o=5,b=140:8d,8d#,8e,c6,8e,c6,8e,2c.6,8c6,8d6,8d#6,8e6,8c6,8d6,e6,8b,d6,2c6,p,8d,8d#,8e,c6,8e,c6,8e,2c.6,8p,8a,8g,8f#,8a,8c6,e6,8d6,8c6,8a,2d6";
const char skRtttlMelody06[] PROGMEM = "Muppets:d=4,o=5,b=250:c6,c6,a,b,8a,b,g,p,c6,c6,a,8b,8a,8p,g.,p,e,e,g,f,8e,f,8c6,8c,8d,e,8e,8e,8p,8e,g,2p,c6,c6,a,b,8a,b,g,p,c6,c6,a,8b,a,g.,p,e,e,g,f,8e,f,8c6,8c,8d,e,8e,d,8d,c";
const char skRtttlMelody07[] PROGMEM = "Xfiles:d=4,o=5,b=125:e,b,a,b,d6,2b.,1p,e,b,a,b,e6,2b.,1p,g6,f#6,e6,d6,e6,2b.,1p,g6,f#6,e6,d6,f#6,2b.,1p,e,b,a,b,d6,2b.,1p,e,b,a,b,e6,2b.,1p,e6,2b.";
const char skRtttlMelody08[] PROGMEM = "Looney:d=4,o=5,b=140:32p,c6,8f6,8e6,8d6,8c6,a.,8c6,8f6,8e6,8d6,8d#6,e.6,8e6,8e6,8c6,8d6,8c6,8e6,8c6,8d6,8a,8c6,8g,8a#,8a,8f";
const char skRtttlMelody09[] PROGMEM = "20thCenFox:d=16,o=5,b=140:b,8p,b,b,2b,p,c6,32p,b,32p,c6,32p,b,32p,c6,32p,b,8p,b,b,b,32p,b,32p,b,32p,b,32p,b,32p,b,32p,b,32p,g#,32p,a,32p,b,8p,b,b,2b,4p,8e,8g#,8b,1c#6,8f#,8a,8c#6,1e6,8a,8c#6,8e6,1e6,8b,8g#,8a,2b";
const char skRtttlMelody10[] PROGMEM = "Bond:d=4,o=5,b=80:32p,16c#6,32d#6,32d#6,16d#6,8d#6,16c#6,16c#6,16c#6,16c#6,32e6,32e6,16e6,8e6,16d#6,16d#6,16d#6,16c#6,32d#6,32d#6,16d#6,8d#6,16c#6,16c#6,16c#6,16c#6,32e6,32e6,16e6,8e6,16d#6,16d6,16c#6,16c#7,c.7,16g#6,16f#6,g#.6";
const char skRtttlMelody11[] PROGMEM = "MASH:d=8,o=5,b=140:4a,4g,f#,g,p,f#,p,g,p,f#,p,2e.,p,f#,e,4f#,e,f#,p,e,p,4d.,p,f#,4e,d,e,p,d,p,e,p,d,p,2c#.,p,d,c#,4d,c#,d,p,e,p,4f#,p,a,p,4b,a,b,p,a,p,b,p,2a.,4p,a,b,a,4b,a,b,p,2a.,a,4f#,a,b,p,d6,p,4e.6,d6,b,p,a,p,2b";
const char skRtttlMelody12[] PROGMEM = "StarWars:d=4,o=5,b=45:32p,32f#,32f#,32f#,8b.,8f#.6,32e6,32d#6,32c#6,8b.6,16f#.6,32e6,32d#6,32c#6,8b.6,16f#.6,32e6,32d#6,32e6,8c#.6,32f#,32f#,32f#,8b.,8f#.6,32e6,32d#6,32c#6,8b.6,16f#.6,32e6,32d#6,32c#6,8b.6,16f#.6,32e6,32d#6,32e6,8c#6";
const char skRtttlMelody13[] PROGMEM = "GoodBad:d=4,o=5,b=56:32p,32a#,32d#6,32a#,32d#6,8a#.,16f#.,16g#.,d#,32a#,32d#6,32a#,32d#6,8a#.,16f#.,16g#.,c#6,32a#,32d#6,32a#,32d#6,8a#.,16f#.,32f.,32d#.,c#,32a#,32d#6,32a#,32d#6,8a#.,16g#.,d#";
const char skRtttlMelody14[] PROGMEM = "TopGun:d=4,o=4,b=31:32p,16c#,16g#,16g#,32f#,32f,32f#,32f,16d#,16d#,32c#,32d#,16f,32d#,32f,16f#,32f,32c#,16f,d#,16c#,16g#,16g#,32f#,32f,32f#,32f,16d#,16d#,32c#,32d#,16f,32d#,32f,16f#,32f,32c#,g#";
const char skRtttlMelody15[] PROGMEM = "A-Team:d=8,o=5,b=125:4d#6,a#,2d#6,16p,g#,4a#,4d#.,p,16g,16a#,d#6,a#,f6,2d#6,16p,c#.6,16c6,16a#,g#.,2a#";
const char skRtttlMelody16[] PROGMEM = "Flinstones:d=4,o=5,b=40:32p,16f6,16a#,16a#6,32g6,16f6,16a#.,16f6,32d#6,32d6,32d6,32d#6,32f6,16a#,16c6,d6,16f6,16a#.,16a#6,32g6,16f6,16a#.,32f6,32f6,32d#6,32d6,32d6,32d#6,32f6,16a#,16c6,a#,16a6,16d.6,16a#6,32a6,32a6,32g6,32f#6,32a6,8g6,16g6,16c.6,32a6,32a6,32g6,32g6,32f6,32e6,32g6,8f6,16f6,16a#.,16a#6,32g6,16f6,16a#.,16f6,32d#6,32d6,32d6,32d#6,32f6,16a#,16c.6,32d6,32d#6,32f6,16a#,16c.6,32d6,32d#6,32f6,16a#6,16c7,8a#.6";
const char skRtttlMelody17[] PROGMEM = "Jeopardy:d=4,o=6,b=125:c,f,c,f5,c,f,2c,c,f,c,f,a.,8g,8f,8e,8d,8c#,c,f,c,f5,c,f,2c,f.,8d,c,a#5,a5,g5,f5,p,d#,g#,d#,g#5,d#,g#,2d#,d#,g#,d#,g#,c.7,8a#,8g#,8g,8f,8e,d#,g#,d#,g#5,d#,g#,2d#,g#.,8f,d#,c#,c,p,a#5,p,g#.5,d#,g#";
const char skRtttlMelody18[] PROGMEM = "Gadget:d=16,o=5,b=50:32d#,32f,32f#,32g#,a#,f#,a,f,g#,f#,32d#,32f,32f#,32g#,a#,d#6,4d6,32d#,32f,32f#,32g#,a#,f#,a,f,g#,f#,8d#";
const char skRtttlMelody19[] PROGMEM = "Smurfs:d=32,o=5,b=200:4c#6,16p,4f#6,p,16c#6,p,8d#6,p,8b,p,4g#,16p,4c#6,p,16a#,p,8f#,p,8a#,p,4g#,4p,g#,p,a#,p,b,p,c6,p,4c#6,16p,4f#6,p,16c#6,p,8d#6,p,8b,p,4g#,16p,4c#6,p,16a#,p,8b,p,8f,p,4f#";
const char skRtttlMelody20[] PROGMEM = "MahnaMahna:d=16,o=6,b=125:c#,c.,b5,8a#.5,8f.,4g#,a#,g.,4d#,8p,c#,c.,b5,8a#.5,8f.,g#.,8a#.,4g,8p,c#,c.,b5,8a#.5,8f.,4g#,f,g.,8d#.,f,g.,8d#.,f,8g,8d#.,f,8g,d#,8c,a#5,8d#.,8d#.,4d#,8d#.";
const char skRtttlMelody21[] PROGMEM = "LeisureSuit:d=16,o=6,b=56:f.5,f#.5,g.5,g#5,32a#5,f5,g#.5,a#.5,32f5,g#5,32a#5,g#5,8c#.,a#5,32c#,a5,a#.5,c#.,32a5,a#5,32c#,d#,8e,c#.,f.,f.,f.,f.,f,32e,d#,8d,a#.5,e,32f,e,32f,c#,d#.,c#";
const char skRtttlMelody22[] PROGMEM = "MissionImp:d=16,o=6,b=95:32d,32d#,32d,32d#,32d,32d#,32d,32d#,32d,32d,32d#,32e,32f,32f#,32g,g,8p,g,8p,a#,p,c7,p,g,8p,g,8p,f,p,f#,p,g,8p,g,8p,a#,p,c7,p,g,8p,g,8p,f,p,f#,p,a#,g,2d,32p,a#,g,2c#,32p,a#,g,2c,a#5,8c,2p,32p,a#5,g5,2f#,32p,a#5,g5,2f,32p,a#5,g5,2e,d#,8d";
// from http://www.lucodes.ca/ringtones.html
const char skRtttlMelody23[] PROGMEM = "PacMan:d=16,o=6,b=140:b5,b,f#,d#,8b,8d#,c,c7,g,f,8c7,8e,b5,b,f#,d#,8b,8d#,32d#,32e,f,32f,32f#,g,32g,32g#,a,8b";
const char skRtttlMelody24[] PROGMEM = "HappyBday:d=4,o=5,b=125:8d.,16d,e,d,g,2f#,8d.,16d,e,d,a,2g,8d.,16d,d6,b,g,f#,2e,8c.6,16c6,b,g,a,2g";
const char skRtttlMelody25[] PROGMEM = "Titanic:d=8,o=6,b=125:e,f#,2g#,16f#,16g#,16f#,e,f#,2b,32c#7,32b,a,g#,4e,2c#,2b5,d#,f#,f#,2g#,16a,16g#,16f#,16e,4f#,2b,g#,b,2c#7,2b,2f#";
const char skRtttlMelody26[] PROGMEM = "Urgent:d=8,o=6,b=500:c,e,d7,c,e,a#,c,e,a,c,e,g,c,e,a,c,e,a#,c,e,d7";
// from http://arcadetones.emuunlim.com/
const char skRtttlMelody27[] PROGMEM = "SuperMario1:d=4,o=5,b=100:16e6,16e6,32p,8e6,16c6,8e6,8g6,8p,8g,8p,8c6,16p,8g,16p,8e,16p,8a,8b,16a#,8a,16g.,16e6,16g6,8a6,16f6,8g6,8e6,16c6,16d6,8b,16p,8c6,16p,8g,16p,8e,16p,8a,8b,16a#,8a,16g.,16e6,16g6,8a6,16f6,8g6,8e6,16c6,16d6,8b,8p,16g6,16f#6,16f6,16d#6,16p,16e6,16p,16g#,16a,16c6,16p,16a,16c6,16d6,8p,16g6,16f#6,16f6,16d#6,16p,16e6,16p,16c7,16p,16c7,16c7,p,16g6,16f#6,16f6,16d#6,16p,16e6,16p,16g#,16a,16c6,16p,16a,16c6,16d6,8p,16d#6,8p,16d6,8p,16c6";
const char skRtttlMelody28[] PROGMEM = "SuperMario2:d=4,o=6,b=100:32c,32p,32c7,32p,32a5,32p,32a,32p,32a#5,32p,32a#,2p,32c,32p,32c7,32p,32a5,32p,32a,32p,32a#5,32p,32a#,2p,32f5,32p,32f,32p,32d5,32p,32d,32p,32d#5,32p,32d#,2p,32f5,32p,32f,32p,32d5,32p,32d,32p,32d#5,32p,32d#";
const char skRtttlMelody29[] PROGMEM = "SuperMario3:d=4,o=5,b=90:32c6,32c6,32c6,8p,16b,16f6,16p,16f6,16f.6,16e.6,16d6,16c6,16p,16e,16p,16c";
const char skRtttlMelody30[] PROGMEM = "Tetris1:d=4,o=5,b=160:e6,8b,8c6,8d6,16e6,16d6,8c6,8b,a,8a,8c6,e6,8d6,8c6,b,8b,8c6,d6,e6,c6,a,2a,8p,d6,8f6,a6,8g6,8f6,e6,8e6,8c6,e6,8d6,8c6,b,8b,8c6,d6,e6,c6,a,a";
const char skRtttlMelody31[] PROGMEM = "Tetris2:d=4,o=5,b=160:d6,32p,c.6,32p,8a,8c6,8a#,16a,16g,f,c,8a,8c6,8g,8a,f,c,d,8d,8e,8g,8f,8e,8d,c,c,c";
const char skRtttlMelody32[] PROGMEM = "Tetris3:d=4,o=5,b=63:d#6,b,c#6,a#,16b,16g#,16a#,16b,16b,16g#,16a#,16b,c#6,g,d#6,16p,16g#,16a#,16b,c#6,16p,16b,16a#,g#,g,g#,16f,16g,16g#,16a#,8d#.6,32d#6,32p,32d#6,32p,32d#6,32p,16d6,16d#6,8f.6,16d6,8a#,8p,8f#6,8d#6,8f#,8g#,a#.,16p,16a#,8d#.6,16f6,16f#6,16f6,16d#6,16a#,8g#.,16b,8d#6,16f6,16d#6,8a#.,16b,16a#,16g#,16f,16f#,d#";
const char skRtttlMelody33[] PROGMEM = "BubbleBobble:d=4,o=5,b=125:8a#6,8a6,8g.6,16f6,8a6,8g6,8f6,8d#6,8g6,8f6,16d#6,8d.6,f.6,16d6,16c6,8a#,8c6,8d6,8d#6,8c6,16d6,8d#.6,8f6,8f6,8g6,16a6,8g.6,8f6,8f6,8g6,8a6,8a#6,8a6,8g.6,16f6,8a6,8g6,8f6,8d#6,8g6,8f6,16d#6,8d.6,f.6,16d6,16c6,8a#,8c6,8d6,8d#6,8c6,16d6,8d#.6,8f6,8f6,8g6,16a6,8f.6,8a#.6";
const char skRtttlMelody34[] PROGMEM = "Duke3d:d=4,o=5,b=90:16f#4,16a4,16p,16b4,8p,16f#4,16b4,16p,16c#,8p,16f#4,16c#,16p,16d,16p,16f#4,16p,8c#,16b4,16a4,16f#4,16e4,16f4,16f#4,16g4,16b4,16a4,16g4,16f#4,16a4,16p,16f#4,16a4,16p,16b4,8p,16f#4,16b4,16p,16c#,8p,16f#4,16c#,16p,16d,16p,16f#4,16p,8c#,16b4,16a4,16f#4,16e4,16f4,16f#4,16g4,16b4,16a4,16g4,16f#4,16a4";
const char skRtttlMelody35[] PROGMEM = "ManiacMansion:d=4,o=5,b=112:8f#4,16f#4,16f#4,8f#,16f#4,8f#4,16f#,16f#4,16f#4,8f#,8f#4,8g#4,16g#4,16g#4,8g#,16g#4,8g#4,16g#,16g#4,16g#4,8g#,8g#4,8a4,16a4,16a4,8a,16a4,8a4,16a,16a4,16a4,8a,8a4,8b4,16b4,16b4,8b,16b4,8b4,16b,8b4,16f#,16b4,8a4,8f#4,16f#4,16f#4,8c#,16f#4,8f#4,16c#,16d#4,16d#4,8e4,8f4,8f#4,16f#4,16f#4,8c#,16f#4,8f#4,16c#,16d#4,16d#4,8e4,8f4,8f#4,16f#4,16f#4,8c#,16f#4,8f#4,16c#,16d#4,16d#4,8e4,8f4,8f#4";
const char skRtttlMelody36[] PROGMEM = "Dott:d=4,o=5,b=125:8d#6,16e6,16p,16c#6,16p,16a,16p,16e,16p,d#.,16p,8d#,16e,16p,16a,16p,16c#6,16p,16e6,16p,d#6,f6,8e#.6";
const char skRtttlMelody37[] PROGMEM = "ImperialMarch:d=4,o=5,b=80:8d.,8d.,8d.,8a#4,16f,8d.,8a#4,16f,d.,32p,8a.,8a.,8a.,8a#,16f,8c#.,8a#4,16f,d.,32p,8d.6,8d,16d,8d6,32p,8c#6,16c6,16b,16a#,8b,32p,16d#,8g#,32p,8g,16f#,16f,16e,8f,32p,16a#4,8c#,32p,8a#4,16c#,8f.,8d,16f,a.,32p,8d.6,8d,16d,8d6,32p,8c#6,16c6,16b,16a#,8b,32p,16d#,8g#,32p,8g,16f#,16f,16e,8f,32p,16a#4,8c#,32p,8a#4,16f,8d.,8a#4,16f,d.";
const char skRtttlMelody38[] PROGMEM = "ImperialShort:d=4,o=5,b=80:8d.,8d.,8d.,8a#4,16f,8d.,8a#4,16f,d.";
const char skRtttlMelody39[] PROGMEM = "Beethoven5th:d=16,o=5,b=100:g,g,g,4d#,4p,f,f,f,4d,4p,g,g,g,d#,g#,g#,g#,g,d#6,d#6,d#6,4c6,8p,g,g,g,d,g#,g#,g#,g,f6,f6,f6,4d6,8p,g6,g6,f6,4d#6,8p,g6,g6,f6,4d#6";
const char skRtttlMelody40[] PROGMEM = "BeethovenElise:d=8,o=5,b=125:32p,e6,d#6,e6,d#6,e6,b,d6,c6,4a.,32p,c,e,a,4b.,32p,e,g#,b,4c.6,32p,e,e6,d#6,e6,d#6,e6,b,d6,c6,4a.,32p,c,e,a,4b.,32p,d,c6,b,2a";
// from http://www.vex.net/~lawrence/ringtones.html
const char skRtttlMelody41[] PROGMEM = "XFile:d=16,o=6,b=355:e.,p,e.,p,e.,p,e.,p,b.,p,b.,p,b.,p,b.,p,a.,p,a.,p,a.,p,a.,p,b.,p,b.,p,b.,p,b.,p,d.7,p,d.7,p,d.7,p,d.7,p,b.,p,b.,p,b.,p,b.,p,b.,p,b.,p,b.,p,b.,p,b.,p,b.,p,b.,p,b.,p";
const char skRtttlMelody42[] PROGMEM = "Fido:d=16,o=6,b=800:f,4p,f,4p,f,4p,f,4p,c,4p,c,4p,c,4p,c,1p,1p,1p,1p";
const char skRtttlMelody43[] PROGMEM = "Fido2:d=8,o=7,b=500:f,f6,f,f6,f,f6,c,c6,c,c6,c,c6,c,c6,c,c6,c,c6,c,c6,c,c6,c,c6,c,c6,c,c6,c,c6,c,c6,c,c6,c,2p";
const char skRtttlMelody44[] PROGMEM = "Intel:d=16,o=5,b=320:d,p,d,p,d,p,g,p,g,p,g,p,d,p,d,p,d,p,a,p,a,p,a,2p";
const char skRtttlMelody45[] PROGMEM = "Knock:d=32,o=4,b=100:e,4p,e,p,e,8p,e,4p,e,8p,e,4p";
const char skRtttlMelody46[] PROGMEM = "Urgent:d=8,o=6,b=500:c,e,d7,c,e,a#,c,e,a,c,e,g,c,e,a,c,e,a#,c,e,d7";
const char skRtttlMelody47[] PROGMEM = "Mosaic:d=8,o=6,b=400:c,e,g,e,c,g,e,g,c,g,c,e,c,g,e,g,e,c";
const char skRtttlMelody48[] PROGMEM = "Mosaic2:d=8,o=6,b=400:c,e,g,e,c,g,e,g,c,g,c,e,c,g,e,g,e,c,p,c5,e5,g5,e5,c5,g5,e5,g5,c5,g5,c5,e5,c5,g5,e5,g5,e5,c5";
const char skRtttlMelody49[] PROGMEM = "Triple:d=8,o=5,b=635:c,e,g,c,e,g,c,e,g,c6,e6,g6,c6,e6,g6,c6,e6,g6,c7,e7,g7,c7,e7,g7,c7,e7,g7";

const char * const skRtttlMelodies[] PROGMEM =
{
    skRtttlMelody01, skRtttlMelody02, skRtttlMelody03, skRtttlMelody04, skRtttlMelody05,
    skRtttlMelody06, skRtttlMelody07, skRtttlMelody08, skRtttlMelody09, skRtttlMelody10,
    skRtttlMelody11, skRtttlMelody12, skRtttlMelody13, skRtttlMelody14, skRtttlMelody15,
    skRtttlMelody16, skRtttlMelody17, skRtttlMelody18, skRtttlMelody19, skRtttlMelody20,
    skRtttlMelody21, skRtttlMelody22, skRtttlMelody23, skRtttlMelody24, skRtttlMelody25,
    skRtttlMelody26, skRtttlMelody27, skRtttlMelody28, skRtttlMelody29, skRtttlMelody30,
    skRtttlMelody31, skRtttlMelody32, skRtttlMelody33, skRtttlMelody34, skRtttlMelody35,
    skRtttlMelody36, skRtttlMelody37, skRtttlMelody38, skRtttlMelody39, skRtttlMelody40,
    skRtttlMelody41, skRtttlMelody42, skRtttlMelody43, skRtttlMelody44, skRtttlMelody45,
    skRtttlMelody46, skRtttlMelody47, skRtttlMelody48, skRtttlMelody49
};

const char * rtttlBuiltinMelody(const char *name)
{
    const char *res = NULL;
    int ix = (int)(sizeof(skRtttlMelodies)/sizeof(skRtttlMelodies[0]));
    const int nameLen = strlen(name);
    while (ix--)
    {
        if (strncmp_PP((const char *)pgm_read_dword(&(skRtttlMelodies[ix])), name, nameLen) == 0)
        {
            res = skRtttlMelodies[ix];
            break;
        }
    }
    return res;
}

const char *rtttlBuiltinMelodyRandom(void)
{
    const int ix = rand() % (sizeof(skRtttlMelodies)/sizeof(*skRtttlMelodies));
    return skRtttlMelodies[ix];
}


//-------------------------------------------------------------------------------------------------------------

#define OCTAVE_OFFSET 0

void rtttlMelody(const char *melodyStr, int16_t *pFreqDur, const int nFreqDur)
{
    // from http://domoticx.com/arduino-melodie-afspelen-rtttl/
    // Copyright 2017 DomoticX

    // Absolutely no error checking in here

    const int16_t notes[] =
    { 0,
      RTTTL_NOTE_C4, RTTTL_NOTE_CS4, RTTTL_NOTE_D4, RTTTL_NOTE_DS4, RTTTL_NOTE_E4, RTTTL_NOTE_F4, RTTTL_NOTE_FS4, RTTTL_NOTE_G4, RTTTL_NOTE_GS4, RTTTL_NOTE_A4, RTTTL_NOTE_AS4, RTTTL_NOTE_B4,
      RTTTL_NOTE_C5, RTTTL_NOTE_CS5, RTTTL_NOTE_D5, RTTTL_NOTE_DS5, RTTTL_NOTE_E5, RTTTL_NOTE_F5, RTTTL_NOTE_FS5, RTTTL_NOTE_G5, RTTTL_NOTE_GS5, RTTTL_NOTE_A5, RTTTL_NOTE_AS5, RTTTL_NOTE_B5,
      RTTTL_NOTE_C6, RTTTL_NOTE_CS6, RTTTL_NOTE_D6, RTTTL_NOTE_DS6, RTTTL_NOTE_E6, RTTTL_NOTE_F6, RTTTL_NOTE_FS6, RTTTL_NOTE_G6, RTTTL_NOTE_GS6, RTTTL_NOTE_A6, RTTTL_NOTE_AS6, RTTTL_NOTE_B6,
      RTTTL_NOTE_C7, RTTTL_NOTE_CS7, RTTTL_NOTE_D7, RTTTL_NOTE_DS7, RTTTL_NOTE_E7, RTTTL_NOTE_F7, RTTTL_NOTE_FS7, RTTTL_NOTE_G7, RTTTL_NOTE_GS7, RTTTL_NOTE_A7, RTTTL_NOTE_AS7, RTTTL_NOTE_B7
    };

    //int16_t melody[2 * RTTTL_MELODY_N + 1];
    int16_t melodyIx = 0;

    uint8_t default_dur = 4;
    uint8_t default_oct = 6;
    int16_t bpm = 63;
    int16_t num;
    int32_t wholenote;
    int32_t duration;
    uint8_t note;
    uint8_t scale;
    const int melodyStrLen = strlen_P(melodyStr);
    char melodyCopy[melodyStrLen + 1];
    strcpy_P(melodyCopy, melodyStr);
    const char *p = melodyCopy;

    // format: d=N,o=N,b=NNN:
    // find the start (skip name, etc)

    while(*p != ':') p++;    // ignore name
    p++;                     // skip ':'

    // get default duration
    if(*p == 'd')
    {
        p++; p++;              // skip "d="
        num = 0;
        while(isdigit((int)(*p)))
        {
            num = (num * 10) + (*p++ - '0');
        }
        if(num > 0) default_dur = num;
        p++;                   // skip comma
    }

    //Serial.print("ddur: "); Serial.println(default_dur, 10);

    // get default octave
    if(*p == 'o')
    {
        p++; p++;              // skip "o="
        num = *p++ - '0';
        if(num >= 3 && num <=7) default_oct = num;
        p++;                   // skip comma
    }

    //Serial.print("doct: "); Serial.println(default_oct, 10);

    // get BPM
    if(*p == 'b')
    {
        p++; p++;              // skip "b="
        num = 0;
        while(isdigit((int)(*p)))
        {
            num = (num * 10) + (*p++ - '0');
        }
        bpm = num;
        p++;                   // skip colon
    }

    //Serial.print("bpm: "); Serial.println(bpm, 10);

    // BPM usually expresses the number of quarter notes per minute
    wholenote = (60 * 1000L / bpm) * 4;  // this is the time for whole note (in milliseconds)

    //Serial.print("wn: "); Serial.println(wholenote, 10);


    // now begin note loop
    while(*p)
    {
        // first, get note duration, if available
        num = 0;
        while(isdigit((int)(*p)))
        {
            num = (num * 10) + (*p++ - '0');
        }

        if(num) duration = wholenote / num;
        else duration = wholenote / default_dur;  // we will need to check if we are a dotted note after

        // now get the note
        note = 0;

        switch(*p)
        {
            case 'c':
                note = 1;
                break;
            case 'd':
                note = 3;
                break;
            case 'e':
                note = 5;
                break;
            case 'f':
                note = 6;
                break;
            case 'g':
                note = 8;
                break;
            case 'a':
                note = 10;
                break;
            case 'b':
                note = 12;
                break;
            case 'p':
            default:
                note = 0;
        }
        p++;

        // now, get optional '#' sharp
        if(*p == '#')
        {
            note++;
            p++;
        }

        // now, get optional '.' dotted note
        if(*p == '.')
        {
            duration += duration/2;
            p++;
        }

        // now, get scale
        if(isdigit((int)(*p)))
        {
            scale = *p - '0';
            p++;
        }
        else
        {
            scale = default_oct;
        }

        scale += OCTAVE_OFFSET;

        if(*p == ',')
            p++;       // skip comma for next note (or we may be at the end)

        // now play the note
        const int16_t dur = duration;
        if (note)
        {
            const int16_t freq = notes[(scale - 4) * 12 + note];
            pFreqDur[melodyIx] = freq;
            melodyIx++;
            //DEBUG("note: RTTTL %4d %4d", freq, dur);

            //Serial.print("Playing: ");
            //Serial.print(scale, 10); Serial.print(' ');
            //Serial.print(note, 10); Serial.print(" (");
            //Serial.print(notes[(scale - 4) * 12 + note], 10);
            //Serial.print(") ");
            //Serial.println(duration, 10);
            //tone1.play(notes[(scale - 4) * 12 + note]);
            //delay(duration);
            //tone1.stop();
        }
        else
        {
            pFreqDur[melodyIx] = RTTTL_NOTE_PAUSE;
            melodyIx++;
            //DEBUG("note: RTTTL ---- %4d", dur);

            //Serial.print("Pausing: ");
            //Serial.println(duration, 10);
            //delay(duration);
        }
        pFreqDur[melodyIx] = dur;
        melodyIx++;

        if (melodyIx > (nFreqDur - 3))
        {
            break;
        }
    }

    pFreqDur[melodyIx] = RTTTL_NOTE_END;
}

