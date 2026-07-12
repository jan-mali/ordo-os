#pragma once

#include <cstdint>

const int MAX_CONSOLE_WIDTH  = 240;
const int MAX_CONSOLE_HEIGHT = 67;

struct PSF2Header {
    uint32_t magic;         /* magic bytes to identify PSF */
    uint32_t version;       /* zero */
    uint32_t headersize;    /* offset of bitmaps in file, 32 */
    uint32_t flags;         /* 0 if there's no unicode table */
    uint32_t numglyph;      /* number of glyphs */
    uint32_t bytesperglyph; /* size of each glyph */
    uint32_t height;        /* height in pixels */
    uint32_t width;         /* width in pixels */
};

class TextConsole {
public:
    void init(int fb_pitch, int fb_height, volatile uint32_t* fb);
    void putc(char c);
    void write(const char* str);
    void set_cursor(int row, int column);
    void set_color(uint32_t fg, uint32_t bg);

    void draw();
    void draw(int row, int column);

private:
    char text[MAX_CONSOLE_WIDTH * MAX_CONSOLE_HEIGHT];
    uint32_t foreground[MAX_CONSOLE_WIDTH * MAX_CONSOLE_HEIGHT];
    uint32_t background[MAX_CONSOLE_WIDTH * MAX_CONSOLE_HEIGHT];

    int columns;
    int rows;

    int pitch;
    int height;

    int cursor_x;
    int cursor_y;

    uint32_t current_fg_color;
    uint32_t current_bg_color;

    bool initialized = false;

    volatile uint32_t* framebuffer_data;

};

extern TextConsole text_console;
