#pragma once

#include "pit.h"

unsigned long TIME_SecondsSinceBoot();
unsigned long long TIME_MillisSinceBoot();
void TIME_SleepMS(unsigned long long ms);

unsigned long long TIME_MillisSinceBoot() {
    return PIT_TICKS;
}

unsigned long TIME_SecondsSinceBoot() {
    if (TIME_MillisSinceBoot() < 1000) return 0;
    return TIME_MillisSinceBoot()/1000;
}

#pragma GCC push_options
#pragma GCC optimize ("O0")
#pragma GCC target("general-regs-only")

void TIME_SleepMS(unsigned long long ms) {
    volatile unsigned long long begin = TIME_MillisSinceBoot();
    while (TIME_MillisSinceBoot() < begin + ms) {}
}

#pragma GCC pop_options
