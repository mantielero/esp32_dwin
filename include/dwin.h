#pragma once
#include <stdint.h>
#include <lvgl.h>

// DWIN T5UIC1 Protocol (Ender 3 V2) constants
extern const uint8_t FRAME_HEADER;
extern const uint8_t FRAME_TAIL[4];

// Command codes
extern const uint8_t CMD_HANDSHAKE;
extern const uint8_t CMD_CLEAR_SCREEN;
extern const uint8_t CMD_SET_POINT;
extern const uint8_t CMD_DRAW_LINE;
extern const uint8_t CMD_DRAW_RECT;
extern const uint8_t CMD_DRAW_BITMAP;
extern const uint8_t CMD_MOVE_AREA;
extern const uint8_t CMD_DRAW_STRING;
extern const uint8_t CMD_VIRT_COPY_PASTE;
extern const uint8_t CMD_BACKLIGHT;
extern const uint8_t CMD_SET_DIRECTION;
extern const uint8_t CMD_UPDATE_LCD;

// Font sizes
extern const uint8_t FONT_6x12;
extern const uint8_t FONT_8x16;
extern const uint8_t FONT_10x20;
extern const uint8_t FONT_12x24;
extern const uint8_t FONT_14x28;
extern const uint8_t FONT_16x32;
extern const uint8_t FONT_20x40;
extern const uint8_t FONT_24x48;
extern const uint8_t FONT_28x56;
extern const uint8_t FONT_32x64;

// Color definitions (RGB565)
extern const uint16_t COLOR_WHITE;
extern const uint16_t COLOR_BLACK;
extern const uint16_t COLOR_RED;
extern const uint16_t COLOR_GREEN;
extern const uint16_t COLOR_BLUE;
extern const uint16_t COLOR_YELLOW;
extern const uint16_t COLOR_MAGENTA;
extern const uint16_t COLOR_CYAN;
extern const uint16_t COLOR_BG_BLACK;
extern const uint16_t COLOR_BG_BLUE;

// Screen dimensions (as defines for compile-time constants)
#define DWIN_WIDTH 272
#define DWIN_HEIGHT 480

// Buffer for commands
extern uint8_t dwin_cmd_buffer[256];
extern uint8_t cmd_idx;

//==============================================================================
// DWIN LOW-LEVEL COMMUNICATION FUNCTIONS
//==============================================================================
void dwin_start_frame();
void dwin_add_byte(uint8_t value);
void dwin_add_word(uint16_t value);
void dwin_add_string(const char* str);
void dwin_send_frame();

//==============================================================================
// DWIN HIGH-LEVEL DRAWING FUNCTIONS
//==============================================================================
void dwin_draw_setup_string(uint16_t x, uint16_t y, uint16_t color, const char* text);
void dwin_clear_screen(uint16_t color);
uint16_t lvgl_to_dwin_color(lv_color_t lvgl_color);

//==============================================================================
// LVGL DRIVER INITIALIZATION
//==============================================================================
void lvgl_driver_init();