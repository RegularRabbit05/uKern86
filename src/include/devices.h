#pragma once

#include "utils.h"

#define PIC1_IMR 0x21
#define PIC2_IMR 0xA1

typedef struct {
    unsigned char second;
    unsigned char minute;
    unsigned char hour;
    unsigned char day;
    unsigned char month;
    unsigned int year;
    unsigned int century;
} CMOSRTC;

void PCSpeaker_Play(unsigned int nFrequence);
void outw(unsigned short port, unsigned short value);
void outb(unsigned short port, unsigned char value);
unsigned char inb(unsigned short port);
void Memory_Copy(unsigned char *dst, const unsigned char *src, unsigned long n);
void Memory_Set(unsigned char *dst, const unsigned char value, unsigned long n);
void Memory_StrCat(unsigned char *dst, unsigned char *src);
void Memory_ShiftLeft(unsigned char *src, unsigned long n);
CMOSRTC CMOS_ReadRTC();
int CMOS_UpdateInProgress();
unsigned char CMOS_GetRegistry(unsigned short reg);

void outw(unsigned short port, unsigned short value) {
    __asm__ volatile ( "outw %w0, %w1" : : "a"(value), "Nd"(port) : "memory" );
}

void outb(unsigned short port, unsigned char value) {
    __asm__ volatile ( "outb %b0, %w1" : : "a"(value), "Nd"(port) : "memory" );
}

unsigned char inb(unsigned short port) {
    unsigned char value;
    __asm__ volatile ( "inb %w1, %b0" : "=a"(value) : "Nd"(port) : "memory" );
    return value;
}

#define CMOS_ADDRESS 0x70
#define CMOS_DATA    0x71

int CMOS_UpdateInProgress() {
    outb(CMOS_ADDRESS, 0x0A);
    return (inb(CMOS_DATA) & 0x80);
}

unsigned char CMOS_GetRegistry(unsigned short reg) {
    outb(CMOS_ADDRESS, reg);
    return inb(CMOS_DATA);
}

CMOSRTC CMOS_ReadRTC() {
    unsigned char last_second;
    unsigned char last_minute;
    unsigned char last_hour;
    unsigned char last_day;
    unsigned char last_month;
    unsigned char last_year;
    unsigned char last_century;
    unsigned char registerB;

    CMOSRTC rtc;

    unsigned short century_register = 0x32;

    while (CMOS_UpdateInProgress());
    rtc.second = CMOS_GetRegistry(0x00);
    rtc.minute = CMOS_GetRegistry(0x02);
    rtc.hour = CMOS_GetRegistry(0x04);
    rtc.day = CMOS_GetRegistry(0x07);
    rtc.month = CMOS_GetRegistry(0x08);
    rtc.year = CMOS_GetRegistry(0x09);
    if (century_register != 0) rtc.century = CMOS_GetRegistry(century_register);

    do {
        last_second = rtc.second;
        last_minute = rtc.minute;
        last_hour = rtc.hour;
        last_day = rtc.day;
        last_month = rtc.month;
        last_year = rtc.year;
        last_century = rtc.century;

        while (CMOS_UpdateInProgress());
        rtc.second = CMOS_GetRegistry(0x00);
        rtc.minute = CMOS_GetRegistry(0x02);
        rtc.hour = CMOS_GetRegistry(0x04);
        rtc.day = CMOS_GetRegistry(0x07);
        rtc.month = CMOS_GetRegistry(0x08);
        rtc.year = CMOS_GetRegistry(0x09);
        if(century_register != 0) {
            rtc.century = CMOS_GetRegistry(century_register);
        }
    } while( (last_second != rtc.second) || (last_minute != rtc.minute) || (last_hour != rtc.hour) || (last_day != rtc.day) || (last_month != rtc.month) || (last_year != rtc.year) || (last_century != rtc.century) );

    registerB = CMOS_GetRegistry(0x0B);

    if (!(registerB & 0x04)) {
        rtc.second = (rtc.second & 0x0F) + ((rtc.second / 16) * 10);
        rtc.minute = (rtc.minute & 0x0F) + ((rtc.minute / 16) * 10);
        rtc.hour = ( (rtc.hour & 0x0F) + (((rtc.hour & 0x70) / 16) * 10) ) | (rtc.hour & 0x80);
        rtc.day = (rtc.day & 0x0F) + ((rtc.day / 16) * 10);
        rtc.month = (rtc.month & 0x0F) + ((rtc.month / 16) * 10);
        rtc.year = (rtc.year & 0x0F) + ((rtc.year / 16) * 10);
        if (century_register != 0) rtc.century = (rtc.century & 0x0F) + ((rtc.century / 16) * 10);
    }

    if (!(registerB & 0x02) && (rtc.hour & 0x80)) rtc.hour = ((rtc.hour & 0x7F) + 12) % 24;

    if(century_register != 0) {
        rtc.year += rtc.century * 100;
    }

    return rtc;
}

void Memory_Copy(unsigned char *dst, const unsigned char *src, unsigned long n) {
    for (unsigned long i = 0; i < n; i++) dst[i] = src[i];
}

void Memory_Set(unsigned char *dst, const unsigned char value, unsigned long n) {
    for (unsigned long i = 0; i < n; i++) dst[i] = value;
}

void Memory_StrCat(unsigned char *dst, unsigned char *src) {
    unsigned long begin = strlen((char*) dst);
    unsigned long end = strlen((char*) src);
    Memory_Copy(dst+begin, src, end+1);
}

void Memory_ShiftLeft(unsigned char *src, unsigned long n) {
    for (unsigned long i = 0; i < n-1; i++) src[i] = src[i+1];
}

void PCSpeaker_Stop() { //Does not work yet
    unsigned char p = inb(0x61) & 0b11111100;
    outb(p, 0x61);
}

void PCSpeaker_Play(unsigned int nFrequence) {
    if (nFrequence == 0) return;
    unsigned int Div;
    unsigned char tmp;

    Div = 1193180 / nFrequence;
    outb(0x43, 0xb6);
    outb(0x42, (unsigned char) (Div) );
    outb(0x42, (unsigned ) (Div >> 8));

    tmp = inb(0x61);
     if (tmp != (tmp | 3)) {
        outb(0x61, tmp | 3);
    }
}