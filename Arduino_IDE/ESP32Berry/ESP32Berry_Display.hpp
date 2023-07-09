/////////////////////////////////////////////////////////////////
/*
  New ESP32Berry Project, The base UI & ChatGPT Client
  For More Information: https://youtu.be/5K6rSw9j5iY
  Created by Eric N. (ThatProject)
*/
/////////////////////////////////////////////////////////////////
#pragma once
#include <lvgl.h>
#include <Wire.h>
#include <vector>
#include "LGFX_T-Deck.h"
#include "ESP32Berry_Config.hpp"

typedef enum {
  WIFI_OFF,
  WIFI_ON,
  APP
} Menu_Event_t;

LV_IMG_DECLARE(mouse_cursor_icon);

class Display {
private:
  TaskHandle_t lvgl_task_handle;
  SemaphoreHandle_t bin_sem;
  friend void update_ui_task(void *pvParameters);
  LGFX *tft;

  lv_obj_t *ui_Main_Screen;
  lv_obj_t *ui_TopPanel;
  lv_obj_t *ui_ControlPanel;
  lv_obj_t *ui_SliderSpeaker;
  lv_obj_t *ui_SliderBrightness;
  lv_obj_t *ui_ImgBtnWiFi;
  lv_obj_t *ui_BtnWiFi;
  lv_obj_t *ui_WiFiPanel;
  lv_obj_t *ui_WiFiPanelCloseBtn;
  lv_obj_t *ui_WiFiLabel;
  lv_obj_t *ui_Userlabel;
  lv_obj_t *ui_TimeLabel;
  lv_obj_t *ui_DateLabel;
  lv_obj_t *ui_BatteryLabel;
  lv_obj_t *ui_Sub_Screen;
  lv_obj_t *ui_Focused_Obj;
  lv_obj_t *ui_Loading;
  lv_obj_t *ui_WiFiList;
  lv_obj_t *ui_WiFiMBox;
  lv_obj_t *ui_WiFiMBoxTitle;
  lv_obj_t *ui_WiFiMBoxPassword;
  lv_obj_t *ui_WiFiMBoxConnectBtn;
  lv_obj_t *ui_WiFiMBoxCloseBtn;
  lv_obj_t *ui_BasePopup;
  lv_obj_t *ui_BasePopupCloseBtn;
  lv_obj_t *ui_BasePopupTitle;
  lv_obj_t *ui_BasePopupMsg;

  uint32_t keypad_get_key();
  void initLVGL();
  void ui_main();
  void ui_second();
  void ui_prep_loading(); 
  void ui_prep_popup_box();
  void ui_popup_open(String title, String msg);
  void ui_WiFi_page();
  
  String add_battery_icon(int percentage);
  typedef void (*FuncPtrInt)(Menu_Event_t, void *);

public:
  lv_obj_t *ui_NotiLabel;
  FuncPtrInt menu_event_cb;
  TaskHandle_t uiNotiTaskHandler;
  Display(FuncPtrInt callback);
  ~Display();
  void initTFT();
  void my_disp_flush(lv_disp_drv_t *disp, const lv_area_t *area, lv_color_t *color_p);
  void my_touch_read(lv_indev_drv_t *indev_driver, lv_indev_data_t *data);
  void my_mouse_read(lv_indev_drv_t *indev_driver, lv_indev_data_t *data);
  void my_key_read(lv_indev_drv_t *indev_driver, lv_indev_data_t *data);
  void ui_wifi_event_callback(lv_event_t *e);
  void textarea_event_cb(lv_event_t *e);
  void ui_event_callback(lv_event_t *e);
  void ui_app_btns_callback(lv_event_t *e);
  lv_obj_t *focused_obj();
  void set_focused_obj(lv_obj_t *obj);
  void update_ui_network(void *data1, void *data2);
  void show_loading_popup(bool isOn);
  void update_time(void *timeStruct);
  void set_notification(const char *msg);
  void update_WiFi_label(void *data1);
  void update_battery(void *data1);
  lv_obj_t *ui_second_screen();
  void goback_main_screen();
  void lv_port_sem_take(void);
  void lv_port_sem_give(void);
  int get_display_width();
  int get_display_height();
};