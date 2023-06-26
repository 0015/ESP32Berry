/////////////////////////////////////////////////////////////////
/*
  ESP32Berry, "ESP-NOW Chat App" Version 0.3
  For More Information: https://youtu.be/UhIXAp2wqjg
  Created by Eric N. (ThatProject)
*/
/////////////////////////////////////////////////////////////////
#pragma once
#include <lvgl.h>
#include <vector>
#include "ESP32Berry_Config.h"
#include "ESP32Berry_Display.h"
#include "ESP32Berry_Network.h"
#include "ESP32Berry_System.h"

class AppBase {
private:
  lv_obj_t *_bodyScreen;
  void ui_app(const char *title);
protected:
  lv_obj_t *appMain;
  lv_obj_t *appTitle;
  lv_obj_t *closeBtn;
public:
  AppBase(Display *display, System *system, Network *network, const char *title);
  ~AppBase();
  Display *_display;
  System *_system;
  Network *_network;

  void base_event_handler(lv_event_t *e);
  virtual void close_app(){};
};