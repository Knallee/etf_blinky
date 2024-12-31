
#include "pattern.h"
#include "scheduler.h"
#include "sys.h"
#include "bb_spi.h"
#include "button.h"
#include "timer.h"

#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>


#define BATTERY_STATUS_SHOW_TIME 2500

enum state
{
    ST_IDLE,
    ST_BTN2_PRESSED,
    ST_BTN1_PRESSED,
    ST_LIGHT_INCREASE,
    ST_LIGHT_DECREASE,
    ST_COLOR_NEXT,
    ST_COLOR_PREV,
};


int main(void)
{
    sys_init();

    enum state st = ST_IDLE;

    patternInit();
    patternFunc currentPat = patternNext();

    while (1)
    {
        event_s ev = evRun();

        switch (st)
        {
            case ST_IDLE:
                if (ev.code == EV_BUTTON_PRESSED)
                {
                    if (btnIsPressed(BUTTON1) && btnIsPressed(BUTTON2))
                    {
                        sys_batteryCheck();
                        _delay_ms(BATTERY_STATUS_SHOW_TIME);
                    }
                    else
                    {
                        st = (ev.eventData == BUTTON1) ? ST_BTN1_PRESSED : ST_BTN2_PRESSED;
                        event_s evHold;
                        evHold.code = EV_BUTTON_HOLD;
                        evHold.eventData = ev.eventData;
                        (void)evAdd(&evHold, &(uint32_t){TIME_500_MS});
                    }
                }
                break;
            case ST_BTN1_PRESSED:
                if (ev.code == EV_BUTTON_RELEASED)
                {
                    if (ev.eventData == BUTTON1) currentPat = patternPrevious();
                    st = ST_IDLE;
                }
                else if (ev.code == EV_BUTTON_PRESSED)
                {
                    if (ev.eventData == BUTTON1)
                    {
                        // This is an error, we should have detected button release before a new pressed event,
                        // most likely due to missed event
                        st = ST_IDLE;
                    }
                    else
                    {
                        st = ST_COLOR_NEXT;
                    }
                }
                else if (ev.code == EV_BUTTON_HOLD)
                {
                    if (ev.eventData == BUTTON1)
                    {
                        patternDecreaseIntensity();
                        event_s evHold;
                        evHold.code = EV_BUTTON_HOLD;
                        evHold.eventData = ev.eventData;
                        (void)evAdd(&evHold, &(uint32_t){TIME_500_MS});
                        st = ST_LIGHT_DECREASE;
                    }
                }
                break;
            case ST_BTN2_PRESSED:
                if (ev.code == EV_BUTTON_RELEASED)
                {
                    if (ev.eventData == BUTTON2) currentPat = patternNext();
                    st = ST_IDLE;
                }
                else if (ev.code == EV_BUTTON_PRESSED)
                {
                    if (ev.eventData == BUTTON1)
                    {
                        st = ST_COLOR_PREV;
                    }
                    else
                    {
                        // This is an error, we should have detected button release before a new pressed event,
                        // most likely due to missed event
                        st = ST_IDLE;
                    }
                }
                else if (ev.code == EV_BUTTON_HOLD)
                {
                    if (ev.eventData == BUTTON2)
                    {
                        patternIncreaseIntensity();
                        event_s evHold;
                        evHold.code = EV_BUTTON_HOLD;
                        evHold.eventData = ev.eventData;
                        (void)evAdd(&evHold, &(uint32_t){TIME_500_MS});
                        st = ST_LIGHT_INCREASE;
                    }
                }
                break;
            case ST_LIGHT_INCREASE:
                if (ev.code == EV_BUTTON_RELEASED)
                {
                    if (ev.eventData == BUTTON2) st = ST_IDLE;
                }
                else if (ev.code == EV_BUTTON_HOLD)
                {
                    patternIncreaseIntensity();
                    event_s evHold;
                    evHold.code = EV_BUTTON_HOLD;
                    evHold.eventData = ev.eventData;
                    (void)evAdd(&evHold, &(uint32_t){TIME_50_MS});
                }
                break;
            case ST_LIGHT_DECREASE:
                if (ev.code == EV_BUTTON_RELEASED)
                {
                    if (ev.eventData == BUTTON1) st = ST_IDLE;
                }
                else if (ev.code == EV_BUTTON_HOLD)
                {
                    patternDecreaseIntensity();
                    event_s evHold;
                    evHold.code = EV_BUTTON_HOLD;
                    evHold.eventData = ev.eventData;
                    evAdd(&evHold, &(uint32_t){TIME_50_MS});
                }
                break;
            case ST_COLOR_NEXT:
                if (ev.code == EV_BUTTON_RELEASED)
                {
                    if (ev.eventData == BUTTON1)
                    {
                        st = ST_IDLE;
                    }
                    else
                    {
                        patternNextColor();
                    }
                }
                break;
            case ST_COLOR_PREV:
                if (ev.code == EV_BUTTON_RELEASED)
                {
                    if (ev.eventData == BUTTON2)
                    {
                        st = ST_IDLE;
                    }
                    else
                    {
                        patternPreviousColor();
                    }
                }
                break;
        }

        if (ev.code == EV_BUTTON_ISR_DISABLED) btnEnableISR(true, ev.eventData);

        if (btnIsPressed(BUTTON_PWR))
        {
            if (patternShutdownSequence()) sys_powerOff();
        }

        currentPat();
    }
}
