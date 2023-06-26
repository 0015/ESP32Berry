/////////////////////////////////////////////////////////////////
/*
  ESP32Berry, "WiFi connection & Simple Note App" Version 0.1
  For More Information: https://youtu.be/wqaxCAcghtk
  Created by Eric N. (ThatProject)
*/
/////////////////////////////////////////////////////////////////
#pragma once
#include <lvgl.h>
#include <vector>
#define LGFX_USE_V1
#include <LovyanGFX.hpp>
#include "ESP32Berry_KeyPad.h"
#include "ESP32Berry_Config.h"

typedef enum {
  WIFI_OFF,
  WIFI_ON,
  Battery,
  SDCARD,
  Restart,
  APP_Note
} Menu_Event_t;


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
  KeyPad *keypad;
  long uiTimer;
  lv_obj_t *logoBtn;
  lv_obj_t *statusLabel;
  lv_obj_t *timeDataLabel;
  lv_obj_t *timeDataBtn;
  lv_obj_t *bodyScreen;
  lv_obj_t *calendar;
  lv_obj_t *menuList;
  lv_obj_t *settinglabel;
  lv_obj_t *settingPage;
  lv_obj_t *settingCloseBtn;
  lv_obj_t *settingWiFiSwitch;
  lv_obj_t *wfList;
  lv_obj_t *popupBox;
  lv_obj_t *popupBoxCloseBtn;
  lv_obj_t *mboxConnect;
  lv_obj_t *mboxTitle;
  lv_obj_t *mboxPassword;
  lv_obj_t *mboxConnectBtn;
  lv_obj_t *mboxCloseBtn;
  lv_obj_t *focusedObj;
  lv_obj_t *popupLoading;
  lv_obj_t *appNoteBtn;
  lv_style_t title_style;
  lv_style_t border_style;

  void initLVGL();
  void initKeyPad();
  void ui_style();
  void ui_main();
  void ui_menu();
  void ui_setting();
  void ui_popup_box();
  void ui_popup_open(String title, String msg);
  void ui_wifi_conenct_box();
  void ui_loading();
  void ui_calendar();
  void ui_apps();
  typedef void (*FuncPtrInt)(Menu_Event_t, char *);

public:
  FuncPtrInt menu_event_cb;
  Display(FuncPtrInt callback);
  ~Display();
  void initTFT();
  void my_disp_flush(lv_disp_drv_t *disp, const lv_area_t *area, lv_color_t *color_p);
  void my_touch_read(lv_indev_drv_t *indev_driver, lv_indev_data_t *data);
  void btn_event_cb(lv_event_t *e);
  void wifi_event_cb(lv_event_t *e);
  void textarea_event_cb(lv_event_t *e);
  void draw_textarea();
  lv_obj_t *focused_obj();
  void set_focused_obj(lv_obj_t *obj);
  void update_ui_network(std::vector<String> newWifiList);
  void show_loading_popup(bool isOn);
  void update_status_bar(std::vector<String> eventLog);
  void update_time(String time);
  lv_obj_t *getBodyScreen();
};