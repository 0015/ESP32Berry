/////////////////////////////////////////////////////////////////
/*
  New ESP32Berry Project, The base UI & ChatGPT Client
  For More Information: https://youtu.be/5K6rSw9j5iY
  Created by Eric N. (ThatProject)
*/
/////////////////////////////////////////////////////////////////
#include "ESP32Berry_AppBase.hpp"

static AppBase *instance = NULL;
AppBase::AppBase(Display *display, System *system, Network *network, const char *title) {
  instance = this;
  _display = display;
  _system = system;
  _network = network;
  this->ui_app(title);
  this->ui_loading();
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

  _bodyScreen = lv_obj_create(_display->ui_second_screen());
  lv_obj_clear_flag(_bodyScreen, LV_OBJ_FLAG_SCROLLABLE);
  lv_obj_set_style_bg_color(_bodyScreen, lv_color_hex(0x000000), LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_bg_opa(_bodyScreen, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_width(_bodyScreen, 320);
  lv_obj_set_height(_bodyScreen, 240);
  lv_obj_set_style_border_opa(_bodyScreen, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_border_width(_bodyScreen, 0, LV_PART_MAIN | LV_STATE_DEFAULT);

  ui_AppPanel = lv_obj_create(_bodyScreen);
  lv_obj_set_width(ui_AppPanel, 320);
  lv_obj_set_height(ui_AppPanel, 210);
  lv_obj_align(ui_AppPanel, LV_ALIGN_BOTTOM_MID, 0, 10);
  lv_obj_clear_flag(ui_AppPanel, LV_OBJ_FLAG_SCROLLABLE);

  LV_FONT_DECLARE(ui_font_MontBold14);
  ui_AppTitle = lv_label_create(_bodyScreen);
  lv_obj_set_width(ui_AppTitle, LV_SIZE_CONTENT);   /// 1
  lv_obj_set_height(ui_AppTitle, LV_SIZE_CONTENT);  /// 1
  lv_obj_set_x(ui_AppTitle, 0);
  lv_obj_set_y(ui_AppTitle, -7);
  lv_obj_set_align(ui_AppTitle, LV_ALIGN_TOP_MID);
  lv_label_set_text(ui_AppTitle, title);
  lv_obj_set_style_text_color(ui_AppTitle, lv_color_hex(0xFFFFFF), LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_text_opa(ui_AppTitle, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_text_font(ui_AppTitle, &ui_font_MontBold14, LV_PART_MAIN | LV_STATE_DEFAULT);

  ui_AppCloseBtn = lv_imgbtn_create(_bodyScreen);
  LV_IMG_DECLARE(btn_close);
  lv_imgbtn_set_src(ui_AppCloseBtn, LV_IMGBTN_STATE_RELEASED, NULL, &btn_close, NULL);
  lv_obj_set_width(ui_AppCloseBtn, 24);
  lv_obj_set_height(ui_AppCloseBtn, 24);
  lv_obj_set_x(ui_AppCloseBtn, -7);
  lv_obj_set_y(ui_AppCloseBtn, -10);
  lv_obj_set_align(ui_AppCloseBtn, LV_ALIGN_TOP_RIGHT);
  lv_obj_add_event_cb(ui_AppCloseBtn, base_event_handler_thunk, LV_EVENT_CLICKED, NULL);
}

void AppBase::ui_loading() {
  ui_Loading = lv_obj_create(_bodyScreen);

  lv_obj_set_size(ui_Loading, 120, 140);
  lv_obj_t *loading_spinner = lv_spinner_create(ui_Loading, 1000, 60);
  lv_obj_set_size(loading_spinner, 80, 80);
  lv_obj_align(loading_spinner, LV_ALIGN_TOP_MID, 0, 0);

  lv_obj_t *loading_label = lv_label_create(ui_Loading);
  lv_label_set_text(loading_label, "Loading...");
  lv_obj_align(loading_label, LV_ALIGN_BOTTOM_MID, 0, 0);
  lv_obj_center(ui_Loading);
  this->show_loading_popup(false);
}

void AppBase::show_loading_popup(bool isOn) {
  if (isOn) {
    lv_obj_clear_flag(ui_Loading, LV_OBJ_FLAG_HIDDEN);
  } else {
    lv_obj_add_flag(ui_Loading, LV_OBJ_FLAG_HIDDEN);
  }
}