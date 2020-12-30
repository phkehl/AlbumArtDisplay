/*!
    \file
    \brief flipflip's Album Art Display: wifi and network things (see \ref FF_WIFI)

    - Copyright (c) 2018 Philippe Kehl (flipflip at oinkzwurgl dot org),
      https://oinkzwurgl.org/projaeggd/album-art-display
*/

#include <WiFiClient.h>

#include "stuff.h"
#include "config.h"
#include "secrets.h"
#include "debug.h"

#include "lms.h"

// ---------------------------------------------------------------------------------------------------------------------

static WiFiClient sLmsClient;

void lmsInit(void)
{
    DEBUG("lms: init (%s:%d)", LMS_CLI_HOST, LMS_CLI_PORT);
}

// ---------------------------------------------------------------------------------------------------------------------

#define QUERY_TIMEOUT 2000 // [ms]

// Send query, get response
static String sLmsQuery(const char *query, const char *expect = NULL)
{
    DEBUG("lms: query [%s]", query);
    sLmsClient.println(query);
    const uint32_t t0 = millis();
    String resp = "";
    while ( (millis() - t0) < QUERY_TIMEOUT )
    {
        if (!sLmsClient.available())
        {
            delay(99);
            continue;
        }
        resp = sLmsClient.readStringUntil('\n');
        bool respOk = false;
        if (expect != NULL)
        {
            respOk = resp.indexOf(expect) >= 0;
        }
        else
        {
            respOk = true;
        }
        if (respOk)
        {
            resp.trim();
            DEBUG("lms: resp  [%s] (%d)", resp.length() > 60 ? (resp.substring(0, 60) + "...").c_str() : resp.c_str(), resp.length());
            break;
        }
    }
    if (resp.length() == 0)
    {
        WARNING("lms: no response");
    }
    return resp;
}

// Extract parameter from response
static String sLmsParam(const String &resp, const char *param, const bool decode = false)
{
    String res = "";
    char search[50];
    snprintf(search, sizeof(search), " %s%%3A", param); // " mode%3A"
    const int ix0 = resp.indexOf(search);
    if (ix0 > 0)
    {
        const int ix1 = resp.indexOf(' ', ix0 + 1);
        res = resp.substring(ix0 + strlen(param) + 1 + 3, ix1 > ix0 ? ix1 : resp.length());
        res.replace("%0A", "");
        res.replace("%0D", "");
    }
    if (decode)
    {
        res.replace("%3A", ":");
        res.replace("%20", " ");
        // TODO: decode all
    }
    return res;
}

// ---------------------------------------------------------------------------------------------------------------------

#define NUM_PLAYERS 10
typedef struct LMS_PLAYER_s
{
    String id;
    String model;
    String name;
    bool   playing;
    String title;
    bool   updated;
} LMS_PLAYER_t;

static LMS_PLAYER_t sLmsPlayers[NUM_PLAYERS];
static int sLmsPlayersCount = 0;
static String sLmsPlayerCurrentId = "";

// ---------------------------------------------------------------------------------------------------------------------

// Print current player knowledge
static void sLmsDumpStatus(void)
{
    for (int ix = 0; ix < sLmsPlayersCount; ix++)
    {
        PRINT("lms: %cPlayer %d/%d: id=[%s] model=[%s] name=[%s] playing=%s title=[%s] updated=%s",
            sLmsPlayers[ix].id == sLmsPlayerCurrentId ? '*' : ' ', ix + 1, sLmsPlayersCount,
            sLmsPlayers[ix].id.c_str(), sLmsPlayers[ix].model.c_str(),  sLmsPlayers[ix].name.c_str(),
            sLmsPlayers[ix].playing ? "yes" : "no",
            sLmsPlayers[ix].title.length() > 20 ? (sLmsPlayers[ix].title.substring(0, 20) + "...").c_str() : sLmsPlayers[ix].title.c_str(),
            sLmsPlayers[ix].updated ? "yes" : "no");
    }
}

// ---------------------------------------------------------------------------------------------------------------------

// Update player knowledge from status response
static bool sLmsUpdateStatus(String &resp)
{
    // Looks like status?
    // e.g. "00%3A04%3A20%3A27%3A7e%3A24 status - 1 subscribe%3ACJK player_name%3ALabor player_connected%3A1..."
    if (resp.indexOf(" status ") <= 0)
    {
        return false;
    }

    // Find player
    int ix = -1;
    for (int cand = 0; cand < sLmsPlayersCount; cand++)
    {
        if (sLmsPlayers[cand].id == resp.substring(0, sLmsPlayers[cand].id.length()))
        {
            ix = cand;
            break;
        }
    }
    if (ix < 0)
    {
        return false;
    }
    String power = sLmsParam(resp, "power");
    String mode  = sLmsParam(resp, "mode");
    String name  = sLmsParam(resp, "player_name", true);
    String title = sLmsParam(resp, "title", true);
    //DEBUG("lms: update %d: resp=[%s]", ix, resp.c_str());
    DEBUG("lms: update %d: power=%s mode=%s name=%s title=%s", ix, power.c_str(), mode.c_str(), name.c_str(), title.c_str());
    if (mode.length() == 0)
    {
        return false;
    }

    sLmsPlayers[ix].name  = name;
    const bool playing = (mode == "play") && (power == "1");
    sLmsPlayers[ix].updated = playing && (sLmsPlayers[ix].title != title);
    sLmsPlayers[ix].title   = playing ? title : "";
    sLmsPlayers[ix].playing = playing;

    return true;
}

// ---------------------------------------------------------------------------------------------------------------------

// Get all players and subscribe to status update
static void sLmsGetPlayers(void)
{
    PRINT("lms: Getting players...");

    // Clear all info
    sLmsPlayersCount = 0;
    for (int ix = 0; ix < NUM_PLAYERS; ix++)
    {
        sLmsPlayers[ix].id      = "";
        sLmsPlayers[ix].model   = "";
        sLmsPlayers[ix].playing = false;
        sLmsPlayers[ix].title   = "";
    }

    // Enumerate players
    int num = NUM_PLAYERS;
    bool haveCurrentPlayer = false;
    for (int ix = 0; ix < num; ix++)
    {
        char query[20];
        snprintf(query, sizeof(query), "players %d 1", ix);
        String resp = sLmsQuery(query, query); // e.g. "players 0 1 count%3A2 playerindex%3A0 playerid%3A0..."
        String count = sLmsParam(resp, "count");
        if ( (count.length() > 0) && (count.toInt() < num) )
        {
            num = count.toInt();
        }
        String playerid  = sLmsParam(resp, "playerid");
        String modelname = sLmsParam(resp, "modelname", true);
        String isplaying = sLmsParam(resp, "isplaying");
        String power     = sLmsParam(resp, "power");
        DEBUG("lms: get %d: playerid=[%s] modelname=[%s] isplaying=[%s] power=[%s]", sLmsPlayersCount, playerid.c_str(), modelname.c_str(), isplaying.c_str(), power.c_str());
        if (playerid.length() > 0)
        {
            sLmsPlayers[sLmsPlayersCount].id      = playerid;
            sLmsPlayers[sLmsPlayersCount].model   = modelname;
            // Synced players will say playing=1 even if power=0
            sLmsPlayers[sLmsPlayersCount].playing = (isplaying == "1") && (power == "1");
            sLmsPlayersCount++;
            if (sLmsPlayerCurrentId == playerid)
            {
                haveCurrentPlayer = true;
            }
        }
        else
        {
            break;
        }
    }

    if (!haveCurrentPlayer)
    {
        sLmsPlayerCurrentId = "";
    }

    if (num == 0)
    {
        WARNING("lms: no players online");
    }

    // Subscribe to status changes
    for (int ix = 0; ix < sLmsPlayersCount; ix++)
    {
        char query[100];
        snprintf(query, sizeof(query), "%s status - 1 subscribe:CJK", sLmsPlayers[ix].id.c_str());
        String resp = sLmsQuery(query, " status ");
        sLmsUpdateStatus(resp);
    }
}

// ---------------------------------------------------------------------------------------------------------------------

bool lmsConnect(void)
{
    DEBUG("lms: connecting to %s:%d", LMS_CLI_HOST, LMS_CLI_PORT);
    if (sLmsClient.connect(LMS_CLI_HOST, LMS_CLI_PORT) == 0)
    {
        ERROR("lms: Failed connecting to %s:%d!", LMS_CLI_HOST, LMS_CLI_PORT);
        return false;
    }

    return sLmsClient.connected() != 0;
}

// ---------------------------------------------------------------------------------------------------------------------

LMS_STATE_t lmsLoop(String &coverArtPlayerId)
{
    static uint32_t lastDumpStatus;
    static uint32_t lastGetPlayers;
    const uint32_t now = millis();

    // Are we still connected?
    if (sLmsClient.connected() == 0)
    {
        ERROR("lms: lost connection");
        return LMS_STATE_FAIL;
    }

    // Get full player info
    if ( (lastGetPlayers == 0) || ((now - lastGetPlayers) > ((sLmsPlayersCount > 0 ? 5 : 1) * 60 * 1000)) )
    {
        sLmsGetPlayers();
        lastGetPlayers = now;
        lastDumpStatus = 0;
    }

    // Update status
    bool statusUpdated = false;
    while (sLmsClient.available())
    {
        String resp = sLmsClient.readStringUntil('\n');
        resp.trim();
        if (sLmsUpdateStatus(resp))
        {
            statusUpdated = true;
        }
    }

    // Work out current display state (playing -> which cover art? not playing
    coverArtPlayerId = "";
    LMS_STATE_t res = LMS_STATE_STOPPED;

    // First check if current player is still playing and perhaps changed the song
    bool haveCurrentPlayer = false;
    for (int ix = 0; ix < sLmsPlayersCount; ix++)
    {
        if ( (sLmsPlayers[ix].id == sLmsPlayerCurrentId) && sLmsPlayers[ix].playing )
        {
            haveCurrentPlayer = true;
            res = LMS_STATE_PLAYING;
            if (sLmsPlayers[ix].updated)
            {
                coverArtPlayerId = sLmsPlayerCurrentId;
                lastDumpStatus = 0;
            }
            break;
        }
    }

    // Make first playing player the current one
    if (!haveCurrentPlayer)
    {
        for (int ix = 0; ix < sLmsPlayersCount; ix++)
        {
            if (sLmsPlayers[ix].playing)
            {
                sLmsPlayerCurrentId = sLmsPlayers[ix].id;
                haveCurrentPlayer = true;
                coverArtPlayerId = sLmsPlayerCurrentId;
                res = LMS_STATE_PLAYING;
                lastDumpStatus = 0;
                break;
            }
        }
    }
    
    // No players are playing
    if (!haveCurrentPlayer)
    {
        sLmsPlayerCurrentId = "";
    }

    // Dump status
    if ( statusUpdated || (lastDumpStatus == 0) || ((now - lastDumpStatus) > 10000) )
    {
        sLmsDumpStatus();
        lastDumpStatus = now;
    }
    
    // Cleanup
    for (int ix = 0; ix < sLmsPlayersCount; ix++)
    {
        sLmsPlayers[ix].updated = false;
    }

    return res;
}

// ---------------------------------------------------------------------------------------------------------------------
// eof
