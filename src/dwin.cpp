#include <Arduino.h>
#include <dwin.h>

#include <HardwareSerial.h>
extern HardwareSerial DWINSerial;

uint8_t dwin_cmd_buffer[256];
uint8_t cmd_idx = 0;

// DWIN T5UIC1 Protocol (Ender 3 V2)
const uint8_t FRAME_HEADER = 0xAA;
const uint8_t FRAME_TAIL[4] = {0xCC, 0x33, 0xC3, 0x3C};

// Commands
const uint8_t CMD_HANDSHAKE = 0x00;
const uint8_t CMD_CLEAR_SCREEN = 0x01;
const uint8_t CMD_SET_POINT = 0x02;
const uint8_t CMD_DRAW_LINE = 0x03;
const uint8_t CMD_DRAW_RECT = 0x05;
const uint8_t CMD_DRAW_BITMAP = 0x08;
const uint8_t CMD_MOVE_AREA = 0x09;
const uint8_t CMD_DRAW_STRING = 0x11;
const uint8_t CMD_VIRT_COPY_PASTE = 0x27;
const uint8_t CMD_BACKLIGHT = 0x30;
const uint8_t CMD_SET_DIRECTION = 0x34;
const uint8_t CMD_UPDATE_LCD = 0x3D;

// Font Sizes
const uint8_t FONT_6x12 = 0x00;
const uint8_t FONT_8x16 = 0x01;
const uint8_t FONT_10x20 = 0x02;
const uint8_t FONT_12x24 = 0x03;
const uint8_t FONT_14x28 = 0x04;
const uint8_t FONT_16x32 = 0x05;
const uint8_t FONT_20x40 = 0x06;
const uint8_t FONT_24x48 = 0x07;
const uint8_t FONT_28x56 = 0x08;
const uint8_t FONT_32x64 = 0x09;

// Colors (RGB565)
const uint16_t COLOR_WHITE = 0xFFFF;
const uint16_t COLOR_BLACK = 0x0000;
const uint16_t COLOR_RED = 0xF800;
const uint16_t COLOR_GREEN = 0x07E0;
const uint16_t COLOR_BLUE = 0x001F;
const uint16_t COLOR_YELLOW = 0xFFE0;
const uint16_t COLOR_MAGENTA = 0xF81F;
const uint16_t COLOR_CYAN = 0x07FF;
const uint16_t COLOR_BG_BLACK = 0x0841;
const uint16_t COLOR_BG_BLUE = 0x1125;

//==============================================================================
// DWIN LOW-LEVEL COMMUNICATION FUNCTIONS
//==============================================================================

/**
 * @brief Starts a new DWIN command frame by adding the header.
 */
void dwin_start_frame() {
  cmd_idx = 0;
  dwin_cmd_buffer[cmd_idx++] = FRAME_HEADER;
}

/**
 * @brief Adds a single byte to the current DWIN command buffer.
 * @param value The byte to add.
 */
void dwin_add_byte(uint8_t value) {
  if (cmd_idx < sizeof(dwin_cmd_buffer)) {
    dwin_cmd_buffer[cmd_idx++] = value;
  }
}

/**
 * @brief Adds a 16-bit word to the buffer in Big-Endian format (High byte first).
 * @param value The 16-bit value to add.
 */
void dwin_add_word(uint16_t value) {
  dwin_add_byte((value >> 8) & 0xFF);
  dwin_add_byte(value & 0xFF);
}

/**
 * @brief Adds a null-terminated string to the buffer.
 * @param str The C-style string to add.
 */
void dwin_add_string(const char* str) {
  while (*str) {
    dwin_add_byte(*str++);
  }
}

/**
 * @brief Finalizes and sends the DWIN command frame over UART.
 */
void dwin_send_frame() {
  DWINSerial.write(dwin_cmd_buffer, cmd_idx);
  DWINSerial.write(FRAME_TAIL, 4);
  delay(1);
}

//==============================================================================
// DWIN HIGH-LEVEL DRAWING FUNCTIONS
//==============================================================================

/**
 * @brief Displays a text string on the screen for setup status messages.
 * @param x The x-coordinate of the top-left corner.
 * @param y The y-coordinate of the top-left corner.
 * @param color The RGB565 text color.
 * @param text The string to display.
 */
void dwin_draw_setup_string(uint16_t x, uint16_t y, uint16_t color, const char* text) {
  uint8_t mode = 0x40 | FONT_6x12;

  dwin_start_frame();
  dwin_add_byte(CMD_DRAW_STRING);
  dwin_add_byte(mode);
  dwin_add_word(color);
  dwin_add_word(COLOR_BLACK);
  dwin_add_word(x);
  dwin_add_word(y);
  dwin_add_string(text);
  dwin_send_frame();
}

/**
 * @brief Clears the entire screen to a specified color.
 * @param color The RGB565 color to fill the screen with.
 */
void dwin_clear_screen(uint16_t color) {
  dwin_start_frame();
  dwin_add_byte(CMD_CLEAR_SCREEN);
  dwin_add_word(color);
  dwin_send_frame();
}

/**
 * @brief Converts a 32-bit LVGL color to DWIN's 16-bit RGB565 format.
 * @param lvgl_color The LVGL color structure.
 * @return The 16-bit RGB565 color word.
 */
uint16_t lvgl_to_dwin_color(lv_color_t lvgl_color) {
  uint16_t r5 = (lvgl_color.ch.red >> 3) & 0x1F;
  uint16_t g6 = (lvgl_color.ch.green >> 2) & 0x3F;
  uint16_t b5 = (lvgl_color.ch.blue >> 3) & 0x1F;
  return (r5 << 11) | (g6 << 5) | b5;
}