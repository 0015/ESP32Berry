/////////////////////////////////////////////////////////////////
/*
  ESP32Berry, "Telegram App" Version 0.2
  For More Information: https://youtu.be/h28_Mvgpe2Y
  Created by Eric N. (ThatProject)
*/
/////////////////////////////////////////////////////////////////
#include "ESP32Berry_AppBase.h"

static AppBase *instance = NULL;
AppBase::AppBase(Display *display, System *system, Network *network, const char *title) {
  instance = this;
  _display = display;
  _bodyScreen = display->get_body_screen();
  _system = system;
  _network = network;
  this->ui_app(title);
}

AppBase::~AppBase() {}

extern "C" void base_event_handler_thunk(lv_event_t *e) {
  instance->base_event_handler(e);
}

void AppBase::base_event_handler(lv_event_t *e) {
  lv_event_code_t code = lv_event_get_code(e);
  lv_obj_t *btn = lv_event_get_target(e);

  if (code == LV_EVENT_CLICKED) {
    this->close_app();
  }
}

void AppBase::ui_app(const char *title) {
  appMain = lv_obj_create(_bodyScreen);
  lv_obj_set_size(appMain, DISPLAY_WIDTH, DISPLAY_HEIGHT - 40);
  lv_obj_align(appMain, LV_ALIGN_TOP_LEFT, -16, -16);
  lv_obj_clear_flag(appMain, LV_OBJ_FLAG_SCROLLABLE);

  closeBtn = lv_btn_create(appMain);
  lv_obj_set_size(closeBtn, 30, 30);
  lv_obj_align(closeBtn, LV_ALIGN_TOP_RIGHT, 12, -4);
  lv_obj_add_event_cb(closeBtn, base_event_handler_thunk, LV_EVENT_CLICKED, NULL);

  lv_obj_t *label = lv_label_create(closeBtn);
  lv_label_set_text(label, LV_SYMBOL_CLOSE);
  lv_obj_center(label);

  appTitle = lv_label_create(appMain);
  lv_label_set_text(appTitle, title);
  lv_obj_align(appTitle, LV_ALIGN_TOP_LEFT, 4, 0);
  lv_obj_set_style_text_font(appTitle, &lv_font_montserrat_20, LV_PART_MAIN | LV_STATE_DEFAULT);
}