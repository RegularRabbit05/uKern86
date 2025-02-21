#pragma once

#define CPU_KERNEL_IDT_SIZE 256

typedef struct {
    unsigned int cr2;
    unsigned int ds;
    unsigned int edi, esi, ebp, esp, ebx, edx, ecx, eax;
    unsigned int int_no, err_code;
    unsigned int eip, csm, eflags, useresp, ss;
} __attribute__((packed)) CPURegisters;

typedef struct {
    unsigned short offset_low;
    unsigned short segment_selector;
    unsigned char zero;
    unsigned char type_attributes;
    unsigned short offset_high;
} __attribute__((packed)) IDTEntry;

typedef struct {
    unsigned short limit;
    unsigned int base;
} __attribute__((packed)) IDTDescriptor;

typedef struct __attribute__((packed)) {
    IDTEntry idet[256];
} IDTTable;

typedef struct {
    IDTTable idt;
    ScreenConsole vga;
} Machine;

void CPU_Reset();
void CPU_Halt();
void CPU_DoubleFault();
void Handler_FaultGeneric(ScreenConsole *scr);
void Handler_FaultWrapper(const char* txt);
void Handler_FaultAutoregister(IDTEntry idt[], const void **ls, unsigned short count);
int CPU_GetRing();
void CPU_DisableInterrupts();
void CPU_EnableInterrupts();
void IDT_SetEntry(IDTEntry *entry, unsigned long handler_address, unsigned char segment_selector, unsigned char attributes);
void IDT_Default(IDTEntry idt[], unsigned short num_entries);
void PIC_Remap();
void IDT_SetIndex(IDTEntry idt[], unsigned short index, void* handler);
void IDT_Commit(IDTEntry idt[], unsigned short num_entries);
void IDT_DisableExceptions();
void PIC_EnableInterrupt(unsigned char IRQline);
void PIC_MaskOutAll();

#include "faults.h"

void IDT_DisableExceptions() {
    IDTTable tb;
    IDT_Default(tb.idet, CPU_KERNEL_IDT_SIZE);
    IDT_Commit(tb.idet, CPU_KERNEL_IDT_SIZE);
}

void CPU_Reset() {
    asm ("jmp 0");
}

void CPU_Halt() {
    asm ("hlt");
}

void CPU_DoubleFault() {
    __asm__ volatile (
        "cli\n\t"
        "ltr %%ax\n\t"   
        "int $3\n\t"     
        :
        : "a" (0x28)              
    );
}

int CPU_GetRing() {
    unsigned long rcs = 0;
    int ring;
    asm ("mov %%cs, %0" : "=r" (rcs));
    ring = (int) (rcs & 3);
    return ring;
}

void CPU_DisableInterrupts() {
    asm volatile ("cli");
}

void CPU_EnableInterrupts() {
    asm volatile ("sti");
}

void Handler_FaultGeneric(ScreenConsole *scr) {
    const char* SAD_FACE =
    "     ___    ____  \n"
    "    |   |  |    | \n"
    "    |___| |    _| \n"
    "     ___  |   |   \n"
    "    |   | |   |   \n"
    "    |___| |   |_  \n"
    "           |____| \n"
    "";
    const char* DF_TEXT_1 = "Oh no, something went horribly wrong!\n";
    const char* DF_TEXT_2 = "The device must be restarted\n";
    
    ScreenConsole_Clear(scr, Inline_VGAColor(0x1, 0x1));
    ScreenConsole_SetCursor(scr, 0, 1);
    ScreenConsole_PrintStr(scr, (char*) SAD_FACE, Inline_VGAColor(0x1, 0xf));
    ScreenConsole_SetCursor(scr, 20, 4);
    ScreenConsole_PrintStr(scr, (char*) DF_TEXT_1, Inline_VGAColor(0x1, 0xf));
    ScreenConsole_SetCursor(scr, 20, 5);
    ScreenConsole_PrintStr(scr, (char*) DF_TEXT_2, Inline_VGAColor(0x1, 0xf));
    ScreenConsole_SetCursor(scr, 0, 10);
    ScreenConsole_PrintStr(scr, "    We will not do that for you\n", Inline_VGAColor(0x1, 0xf));
    ScreenConsole_PrintStr(scr, "    Fault type: ", Inline_VGAColor(0x1, 0xf));
}

void Handler_FaultWrapper(const char* txt) {
    CPU_DisableInterrupts();
    ScreenConsole scr = ScreenConsole_New();
    Handler_FaultGeneric(&scr);
    ScreenConsole_PrintStr(&scr, (char*) txt, Inline_VGAColor(0x1, 0xc));
    IDT_DisableExceptions();
    CPU_Halt();
}

void Handler_FaultAutoregister(IDTEntry idt[], const void **ls, unsigned short count) {
    if (ls == 0 || count == 0) return;
    for (unsigned short i = 0; i < count; i++) {
        if (ls[i] == 0) continue;
        IDT_SetIndex(idt, i, (void*) ls[i]);
    }
}

void IDT_SetEntry(IDTEntry *entry, unsigned long handler_address, unsigned char segment_selector, unsigned char attributes) {
    entry->offset_low = handler_address & 0xFFFF;
    entry->offset_high = (handler_address >> 16) & 0xFFFF;
    entry->segment_selector = segment_selector;
    entry->zero = 0;
    entry->type_attributes = attributes | 0x60;
}

#pragma GCC push_options
#pragma GCC optimize ("O0")
#pragma GCC target("general-regs-only")

__attribute__ ((interrupt)) void IDT_DefaultHandler(CPURegisters *) {
}

#pragma GCC pop_options

void PIC_Remap() {
    outb(0x20, 0x11);
    outb(0xA0, 0x11);
    outb(0x21, 0x20);
    outb(0xA1, 0x28);
    outb(0x21, 0x04);
    outb(0xA1, 0x02);
    outb(0x21, 0x01);
    outb(0xA1, 0x01);
    outb(0x21, 0x0);
    outb(0xA1, 0x0);
}

void IDT_Default(IDTEntry idt[], unsigned short num_entries) {
    for (int i = 0; i < num_entries; i++) IDT_SetIndex(&idt[i], i, IDT_DefaultHandler); 
}

void IDT_SetIndex(IDTEntry idt[], unsigned short index, void* handler) {
    unsigned char code_segment_selector = 0x08; 
    unsigned char present_and_interrupt_gate = 0x8E;
    IDT_SetEntry(&idt[index], (unsigned long) handler, code_segment_selector, present_and_interrupt_gate);
}

void IDT_Commit(IDTEntry idt[], unsigned short num_entries) {
    IDTDescriptor idt_descriptor;
    idt_descriptor.limit = (num_entries * sizeof(IDTEntry)) - 1;
    idt_descriptor.base = (unsigned long)idt;
    __asm__ volatile("lidt %0" : : "m" (idt_descriptor));
    __asm__ volatile ("sti");
}

void PIC_EnableInterrupt(unsigned char IRQline) {
    unsigned short port;
    unsigned char value;

    if (IRQline < 8) {
        port = 0x21;
    } else {
        port = 0xA1;
        IRQline -= 8;
    }
    value = ~inb(port) | ~(1 << IRQline);
    outb(port, ~value);        
}

void PIC_MaskOutAll() {
    outb(0x21, 0xff);
    outb(0xa1, 0xff);
}