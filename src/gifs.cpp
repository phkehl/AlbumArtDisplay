/*!
    \file
    \brief flipflip's Album Art Display: GIFs (see \ref FF_GIFs)

    - Copyright (c) 2018 Philippe Kehl (flipflip at oinkzwurgl dot org),
      https://oinkzwurgl.org/projaeggd/album-art-display
*/

#include <FS.h>
#include <SPIFFS.h>
#include <stdlib.h>

#include "debug.h"

#include "display.h"
#include "gifs.h"

/* ****************************************************************************************************************** */
#define MAX_GIFS 100
#define MAX_NAME 20
static char sGifFiles[MAX_GIFS][MAX_NAME];
static int sGifNumFiles;

static int sGifSort(const void *a, const void *b)
{
    return strcmp((const char *)a, (const char *)b);
}

void gifsInit(void)
{
    //DEBUG("gifs: init");
    if (!SPIFFS.begin(false))
    {
        WARNING("gifs: fail mount fs");
        return;
    }

    File root = SPIFFS.open("/");
    if (!root)
    {
        WARNING("gifs: no fs!?");
        return;
    }
    if (!root.isDirectory())
    {
        WARNING("gifs: no dir!?");
        return;
    }

    File file = root.openNextFile();
    while (file && (sGifNumFiles < MAX_GIFS))
    {
        if (file.isDirectory())
        {
            // ...
        }
        else
        {
            const char *name = file.name();
            const int len = strlen(name);
            if ( (len > 4) && (len < (MAX_NAME - 1)) && (strcmp(&name[len - 4], ".gif") == 0))
            {
                strcat(sGifFiles[sGifNumFiles], name);
                //DEBUG("gifs: [%d] %s", sGifNumFiles, name);
                sGifNumFiles++;
            }
        }
        file = root.openNextFile();
    }
    qsort(sGifFiles, sGifNumFiles, MAX_NAME, sGifSort);


    DEBUG("gifs: init (%d gifs, fs %d/%d used)", sGifNumFiles, (int)SPIFFS.usedBytes(), (int)SPIFFS.totalBytes());
}

// ---------------------------------------------------------------------------------------------------------------------

const char *gifsGetRandom(void)
{
    if (sGifNumFiles > 0)
    {
        const int ix = random(sGifNumFiles) % sGifNumFiles;
        return sGifFiles[ix];
    }
    else
    {
        return "/no_gifs_available";
    }
}

const char *gifsGetNext(void)
{
    if (sGifNumFiles > 0)
    {
        static int ix;
        const char *name = sGifFiles[ix];
        ix++;
        ix %= sGifNumFiles;
        return name;
    }
    else
    {
        return "/no_gifs_available";
    }
}

/* ****************************************************************************************************************** */
// eof
