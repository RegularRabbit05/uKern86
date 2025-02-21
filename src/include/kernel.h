#pragma once

#include "cpu.h"
#include "pit.h"
#include "screen.h"
#include "input.h"

void kernel_main(void) {
    PIC_MaskOutAll();
    PIC_Remap();

    Machine machine;

    IDT_Default(machine.idt.idet, CPU_KERNEL_IDT_SIZE);
    Handler_FaultAutoregister(machine.idt.idet, FAULTS_HANDLERS_LIST, sizeof(FAULTS_HANDLERS_LIST)/sizeof(FAULTS_HANDLERS_LIST[0]));
    IDT_Commit(machine.idt.idet, CPU_KERNEL_IDT_SIZE);

    PIT_Init(&machine);
    INPUT_EnableKeyboard(&machine);

    IDT_Commit(machine.idt.idet, CPU_KERNEL_IDT_SIZE);

    machine.vga = ScreenConsole_New();
    ScreenConsole_ClearD(&machine.vga);

    main(&machine);

    CPU_DisableInterrupts();
    CPU_Halt();
    CPU_Reset();
}