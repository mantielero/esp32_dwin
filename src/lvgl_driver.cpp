/**
 * @file lvgl_driver.cpp
 * @author DWIN (AI Assistant)
 * @brief LVGL Driver for DWIN T5UIC1 Display (DMT48270C043_04WNZ11) on ESP32.
 * @version 1.2
 * @date 2025-10-05
 *
 * @details This code provides a complete driver to run LVGL on an ESP32 connected to
 * the DWIN display found in Creality Ender 3 v2 printers.
 * THIS IS A CORRECTED BUT STILL VERY SLOW IMPLEMENTATION.
 * The DWIN display protocol is not optimized for full-screen raw pixel updates,
 * which makes LVGL performance inherently poor. This code is for educational/debugging purposes.
 */

#include <Arduino.h>
#include <lvgl.h>
#include <dwin.h>

//==============================================================================
// LVGL PORTING CONFIGURATION
//==============================================================================

static lv_disp_draw_buf_t disp_buf;
static lv_disp_drv_t disp_drv;

// Buffer size: The DWIN display is 480x272, but we are rotating it to 272x480.
// A buffer of 20 lines for a 272px width is reasonable.
#define LV_DISP_BUF_SIZE (272 * 20)
static lv_color_t buf_1[LV_DISP_BUF_SIZE];

hw_timer_t *lvgl_timer = NULL;

//==============================================================================
// LVGL PORTING LAYER
//==============================================================================

/**
 * @brief LVGL display flush callback.
 * @param disp_drv Pointer to the LVGL display driver.
 * @param area The rectangular area to be updated.
 * @param color_p Pointer to the buffer containing the pixel data.
 * @note This function is the heart of the driver. It takes pixel data from LVGL
 * and sends the corresponding DWIN commands. The performance bottleneck is here.
 */
static void dwin_disp_flush(lv_disp_drv_t *disp_drv, const lv_area_t *area, lv_color_t *color_p) {
  int32_t width = lv_area_get_width(area);
  int32_t height = lv_area_get_height(area);

  // DWIN command pattern explanation for CMD_DRAW_RECT (0x05):
  // FRAME_HEADER | CMD | Mode | Color (2B) | X_start (2B) | Y_start (2B) | X_end (2B) | Y_end (2B) | FRAME_TAIL
  // AA 05 01 F8 00 00 64 00 64 00 C8 00 C8 CC 33 C3 3C -> Fills a red square at (100,100) of size 100x100.

  // Optimization for solid color areas (e.g., backgrounds, buttons without gradients).
  // This is the MOST EFFICIENT way to draw with this display.
  bool is_solid = true;
  lv_color_t first_color = *color_p;
  for (int32_t i = 1; i < width * height; i++) {
    if (color_p[i].full != first_color.full) {
      is_solid = false;
      break;
    }
  }

  if (is_solid) {
    uint16_t dwin_color = lvgl_to_dwin_color(first_color);
    dwin_start_frame();
    dwin_add_byte(CMD_DRAW_RECT); // Command 0x05: Draw Rectangle
    dwin_add_byte(0x01);          // Mode 0x01: Fill the area
    dwin_add_word(dwin_color);
    dwin_add_word(area->x1);
    dwin_add_word(area->y1);
    dwin_add_word(area->x2);
    dwin_add_word(area->y2);
    dwin_send_frame();
  } 
  // Fallback for multi-color areas (gradients, text, images).
  // THIS IS EXTREMELY SLOW and will be the main performance bottleneck.
  // We send each pixel one by one. The original code tried to optimize this by sending
  // small rectangles, but that also sends too many commands. This is more direct,
  // but still not fast enough for a fluid UI.
  else {
    for (int32_t y = 0; y < height; y++) {
      for (int32_t x = 0; x < width; x++) {
        uint16_t dwin_color = lvgl_to_dwin_color(*color_p);
        
        // DWIN command pattern explanation for CMD_SET_POINT (0x02):
        // FRAME_HEADER | CMD | Color(2B) | Nx | Ny | X0(2B) | Y0(2B) | ... | FRAME_TAIL
        // Using Nx=1, Ny=1 to draw a single pixel.
        dwin_start_frame();
        dwin_add_byte(CMD_SET_POINT); // Command 0x02: Set Point
        dwin_add_word(dwin_color);    // Color of the pixel
        dwin_add_byte(0x01);          // Nx: 1 pixel width
        dwin_add_byte(0x01);          // Ny: 1 pixel height
        dwin_add_word(area->x1 + x);  // Absolute X coordinate
        dwin_add_word(area->y1 + y);  // Absolute Y coordinate
        dwin_send_frame();
        
        color_p++; // Move to the next pixel in the buffer
      }
    }
  }

  // Tell LVGL that we are done flushing and it can send the next chunk.
  lv_disp_flush_ready(disp_drv);
}

/**
 * @brief Interrupt service routine for the LVGL tick timer.
 */
void IRAM_ATTR lvgl_tick_handler() {
  lv_tick_inc(5); // Tell LVGL 5ms have passed
}

/**
 * @brief Initializes the LVGL library and the custom DWIN display driver.
 */
void lvgl_driver_init() {
  // Initialize LVGL Tick Timer using ESP32 hardware timer
  lvgl_timer = timerBegin(0, 80, true); // Timer 0, prescaler 80, count up
  timerAttachInterrupt(lvgl_timer, &lvgl_tick_handler, true);
  timerAlarmWrite(lvgl_timer, 5000, true); // Interrupt every 5000 ticks (5ms)
  timerAlarmEnable(lvgl_timer);

  // Initialize LVGL Core
  lv_init();
  dwin_draw_setup_string(10, 30, COLOR_WHITE, "LVGL Core Initialized.");

  // Initialize LVGL Display Buffer
  lv_disp_draw_buf_init(&disp_buf, buf_1, NULL, LV_DISP_BUF_SIZE);
  dwin_draw_setup_string(10, 50, COLOR_WHITE, "LVGL Draw Buffer Ready.");

  // Initialize and Register Display Driver
  lv_disp_drv_init(&disp_drv);
  disp_drv.hor_res = 272; // Horizontal resolution after 90-degree rotation
  disp_drv.ver_res = 480; // Vertical resolution after 90-degree rotation
  disp_drv.flush_cb = dwin_disp_flush;
  disp_drv.draw_buf = &disp_buf;
  lv_disp_drv_register(&disp_drv);
  dwin_draw_setup_string(10, 70, COLOR_WHITE, "DWIN Driver Registered.");
}