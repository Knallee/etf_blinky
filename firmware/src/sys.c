#include "sys.h"

#include "adc.h"
#include "bb_spi.h"
#include "button.h"
#include "pattern.h"
#include "timer.h"

#include <avr/interrupt.h>
#include <avr/io.h>

#define SYS_PWR_EN_PORT (PORTA)
#define SYS_PWR_EN_PIN  (3)
#define SYS_DBG_LED     (1 << PB1)

void sys_audioCheck(void);

void sys_init(void)
{
    DDRB |= SYS_DBG_LED;
    btnInit();
    sys_powerOn();
    //evInit();
    bb_spi_init();
    if (!patternBootSequence()) sys_powerOff();

    // don't fully start the system until user releases the pwr button
    while (btnIsPressed(BUTTON_PWR));


    adc_init();
    adcSetAudioChannel();
    adc_interrupt_enable();
    timerInit();
    timerStart();
    sei();
    // while (1) sys_audioCheck();
}

uint8_t sys_enterCritical(void)
{
    uint8_t status = SREG;
    cli();
    return status;
}

void sys_exitCritical(uint8_t status)
{
    SREG = status;
}

void sys_powerOn(void)
{
    DDRA |= (1 << SYS_PWR_EN_PIN);
    SYS_PWR_EN_PORT |= (1 << SYS_PWR_EN_PIN);
}

void sys_powerOff(void)
{
    // wait until the pwr button is released, otherwise we can not turn the device off
    while (btnIsPressed(BUTTON_PWR));

    SYS_PWR_EN_PORT &= ~(1 << SYS_PWR_EN_PIN);
}

void sys_debugLedOn(bool on)
{
    if (on)
    {
        PORTB |= SYS_DBG_LED;
    }
    else
    {
        PORTB &= ~SYS_DBG_LED;
    }
}

void sys_batteryCheck(void)
{
    adcSetVbatChannel();
    adc_start();
    while (!adc_isDone())
    {
    }
    // We don't need all that (10-bit) resolution
    uint8_t level = adc_get_val() >> 2;
    patternBatteryLevel(level);
}
