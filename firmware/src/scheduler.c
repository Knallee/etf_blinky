#include "scheduler.h"
#include "sys.h"
#include "timer.h"

#include "dl_utils.h"

#ifndef DL_MAX_EVENTS
    #warning "DL_MAX_EVENTS not defined, defaults to 5"
    #define DL_MAX_EVENTS 5
#endif


static volatile event_s g_eventQueue[DL_MAX_EVENTS];
static volatile u8 g_eventToRun = 0;
static volatile u8 g_nextFreeEvent = 0;

void evInit(void)
{
    u8 status = sys_enterCritical();
    for (int i = 0; i < DL_MAX_EVENTS; i++)
    {
        g_eventQueue[i].code = EV_NOP;
    }
    sys_exitCritical(status);
}


bool evAdd(event_s ev)
{
    return evSchedule(ev, 0, false);
}


bool evSchedule(event_s ev, uint32_t delay, bool reSchedule)
{
#ifndef TEST
    // thread sanitizer complains about this, so remove during test (only affects performance)
    if (g_eventQueue[g_nextFreeEvent].code != EV_NOP) return false;
#endif

    u8 status = sys_enterCritical();
    if (g_eventQueue[g_nextFreeEvent].code != EV_NOP)
    {
        sys_exitCritical(status);
        return false;
    }

    g_eventQueue[g_nextFreeEvent].code = ev.code;
    g_eventQueue[g_nextFreeEvent].eventData = ev.eventData;
    g_eventQueue[g_nextFreeEvent].timeToRun = reSchedule ? ev.timeToRun : timerGetUptime() + delay;
    g_nextFreeEvent = (g_nextFreeEvent + 1) % DL_MAX_EVENTS;

    sys_exitCritical(status);
    return true;
}


event_s evRun(void)
{
#ifndef TEST
    // thread sanitizer complains about this, so remove during test (only affects performance)
    if (g_eventQueue[g_eventToRun].code == EV_NOP) return (event_s) NEW_EVENT();
#endif

    u8 status = sys_enterCritical();
    if (g_eventQueue[g_eventToRun].code == EV_NOP)
    {
        sys_exitCritical(status);
        return (event_s) NEW_EVENT();
    }

    event_s ev = g_eventQueue[g_eventToRun];
    g_eventQueue[g_eventToRun].code = EV_NOP;
    g_eventQueue[g_eventToRun].eventData = 0;
    g_eventToRun = (g_eventToRun + 1) % DL_MAX_EVENTS;
    if (timerGetUptime() < ev.timeToRun)
    {
        // re-schedule this
        evSchedule(ev, 0, true);
        sys_exitCritical(status);
        return (event_s) NEW_EVENT();
    }


    sys_exitCritical(status);

    return ev;
}

