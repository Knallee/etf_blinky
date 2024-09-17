#include "button.h"
#include "scheduler.h"
#include "sys.h"

#include <avr/interrupt.h>
#include <avr/io.h>
#include <stdbool.h>
#include <util/delay.h>

// PCINT7 and PCINT10
#define BTN1_PCINT	2
#define BTN2_PCINT	7

#define BTN_PWR 0
#define BTN1	2
#define BTN2	7

#define BTN1_INTERRUPT	PCINT1_vect
#define BTN2_INTERRUPT	PCINT0_vect


static volatile uint8_t btn1_pressed, btn1_prev, btn1_crnt, btn2_pressed, btn2_prev, btn2_crnt;

void button_init()
{		
	GIMSK |= (1 << PCIE1) | (1 << PCIE0);
	PCMSK0 |= (1 << BTN2_PCINT); // PCINT7
	PCMSK1 |= (1 << BTN1_PCINT); // PCINT10
	
	DDRB &= ~(1 << BTN1);
	DDRA &= ~(1 << BTN2);
	DDRB &= ~(1 << BTN_PWR);
}

void btnPressed(u16 data)
{
    if (data == 1)
    {
        sys_debugLedOn(true);
    }
    else
    {
        sys_debugLedOn(false);
    }
}


bool btnPwrPressed(void)
{
    return (PINB & (1 << BTN_PWR)) ? true : false;
}


ISR(BTN1_INTERRUPT)
{
    btn1_crnt = (PINB & (1 << BTN1)) >> BTN1;
	
	if (btn1_crnt > btn1_prev)
    {
		btn1_pressed = 1;
        event_s ev = {
            .code = EV_BUTTON_PRESSED,
            .eventData = 1,
        };

        dl_addEvent(ev);
	}
    else
    {
        btn1_pressed = 0;
    }
	
	btn1_prev = btn1_crnt;

    //sys_powerOff
}

ISR(BTN2_INTERRUPT)
{
    btn2_crnt = (PINA & (1 << BTN2)) >> BTN2;
    
    if (btn2_crnt > btn2_prev)
    {
	    btn2_pressed = 1;
        event_s ev = {
            .code = EV_BUTTON_PRESSED,
            .eventData = 2,
        };

        dl_addEvent(ev);
    }
    else
    {
        btn2_pressed = 0;
    }

    btn2_prev = btn2_crnt;
}

