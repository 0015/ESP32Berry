/////////////////////////////////////////////////////////////////
/*
  ESP32Berry, "Telegram App" Version 0.2
  For More Information: https://youtu.be/h28_Mvgpe2Y
  Created by Eric N. (ThatProject)
*/
/////////////////////////////////////////////////////////////////
#pragma once
#include "ESP32Berry_Config.h"
#include "ESP32Berry_AppBase.h"

class AppTelegram : public AppBase {
private:
  lv_obj_t *_bodyScreen;
  lv_obj_t *textField;
  lv_obj_t *sendBtn;
  lv_obj_t *msgList;
  void draw_ui();
  void add_msg(bool isMine, String msg);
  lv_style_t msgStyle;

public:
  AppTelegram(Display *display, System *system, Network *network, const char *title);
  ~AppTelegram();
  void tg_event_handler(lv_event_t *e);
  void telegram_cb(FB_msg &msg);
  void close_app();
};