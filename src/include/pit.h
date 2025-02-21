#pragma once

#include "cpu.h"
#include "devices.h"
#include "screen.h"

#define PIT_CHANNEL0      0x40 
#define PIT_CHANNEL1      0x41 
#define PIT_CHANNEL2      0x42 
#define PIT_CMDREG        0x43 

void PIT_Init(Machine *machine);
void PIT_Out();
void PIT_Set(int hz);

#pragma GCC push_options
#pragma GCC optimize ("O0")
#pragma GCC target("general-regs-only")

volatile unsigned long long PIT_TICKS;
void* PIT_CALLBACK_FUN;
void* PIT_CALLBACK_DAT;
__attribute__ ((interrupt)) void PIT_Handler(CPURegisters *) {
    asm volatile ("cli");
    PIT_TICKS++;
    if (PIT_CALLBACK_FUN != 0) ((void(*)(void*))PIT_CALLBACK_FUN)(PIT_CALLBACK_DAT);
    outb(0x20, 0x20);
    asm volatile ("sti");
}

#pragma GCC pop_options

void PIT_SetCallback(void* fun, void* dat) {
    PIT_CALLBACK_FUN = fun;
    PIT_CALLBACK_DAT = dat;
}

void PIT_Set(int hz) {
    int divisor = 1193182 / hz;       
    outb(PIT_CMDREG , 0x36);             
    outb(PIT_CHANNEL0, divisor & 0xFF);   
    outb(PIT_CHANNEL0, divisor >> 8);  
}

void PIT_Init(Machine *machine) {
    PIT_TICKS = 0;
    PIT_CALLBACK_FUN = 0;
    PIT_CALLBACK_DAT = 0;
    IDT_SetIndex(machine->idt.idet, 0x20, PIT_Handler);
    PIT_Set(1193180 / 1000);
    PIT_Out();
}

void PIT_Out() {
    PIC_EnableInterrupt(0);
}