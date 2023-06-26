/////////////////////////////////////////////////////////////////
/*
  ESP32Berry, "Telegram App" Version 0.2
  For More Information: https://youtu.be/h28_Mvgpe2Y
  Created by Eric N. (ThatProject)
*/
/////////////////////////////////////////////////////////////////
#pragma once
#include <lvgl.h>
#include <vector>
#include "ESP32Berry_Config.h"
#include "ESP32Berry_Display.h"
#include "ESP32Berry_System.h"

typedef enum {
  WIFI_OFF,
  WIFI_ON,
  Battery,
  SDCARD,
  Restart,
} Menu_Event_t;

class Menu {
private:
  lv_obj_t *_uiRoot;
  lv_obj_t *_bodyScreen;
  lv_obj_t *logoBtn;
  lv_obj_t *menuList;
  lv_obj_t *uiMenuWiFI;
  lv_obj_t *uiWiFiSwitch;
  lv_obj_t *uiWiFiList;
  lv_obj_t *uiWiFiCloseBtn;
  lv_obj_t *mboxConnect;
  lv_obj_t *mboxTitle;
  lv_obj_t *mboxPassword;
  lv_obj_t *mboxConnectBtn;
  lv_obj_t *mboxCloseBtn;

  lv_obj_t *uiMenuSDCard;
  lv_obj_t *uiMenuCloseBtn;

  lv_obj_t *uiArc;
  lv_obj_t *uiPercentage;
  lv_obj_t *uiTotalBytes;
  lv_obj_t *uiUsedBytes;

  lv_style_t borderStyle;
  lv_style_t batteryStyle;
  lv_obj_t *uiMenuBattery;
  lv_obj_t *uiBatteryCloseBtn;
  lv_obj_t *batteryMeter;
  lv_obj_t *uiBatteryVolts;
  lv_obj_t *uiBatteryPercent;

  void ui_menu();
  void ui_menu_open(bool isOn);
  void ui_menu_items();
  void ui_menu_wifi();
  void ui_wifi_conenct_box();

  void ui_menu_battery();
  void ui_menu_sdcard();
  void get_sdcard_info();
  typedef void (*FuncPtrInt)(Menu_Event_t, char *);

public:
  FuncPtrInt menu_event_cb;
  Menu(Display *display, System *system, FuncPtrInt callback);
  ~Menu();
  Display *_display;
  System *_system;

  void menu_event_handler(lv_event_t *e);
  void wifi_event_cb(lv_event_t *e);
  void update_ui_network(std::vector<String> newWifiList);
  void ui_network_switch(bool isOn);
  void update_battery(String info);
  void open_menu_sdcard();
};