#pragma once

#define SCREEEN_WIDTH 80
#define SCREEN_HEIGHT 25
#define SCREEN_DPT 2
#define SCREEN_DEFAULT_COLOR 0x0f

#define SCREEN_VGA_FONT_CHARS 256
#define SCREEN_VGA_FONT_BYTES 32

typedef struct {
    unsigned char* base;
    unsigned char* current;
    unsigned short width;
    unsigned short height;
    unsigned char dpt;
    unsigned char color;
} ScreenConsole;

typedef struct {
    unsigned char fb[SCREEEN_WIDTH*SCREEN_HEIGHT*2];
} ScreenConsoleFrameBuffer;

typedef struct {
    unsigned char font[SCREEN_VGA_FONT_CHARS*SCREEN_VGA_FONT_BYTES];
} __attribute__((packed)) ScreenConsoleFont;

unsigned char Inline_VGAColor(const unsigned char high_bg, const unsigned char low_fg) {
    return (high_bg << 4) | (low_fg & 0b00001111);
}

#include "devices.h"

ScreenConsole ScreenConsole_New();
void ScreenConsole_OverflowCheck(ScreenConsole *ptr);
void ScreenConsole_Clear(ScreenConsole *ptr, unsigned char color);
void ScreenConsole_ClearD(ScreenConsole *ptr);
void ScreenConsole_Print(ScreenConsole *ptr, unsigned char ch, unsigned char color);
void ScreenConsole_PrintD(ScreenConsole *ptr, unsigned char ch);
void ScreenConsole_PrintStr(ScreenConsole *ptr, char* str, unsigned char color);
void ScreenConsole_PrintStrD(ScreenConsole *ptr, char* str);
void ScreenConsole_Skip(ScreenConsole *ptr);
void ScreenConsole_NextLine(ScreenConsole *ptr);
void ScreenConsole_SetCursor(ScreenConsole *ptr, unsigned short x, unsigned short y);
void ScreenConsole_SetBackgroundColorClear(ScreenConsole *ptr, unsigned short line, unsigned char color);
void ScreenConsole_SetBackgroundColor(ScreenConsole *ptr, unsigned short line, unsigned char color);
void ScreenConsole_FontPoke(unsigned long relAddr, unsigned char val);
void Screen_VGAFontTransactionEnd();
unsigned char* Screen_VGAFontTransactionBegin();
void ScreenConsole_ReplaceTile(unsigned char ch, unsigned char* b32Font);
void ScreenConsole_ToFontMemory(unsigned long relAddr, unsigned char* src, unsigned long n);
void ScreenConsole_FromFontMemory(unsigned char* dst, unsigned long relAddr, unsigned long n);
void ScreenConsole_AsFont(ScreenConsoleFont *font);
void ScreenConsole_FromFont(ScreenConsoleFont *font);

ScreenConsole ScreenConsole_New() {
    ScreenConsole ret;
    ScreenConsole *empty = &ret;
    empty->base = (unsigned char*) 0xb8000;
    empty->current = empty->base;
    empty->width = SCREEEN_WIDTH;
    empty->height = SCREEN_HEIGHT;
    empty->dpt = SCREEN_DPT;
    empty->color = SCREEN_DEFAULT_COLOR;
    return ret;
}

void ScreenConsole_OverflowCheck(ScreenConsole *ptr) {
    if (ptr->current >= ptr->base+(ptr->width*ptr->height*ptr->dpt)) ptr->current = ptr->base;
}

void ScreenConsole_Clear(ScreenConsole *ptr, unsigned char color) {
    ptr->current = ptr->base;
    for (unsigned int i = 0; i < ptr->width*ptr->height*ptr->dpt; i+=ptr->dpt) {
        ptr->base[i] = 0;
        ptr->base[i+ptr->dpt-1] = color;
    }
}

void ScreenConsole_ClearD(ScreenConsole *ptr) {
    ScreenConsole_Clear(ptr, 0);
}

void ScreenConsole_Print(ScreenConsole *ptr, unsigned char ch, unsigned char color) {
    if (ch == '\n') ScreenConsole_NextLine(ptr);
    ScreenConsole_OverflowCheck(ptr);
    ptr->current[0] = ch;
    ptr->current[1] = color;
    ptr->current+=ptr->dpt;
}

void ScreenConsole_PrintD(ScreenConsole *ptr, unsigned char ch) {
    ScreenConsole_Print(ptr, ch, ptr->color);
}

void ScreenConsole_Skip(ScreenConsole *ptr) {
    ScreenConsole_OverflowCheck(ptr);
    ptr->current+=2;
}

void ScreenConsole_NextLine(ScreenConsole *ptr) {
    signed long cursor = (ptr->current - ptr->base) / ptr->dpt;
    cursor = cursor % ptr->width;
    for (; cursor < ptr->width; cursor++) ScreenConsole_Skip(ptr);
}

void ScreenConsole_SetBackgroundColorClear(ScreenConsole *ptr, unsigned short line, unsigned char color) {
    unsigned char* cursor = ptr->current;
    ScreenConsole_SetCursor(ptr, 0, line);
    for (unsigned short i = 0; i < ptr->width; i++) {ptr->current[i*2] = 0; ptr->current[i*2+1] = color;}
    ptr->current = cursor;
}

void ScreenConsole_SetBackgroundColor(ScreenConsole *ptr, unsigned short line, unsigned char color) {
    unsigned char* cursor = ptr->current;
    ScreenConsole_SetCursor(ptr, 0, line);
    for (unsigned short i = 0; i < ptr->width; i++) ptr->current[i*2+1] = color;
    ptr->current = cursor;
}

void ScreenConsole_PrintStr(ScreenConsole *ptr, char* str, unsigned char color) {
    while(*str) {
        if (*str == '\n') {
            ScreenConsole_NextLine(ptr);
        } else {
            ScreenConsole_Print(ptr, *str, color);
        }
        str++;
    }
}

void ScreenConsole_PrintStrD(ScreenConsole *ptr, char* str) {
    ScreenConsole_PrintStr(ptr, str, ptr->color);
}

void ScreenConsole_SetCursor(ScreenConsole *ptr, unsigned short x, unsigned short y) {
    ptr->current = ptr->base;
    ptr->current += ptr->dpt*(y*ptr->width);
    ptr->current += x*ptr->dpt;
    ScreenConsole_OverflowCheck(ptr);
}

void Screen_VGAFontUnlock() {
    inb(0x3DA); //send port 0x3C0 to index state
    outw(0x03ce, 5);
    outw(0x03ce, 0x0406);
    outw(0x03c4, 0x0402);
    outw(0x03c4, 0x0604);
}

void Screen_VGAFontLock() {
    inb(0x3DA); //send port 0x3C0 to index state
    outw(0x03c4, 0x0302);
    outw(0x03c4, 0x0204);
    outw(0x03ce, 0x1005);
    outw(0x03ce, 0x0E06);
}

unsigned char* Screen_VGAFontTransactionBegin() {
    asm volatile ("cli");
    Screen_VGAFontUnlock();
    return (unsigned char *) 0x0A0000;
}

void Screen_VGAFontTransactionEnd() {
    Screen_VGAFontLock();
    asm volatile ("sti");
}

void ScreenConsole_FontPoke(unsigned long relAddr, unsigned char val) {
    unsigned char *fm = Screen_VGAFontTransactionBegin();
    fm[relAddr] = val;
    Screen_VGAFontTransactionEnd();
}

void ScreenConsole_ReplaceTile(unsigned char ch, unsigned char* b32Font) {
    ScreenConsole_ToFontMemory(ch*SCREEN_VGA_FONT_BYTES, b32Font, SCREEN_VGA_FONT_BYTES);
}

void ScreenConsole_ToFontMemory(unsigned long relAddr, unsigned char* src, unsigned long n) {
    unsigned char* fm = Screen_VGAFontTransactionBegin();
    Memory_Copy(fm+relAddr, src, n);
    Screen_VGAFontTransactionEnd();
}

void ScreenConsole_FromFontMemory(unsigned char* dst, unsigned long relAddr, unsigned long n) {
    unsigned char* fm = Screen_VGAFontTransactionBegin();
    Memory_Copy(dst, fm+relAddr, n);
    Screen_VGAFontTransactionEnd();
}

void ScreenConsole_AsFont(ScreenConsoleFont *font) {
    unsigned char* ptr = (unsigned char*) &font->font;
    Memory_Set(ptr, 0, SCREEN_VGA_FONT_CHARS*SCREEN_VGA_FONT_BYTES);
    unsigned char* fm = Screen_VGAFontTransactionBegin();
    Memory_Copy(ptr, fm, SCREEN_VGA_FONT_CHARS*SCREEN_VGA_FONT_BYTES);
    Screen_VGAFontTransactionEnd();
}

void ScreenConsole_FromFont(ScreenConsoleFont *font) {
    unsigned char* fm = Screen_VGAFontTransactionBegin();
    Memory_Copy(fm, (unsigned char*) &font->font, SCREEN_VGA_FONT_CHARS*SCREEN_VGA_FONT_BYTES);
    Screen_VGAFontTransactionEnd();
}