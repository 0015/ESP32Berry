/////////////////////////////////////////////////////////////////
/*
  New ESP32Berry Project, The base UI & ChatGPT Client
  For More Information: https://youtu.be/5K6rSw9j5iY
  Created by Eric N. (ThatProject)
*/
/////////////////////////////////////////////////////////////////
#pragma once
#include "ESP32Berry_Config.hpp"
#include "ESP32Berry_AppBase.hpp"
#include <ArduinoJson.h>
#include <ChatGPT.hpp>

class AppChatGPT : public AppBase {
private:
  lv_style_t msgStyle;
  lv_obj_t *textField;
  lv_obj_t *sendBtn;
  lv_obj_t *msgList;
  int display_width;
  WiFiClientSecure client;
  void draw_ui();

public:
  ChatGPT<WiFiClientSecure> * chat_gpt;
  AppChatGPT(Display *display, System *system, Network *network, const char *title);
  ~AppChatGPT();
  void add_msg(bool isMine, String msg);
  void tg_event_handler(lv_event_t *e);
  void clean_input_field();
  void close_app();
};