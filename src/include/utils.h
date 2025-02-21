#pragma once

__attribute__((target("no-sse,no-mmx"))) unsigned long strlen(char* str) {
    unsigned long len = 0;
    for (; str[len]; len++);
    return len;
}

__attribute__((target("no-sse,no-mmx"))) void uint16_to_hex(unsigned short value, char* buffer) {
    const char UTILS_HEX_CHARS[] = "0123456789ABCDEF";
    buffer[0] = UTILS_HEX_CHARS[(value >> 12) & 0xF];
    buffer[1] = UTILS_HEX_CHARS[(value >> 8) & 0xF];  
    buffer[2] = UTILS_HEX_CHARS[(value >> 4) & 0xF];  
    buffer[3] = UTILS_HEX_CHARS[value & 0xF];         
    buffer[4] = '\0';
}

unsigned long append_uint32_str_std(unsigned int value, char* buffer) {
    unsigned long len = strlen(buffer);
    unsigned long digits = 0;
    do {
        buffer[len+digits] = (value % 10) + 0x30;
        buffer[len+digits+1] = 0;
        value = (value - (value%10)) / 10;
        digits++;
    } while (value > 0);
    for (unsigned long i = 0; i < digits / 2; i++) {
        char tmp = buffer[len+i];
        buffer[len+i] = buffer[len+digits-i-1];
        buffer[len+digits-i-1] = tmp;
    }
    return len+digits;
}