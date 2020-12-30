/*!
    \file
    \brief flipflip's Album Art Display: handy stuff (see \ref FF_STUFF)

    - Copyright (c) 2020 Philippe Kehl (flipflip at oinkzwurgl dot org),
      https://oinkzwurgl.org/projaeggd/album-art-display

    \defgroup FF_STUFF STUFF
    \ingroup FF

    @{
*/
#ifndef __STUFF_H__
#define __STUFF_H__

#include <Arduino.h>

#ifdef __cplusplus
extern "C" {
#endif


/* ***** handy macros **************************************************************************** */

/*!
    \name Handy Macros
    @{
*/

//#define BIT(bit) (1<<(bit))   //!< bit
#ifndef UNUSED
#  define UNUSED(foo) (void)foo //!< unused variable
#endif
#ifndef NULL
#  define NULL (void *)0    //!< null pointer     \hideinitializer
#endif /* NULL */
#define NUMOF(x) (sizeof(x)/sizeof(*(x)))       //!< number of elements in vector     \hideinitializer
#define ENDLESS true          //!< for endless while loops     \hideinitializer
#define FALLTHROUGH           //!< switch fall-through marker     \hideinitializer
#define __PAD(n) uint8_t __PADNAME(__LINE__)[n]  //!< struct padding macro     \hideinitializer
#define __PADFILL { 0 }           //!< to fill const padding     \hideinitializer
#define MIN(a, b)  ((b) < (a) ? (b) : (a)) //!< smaller value of a and b     \hideinitializer
#define MAX(a, b)  ((b) > (a) ? (b) : (a)) //!< bigger value of a and b     \hideinitializer
#define ABS(a) ((a) > 0 ? (a) : -(a)) //!< absolute value     \hideinitializer
#define CLIP(x, a, b) ((x) <= (a) ? (a) : ((x) >= (b) ? (b) : (x))) //!< clip value in range [a:b]     \hideinitializer
#define STRINGIFY(x) _STRINGIFY(x) //!< stringify argument     \hideinitializer
#define CONCAT(a, b) _CONCAT(a, b) //!< concatenate arguments     \hideinitializer
#define SWAP2(x) ( (( (x) >>  8)                                                             | ( (x) <<  8)) )
#define SWAP4(x) ( (( (x) >> 24) | (( (x) & 0x00FF0000) >>  8) | (( (x) & 0x0000FF00) <<  8) | ( (x) << 24)) )
//@}

#ifndef __DOXYGEN__
#  define _STRINGIFY(x) #x
#  define _CONCAT(a, b)  a ## b
#  define ___PADNAME(x) __pad##x
#  define __PADNAME(x) ___PADNAME(x)
#endif

//! \name Compiler Hints etc.
//@{
#define __PURE()              __attribute__ ((pure))          //!< pure \hideinitializer
#define __IRQ()               __attribute__ ((interrupt))     //!< irq \hideinitializer
#define __WEAK()              __attribute__ ((weak))          //!< weak \hideinitializer
#define __PACKED              __attribute__ ((packed))        //!< packed \hideinitializer
#define __ALIGN(n)            __attribute__ ((aligned (n)))   //!< align \hideinitializer
#ifdef __INLINE
#  undef __INLINE
#endif
#define __INLINE              inline                                 //!< inline \hideinitializer
#define __NOINLINE            __attribute__((noinline))              //!< no inline \hideinitializer
#define __FORCEINLINE         __attribute__((always_inline)) inline  //!< force inline (also with -Os) \hideinitializer
#define __USED                __attribute__((used))                  //!< used \hideinitializer
#define __NORETURN            __attribute__((noreturn))              //!< no return \hideinitializer
#define __PRINTF(six, aix)    __attribute__((format(printf, six, aix))) //!< printf() style func \hideinitializer
#define __SECTION(sec)        __attribute__((section (STRINGIFY(sec)))); //!< place symbol in section \hideinitializer
#define __NAKED               __attribute__((naked))                 //!< naked function \hideinitializer
//@}


/* ***** handy macros **************************************************************************** */

/*!
    \name Handy pgmspace functions
    
    Some functions that are not in pgmspace.h.
    
    S.a. https://arduino-esp8266.readthedocs.io/en/latest/PROGMEM.html#functions-to-read-back-from-progmem
    
    @{
*/
#ifdef ESP8266

//! ESP8266 base address for flash mapped for code execution (I think)
#define ESP8266_FLASH_BASE 0x40200000

//! size of mapped flash region (apparently one MB can be mapped for code execution)
#define ESP8266_FLASH_SIZE   0x100000

//! ESP8266 base address for the data RAM
#define ESP8266_DRAM_BASE  0x3ffe8000

//! ESP8266 size of the data RAM (80kb)
#define ESP8266_DRAM_SIZE     0x14000

//! evaluates to true of the given address is in the data RAM \hideinitializer
#define ESP8266_IS_IN_DRAM(p) ( ((const void *)(p) < (const void *)ESP_DRAM_BAESP8266_DRAM_BASESE) || ((const void *)(p) > (const void *)(ESP8266_DRAM_BASE + ESP8266_DRAM_SIZE)) ? false : true )

#endif // ESP8266

// strncmp() that can use both strings from ROM (and RAM)
int strncmp_PP(const char *s1, const char *s2, int size);

// strcmp() that can use both strings from ROM (and RAM)
#define strcmp_PP(s1, s2) strncmp_PP(s1, s2, INT32_MAX)

// strcasestr() that can find needle from ROM (and RAM)
char *strcasestr_P(const char *haystack, const char *needle);

//@}

/* *********************************************************************************************** */


/* ***** funky functions ************************************************************************* */

/*!
    \name Some funky functions
    @{
*/

//! set POSIX time
void setTime(const uint32_t timestamp);

//! get (approximate) POSIX time
uint32_t getTime(void);

/* *********************************************************************************************** */

#ifdef __cplusplus
}
#endif

#endif // __STUFF_H__
//@}
// eof

