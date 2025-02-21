//Boot asm 0x10000C (0+12b)

#ifndef KERNEL_C
#define KERNEL_C

#include "include/screen.h"
#include "include/cpu.h"
#include "include/devices.h"
#include "include/pit.h"
#include "include/time.h"

void main(Machine *machine);

#include "include/kernel.h"
#include "game.c"

void CALLBACK_UpdateTimeOS(void* scr) {
    ScreenConsole *vga = (ScreenConsole *) scr;
    char timeDateBuffer[24];
    timeDateBuffer[0] = 0;
    CMOSRTC rtc = CMOS_ReadRTC();
    unsigned char* ptr = vga->current;
    append_uint32_str_std((int) rtc.hour + 100, timeDateBuffer);
    Memory_ShiftLeft((unsigned char*) timeDateBuffer+strlen(timeDateBuffer)-3, 4);
    Memory_StrCat((unsigned char*) timeDateBuffer, (unsigned char*) ":");
    append_uint32_str_std((int) rtc.minute + 100, timeDateBuffer);
    Memory_ShiftLeft((unsigned char*) timeDateBuffer+strlen(timeDateBuffer)-3, 4);
    Memory_StrCat((unsigned char*) timeDateBuffer, (unsigned char*) ":");
    append_uint32_str_std((int) rtc.second + 100, timeDateBuffer);
    Memory_ShiftLeft((unsigned char*) timeDateBuffer+strlen(timeDateBuffer)-3, 4);
    Memory_StrCat((unsigned char*) timeDateBuffer, (unsigned char*) " ");
    append_uint32_str_std((int) rtc.day + 100, timeDateBuffer);
    Memory_ShiftLeft((unsigned char*) timeDateBuffer+strlen(timeDateBuffer)-3, 4);
    Memory_StrCat((unsigned char*) timeDateBuffer, (unsigned char*) "/");
    append_uint32_str_std((int) rtc.month + 100, timeDateBuffer);
    Memory_ShiftLeft((unsigned char*) timeDateBuffer+strlen(timeDateBuffer)-3, 4);
    Memory_StrCat((unsigned char*) timeDateBuffer, (unsigned char*) "/");
    append_uint32_str_std((int) rtc.year, timeDateBuffer);
    ScreenConsole_SetBackgroundColorClear(vga, 0, Inline_VGAColor(0x0, 0x0));
    ScreenConsole_SetCursor(vga, vga->width-strlen(timeDateBuffer), 0);
    ScreenConsole_PrintStrD(vga, timeDateBuffer);
    vga->current = ptr;
}

void main(Machine *machine) {
    ScreenConsole* vga = &machine->vga;
    ScreenConsole_Clear(vga, Inline_VGAColor(0xf, 0xf));
    PIT_SetCallback(CALLBACK_UpdateTimeOS, vga);
    
    Game_Play(machine);
}

#endif