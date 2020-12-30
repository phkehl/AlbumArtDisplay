/*!
    \file
    \brief flipflip's Album Art Display: handy stuff (see \ref FF_STUFF)

    - Copyright (c) 2020 Philippe Kehl (flipflip at oinkzwurgl dot org),
      https://oinkzwurgl.org/projaeggd/album-art-display

    \defgroup FF_STUFF STUFF
    \ingroup FF

    @{
*/

#include <pgmspace.h>
#include <ctype.h>

#include "stuff.h"

int strncmp_PP(const char *s1, const char *s2, int size)
{
   int res = 0;

   while (size > 0)
   {
       const char c1 = (char)pgm_read_byte(s1++);
       const char c2 = (char)pgm_read_byte(s2++);
       res = (int)c1 - (int)c2;
       if ( (res != 0) || (c2 == '\0') )
       {
           break;
       }
       size--;
   }

   return res;
}

char *strcasestr_P(const char *haystack, const char *needle)
{
    if (haystack[0] == 0)
    {
        if (pgm_read_byte(needle))
        {
            return NULL;
        }
        return (char *)haystack;
    }

    while (*haystack)
    {
        int i = 0;
        while (true)
        {
            char n = pgm_read_byte(needle + i);
            if (n == 0)
            {
                return (char *)haystack;
            }
            if (tolower((int)n) != tolower((int)haystack[i]))
            {
                break;
            }
            ++i;
        }
        ++haystack;
    }
    return NULL;
}

/* *********************************************************************************************** */

static uint32_t sTimePosix;
static uint32_t sTimeMillis;

void setTime(const uint32_t timestamp)
{
    sTimePosix = timestamp;
    sTimeMillis = millis();
}

uint32_t getTime(void)
{
    const uint32_t now = millis();
    return sTimePosix + ((now - sTimeMillis) / 1000);
}


/* *********************************************************************************************** */

//@}
// eof
