#ifndef TIMER_DEAKIN_H
#define TIMER_DEAKIN_H

#include "sam.h"

void TCC0_init(uint32_t ms_tick); // Initialize TCC0 to trigger interrupt every ms_tick milliseconds

#endif
