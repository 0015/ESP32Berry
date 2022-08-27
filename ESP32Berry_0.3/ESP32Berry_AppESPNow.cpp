/////////////////////////////////////////////////////////////////
/*
  ESP32Berry, "ESP-NOW Chat App" Version 0.3
  For More Information: https://youtu.be/UhIXAp2wqjg
  Created by Eric N. (ThatProject)
*/
/////////////////////////////////////////////////////////////////
#include "ESP32Berry_AppESPNow.h"

static AppESPNow *instance = NULL;

AppESPNow::AppESPNow(Display *display, System *system, Network *network, const char *title)
  : AppBase(display, system, network, title) {
  _bodyScreen = display->get_body_screen();
  instance = this;
  this->draw_ui();
}

AppESPNow::~AppESPNow() {}

extern "C" void esp_textarea_event_cb_thunk(lv_event_t *e) {
  instance->_display->textarea_event_cb(e);
}

extern "C" void esp_event_handler_thunk(lv_event_t *e) {
  instance->esp_event_handler(e);
}

extern "C" void esp_on_data_sent_thunk(const uint8_t *macAddr, esp_now_send_status_t status) {
  instance->esp_on_data_sent(macAddr, status);
}

extern "C" void esp_on_data_receive_thunk(const uint8_t *macAddr, const uint8_t *incomingData, int len) {
  instance->esp_on_data_receive(macAddr, incomingData, len);
}

void AppESPNow::esp_on_data_sent(const uint8_t *macAddr, esp_now_send_status_t status) {
  if (status == ESP_NOW_SEND_SUCCESS) {
    String myMsg = String(lv_textarea_get_text(textField));
    myMsg.trim();
    if (myMsg.length() > 0) {
      if ((_system->get_local_time_for_msg()).length() > 0) {
        this->add_msg(true, _system->get_local_time_for_msg() + "\n" + myMsg);
      } else {
        this->add_msg(true, myMsg);
      }
    }
  } else {
    this->add_msg(true, "Error sending the data");
  }
}

void AppESPNow::esp_on_data_receive(const uint8_t *macAddr, const uint8_t *incomingData, int len) {
  memcpy(&received_message, incomingData, sizeof(received_message));
  if (len > 0) {
    String id = received_message.id;
    String msg = received_message.msg;
    if ((_system->get_local_time_for_msg()).length() > 0) {
      char buffer[len + (_system->get_local_time_for_msg()).length()];
      sprintf(buffer, "%s\n[%s] - %s", _system->get_local_time_for_msg(), id, msg);
      this->add_msg(false, String(buffer));
    } else {
      char buffer[len];
      sprintf(buffer, "[%s] - %s", id, msg);
      this->add_msg(false, String(buffer));
    }
  }
}

String AppESPNow::get_string_mac(uint8_t addr[]) {
  String temp;
  for (int i = 0; i < 6; ++i) {
    char buf[3];
    sprintf(buf, "%02X", addr[i]);
    temp += buf;
    if (i < 5) temp += ':';
  }
  return temp;
}

void AppESPNow::esp_event_handler(lv_event_t *e) {
  lv_event_code_t code = lv_event_get_code(e);
  lv_obj_t *obj = lv_event_get_target(e);

  if (code == LV_EVENT_CLICKED) {
    if (obj == sendBtn) {
      String myMsg = String(lv_textarea_get_text(textField));
      myMsg.trim();
      if (myMsg.length() > 0) {

        send_message.id = DEVICE_NAME;
        send_message.msg = myMsg;
        esp_now_send(peerMacAddr, (uint8_t *)&send_message, sizeof(send_message));
      }
    } else if (obj == mboxConnectBtn) {

      String peerAddr = String(lv_textarea_get_text(mboxInput));
      peerAddr.trim();
      if (peerAddr.length() != 12) {
        _display->ui_popup_open("[Error]", "Check The Peer's Mac Address.\n(Enter without colons.)");
        return;
      }

      lv_obj_move_background(mboxConnect);
      lv_obj_add_flag(mboxConnect, LV_OBJ_FLAG_HIDDEN);

      sscanf(peerAddr.c_str(), "%2hhx%2hhx%2hhx%2hhx%2hhx%2hhx", &peerMacAddr[0], &peerMacAddr[1], &peerMacAddr[2], &peerMacAddr[3], &peerMacAddr[4], &peerMacAddr[5]);
      memset(&peerInfo, 0, sizeof(peerInfo));
      memcpy(peerInfo.peer_addr, peerMacAddr, 6);
      peerInfo.encrypt = false;
      peerInfo.channel = WiFi.channel();
      this->init_espnow();
    }
  }
}

void AppESPNow::draw_ui() {
  lv_style_init(&msgStyle);
  lv_style_set_bg_color(&msgStyle, lv_color_white());
  lv_style_set_pad_ver(&msgStyle, 8);
  lv_style_set_border_color(&msgStyle, lv_palette_main(LV_PALETTE_BLUE));
  lv_style_set_border_width(&msgStyle, 2);
  lv_style_set_border_opa(&msgStyle, LV_OPA_50);
  lv_style_set_border_side(&msgStyle, LV_BORDER_SIDE_BOTTOM);

  myMacLabel = lv_label_create(appMain);
  lv_label_set_long_mode(myMacLabel, LV_LABEL_LONG_SCROLL_CIRCULAR);
  lv_obj_set_width(myMacLabel, 240);
  lv_label_set_text(myMacLabel, _network->get_mac_address().c_str());
  lv_obj_align_to(myMacLabel, appTitle, LV_ALIGN_OUT_RIGHT_TOP, 14, -7);

  bottomPart = lv_obj_create(appMain);
  lv_obj_remove_style_all(bottomPart);
  lv_obj_add_flag(bottomPart, LV_OBJ_FLAG_HIDDEN);
  lv_obj_set_size(bottomPart, DISPLAY_WIDTH, 50);
  lv_obj_align(bottomPart, LV_ALIGN_BOTTOM_MID, 0, 20);
  lv_obj_clear_flag(bottomPart, LV_OBJ_FLAG_SCROLLABLE);

  textField = lv_textarea_create(bottomPart);
  lv_obj_set_size(textField, DISPLAY_WIDTH * 2 / 3 - 10, 40);
  lv_obj_align(textField, LV_ALIGN_LEFT_MID, 10, 0);
  lv_textarea_set_placeholder_text(textField, "typing?");
  lv_obj_add_event_cb(textField, esp_textarea_event_cb_thunk, LV_EVENT_FOCUSED, NULL);
  lv_obj_add_event_cb(textField, esp_textarea_event_cb_thunk, LV_EVENT_DEFOCUSED, NULL);

  sendBtn = lv_btn_create(bottomPart);
  lv_obj_set_size(sendBtn, DISPLAY_WIDTH * 1 / 3 - 20, 40);

  lv_obj_align(sendBtn, LV_ALIGN_RIGHT_MID, 0, 0);
  lv_obj_t *btnLabel = lv_label_create(sendBtn);
  lv_label_set_text(btnLabel, "Send");
  lv_obj_center(btnLabel);
  lv_obj_add_event_cb(sendBtn, esp_event_handler_thunk, LV_EVENT_CLICKED, NULL);

  msgList = lv_list_create(appMain);
  lv_obj_set_size(msgList, DISPLAY_WIDTH, 180);
  lv_obj_align_to(msgList, bottomPart, LV_ALIGN_OUT_TOP_MID, 0, -2);

  ui_espnow_conenct_box();
}

void AppESPNow::init_espnow() {

  esp_wifi_set_channel(WiFi.channel(), WIFI_SECOND_CHAN_NONE);

  if (esp_now_init() != ESP_OK) {
    _display->ui_popup_open("[Error]", "There was an error initializing ESP-NOW");
    return;
  }

  if (esp_now_add_peer(&peerInfo) != ESP_OK) {
    lv_obj_move_foreground(mboxConnect);
    lv_obj_clear_flag(mboxConnect, LV_OBJ_FLAG_HIDDEN);
    _display->ui_popup_open("[Error]", "Failed to add peer");
    return;
  }

  esp_now_register_send_cb(esp_on_data_sent_thunk);
  esp_now_register_recv_cb(esp_on_data_receive_thunk);

  lv_obj_clear_flag(bottomPart, LV_OBJ_FLAG_HIDDEN);

  String connectedInfo = "My Mac: ";
  connectedInfo += _network->get_mac_address();
  connectedInfo += "\n";
  connectedInfo += "Peer Mac: ";
  connectedInfo += get_string_mac(peerMacAddr);

  lv_label_set_text(myMacLabel, connectedInfo.c_str());

  lv_textarea_set_text(textField, "Hello!");
  lv_event_send(sendBtn, LV_EVENT_CLICKED, NULL);
}

void AppESPNow::ui_espnow_conenct_box() {
  mboxConnect = lv_obj_create(appMain);
  lv_obj_set_size(mboxConnect, DISPLAY_WIDTH * 2 / 3, DISPLAY_HEIGHT / 2);
  lv_obj_center(mboxConnect);

  lv_obj_t *mboxTitle = lv_label_create(mboxConnect);
  lv_label_set_text(mboxTitle, "Enter the Mac Address of the connected device");
  lv_obj_set_size(mboxTitle, DISPLAY_WIDTH * 2 / 3 - 40, 40);
  lv_obj_align(mboxTitle, LV_ALIGN_TOP_MID, 0, 0);

  mboxInput = lv_textarea_create(mboxConnect);
  lv_obj_set_size(mboxInput, DISPLAY_WIDTH * 2 / 3 - 40, 40);
  lv_obj_align_to(mboxInput, mboxTitle, LV_ALIGN_TOP_MID, 0, 40);
  lv_textarea_set_placeholder_text(mboxInput, "Like 30AEA45DD4B4");
  lv_obj_add_event_cb(mboxInput, esp_textarea_event_cb_thunk, LV_EVENT_FOCUSED, NULL);
  lv_obj_add_event_cb(mboxInput, esp_textarea_event_cb_thunk, LV_EVENT_DEFOCUSED, NULL);

  mboxConnectBtn = lv_btn_create(mboxConnect);
  lv_obj_add_event_cb(mboxConnectBtn, esp_event_handler_thunk, LV_EVENT_ALL, NULL);
  lv_obj_align(mboxConnectBtn, LV_ALIGN_BOTTOM_RIGHT, 0, 0);
  lv_obj_t *btnLabel = lv_label_create(mboxConnectBtn);
  lv_label_set_text(btnLabel, "Connect");
  lv_obj_center(btnLabel);
}

void AppESPNow::add_msg(bool isMine, String msg) {
  lv_obj_t *text = lv_list_add_text(msgList, msg.c_str());
  lv_obj_add_style(text, &msgStyle, 0);
  lv_label_set_long_mode(text, LV_LABEL_LONG_WRAP);
  lv_obj_set_style_text_align(text, isMine ? LV_TEXT_ALIGN_RIGHT : LV_TEXT_ALIGN_LEFT, 0);
  lv_obj_scroll_to_y(msgList, lv_obj_get_scroll_y(msgList) + lv_obj_get_height(msgList), LV_ANIM_ON);
  if (isMine) lv_textarea_set_text(textField, "");
}

void AppESPNow::close_app() {
  if (appMain != NULL) {
    if (esp_now_del_peer(peerMacAddr) == ESP_OK) {
      esp_now_unregister_recv_cb();
      esp_now_unregister_send_cb();
      esp_now_deinit();
    }
    delay(1);
    lv_obj_del_async(appMain);
    appMain = NULL;
    delete this;
  }
}