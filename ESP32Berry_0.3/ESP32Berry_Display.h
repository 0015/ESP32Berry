/////////////////////////////////////////////////////////////////
/*
  ESP32Berry, "ESP-NOW Chat App" Version 0.3
  For More Information: https://youtu.be/UhIXAp2wqjg
  Created by Eric N. (ThatProject)
*/
/////////////////////////////////////////////////////////////////
#pragma once
#include <lvgl.h>
#define LGFX_USE_V1
#include <LovyanGFX.hpp>
#include "ESP32Berry_Config.h"

#ifdef ESP32BERRY
#include "ESP32Berry_KeyPad.h"
#endif

typedef enum {
  APP_Note,
  APP_Telegram,
  APP_ESPNOW
} App_Event_t;

class LGFX : public lgfx::LGFX_Device {
  lgfx::Panel_ILI9488 _panel_instance;
  lgfx::Bus_SPI _bus_instance;
  lgfx::Touch_XPT2046 _touch_instance;

public:
  LGFX(void) {
    {
      auto cfg = _bus_instance.config();
      cfg.spi_host = VSPI_HOST;
      cfg.spi_mode = 0;
      cfg.freq_write = 40000000;
      cfg.freq_read = 16000000;
      cfg.spi_3wire = false;
      cfg.use_lock = true;
      cfg.dma_channel = 1;
      cfg.pin_sclk = 18;
      cfg.pin_mosi = 23;
      cfg.pin_miso = -1;
      cfg.pin_dc = 14;
      _bus_instance.config(cfg);
      _panel_instance.setBus(&_bus_instance);
    }

    {
      auto cfg = _panel_instance.config();

      cfg.pin_cs = 13;
      cfg.pin_rst = 12;
      cfg.pin_busy = -1;
      cfg.memory_width = 320;
      cfg.memory_height = 480;
      cfg.panel_width = 320;
      cfg.panel_height = 480;
      cfg.offset_x = 0;
      cfg.offset_y = 0;
      cfg.offset_rotation = 2;
      cfg.dummy_read_pixel = 8;
      cfg.dummy_read_bits = 1;
      cfg.readable = true;
      cfg.invert = false;
      cfg.rgb_order = false;
      cfg.dlen_16bit = false;
      cfg.bus_shared = true;

      _panel_instance.config(cfg);
    }
    {
      auto cfg = _touch_instance.config();
      cfg.x_min = 0;
      cfg.x_max = 319;
      cfg.y_min = 0;
      cfg.y_max = 479;
      cfg.pin_int = -1;
      cfg.bus_shared = true;
      cfg.offset_rotation = 0;
      cfg.spi_host = VSPI_HOST;
      cfg.freq = 1000000;
      cfg.pin_sclk = 18;
      cfg.pin_mosi = 23;
      cfg.pin_miso = 19;
      cfg.pin_cs = 4;
      _touch_instance.config(cfg);
      _panel_instance.setTouch(&_touch_instance);
    }

    setPanel(&_panel_instance);
  }
};

class Display {
private:
  SemaphoreHandle_t bin_sem;
  friend void update_ui_task(void *pvParameters);

  LGFX *tft;
  #ifdef ESP32BERRY
  KeyPad *keypad;  
  #else
  lv_obj_t *kb;
  #endif
  long uiTimer;
  lv_obj_t *uiRoot;
  lv_obj_t *timeDataLabel;
  lv_obj_t *timeDataBtn;
  lv_obj_t *bodyScreen;
  lv_obj_t *calendar;
  lv_obj_t *networkIcon;
  lv_obj_t *popupBox;
  lv_obj_t *popupBoxCloseBtn;
  lv_obj_t *focusedObj;
  lv_obj_t *popupLoading;
  lv_obj_t *appNoteBtn;
  lv_obj_t *appTelegramBtn;
  lv_obj_t *appESPNOWBtn;
  lv_obj_t *batLabel;
  lv_obj_t *notiBtn;
  lv_obj_t *notiLabel;
  lv_style_t titleStyle;
  lv_style_t borderStyle;
  lv_style_t notiStyle;

  void init_tft();
  void init_lvgl();
  void init_keypad();
  
  void ui_style();
  void ui_main();
  void ui_popup_box();
  void ui_calendar();
  void ui_apps();
  void ui_noti();
  void ui_noti_anim();
  String add_battery_icon(int percentage);
  typedef void (*FuncPtrInt)(App_Event_t, char *);

public:
  FuncPtrInt app_event_cb;
  Display(FuncPtrInt callback);
  ~Display();
  void my_disp_flush(lv_disp_drv_t *disp, const lv_area_t *area, lv_color_t *color_p);
  void my_touch_read(lv_indev_drv_t *indev_driver, lv_indev_data_t *data);
  void ui_popup_open(String title, String msg);
  void btn_event_cb(lv_event_t *e);
  void textarea_event_cb(lv_event_t *e);
  void anim_x_cb(void *var, int32_t v);
  void set_wifi_icon(bool isConnected);
  void set_focused_obj(lv_obj_t *obj);
  void show_loading_popup(bool isOn);
  void update_time(String time);
  void update_battery(String info);
  void launch_noti(String msg);
  LGFX * get_lgfx();
  lv_obj_t *focused_obj();
  lv_obj_t *get_ui_root();
  lv_obj_t *get_body_screen();
  SemaphoreHandle_t get_mutex();
};