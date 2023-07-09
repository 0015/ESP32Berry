/////////////////////////////////////////////////////////////////
/*
  New ESP32Berry Project, The base UI & ChatGPT Client
  For More Information: https://youtu.be/5K6rSw9j5iY
  Created by Eric N. (ThatProject)
*/
/////////////////////////////////////////////////////////////////
#include "ESP32Berry_AppChatGPT.hpp"

static AppChatGPT *instance = NULL;

AppChatGPT::AppChatGPT(Display *display, System *system, Network *network, const char *title)
  : AppBase(display, system, network, title) {
  instance = this;
  display_width = display->get_display_width();
  client.setInsecure();
  chat_gpt = new ChatGPT<WiFiClientSecure>(&client, "v1", OPENAI_API_KEY);
  this->draw_ui();
}

AppChatGPT::~AppChatGPT() {}

extern "C" void tg_textarea_event_cb_thunk(lv_event_t *e) {
  instance->_display->textarea_event_cb(e);
}

extern "C" void tg_event_handler_thunk(lv_event_t *e) {
  instance->tg_event_handler(e);
}

void chatGPTtask(void *pvParameters) {
  instance->show_loading_popup(true);
  std::string str = std::string((char *)pvParameters);
  instance->clean_input_field();

  String result;
  Serial.println("[ChatGPT] Only print a content message");
  if (instance->chat_gpt->simple_message("gpt-3.5-turbo-16k-0613", "user", str.c_str(), result)) {
    Serial.println("===OK===");
    Serial.println(result);
  } else {
    Serial.println("===ERROR===");
    Serial.println(result);
  }
  instance->_display->lv_port_sem_take();
  instance->add_msg(false, result);
  instance->show_loading_popup(false);
  instance->_display->lv_port_sem_give();

  vTaskDelete(NULL);
}


void AppChatGPT::tg_event_handler(lv_event_t *e) {
  lv_event_code_t code = lv_event_get_code(e);
  lv_obj_t *obj = lv_event_get_target(e);

  if (code == LV_EVENT_CLICKED) {
    if (obj == sendBtn) {
      String myMsg = String(lv_textarea_get_text(textField));
      myMsg.trim();
      if (myMsg.length() > 0) {
        this->add_msg(true, myMsg);
        xTaskCreate(chatGPTtask, "chatGPTtask", 10000, (void *)lv_textarea_get_text(textField), 1, NULL);
      }
    }
  }
}

void AppChatGPT::draw_ui() {
  lv_style_init(&msgStyle);
  lv_style_set_bg_color(&msgStyle, lv_color_white());
  lv_style_set_pad_ver(&msgStyle, 8);
  lv_style_set_border_color(&msgStyle, lv_color_hex(0x989898));
  lv_style_set_border_width(&msgStyle, 2);
  lv_style_set_border_opa(&msgStyle, LV_OPA_50);
  lv_style_set_border_side(&msgStyle, LV_BORDER_SIDE_BOTTOM);

  lv_obj_t *bottomPart = lv_obj_create(ui_AppPanel);
  lv_obj_remove_style_all(bottomPart);
  lv_obj_set_size(bottomPart, display_width, 50);
  lv_obj_align(bottomPart, LV_ALIGN_BOTTOM_MID, 0, 20);
  lv_obj_clear_flag(bottomPart, LV_OBJ_FLAG_SCROLLABLE);

  textField = lv_textarea_create(bottomPart);
  lv_obj_set_size(textField, display_width * 2 / 3 - 10, 38);
  lv_obj_align(textField, LV_ALIGN_LEFT_MID, 10, 0);
  lv_textarea_set_placeholder_text(textField, "typing?");
  lv_obj_add_event_cb(textField, tg_textarea_event_cb_thunk, LV_EVENT_FOCUSED, NULL);
  lv_obj_add_event_cb(textField, tg_textarea_event_cb_thunk, LV_EVENT_DEFOCUSED, NULL);

  sendBtn = lv_btn_create(bottomPart);
  lv_obj_set_size(sendBtn, display_width * 1 / 3 - 20, 38);

  lv_obj_align(sendBtn, LV_ALIGN_RIGHT_MID, 0, 0);
  lv_obj_t *btnLabel = lv_label_create(sendBtn);
  lv_label_set_text(btnLabel, "Send");
  lv_obj_center(btnLabel);
  lv_obj_add_event_cb(sendBtn, tg_event_handler_thunk, LV_EVENT_CLICKED, NULL);

  msgList = lv_list_create(ui_AppPanel);
  lv_obj_set_size(msgList, display_width, 160);
  lv_obj_align_to(msgList, bottomPart, LV_ALIGN_OUT_TOP_MID, 0, -2);
  lv_obj_set_style_border_opa(msgList, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_border_width(msgList, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
}

void AppChatGPT::add_msg(bool isMine, String msg) {
  lv_obj_t *text = lv_list_add_text(msgList, msg.c_str());
  lv_obj_add_style(text, &msgStyle, 0);
  lv_label_set_long_mode(text, LV_LABEL_LONG_WRAP);
  lv_obj_set_style_text_align(text, isMine ? LV_TEXT_ALIGN_RIGHT : LV_TEXT_ALIGN_LEFT, 0);
  lv_obj_scroll_to_y(msgList, lv_obj_get_scroll_y(msgList) + lv_obj_get_height(msgList), LV_ANIM_ON);
}

void AppChatGPT::clean_input_field() {
  lv_textarea_set_text(textField, "");
}

void AppChatGPT::close_app() {
  _display->goback_main_screen();
  lv_obj_del(_bodyScreen);
  delete this;
}
