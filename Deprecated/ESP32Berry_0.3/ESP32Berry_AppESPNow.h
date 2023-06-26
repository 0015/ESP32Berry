/////////////////////////////////////////////////////////////////
/*
  ESP32Berry, "ESP-NOW Chat App" Version 0.3
  For More Information: https://youtu.be/UhIXAp2wqjg
  Created by Eric N. (ThatProject)
*/
/////////////////////////////////////////////////////////////////
#pragma once
#include "ESP32Berry_Config.h"
#include "ESP32Berry_AppBase.h"
#include <esp_now.h>
#include <esp_wifi.h>

class AppESPNow : public AppBase {
private:
  lv_obj_t *_bodyScreen;
  lv_obj_t *bottomPart;
  lv_obj_t *textField;
  lv_obj_t *sendBtn;
  lv_obj_t *msgList;
  lv_obj_t *myMacLabel;
  lv_obj_t *mboxConnect;
  lv_obj_t *mboxInput;
  lv_obj_t *mboxConnectBtn;
  lv_style_t msgStyle;
  esp_now_peer_info_t peerInfo;
  uint8_t peerMacAddr[6];
  typedef struct struct_message {
    String id;
    String msg;
  } struct_message;
  struct_message received_message, send_message;
  void draw_ui();
  void add_msg(bool isMine, String msg);
  void init_espnow();
  void ui_espnow_conenct_box();
  String get_string_mac(uint8_t addr[]);

public:
  AppESPNow(Display *display, System *system, Network *network, const char *title);
  ~AppESPNow();
  void esp_event_handler(lv_event_t *e);
  void esp_on_data_sent(const uint8_t *macAddr, esp_now_send_status_t status);
  void esp_on_data_receive(const uint8_t *macAddr, const uint8_t *incomingData, int len);
  void close_app();
};