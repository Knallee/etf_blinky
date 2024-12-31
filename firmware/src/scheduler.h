#pragma once

#include <stdbool.h>
#include <stdint.h>


enum eventCode
{
    EV_NOP,
    EV_BUTTON_PRESSED,
    EV_BUTTON_RELEASED,
    EV_BUTTON_HOLD,
    EV_BUTTON_ISR_DISABLED,
};

#define NEW_EVENT()                                                                                                    \
    {                                                                                                                  \
        EV_NOP, 0, 0                                                                                                   \
    }


// TODO(noxet): To save space, we can use a bitfield for the eventdata and timeToRun, e.g.
// uint32_t eventData : 8;
// uint32_t timeToRun : 24;
typedef struct
{
    uint32_t timeToRun;
    uint16_t eventData;
    enum eventCode code;
} event_s;


void evInit(void);

/**
 * Add event to the queue, with a delay of "delay" time.
 * The delay time shall be the ones defined in the timer.h file,
 * such as TIMER_NOW, TIMER_100_MS etc.
 */
bool evAdd(event_s* ev, uint32_t* delay);

/**
 * The event handler.
 * Polls an event from the queue and returns it.
 * If the event is scheduled for a later time, it will be pop'd
 * and re-pushed to the queue to avoid fragmentation.
 */
event_s evRun(void);
