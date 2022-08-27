/////////////////////////////////////////////////////////////////
/*
  ESP32Berry, "ESP-NOW Chat App" Version 0.3
  For More Information: https://youtu.be/UhIXAp2wqjg
  Created by Eric N. (ThatProject)
*/
/////////////////////////////////////////////////////////////////
#include "ESP32Berry_AppTelegram.h"

static AppTelegram *instance = NULL;

extern "C" void telegram_app_cb_thunk(FB_msg &msg) {
  instance->telegram_cb(msg);
}

AppTelegram::AppTelegram(Display *display, System *system, Network *network, const char *title)
  : AppBase(display, system, network, title) {
  _bodyScreen = display->get_body_screen();
  instance = this;

  this->draw_ui();
  _network->telegram_set_cb_func(telegram_app_cb_thunk);
}

AppTelegram::~AppTelegram() {}

extern "C" void tg_textarea_event_cb_thunk(lv_event_t *e) {
  instance->_display->textarea_event_cb(e);
}

extern "C" void tg_event_handler_thunk(lv_event_t *e) {
  instance->tg_event_handler(e);
}

void AppTelegram::tg_event_handler(lv_event_t *e) {
  lv_event_code_t code = lv_event_get_code(e);
  lv_obj_t *obj = lv_event_get_target(e);

  if (code == LV_EVENT_CLICKED) {
    if (obj == sendBtn) {
      String myMsg = String(lv_textarea_get_text(textField));
      myMsg.trim();
      if (myMsg.length() > 0) {

        if (_network->telegram_send(myMsg)) {
          this->add_msg(true, myMsg);
        } else {
          lv_textarea_set_text(textField, "");
        }
      }
    }
  }
}

void AppTelegram::draw_ui() {
  lv_style_init(&msgStyle);
  lv_style_set_bg_color(&msgStyle, lv_color_white());
  lv_style_set_pad_ver(&msgStyle, 8);
  lv_style_set_border_color(&msgStyle, lv_palette_main(LV_PALETTE_BLUE));
  lv_style_set_border_width(&msgStyle, 2);
  lv_style_set_border_opa(&msgStyle, LV_OPA_50);
  lv_style_set_border_side(&msgStyle, LV_BORDER_SIDE_BOTTOM);

  lv_obj_t *bottomPart = lv_obj_create(appMain);
  lv_obj_remove_style_all(bottomPart);
  lv_obj_set_size(bottomPart, DISPLAY_WIDTH, 50);
  lv_obj_align(bottomPart, LV_ALIGN_BOTTOM_MID, 0, 20);
  lv_obj_clear_flag(bottomPart, LV_OBJ_FLAG_SCROLLABLE);

  textField = lv_textarea_create(bottomPart);
  lv_obj_set_size(textField, DISPLAY_WIDTH * 2 / 3 - 10, 40);
  lv_obj_align(textField, LV_ALIGN_LEFT_MID, 10, 0);
  lv_textarea_set_placeholder_text(textField, "typing?");
  lv_obj_add_event_cb(textField, tg_textarea_event_cb_thunk, LV_EVENT_FOCUSED, NULL);
  lv_obj_add_event_cb(textField, tg_textarea_event_cb_thunk, LV_EVENT_DEFOCUSED, NULL);

  sendBtn = lv_btn_create(bottomPart);
  lv_obj_set_size(sendBtn, DISPLAY_WIDTH * 1 / 3 - 20, 40);

  lv_obj_align(sendBtn, LV_ALIGN_RIGHT_MID, 0, 0);
  lv_obj_t *btnLabel = lv_label_create(sendBtn);
  lv_label_set_text(btnLabel, "Send");
  lv_obj_center(btnLabel);
  lv_obj_add_event_cb(sendBtn, tg_event_handler_thunk, LV_EVENT_CLICKED, NULL);

  msgList = lv_list_create(appMain);
  lv_obj_set_size(msgList, DISPLAY_WIDTH, 180);
  lv_obj_align_to(msgList, bottomPart, LV_ALIGN_OUT_TOP_MID, 0, -2);
}

void AppTelegram::add_msg(bool isMine, String msg) {
  lv_obj_t *text = lv_list_add_text(msgList, msg.c_str());
  lv_obj_add_style(text, &msgStyle, 0);
  lv_label_set_long_mode(text, LV_LABEL_LONG_WRAP);
  lv_obj_set_style_text_align(text, isMine ? LV_TEXT_ALIGN_RIGHT : LV_TEXT_ALIGN_LEFT, 0);
  lv_obj_scroll_to_y(msgList, lv_obj_get_scroll_y(msgList) + lv_obj_get_height(msgList), LV_ANIM_ON);
  if (isMine)
    lv_textarea_set_text(textField, "");
}

void AppTelegram::close_app() {
  _network->telegram_reset_cb_func();
  if (appMain != NULL) {
    lv_obj_del_async(appMain);
    appMain = NULL;
    delete this;
  }
}

void AppTelegram::telegram_cb(FB_msg &msg) {
  String telegramMsg = "[" + msg.username + "] - " + msg.text;
  this->add_msg(false, telegramMsg);
}