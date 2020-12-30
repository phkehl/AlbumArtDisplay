/*!
    \file
    \brief flipflip's Album Art Display: debugging output (see \ref FF_DEBUG)

    - Copyright (c) 2020 Philippe Kehl (flipflip at oinkzwurgl dot org),
      https://oinkzwurgl.org/projaeggd/album-art-display
*/

#include <Ticker.h>

#include "stuff.h"
#include "debug.h"

static Ticker sMonTicker;

static DEBUG_MON_FUNC_t sMonFuncs[10];
static int sMonFuncsNum;

void debugRegisterMon(DEBUG_MON_FUNC_t monFunc)
{
    if (sMonFuncsNum < NUMOF(sMonFuncs))
    {
        sMonFuncs[sMonFuncsNum++] = monFunc;
    }
}

// Task info, needs specially configured Arduino/ESP32 SDK see README.md
#if defined(ESP32)
#  include <freertos/FreeRTOS.h>
#  include <freertos/task.h>
#  if defined(CONFIG_FREERTOS_USE_TRACE_FACILITY)
#    define MAX_TASKS  25
static int sTaskSortFunc(const void *a, const void *b)
{
    //return (int)((const TaskStatus_t *)a)->xTaskNumber - (int)((const TaskStatus_t *)b)->xTaskNumber;

    const TaskStatus_t *pA = (const TaskStatus_t *)a;
    const TaskStatus_t *pB = (const TaskStatus_t *)b;
    if (pA->xCoreID != pB->xCoreID)
    {
        return (int)pA->xCoreID - (int)pB->xCoreID;
    }
    else if (pA->uxBasePriority != pB->uxBasePriority)
    {
        return (int)pB->uxBasePriority - (int)pA->uxBasePriority;
    }
    else
    {
        return (int)pA->xTaskNumber - (int)pB->xTaskNumber;
    }
}

static void sPrintTasks(void)
{
    const int nTasks = uxTaskGetNumberOfTasks();
    if (nTasks > MAX_TASKS)
    {
        ERROR("mon: too many tasks");
        return;
    }

    // allocate memory for tasks status
    const unsigned int allocSize = nTasks * sizeof(TaskStatus_t);
    TaskStatus_t *pTasks = (TaskStatus_t *)malloc(allocSize);
    if (pTasks == NULL)
    {
        ERROR("mon: malloc");
        return;
    }
    memset(pTasks, 0, allocSize);

    uint32_t totalRuntime;
    const int nnTasks = uxTaskGetSystemState(pTasks, nTasks, &totalRuntime);
    if (nTasks != nnTasks)
    {
        ERROR("mon: %u != %u", nTasks, nnTasks);
        free(pTasks);
        return;
    }
    
    // sort by task ID
    qsort(pTasks, nTasks, sizeof(TaskStatus_t), sTaskSortFunc);

    // total runtime (tasks, OS, ISRs) since we checked last
    static uint32_t sLastTotalRuntime;
    {
        const uint32_t runtime = totalRuntime;
        totalRuntime = totalRuntime - sLastTotalRuntime;
        sLastTotalRuntime = runtime;
    }
    // calculate time spent in each task since we checked last
    static uint32_t sLastRuntimeCounter[MAX_TASKS];
    uint32_t totalRuntimeTasks = 0;
    for (int ix = 0; ix < nTasks; ix++)
    {
        TaskStatus_t *pTask = &pTasks[ix];
        const uint32_t runtime = pTask->ulRunTimeCounter;
        pTask->ulRunTimeCounter = pTask->ulRunTimeCounter - sLastRuntimeCounter[ix];
        sLastRuntimeCounter[ix] = runtime;
        totalRuntimeTasks += pTask->ulRunTimeCounter;
    }

    // FIXME: why?
    if (totalRuntimeTasks > totalRuntime)
    {
        totalRuntime = totalRuntimeTasks;
    }

    // print tasks info
    for (int ix = 0; ix < nTasks; ix++)
    {
        const TaskStatus_t *pkTask = &pTasks[ix];
        char state = '?';
        switch (pkTask->eCurrentState)
        {
            case eRunning:   state = 'X'; break;
            case eReady:     state = 'R'; break;
            case eBlocked:   state = 'B'; break;
            case eSuspended: state = 'S'; break;
            case eDeleted:   state = 'D'; break;
            //case eInvalid:   state = 'I'; break;
        }
        const char core = pkTask->xCoreID == tskNO_AFFINITY ? '*' : ('0' + pkTask->xCoreID);

        char perc[8];
        if (pkTask->ulRunTimeCounter)
        {
            const double p = (double)pkTask->ulRunTimeCounter * 100.0 / (double)totalRuntimeTasks;
            if (p < 0.05)
            {
                strcpy(perc, "<0.1%");
            }
            else if (p > 100.0) // FIXME ulRunTimeCounter(s) is (are) completely wrong sometimes
            {
                strcpy(perc, "?.?%");
            }
            else
            {
                snprintf(perc, sizeof(perc), "%5.1f%%", p);
            }
        }
        else
        {
            strcpy(perc, "0.0%");
        }
        DEBUG("mon: tsk: %02d %-16s %c %c %2d-%2d %4u %6s",
            (int)pkTask->xTaskNumber, pkTask->pcTaskName, state, core,
            (int)pkTask->uxCurrentPriority, (int)pkTask->uxBasePriority,
            pkTask->usStackHighWaterMark, perc);
    }

    free(pTasks);
}

#  endif // CONFIG_FREERTOS_USE_TRACE_FACILITY
#endif // ESP32

static void sDebugMon(void)
{
    DEBUG("mon: -----------------------------------------------------------------------------------------------");
#if defined(ESP8266)
    uint32_t heapFree;
    uint16_t heapMax;
    uint8_t heapFrag;
    ESP.getHeapStats(&heapFree, &heapMax, &heapFrag);
    DEBUG("mon: debug: heap=%u/%u/%u%%", heapFree, heapMax, heapFrag);
#elif defined(ESP32)
    DEBUG("mon: debug: heap=%u/%u/%u (%u/%u/%u)",
        heap_caps_get_minimum_free_size(MALLOC_CAP_DEFAULT),
        heap_caps_get_largest_free_block(MALLOC_CAP_DEFAULT),
        heap_caps_get_free_size(MALLOC_CAP_DEFAULT),
        ESP.getMinFreeHeap(), ESP.getMaxAllocHeap(), ESP.getFreeHeap());
#endif
    for (int ix = 0; ix < sMonFuncsNum; ix++)
    {
        sMonFuncs[ix]();
    }

#if defined(ESP32) && defined(CONFIG_FREERTOS_USE_TRACE_FACILITY)
    sPrintTasks();
#endif // ESP32 && CONFIG_FREERTOS_USE_TRACE_FACILITY

    DEBUG("mon: -----------------------------------------------------------------------------------------------");
}

void debugInit(void)
{
    Serial.begin(115200);
#ifdef ESP8266
    Serial.setDebugOutput(true);
#endif
    Serial.println();
    Serial.println();
    Serial.println();
    DEBUG("debug: init");
    
    sMonTicker.attach_ms(5000, sDebugMon);
}

void HEXDUMP(const void *pkData, int size)
{
    const char hexdigits[] = "0123456789abcdef";
    const char *data = (const char *)pkData;
    for (int ix = 0; ix < size; )
    {
        char buf[128];
        memset(buf, ' ', sizeof(buf));
        for (int ix2 = 0; ix2 < 16; ix2++)
        {
            const uint8_t c = data[ix + ix2];
            buf[3 * ix2    ] = hexdigits[ (c >> 4) & 0xf ];
            buf[3 * ix2 + 1] = hexdigits[  c       & 0xf ];
            buf[3 * 16 + 2 + ix2] = isprint((int)c) ? c : '.';
            buf[3 * 16 + 3 + ix2] = '\0';
        }
        DEBUG("0x%08x  %s", (uint32_t)data + ix, buf);
        ix += 16;
    }
}

