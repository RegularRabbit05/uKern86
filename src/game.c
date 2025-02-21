#pragma once

#include "kernel.c"

typedef struct {
    int moveWait;
    short x, y;
    ScreenConsole fbVGA;
} Game;

typedef struct {
    float x, y;         
    float vx, vy;   
} Ball;

typedef struct {
    float x, y;
    float speed;
} Paddle;

void Game_Load(Game *game) {
    Memory_Set((unsigned char*) &game, 0, sizeof(Game));

    unsigned char tilemap[][32] = {
        {
            0b00000001,
            0b00000001,
            0b00000001,
            0b00000001,
            0b00000001,
            0b00000001,
            0b00000001,
            0b00000001,
            0b00000001,
            0b00000001,
            0b00000001,
            0b00000001,
            0b00000001,
            0b00000001,
            0b00000001,
            0b00000001,
        },
        {
            0b10000000,
            0b10000000,
            0b10000000,
            0b10000000,
            0b10000000,
            0b10000000,
            0b10000000,
            0b10000000,
            0b10000000,
            0b10000000,
            0b10000000,
            0b10000000,
            0b10000000,
            0b10000000,
            0b10000000,
            0b10000000,
        },
        {
            0b00000000,
            0b00000000,
            0b00000000,
            0b00000000,
            0b00011000,
            0b00111100,
            0b01111110,
            0b11111111,
            0b11111111,
            0b01111110,
            0b00111100,
            0b00011000,
            0b00000000,
            0b00000000,
            0b00000000,
            0b00000000,
        }
    };

    for (unsigned long i = 0; i < sizeof(tilemap)/sizeof(tilemap[0]); i++) {
        ScreenConsole_ReplaceTile('a'+i, tilemap[i]);
    }
}

char Ball_PaddleCollision(Ball *ball, Paddle *paddle) {
    if (ball->x+1 >= paddle->x && ball->x-1 <= paddle->x+1) {
        if (ball->y+1 >= paddle->y && ball->y-1 <= paddle->y+5) {
            ball->vx = -ball->vx;
            return 1;
        }
    }
    return 0;
}

char Ball_WallCollision(Ball *ball, float screen_height) {
    if (ball->y-1 <= 0) {               // top wall
        ball->vy = -ball->vy;
        return 1;
    }
    if (ball->y+1 >= screen_height) {   // bottom wall
        ball->vy = -ball->vy; 
        return 2;
    }
    return 0;
}

void Game_Play(Machine *machine) {
    Paddle paddle1, paddle2;
    Ball ball;
    Game instance;
    Game_Load(&instance);
    instance.fbVGA = machine->vga;
    ScreenConsole_Clear(&instance.fbVGA, Inline_VGAColor(0x8, 0));

    paddle1.x = 0;       
    paddle1.y = 15;

    paddle2.x = 79;
    paddle2.y = 15;

    ball.x = 40;
    ball.y = 15;
    ball.vx = 1;
    ball.vy = 1;

    instance.moveWait = 0;

    Ball previous = ball;
    unsigned char fg = Inline_VGAColor(0x8, 0x0);
    unsigned long long begin = TIME_MillisSinceBoot();
    while (1) {
        const unsigned long long now = TIME_MillisSinceBoot();
        const unsigned long long delta = now - begin;
        unsigned int key = INPUT_KeyboardPOP();
        begin = now;
        ScreenConsole *vga = &instance.fbVGA;

        ScreenConsole_SetCursor(vga, 0, 2);
        instance.moveWait -= delta;
        if (instance.moveWait <= 0) {
            instance.moveWait = 0;
            ball.x += ball.vx;
            ball.y += ball.vy;
            char p1 = Ball_PaddleCollision(&ball, &paddle1);
            char p2 = Ball_PaddleCollision(&ball, &paddle2);
            char w = Ball_WallCollision(&ball, 24);

            if (p1) PCSpeaker_Play(1);
            if (p2) PCSpeaker_Play(2);
            if (w)  PCSpeaker_Play(w == 1 ? 4 : 5);
        
            if (ball.x < 0 || ball.x >= 80) { 
                ball.x = 40;
                ball.y = 15;
                ball.vx = 1;
                ball.vy = 1;
                PCSpeaker_Play(3);
            }

            instance.moveWait = 40;
        }

        ScreenConsole_SetCursor(vga, previous.x, previous.y);
        if (previous.x != ball.x || previous.y != ball.y) ScreenConsole_Print(vga, ' ', fg);

        if (key == INPUT_KEYBOARD_DOWN) {
            ScreenConsole_SetCursor(vga, paddle2.x, paddle2.y);
            ScreenConsole_Print(vga, ' ', fg);
            paddle2.y++;
            if (paddle2.y >= 20) paddle2.y = 20;
        }
        if (key == INPUT_KEYBOARD_UP) {
            ScreenConsole_SetCursor(vga, paddle2.x, paddle2.y+4);
            ScreenConsole_Print(vga, ' ', fg);
            paddle2.y--;
            if (paddle2.y <= 1) paddle2.y = 1;
        }

        if (ball.x < 20) paddle1.y = ball.y-2;
        if (paddle1.y <= 0) paddle1.y = 1;
        if (paddle1.y >= 20) paddle1.y = 20;
        for (int i = 1; i < vga->height; i++) {
            if (i >= paddle1.y && i < paddle1.y+5) continue;
            ScreenConsole_SetCursor(vga, paddle1.x, i);
            ScreenConsole_Print(vga, ' ', fg);
        }

#ifdef AUTOMATIC_PADDLE_2
        if (ball.x > 60) paddle2.y = ball.y-2;
        if (paddle2.y <= 0) paddle2.y = 1;
        if (paddle2.y >= 20) paddle2.y = 20;
        for (int i = 1; i < vga->height; i++) {
            if (i >= paddle2.y && i < paddle2.y+5) continue;
            ScreenConsole_SetCursor(vga, paddle2.x, i);
            ScreenConsole_Print(vga, ' ', fg);
        }
#endif

        ScreenConsole_SetCursor(vga, paddle1.x, paddle1.y);
        for (int i = 0; i < 5; i++) {
            ScreenConsole_Print(vga, 'a', Inline_VGAColor(0x8, 0xC));
            vga->current-=vga->dpt;
            vga->current+=vga->dpt*vga->width;        
        }
        ScreenConsole_SetCursor(vga, ball.x, ball.y);
        ScreenConsole_Print(vga, 'c', Inline_VGAColor(0x8, 0x9));
        ScreenConsole_SetCursor(vga, paddle2.x, paddle2.y);
        for (int i = 0; i < 5; i++) {
            ScreenConsole_Print(vga, 'b', Inline_VGAColor(0x8, 0xA));
            vga->current-=vga->dpt;
            vga->current+=vga->dpt*vga->width;
        }

        previous = ball;
    }
}