#pragma once

#include "cpu.h"
#include "utils.h"

#pragma GCC push_options
#pragma GCC optimize ("O0")
#pragma GCC target("general-regs-only")

__attribute__ ((interrupt))
void Handler_DivisionErrorFault(CPURegisters *) {
    Handler_FaultWrapper("Division Error (0x00)");
}

__attribute__ ((interrupt))
void Handler_InvalidOpcodeFault(CPURegisters *) {
    Handler_FaultWrapper("Invalid Opcode (0x06)");
}

__attribute__ ((interrupt))
void Handler_DeviceUnavailableFault(CPURegisters *) {
    Handler_FaultWrapper("Device not available (0x07)");
}

__attribute__ ((interrupt))
void Handler_DoubleFault(CPURegisters *reg) {
    CPU_DisableInterrupts();
    char buf[48] = "Double Fault (0x08)";
    unsigned long sz = strlen(buf);
    buf[sz+0] = ' ';
    buf[sz+1] = '(';
    buf[sz+2] = 0;
    uint16_to_hex(reg->int_no, buf+strlen(buf));
    uint16_to_hex(reg->err_code, buf+strlen(buf));
    sz = strlen(buf);
    buf[sz+0] = ')';
    buf[sz+1] = ' ';
    buf[sz+2] = '(';
    buf[sz+3] = '0';
    buf[sz+4] = 'x';
    buf[sz+5] = '\0';
    uint16_to_hex(reg->eflags, buf+strlen(buf));
    uint16_to_hex(reg->eflags >> 16, buf+strlen(buf));
    sz = strlen(buf);
    buf[sz] = ')';
    buf[sz+1] = '\0';
    Handler_FaultWrapper(buf);
}

__attribute__ ((interrupt))
void Handler_FPUError(CPURegisters *) {
    Handler_FaultWrapper("x87 FPU Error (0x10)");
}

__attribute__ ((interrupt))
void Handler_Ignored(CPURegisters *) {outb(0x20,0x20);}

const void* FAULTS_HANDLERS_LIST[] = {
/*00*/Handler_DivisionErrorFault,
/*01*/0,
/*02*/0,
/*03*/0,
/*04*/0,
/*05*/0,
/*06*/Handler_InvalidOpcodeFault,
/*07*/Handler_DeviceUnavailableFault,
/*08*/Handler_DoubleFault,
/*09*/0,
/*0A*/0,
/*0B*/0,
/*0C*/0,
/*0D*/0,
/*0E*/0,
/*0F*/0,
/*10*/Handler_FPUError,
/*11*/0,
/*12*/0,
/*13*/0,
/*14*/0,
/*15*/0,
/*16*/0,
/*17*/0,
};

#pragma GCC pop_options
