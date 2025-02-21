#pragma once

#include "cpu.h"
#include "devices.h"

void INPUT_EnableKeyboard(Machine *machine);
void INPUT_KeyboardHandlerWrapped();
unsigned int INPUT_KeyboardPOP();

volatile unsigned short INPUT_KEYBOARD_QUEUE_SIZE;
volatile unsigned int INPUT_KEYBOARD_QUEUE_BUFFER[64];
volatile unsigned char INPUT_KEYBOARD_ISSHIFT;

#define INPUT_KEYBOARD_SHIFT 42

#pragma GCC push_options
#pragma GCC optimize ("O0")
#pragma GCC target("general-regs-only")

#define UNKNOWN 0

#define INPUT_KEYBOARD_ESC      0x01000000
#define INPUT_KEYBOARD_CTRL     0x1d000000
#define INPUT_KEYBOARD_ALT      0x38000000
#define INPUT_KEYBOARD_CAPS     0x3a000000
#define INPUT_KEYBOARD_F1       0x3b000000
#define INPUT_KEYBOARD_F2       0x3c000000
#define INPUT_KEYBOARD_F3       0x3d000000
#define INPUT_KEYBOARD_F4       0x3e000000
#define INPUT_KEYBOARD_F5       0x3f000000
#define INPUT_KEYBOARD_F6       0x40000000
#define INPUT_KEYBOARD_F7       0x41000000
#define INPUT_KEYBOARD_F8       0x42000000
#define INPUT_KEYBOARD_F9       0x43000000
#define INPUT_KEYBOARD_F10      0x44000000
#define INPUT_KEYBOARD_F11      0x57000000
#define INPUT_KEYBOARD_F12      0x58000000
#define INPUT_KEYBOARD_UP       0x48000000
#define INPUT_KEYBOARD_LEFT     0x4b000000
#define INPUT_KEYBOARD_RIGHT    0x4d000000
#define INPUT_KEYBOARD_DOWN     0x50000000
#define INPUT_KEYBOARD_END      0x4f000000
#define INPUT_KEYBOARD_DEL      0x53000000
#define INPUT_KEYBOARD_ENTER    '\n'

const unsigned int INPUT_KEYBOARD_LOW[128] = {
    UNKNOWN,INPUT_KEYBOARD_ESC,'1','2','3','4','5','6','7','8', '9','0','-','=','\b','\t','q','w','e','r','t','y','u','i','o','p','[',']','\n',INPUT_KEYBOARD_CTRL,'a','s','d','f','g','h','j','k','l',';','\'','`',INPUT_KEYBOARD_SHIFT,'\\','z','x','c','v','b','n','m',',',
    '.','/',UNKNOWN,'*',INPUT_KEYBOARD_ALT,' ',INPUT_KEYBOARD_CAPS,INPUT_KEYBOARD_F1,INPUT_KEYBOARD_F2,INPUT_KEYBOARD_F3,INPUT_KEYBOARD_F4,INPUT_KEYBOARD_F5,INPUT_KEYBOARD_F6,INPUT_KEYBOARD_F7,INPUT_KEYBOARD_F8,INPUT_KEYBOARD_F9,INPUT_KEYBOARD_F10,UNKNOWN,UNKNOWN,UNKNOWN,INPUT_KEYBOARD_UP,UNKNOWN,'-',INPUT_KEYBOARD_LEFT,UNKNOWN,INPUT_KEYBOARD_RIGHT,
    '+',INPUT_KEYBOARD_END,INPUT_KEYBOARD_DOWN,UNKNOWN,UNKNOWN,INPUT_KEYBOARD_DEL,UNKNOWN,UNKNOWN,UNKNOWN,INPUT_KEYBOARD_F11,INPUT_KEYBOARD_F12,UNKNOWN,UNKNOWN,UNKNOWN,UNKNOWN,UNKNOWN,UNKNOWN,
    UNKNOWN,UNKNOWN,UNKNOWN,UNKNOWN,UNKNOWN,UNKNOWN,UNKNOWN,UNKNOWN,UNKNOWN,UNKNOWN,UNKNOWN,UNKNOWN,UNKNOWN,UNKNOWN,
    UNKNOWN,UNKNOWN,UNKNOWN,UNKNOWN,UNKNOWN,UNKNOWN,UNKNOWN,UNKNOWN,UNKNOWN,UNKNOWN,UNKNOWN,UNKNOWN,UNKNOWN,UNKNOWN,
    UNKNOWN,UNKNOWN,UNKNOWN,UNKNOWN,UNKNOWN
};
    
const unsigned int INPUT_KEYBOARD_HIGH[128] = {
    UNKNOWN,INPUT_KEYBOARD_ESC,'!','@','#','$','%','^','&','*','(',')','_','+','\b','\t','Q','W','E','R','T','Y','U','I','O','P','{','}','\n',INPUT_KEYBOARD_CTRL,'A','S','D','F','G','H','J','K','L',':','"','~',INPUT_KEYBOARD_SHIFT,'|','Z','X','C','V','B','N','M','<','>',
    '?',UNKNOWN,'*',INPUT_KEYBOARD_ALT,' ',INPUT_KEYBOARD_CAPS,INPUT_KEYBOARD_F1,INPUT_KEYBOARD_F2,INPUT_KEYBOARD_F3,INPUT_KEYBOARD_F4,INPUT_KEYBOARD_F5,INPUT_KEYBOARD_F6,INPUT_KEYBOARD_F7,INPUT_KEYBOARD_F8,INPUT_KEYBOARD_F9,INPUT_KEYBOARD_F10,UNKNOWN,UNKNOWN,UNKNOWN,INPUT_KEYBOARD_UP,UNKNOWN,'-',INPUT_KEYBOARD_LEFT,UNKNOWN,INPUT_KEYBOARD_RIGHT,
    '+',INPUT_KEYBOARD_END,INPUT_KEYBOARD_DOWN,UNKNOWN,UNKNOWN,INPUT_KEYBOARD_DEL,UNKNOWN,UNKNOWN,UNKNOWN,INPUT_KEYBOARD_F11,INPUT_KEYBOARD_F12,UNKNOWN,UNKNOWN,UNKNOWN,UNKNOWN,
    UNKNOWN,UNKNOWN,UNKNOWN,UNKNOWN,UNKNOWN,UNKNOWN,UNKNOWN,UNKNOWN,UNKNOWN,UNKNOWN,UNKNOWN,UNKNOWN,UNKNOWN,UNKNOWN,
    UNKNOWN,UNKNOWN,UNKNOWN,UNKNOWN,UNKNOWN,UNKNOWN,UNKNOWN,UNKNOWN,UNKNOWN,UNKNOWN,UNKNOWN,UNKNOWN,UNKNOWN,UNKNOWN,
    UNKNOWN,UNKNOWN,UNKNOWN,UNKNOWN,UNKNOWN,UNKNOWN,UNKNOWN
};

#undef UNKNOWN

void INPUT_KeyboardHandlerWrapped() {
    unsigned char code = inb(0x60);
    unsigned char press = inb(0x60);
    if (code == INPUT_KEYBOARD_SHIFT) {
        INPUT_KEYBOARD_ISSHIFT = press == 0 ? 0 : 1;
        return;
    }

    switch(code){
        case 0:
        case 1:
        case 29:
        case 56:
        case 59:
        case 60:
        case 61:
        case 62:
        case 63:
        case 64:
        case 65:
        case 66:
        case 67:
        case 68:
        case 87:
        case 88:
            return;
        default: break;
    }

    if (press == 0) return;
    if (code >= 128) return;
    if (INPUT_KEYBOARD_QUEUE_SIZE >= sizeof(INPUT_KEYBOARD_QUEUE_BUFFER)/sizeof(INPUT_KEYBOARD_QUEUE_BUFFER[0])) {
        PCSpeaker_Play(1);
        return;
    }

    INPUT_KEYBOARD_QUEUE_BUFFER[INPUT_KEYBOARD_QUEUE_SIZE] = INPUT_KEYBOARD_ISSHIFT == 0 ? INPUT_KEYBOARD_LOW[code] : INPUT_KEYBOARD_HIGH[code];
    INPUT_KEYBOARD_QUEUE_SIZE++;
}

__attribute__ ((interrupt)) void INPUT_KeyboardHandler(CPURegisters *) {
    INPUT_KeyboardHandlerWrapped();
    outb(0x20, 0x20);
}

#pragma GCC pop_options

void INPUT_EnableKeyboard(Machine *machine) {
    INPUT_KEYBOARD_QUEUE_SIZE = 0;
    INPUT_KEYBOARD_ISSHIFT = 0;
    for (unsigned int i = 0; i < sizeof(INPUT_KEYBOARD_QUEUE_BUFFER)/sizeof(INPUT_KEYBOARD_QUEUE_BUFFER[0]); i++) INPUT_KEYBOARD_QUEUE_BUFFER[i] = 0;
    IDT_SetIndex(machine->idt.idet, 0x21, INPUT_KeyboardHandler);
    PIC_EnableInterrupt(1);
}

unsigned int INPUT_KeyboardPOP() {
    if (INPUT_KEYBOARD_QUEUE_SIZE == 0) return 0;
    INPUT_KEYBOARD_QUEUE_SIZE--;
    return INPUT_KEYBOARD_QUEUE_BUFFER[INPUT_KEYBOARD_QUEUE_SIZE];
}