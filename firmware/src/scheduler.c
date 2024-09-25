#include "scheduler.h"
#include "sys.h"
#include "timer.h"


#ifndef DL_MAX_EVENTS
    #warning "DL_MAX_EVENTS not defined, defaults to 5"
    #define DL_MAX_EVENTS 5
#endif


static volatile event_s g_eventQueue[DL_MAX_EVENTS];
static volatile uint8_t g_eventToRun = 0;
static volatile uint8_t g_nextFreeEvent = 0;

enum scheduleMode
{
    SCHEDULE,
    RESCHEDULE,
};

static bool evSchedule(event_s ev, uint32_t delay, enum scheduleMode mode);


void evInit(void)
{
    uint8_t status = sys_enterCritical();
    for (int i = 0; i < DL_MAX_EVENTS; i++)
    {
        g_eventQueue[i].code = EV_NOP;
    }
    sys_exitCritical(status);
}


/**
 * Add event to the queue, with a delay of "delay" time.
 * The delay time shall be the ones defined in the timer.h file,
 * such as TIMER_NOW, TIMER_100_MS etc.
 */
bool evAdd(event_s ev, uint32_t delay)
{
    return evSchedule(ev, delay, SCHEDULE);
}


/**
 * The event handler.
 * Polls an event from the queue and returns it.
 * If the event is scheduled for a later time, it will be pop'd
 * and re-pushed to the queue to avoid fragmentation.
 */
event_s evRun(void)
{
#ifndef TEST
    // thread sanitizer complains about this, so remove during test (only affects performance)
    if (g_eventQueue[g_eventToRun].code == EV_NOP) return (event_s) NEW_EVENT();
#endif

    uint8_t status = sys_enterCritical();
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
        evSchedule(ev, 0, RESCHEDULE);
        sys_exitCritical(status);
        return (event_s) NEW_EVENT();
    }

    sys_exitCritical(status);

    return ev;
}


static bool evSchedule(event_s ev, uint32_t delay, enum scheduleMode mode)
{
#ifndef TEST
    // thread sanitizer complains about this, so remove during test (only affects performance)
    if (g_eventQueue[g_nextFreeEvent].code != EV_NOP) return false;
#endif

    uint8_t status = sys_enterCritical();
    if (g_eventQueue[g_nextFreeEvent].code != EV_NOP)
    {
        sys_exitCritical(status);
        return false;
    }

    g_eventQueue[g_nextFreeEvent].code = ev.code;
    g_eventQueue[g_nextFreeEvent].eventData = ev.eventData;
    switch (mode)
    {
        case SCHEDULE:
            g_eventQueue[g_nextFreeEvent].timeToRun = timerGetUptime() + delay;
            break;
        case RESCHEDULE:
            g_eventQueue[g_nextFreeEvent].timeToRun = ev.timeToRun;
            break;
    }
    g_nextFreeEvent = (g_nextFreeEvent + 1) % DL_MAX_EVENTS;

    sys_exitCritical(status);
    return true;
}
