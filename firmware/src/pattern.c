#include "pattern.h"
#include "bb_spi.h"
#include "button.h"
#include "adc.h"
#include "sys.h"
#include "timer.h"

#include <stdint.h>
#include <string.h>
#include <util/delay.h>
//#include <stdfix.h>


#define NUM_LEDS     10
#define MAX_PATTERNS 4
static patternFunc patterns[MAX_PATTERNS];
static uint8_t currPattern;

#define LED_INTENSITY_STEP 1
static uint8_t ledIntensity = 1;

enum color
{
    RED = 0,
    GREEN,
    BLUE,

    COLOR_MAX
};


struct __attribute__((packed)) LED_s
{
    uint8_t r;
    uint8_t g;
    uint8_t b;
    uint8_t alpha; //TODO(noxet): remove. seems to increase flash by 4B??
};


static enum color currColor = RED;
static struct LED_s ledState[10];


static void patternAudioCheck(void);
static void patternAudioMiddleOut(void);
static void patternKnightRider(void);
static void patternAudioLevel(uint16_t level);
static void patternAudioMiddleOutLevel(uint16_t level);
static void setLeds(void);
static void clearLeds(void);

//TODO(noxet): remove this later
static void staticColorRed(void)
{
    clearLeds();
    for (int i = 0; i < 10; i++)
    {
        switch (currColor)
        {
            case RED:
                ledState[i].r = 255;
                break;
            case GREEN:
                ledState[i].g = 255;
                break;
            case BLUE:
                ledState[i].b = 255;
                break;
            case COLOR_MAX:
                __builtin_unreachable();
            default:
                __builtin_unreachable();
        }
    }
    setLeds();
}

void patternInit(void)
{
    currPattern = MAX_PATTERNS - 1; // we call "patternNext" upon boot, so we start at the first
    patterns[0] = patternAudioCheck;
    patterns[1] = patternAudioMiddleOut;
    patterns[2] = patternKnightRider;
    patterns[3] = staticColorRed;
}


patternFunc patternNext(void)
{
    currPattern++;
    if (currPattern == MAX_PATTERNS) currPattern = 0;

    return patterns[currPattern];
}


patternFunc patternPrevious(void)
{
    if (currPattern == 0)
        currPattern = MAX_PATTERNS - 1;
    else
        currPattern--;

    return patterns[currPattern];
}


void patternIncreaseIntensity(void)
{
    // We only have 5 bits in the protocol, hence 31 being max
    if (ledIntensity >= 31 - LED_INTENSITY_STEP)
    {
        ledIntensity = 31;
    }
    else
    {
        ledIntensity += LED_INTENSITY_STEP;
    }
}


void patternDecreaseIntensity(void)
{
    // min val is 1 otherwise the LEDs will be turned off, which might be confusing
    if (ledIntensity <= LED_INTENSITY_STEP)
    {
        ledIntensity = 1;
    }
    else
    {
        ledIntensity -= LED_INTENSITY_STEP;
    }
}


void patternNextColor(void)
{
    if (currColor == COLOR_MAX)
    {
        currColor = 0;
    }
    else
    {
        currColor++;
    }
}


void patternPreviousColor(void)
{
    if (currColor == 0)
    {
        currColor = COLOR_MAX - 1;
    }
    else
    {
        currColor--;
    }
}


bool patternBootSequence(void)
{
    clearLeds();
    for (int i = 0; i < NUM_LEDS; i++)
    {
        ledState[i].g = 1 + (i + 1) * 10;
        setLeds();
        if (!btnIsPressed(BUTTON_PWR)) return false;
        _delay_ms(100);
    }

    return true;
}


bool patternShutdownSequence(void)
{
    clearLeds();
    for (int i = 0; i < NUM_LEDS; i++)
    {
        ledState[i].r = 1 + (i + 1) * 10;
    }

    setLeds();

    for (int i = NUM_LEDS - 1; i >= 0; i--)
    {
        ledState[i].r = 0;
        if (!btnIsPressed(BUTTON_PWR))
        {
            clearLeds();
            setLeds();
            return false;
        }
        _delay_ms(100);
        setLeds();
    }

    return true;
}


/**
 * The practical maximum of the batteries are 5V (3 * 1.65).
 * This is being divided by 2 due to the resistor divider, thus we will
 * read max 2.5V at the ADC. This value is then shifted down by 2
 * to fit into a uint8 variable.
 *
 * Thus, the maximum we will read is ca. 142, thus we set the upper limit around 130
 * to denote fully charged batteries. We set the lower limit to 50 (around 2V),
 * and we linearly interpolate the battery voltage from here.
 */
void patternBatteryLevel(uint8_t level)
{
    clearLeds();
    uint8_t limit = 50; // This is around 2V battery voltage.
    for (int i = 0; i < 10; i++)
    {
        if (level > limit)
        {
            ledState[i].r = 42;
        }
        limit += 9;
    }
    setLeds();
}


static void patternAudioLevel(uint16_t level)
{
    //clearLeds();

    uint8_t majColor = 0;
    uint8_t minColor = 0;

    switch (currColor)
    {
        case RED:
            majColor = RED;
            minColor = GREEN;
        break;
        case GREEN:
            majColor = GREEN;
            minColor = BLUE;
        break;
        case BLUE:
            majColor = BLUE;
            minColor = RED;
        break;
        case COLOR_MAX:
            __builtin_unreachable();
        default:
            __builtin_unreachable();
    }

    if (level > 250)
    {
        // index the LED struct, the hacky way..
        *((uint8_t *) (&ledState[9]) + majColor) = 127;
    }
    if (level > 225)
    {
        //ledState[8].r = 127;
        *((uint8_t *) (&ledState[8]) + majColor) = 127;
        *((uint8_t *) (&ledState[5]) + minColor) = 1;
    }
    if (level > 200)
    {
        //ledState[7].r = 127;
        *((uint8_t *) (&ledState[7]) + majColor) = 127;
        *((uint8_t *) (&ledState[5]) + minColor) = 5;
    }
    if (level > 175)
    {
        //ledState[6].r = 127;
        *((uint8_t *) (&ledState[6]) + majColor) = 127;
        *((uint8_t *) (&ledState[5]) + minColor) = 10;
    }
    if (level > 150)
    {
        *((uint8_t *) (&ledState[5]) + majColor) = 127;
        *((uint8_t *) (&ledState[5]) + minColor) = 15;
        //ledState[5].r = 127;
        //ledState[5].g = 1;
    }
    if (level > 125)
    {
        *((uint8_t *) (&ledState[4]) + majColor) = 127;
        *((uint8_t *) (&ledState[4]) + minColor) = 20;
        //ledState[4].r = 127;
        //ledState[4].g = 5;
    }
    if (level > 100)
    {
        *((uint8_t *) (&ledState[3]) + majColor) = 127;
        *((uint8_t *) (&ledState[3]) + minColor) = 25;
        //ledState[3].r = 127;
        //ledState[3].g = 10;
    }
    if (level > 75)
    {
        *((uint8_t *) (&ledState[2]) + majColor) = 127;
        *((uint8_t *) (&ledState[2]) + minColor) = 50;
        //ledState[2].r = 127;
        //ledState[2].g = 15;
    }
    if (level > 50)
    {
        *((uint8_t *) (&ledState[1]) + majColor) = 127;
        *((uint8_t *) (&ledState[1]) + minColor) = 75;
        //ledState[1].r = 127;
        //ledState[1].g = 20;
    }
    if (level > 0)
    {
        *((uint8_t *) (&ledState[0]) + majColor) = 127;
        *((uint8_t *) (&ledState[0]) + minColor) = 100;
        //ledState[0].r = 127;
        //ledState[0].g = 25;
    }
    setLeds();
}


static uint16_t getAudioAdcVal(void)
{
    adcSetAudioChannel();
    uint16_t dcoff = 400; // 350;
    adc_start();
    while (!adc_isDone())
    {
    }
    uint16_t level = adc_get_val();
    if (dcoff > level)
    {
        level = 0;
    }
    else
    {
        level -= dcoff;
    }

    return level;
}


static void patternAudioCheck(void)
{
    static uint16_t max = 0;
    static uint32_t prev = 0;
    uint16_t level = getAudioAdcVal();

    if (level >= max)
    {
        max = level;
        patternAudioLevel(max);
        //prev = timerGetUptime();
    }

    if (timerGetUptime() >= prev + TIME_50_MS)
    {
        clearLeds();
        max = 0;
        prev = timerGetUptime();
    }
}

uint8_t levels[5] = {0, 100, 150, 200, 250};
static void patternAudioMiddleOutLevel(uint16_t level)
{
    clearLeds();

    uint8_t majColor = 0;
    uint8_t minColor = 0;

    switch (currColor)
    {
        case RED:
            majColor = RED;
            minColor = GREEN;
        break;
        case GREEN:
            majColor = GREEN;
            minColor = BLUE;
        break;
        case BLUE:
            majColor = BLUE;
            minColor = RED;
        break;
        case COLOR_MAX:
            __builtin_unreachable();
        default:
            __builtin_unreachable();
    }

    for (int i = 0; i < 5; i++)
    {
        if (level > levels[i])
        {
            *((uint8_t *) (&ledState[4 - i]) + majColor) = 255;
            *((uint8_t *) (&ledState[4 - i]) + minColor) = 128 - i * 32;
            *((uint8_t *) (&ledState[5 + i]) + majColor) = 255;
            *((uint8_t *) (&ledState[5 + i]) + minColor) = 128 - i * 32;
            _delay_ms(5);
            setLeds();
        }
    }

    setLeds();
}


static void patternAudioMiddleOut(void)
{
    static uint16_t max = 0;
    static uint32_t prev = 0;
    uint16_t level = getAudioAdcVal();

    if (level >= max)
    {
        max = level;
        patternAudioMiddleOutLevel(max);
        prev = timerGetUptime();
    }

    if (timerGetUptime() >= prev + TIME_50_MS)
    {
        clearLeds();
        max = 0;
    }
}


static void patternKnightRider(void)
{
    static uint32_t prevBPM = 0;
    static uint32_t prevAnim = 0;
    static uint32_t dt = 10;
    static uint8_t dir = 1;
    static uint8_t idx = 1;

    uint8_t majColor = 0;
    uint8_t minColor = 0;

    switch (currColor)
    {
        case RED:
            majColor = RED;
            minColor = GREEN;
        break;
        case GREEN:
            majColor = GREEN;
            minColor = BLUE;
        break;
        case BLUE:
            majColor = BLUE;
            minColor = RED;
        break;
        case COLOR_MAX:
            __builtin_unreachable();
        default:
            __builtin_unreachable();
    }

    uint16_t level = getAudioAdcVal();

    const uint32_t currTime = timerGetUptime();
    if (level >= 200)
    {
        // need to cover 8 LEDs in this time
        dt = (currTime - prevBPM) >> 3;
        clearLeds();
        // TODO(noxet): schedule event to syncronize every 5s? i.e. set idx to 1 when kick hits
        //idx = 1;
        prevBPM = currTime;
        // "debounce" the reading, since the level for a kick hit may be longer than 1 ADC reading
        // causing a really quick "BPM"
        _delay_ms(100);
    }

    if (currTime - prevAnim > dt)
    {
        clearLeds();
        *((uint8_t *) (&ledState[idx - 1]) + majColor) = 200;
        *((uint8_t *) (&ledState[idx - 1]) + minColor) = 10;
        *((uint8_t *) (&ledState[idx]) + majColor) = 255;
        *((uint8_t *) (&ledState[idx + 1]) + majColor) = 200;
        *((uint8_t *) (&ledState[idx + 1]) + minColor) = 10;
        if (idx == 8) dir = -1;
        if (idx == 1) dir = 1;
        idx += dir;
        setLeds();
        prevAnim = currTime;
    }
}


static void setLeds(void)
{
    bb_spi_byte(0);
    bb_spi_byte(0);
    bb_spi_byte(0);
    bb_spi_byte(0);

    for (int i = 0; i < NUM_LEDS; i++)
    {
        bb_spi_byte(0xE0 | ledIntensity);
        bb_spi_byte(ledState[i].b);
        bb_spi_byte(ledState[i].g);
        bb_spi_byte(ledState[i].r);
    }

    bb_spi_byte(255);
    bb_spi_byte(255);
    bb_spi_byte(255);
    bb_spi_byte(255);
}


static void clearLeds(void)
{
    memset(ledState, 0, NUM_LEDS * sizeof(struct LED_s));
}
