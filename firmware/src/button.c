#include "button.h"
#include "scheduler.h"
#include "timer.h"

#include <avr/interrupt.h>
#include <avr/io.h>
#include <stdbool.h>
#include <stdint.h>
#include <util/delay.h>

// NOTE(dist): Use this macro to toggle between "branchless" and switch-statement
//             implementations. It seems like switch-statements are very well
//             optimized on AVR targets when using __builtin_unreachable()!
#define USE_BRANCHLESS_FUNCTIONS 0

// NOTE(dist): Use this macro to force btn_handler() to be inlined in ISRs
#define FORCE_BTN_HANDLER_INLINE 0

#if FORCE_BTN_HANDLER_INLINE
#define BTN_HANDLER_INLINE __attribute((always_inline))
#else
#define BTN_HANDLER_INLINE
#endif

// PCINT7 and PCINT10
#define BTN1_PCINT 2
#define BTN2_PCINT 7

#define BTN_PWR 0
#define BTN1    2
#define BTN2    7

#define BTN1_INTERRUPT PCINT1_vect
#define BTN2_INTERRUPT PCINT0_vect

void btnInit()
{
    GIMSK |= (1 << PCIE1) | (1 << PCIE0);

    btnEnableISR(true, BUTTON1);
    btnEnableISR(true, BUTTON2);
    // NOTE(dist): No interrupt on BUTTON_PWR

    DDRB &= ~(1 << BTN1);
    DDRA &= ~(1 << BTN2);
    DDRB &= ~(1 << BTN_PWR);
}

#if USE_BRANCHLESS_FUNCTIONS
static volatile uint8_t btnPrev[NUM_BUTTONS] = {0, 0, 0};

BTN_HANDLER_INLINE
static inline void btn_handler(enum button btn);

static volatile uint8_t* const pcmsk[NUM_BUTTONS] = {&PCMSK1, &PCMSK0, 0};
static volatile uint8_t* const btnPort[NUM_BUTTONS] = {&PINB, &PINA, &PINB};
static const uint8_t btnMask[NUM_BUTTONS] = {(1 << BTN1), (1 << BTN2), (1 << BTN_PWR)};

bool btnIsPressed(enum button btn)
{
    return *btnPort[btn] & btnMask[btn];
}

void btnEnableISR(bool enable, enum button btn)
{
    if (btn == BUTTON_PWR) return;

    if (enable) {
        *pcmsk[btn] |= btnMask[btn];
        return;
    }

    *pcmsk[btn] &= ~btnMask[btn];
}

BTN_HANDLER_INLINE
static inline void btn_handler(enum button btn)
{
    event_s ev = NEW_EVENT();
    ev.eventData = btn;

    if (btnIsPressed(btn)) {
        ev.code = EV_BUTTON_RELEASED;
    } else {
        ev.code = EV_BUTTON_RELEASED;
    }
    (void)evAdd(&ev, &(uint32_t){TIME_50_MS});

    // NOTE(noxet): There is a possible HW bug where we get multiple interrupt on BTN2.
    // possibly due to slow decay from debounce filter together with SPI being routed too close.
    btnEnableISR(false, btn);

    ev.code = EV_BUTTON_ISR_DISABLED;
    (void)evAdd(&ev, &(uint32_t){TIME_50_MS});
}

ISR(BTN1_INTERRUPT)
{
    // NOTE(dist): From datasheet "Any change on any enabled PCINT[11:8] pin will cause an interrupt"
    btn_handler(BUTTON1);
}

ISR(BTN2_INTERRUPT)
{
    // NOTE(dist): From datasheet "Any change on any enabled PCINT[11:8] pin will cause an interrupt"
    btn_handler(BUTTON2);
}
#else /* USE_BRANCHLESS_FUNCTIONS */
BTN_HANDLER_INLINE
static inline void btn_handler(enum button btn);

bool btnIsPressed(enum button btn)
{
    volatile uint8_t* port;
    uint8_t btn_mask;

    switch (btn) {
        case BUTTON1:
            port = &PINB;
            btn_mask = 1 << BTN1;
            break;
        case BUTTON2:
            port = &PINA;
            btn_mask = 1 << BTN2;
            break;
        case BUTTON_PWR:
            port = &PINB;
            btn_mask = 1 << BTN_PWR;
            break;
        case NUM_BUTTONS:
            __builtin_unreachable();
        default:
            __builtin_unreachable();
    }

    return *port & btn_mask;
}

void btnEnableISR(bool enable, enum button btn)
{
    volatile uint8_t* pcmsk;
    uint8_t btn_mask;

    if (btn == BUTTON_PWR) return;

    switch (btn) {
        case BUTTON1:
            pcmsk = &PCMSK1;
            btn_mask = 1 << BTN1;
            break;
        case BUTTON2:
            pcmsk = &PCMSK0;
            btn_mask = 1 << BTN2;
            break;
        case BUTTON_PWR:
            __builtin_unreachable();
        case NUM_BUTTONS:
            __builtin_unreachable();
        default:
            __builtin_unreachable();
    }

    if (enable) {
        *pcmsk |= btn_mask;
        return;
    }

    *pcmsk &= ~btn_mask;
}

BTN_HANDLER_INLINE
static inline void btn_handler(enum button btn)
{
    event_s ev = NEW_EVENT();
    ev.eventData = btn;

    if (btnIsPressed(btn)) {
        ev.code = EV_BUTTON_PRESSED;
    } else {
        ev.code = EV_BUTTON_RELEASED;
    }
    (void)evAdd(&ev, &(uint32_t){TIME_50_MS});

    // NOTE(noxet): There is a possible HW bug where we get multiple interrupt on BTN2.
    // possibly due to slow decay from debounce filter together with SPI being routed too close.
    btnEnableISR(false, btn);

    ev.code = EV_BUTTON_ISR_DISABLED;
    (void)evAdd(&ev, &(uint32_t){TIME_50_MS});
}

ISR(BTN1_INTERRUPT)
{
    // NOTE(dist): From datasheet "Any change on any enabled PCINT[11:8] pin will cause an interrupt"

    btn_handler(BUTTON1);
}

ISR(BTN2_INTERRUPT)
{
    // NOTE(dist): From datasheet "Any change on any enabled PCINT[11:8] pin will cause an interrupt"

    btn_handler(BUTTON2);
}
#endif /* USE_BRANCHLESS_FUNCTIONS */
