#include <Arduino.h>
#include <HardwareSerial.h>
#include <dwin.h>
#include <lvgl.h>

//==============================================================================
// DWIN PROTOCOL & DISPLAY CONFIGURATION
//==============================================================================

HardwareSerial DWINSerial(2);
#define DWIN_RX_PIN 16
#define DWIN_TX_PIN 17
#define DWIN_BAUD_RATE 115200

//==============================================================================
// MAIN APPLICATION LOGIC
//==============================================================================

/**
 * @brief Creates a simple test HMI screen with LVGL widgets.
 */
void create_test_hmi() {
  lv_obj_t *scr = lv_scr_act();
  lv_obj_set_style_bg_color(scr, lv_color_black(), LV_PART_MAIN);

  // Create a title label
  lv_obj_t *label = lv_label_create(scr);
  lv_label_set_text(label, "ESP32 + DWIN + LVGL");
  lv_obj_set_style_text_color(label, lv_color_white(), LV_PART_MAIN);
  lv_obj_set_style_text_font(label, &lv_font_montserrat_14, LV_PART_MAIN);
  lv_obj_align(label, LV_ALIGN_TOP_MID, 0, 20);

  // Create a button with a label
  lv_obj_t *btn = lv_btn_create(scr);
  lv_obj_align(btn, LV_ALIGN_CENTER, 0, -50);
  lv_obj_set_size(btn, 150, 50);
  lv_obj_set_style_bg_color(btn, lv_color_hex(0x007BFF), LV_PART_MAIN);
  lv_obj_set_style_shadow_width(btn, 10, LV_PART_MAIN);
  lv_obj_set_style_shadow_color(btn, lv_color_hex(0x0056b3), LV_PART_MAIN);

  lv_obj_t *btn_label = lv_label_create(btn);
  lv_label_set_text(btn_label, "Press Me");
  lv_obj_set_style_text_color(btn_label, lv_color_white(), LV_PART_MAIN);
  lv_obj_center(btn_label);

  // Create a slider
  lv_obj_t *slider = lv_slider_create(scr);
  lv_obj_set_width(slider, 200);
  lv_obj_align(slider, LV_ALIGN_CENTER, 0, 50);
  lv_slider_set_value(slider, 70, LV_ANIM_ON);
  lv_obj_set_style_bg_color(slider, lv_color_hex(0x00FF00), LV_PART_INDICATOR);
}

/**
 * @brief Arduino setup function. Runs once on startup.
 */
void setup() {
  Serial.begin(DWIN_BAUD_RATE);
  DWINSerial.begin(DWIN_BAUD_RATE, SERIAL_8N1, DWIN_RX_PIN, DWIN_TX_PIN);

  delay(500);
  Serial.println("\n--- DWIN LVGL Driver Initialization ---");
  dwin_draw_setup_string(10, 10, COLOR_WHITE, "Serial Ports Initialized.");

  // Set Screen Orientation to 90 degrees
  dwin_start_frame();
  dwin_add_byte(CMD_SET_DIRECTION);
  dwin_add_byte(0x5A);
  dwin_add_byte(0xA5);
  dwin_add_byte(0x01);
  dwin_send_frame();
  
  // Clear screen to black
  dwin_clear_screen(COLOR_BLACK);
  dwin_draw_setup_string(10, 110, COLOR_GREEN, "Orientation: 90 deg. Screen Cleared.");
  
  // Initialize LVGL
  lvgl_driver_init();
  dwin_draw_setup_string(10, 130, COLOR_GREEN, "LVGL Setup Complete. Starting HMI...");
  delay(1000);
  
  // Create user interface
  create_test_hmi();

  Serial.println("Initialization complete. Running LVGL handler.");
}

/**
 * @brief Arduino loop function. Runs continuously.
 */
void loop() {
  lv_timer_handler();
  delay(5);
}