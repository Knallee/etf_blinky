#pragma once

#include <stdbool.h>

enum button
{
    BUTTON1 = 0,
    BUTTON2,
    BUTTON_PWR,
    NUM_BUTTONS
};

void btnInit();
bool btnIsPressed(enum button btn);
void btnEnableISR(bool enable, enum button btn);

