/////////////////////////////////////////////////////////////////
/*
  Settings Screen construction
*/
/////////////////////////////////////////////////////////////////
#include "UI/ESP32Berry_Display.hpp"
#include "../../Configurations/secrets.h"

extern "C" void ui_event_callback_thunk(lv_event_t *e);

void Display::ui_settings()
{
  LV_IMG_DECLARE(img_background);
  LV_IMG_DECLARE(icon_brightness);
  LV_IMG_DECLARE(icon_speaker);
  LV_IMG_DECLARE(icon_wifi);
  LV_IMG_DECLARE(icon_chatgpt);

  ui_ControlPanel = lv_obj_create(ui_Main_Screen);
  lv_obj_set_height(ui_ControlPanel, lv_pct(80));
  lv_obj_set_width(ui_ControlPanel, lv_pct(80));
  lv_obj_set_x(ui_ControlPanel, 0);
  lv_obj_set_y(ui_ControlPanel, lv_pct(10));
  lv_obj_set_align(ui_ControlPanel, LV_ALIGN_TOP_MID);
  lv_obj_add_flag(ui_ControlPanel, LV_OBJ_FLAG_HIDDEN);
  lv_obj_clear_flag(ui_ControlPanel, LV_OBJ_FLAG_SCROLLABLE);
  lv_obj_set_style_bg_color(ui_ControlPanel, lv_color_hex(0xFFFFFF), LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_bg_opa(ui_ControlPanel, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_border_width(ui_ControlPanel, 0, LV_PART_MAIN | LV_STATE_DEFAULT);

  lv_obj_t *ui_LabelScreenTimeout = lv_label_create(ui_ControlPanel);
  lv_obj_set_width(ui_LabelScreenTimeout, LV_SIZE_CONTENT);
  lv_obj_set_height(ui_LabelScreenTimeout, LV_SIZE_CONTENT);
  lv_obj_set_x(ui_LabelScreenTimeout, 0);
  lv_obj_set_y(ui_LabelScreenTimeout, -4);
  lv_label_set_text(ui_LabelScreenTimeout, "Screen Timeout (min):");
  lv_obj_set_style_text_color(ui_LabelScreenTimeout, lv_color_hex(0x3D3D3D), LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_text_opa(ui_LabelScreenTimeout, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_text_font(ui_LabelScreenTimeout, &lv_font_montserrat_12, LV_PART_MAIN | LV_STATE_DEFAULT);

  ui_LabelScreenTimeoutValue = lv_label_create(ui_ControlPanel);
  lv_obj_set_width(ui_LabelScreenTimeoutValue, LV_SIZE_CONTENT);
  lv_obj_set_height(ui_LabelScreenTimeoutValue, LV_SIZE_CONTENT);
  lv_obj_set_x(ui_LabelScreenTimeoutValue, 140);
  lv_obj_set_y(ui_LabelScreenTimeoutValue, -4);
  lv_label_set_text(ui_LabelScreenTimeoutValue, "5");
  lv_obj_set_style_text_color(ui_LabelScreenTimeoutValue, lv_color_hex(0x3D3D3D), LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_text_opa(ui_LabelScreenTimeoutValue, 255, LV_PART_MAIN | LV_STATE_DEFAULT);

  ui_ScreenTimeout = lv_slider_create(ui_ControlPanel);
  lv_slider_set_range(ui_ScreenTimeout, 1, 60);
  lv_obj_set_width(ui_ScreenTimeout, 212);
  lv_obj_set_height(ui_ScreenTimeout, 8);
  lv_obj_set_x(ui_ScreenTimeout, 10);
  lv_obj_set_y(ui_ScreenTimeout, 22);
  lv_obj_set_style_bg_color(ui_ScreenTimeout, lv_color_hex(0x989898), LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_bg_opa(ui_ScreenTimeout, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_bg_color(ui_ScreenTimeout, lv_color_hex(0xE95622), LV_PART_INDICATOR | LV_STATE_DEFAULT);
  lv_obj_set_style_bg_opa(ui_ScreenTimeout, 255, LV_PART_INDICATOR | LV_STATE_DEFAULT);
  lv_obj_set_style_bg_color(ui_ScreenTimeout, lv_color_hex(0xFFFFFF), LV_PART_KNOB | LV_STATE_DEFAULT);
  lv_obj_set_style_bg_opa(ui_ScreenTimeout, 255, LV_PART_KNOB | LV_STATE_DEFAULT);
  lv_obj_set_style_border_color(ui_ScreenTimeout, lv_color_hex(0x000000), LV_PART_KNOB | LV_STATE_DEFAULT);
  lv_obj_set_style_border_opa(ui_ScreenTimeout, 255, LV_PART_KNOB | LV_STATE_DEFAULT);
  lv_obj_set_style_border_width(ui_ScreenTimeout, 1, LV_PART_KNOB | LV_STATE_DEFAULT);
  lv_slider_set_value(ui_ScreenTimeout, 1, LV_ANIM_OFF);
  lv_obj_add_event_cb(ui_ScreenTimeout, ui_event_callback_thunk, LV_EVENT_VALUE_CHANGED, NULL);

  lv_obj_t *ui_IconBrightness = lv_img_create(ui_ControlPanel);
  lv_img_set_src(ui_IconBrightness, &icon_brightness);
  lv_obj_set_width(ui_IconBrightness, LV_SIZE_CONTENT);
  lv_obj_set_height(ui_IconBrightness, LV_SIZE_CONTENT);
  lv_obj_set_x(ui_IconBrightness, 0);
  lv_obj_set_y(ui_IconBrightness, 40);
  lv_obj_add_flag(ui_IconBrightness, LV_OBJ_FLAG_ADV_HITTEST);
  lv_obj_clear_flag(ui_IconBrightness, LV_OBJ_FLAG_SCROLLABLE);

  ui_SliderBrightness = lv_slider_create(ui_ControlPanel);
  lv_slider_set_range(ui_SliderBrightness, 10, 255);
  lv_obj_set_width(ui_SliderBrightness, 190);
  lv_obj_set_height(ui_SliderBrightness, 8);
  lv_obj_set_x(ui_SliderBrightness, 32);
  lv_obj_set_y(ui_SliderBrightness, 44);
  lv_obj_set_style_bg_color(ui_SliderBrightness, lv_color_hex(0x989898), LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_bg_opa(ui_SliderBrightness, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_bg_color(ui_SliderBrightness, lv_color_hex(0xE95622), LV_PART_INDICATOR | LV_STATE_DEFAULT);
  lv_obj_set_style_bg_opa(ui_SliderBrightness, 255, LV_PART_INDICATOR | LV_STATE_DEFAULT);
  lv_obj_set_style_bg_color(ui_SliderBrightness, lv_color_hex(0xFFFFFF), LV_PART_KNOB | LV_STATE_DEFAULT);
  lv_obj_set_style_bg_opa(ui_SliderBrightness, 255, LV_PART_KNOB | LV_STATE_DEFAULT);
  lv_obj_set_style_border_color(ui_SliderBrightness, lv_color_hex(0x000000), LV_PART_KNOB | LV_STATE_DEFAULT);
  lv_obj_set_style_border_opa(ui_SliderBrightness, 255, LV_PART_KNOB | LV_STATE_DEFAULT);
  lv_obj_set_style_border_width(ui_SliderBrightness, 1, LV_PART_KNOB | LV_STATE_DEFAULT);
  lv_slider_set_value(ui_SliderBrightness, 255, LV_ANIM_OFF);
  lv_obj_add_event_cb(ui_SliderBrightness, ui_event_callback_thunk, LV_EVENT_VALUE_CHANGED, NULL);

  lv_obj_t *ui_IconSpeaker = lv_img_create(ui_ControlPanel);
  lv_img_set_src(ui_IconSpeaker, &icon_speaker);
  lv_obj_set_width(ui_IconSpeaker, LV_SIZE_CONTENT);
  lv_obj_set_height(ui_IconSpeaker, LV_SIZE_CONTENT);
  lv_obj_set_x(ui_IconSpeaker, 0);
  lv_obj_set_y(ui_IconSpeaker, 60);
  lv_obj_add_flag(ui_IconSpeaker, LV_OBJ_FLAG_ADV_HITTEST);
  lv_obj_clear_flag(ui_IconSpeaker, LV_OBJ_FLAG_SCROLLABLE);

  ui_SliderSpeaker = lv_slider_create(ui_ControlPanel);
  lv_slider_set_range(ui_SliderSpeaker, 0, 21);
  lv_obj_set_width(ui_SliderSpeaker, 190);
  lv_obj_set_height(ui_SliderSpeaker, 8);
  lv_obj_set_x(ui_SliderSpeaker, 32);
  lv_obj_set_y(ui_SliderSpeaker, 64);
  lv_obj_set_style_bg_color(ui_SliderSpeaker, lv_color_hex(0x989898), LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_bg_opa(ui_SliderSpeaker, 255, LV_PART_MAIN | LV_STATE_DEFAULT);

  lv_obj_set_style_bg_color(ui_SliderSpeaker, lv_color_hex(0xE95622), LV_PART_INDICATOR | LV_STATE_DEFAULT);
  lv_obj_set_style_bg_opa(ui_SliderSpeaker, 255, LV_PART_INDICATOR | LV_STATE_DEFAULT);

  lv_obj_set_style_bg_color(ui_SliderSpeaker, lv_color_hex(0xFFFFFF), LV_PART_KNOB | LV_STATE_DEFAULT);
  lv_obj_set_style_bg_opa(ui_SliderSpeaker, 255, LV_PART_KNOB | LV_STATE_DEFAULT);
  lv_obj_set_style_border_color(ui_SliderSpeaker, lv_color_hex(0x000000), LV_PART_KNOB | LV_STATE_DEFAULT);
  lv_obj_set_style_border_opa(ui_SliderSpeaker, 255, LV_PART_KNOB | LV_STATE_DEFAULT);
  lv_obj_set_style_border_width(ui_SliderSpeaker, 1, LV_PART_KNOB | LV_STATE_DEFAULT);
  lv_slider_set_value(ui_SliderSpeaker, 21, LV_ANIM_OFF);
  lv_obj_add_event_cb(ui_SliderSpeaker, ui_event_callback_thunk, LV_EVENT_VALUE_CHANGED, NULL);

  lv_obj_t *ui_PanelWifi = lv_obj_create(ui_ControlPanel);
  lv_obj_set_width(ui_PanelWifi, 100);
  lv_obj_set_height(ui_PanelWifi, 30);
  lv_obj_set_align(ui_PanelWifi, LV_ALIGN_BOTTOM_LEFT);
  lv_obj_clear_flag(ui_PanelWifi, LV_OBJ_FLAG_SCROLLABLE);

  ui_BtnWiFi = lv_btn_create(ui_PanelWifi);
  lv_obj_set_width(ui_BtnWiFi, 60);
  lv_obj_set_height(ui_BtnWiFi, 50);
  lv_obj_set_x(ui_BtnWiFi, 16);
  lv_obj_set_y(ui_BtnWiFi, 0);
  lv_obj_set_align(ui_BtnWiFi, LV_ALIGN_LEFT_MID);
  lv_obj_add_flag(ui_BtnWiFi, LV_OBJ_FLAG_SCROLL_ON_FOCUS);
  lv_obj_clear_flag(ui_BtnWiFi, LV_OBJ_FLAG_SCROLLABLE);
  lv_obj_set_style_bg_color(ui_BtnWiFi, lv_color_hex(0xFFFFFF), LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_bg_opa(ui_BtnWiFi, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_add_event_cb(ui_BtnWiFi, ui_event_callback_thunk, LV_EVENT_CLICKED, NULL);

  lv_obj_t *ui_Label2 = lv_label_create(ui_BtnWiFi);
  lv_obj_set_width(ui_Label2, 60);
  lv_obj_set_height(ui_Label2, LV_SIZE_CONTENT);
  lv_obj_set_x(ui_Label2, -10);
  lv_obj_set_y(ui_Label2, 0);
  lv_obj_set_align(ui_Label2, LV_ALIGN_LEFT_MID);
  lv_label_set_text(ui_Label2, "WiFi");
  lv_obj_set_style_text_color(ui_Label2, lv_color_hex(0x3D3D3D), LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_text_opa(ui_Label2, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_text_font(ui_Label2, &lv_font_montserrat_12, LV_PART_MAIN | LV_STATE_DEFAULT);

  ui_ImgBtnWiFi = lv_imgbtn_create(ui_PanelWifi);
  lv_imgbtn_set_src(ui_ImgBtnWiFi, LV_IMGBTN_STATE_RELEASED, NULL, &icon_wifi, NULL);
  lv_imgbtn_set_src(ui_ImgBtnWiFi, LV_IMGBTN_STATE_PRESSED, NULL, &icon_wifi, NULL);
  lv_imgbtn_set_src(ui_ImgBtnWiFi, LV_IMGBTN_STATE_DISABLED, NULL, &icon_wifi, NULL);
  lv_imgbtn_set_src(ui_ImgBtnWiFi, LV_IMGBTN_STATE_CHECKED_PRESSED, NULL, &icon_wifi, NULL);
  lv_imgbtn_set_src(ui_ImgBtnWiFi, LV_IMGBTN_STATE_CHECKED_RELEASED, NULL, &icon_wifi, NULL);
  lv_imgbtn_set_src(ui_ImgBtnWiFi, LV_IMGBTN_STATE_CHECKED_DISABLED, NULL, &icon_wifi, NULL);
  lv_obj_set_width(ui_ImgBtnWiFi, 20);
  lv_obj_set_height(ui_ImgBtnWiFi, 20);
  lv_obj_set_x(ui_ImgBtnWiFi, -10);
  lv_obj_set_y(ui_ImgBtnWiFi, 0);
  lv_obj_set_align(ui_ImgBtnWiFi, LV_ALIGN_LEFT_MID);
  lv_obj_add_flag(ui_ImgBtnWiFi, LV_OBJ_FLAG_CHECKABLE);
  lv_obj_set_style_radius(ui_ImgBtnWiFi, 8, LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_bg_color(ui_ImgBtnWiFi, lv_color_hex(0xE95622), LV_PART_MAIN | LV_STATE_CHECKED);
  lv_obj_set_style_bg_opa(ui_ImgBtnWiFi, 255, LV_PART_MAIN | LV_STATE_CHECKED);
  lv_obj_add_event_cb(ui_ImgBtnWiFi, ui_event_callback_thunk, LV_EVENT_CLICKED, NULL);
  lv_obj_add_event_cb(ui_TopPanel, ui_event_callback_thunk, LV_EVENT_CLICKED, NULL);

  ui_PanelCursor = lv_obj_create(ui_ControlPanel);
  lv_obj_set_width(ui_PanelCursor, 30);
  lv_obj_set_height(ui_PanelCursor, 30);
  lv_obj_set_align(ui_PanelCursor, LV_ALIGN_BOTTOM_RIGHT);
  lv_obj_clear_flag(ui_PanelCursor, LV_OBJ_FLAG_SCROLLABLE);
  // lv_obj_set_style_bg_color(ui_PanelCursor, lv_color_hex(0xE95622), LV_PART_MAIN | LV_STATE_DEFAULT); // #E95622

  ui_ImgBtnCursor = lv_imgbtn_create(ui_PanelCursor);
  lv_imgbtn_set_src(ui_ImgBtnCursor, LV_IMGBTN_STATE_RELEASED, NULL, &mouse_cursor_icon, NULL);
  lv_imgbtn_set_src(ui_ImgBtnCursor, LV_IMGBTN_STATE_PRESSED, NULL, &mouse_cursor_icon, NULL);
  lv_imgbtn_set_src(ui_ImgBtnCursor, LV_IMGBTN_STATE_DISABLED, NULL, &mouse_cursor_icon, NULL);
  lv_imgbtn_set_src(ui_ImgBtnCursor, LV_IMGBTN_STATE_CHECKED_PRESSED, NULL, &mouse_cursor_icon, NULL);
  lv_imgbtn_set_src(ui_ImgBtnCursor, LV_IMGBTN_STATE_CHECKED_RELEASED, NULL, &mouse_cursor_icon, NULL);
  lv_imgbtn_set_src(ui_ImgBtnCursor, LV_IMGBTN_STATE_CHECKED_DISABLED, NULL, &mouse_cursor_icon, NULL);
  lv_obj_set_width(ui_ImgBtnCursor, 14);
  lv_obj_set_height(ui_ImgBtnCursor, 20);
  lv_obj_clear_flag(ui_ImgBtnCursor, LV_OBJ_FLAG_SCROLLABLE);
  lv_obj_set_align(ui_ImgBtnCursor, LV_ALIGN_CENTER);
  lv_obj_add_flag(ui_ImgBtnCursor, LV_OBJ_FLAG_CHECKABLE);
  lv_obj_set_style_radius(ui_ImgBtnCursor, 8, LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_bg_opa(ui_ImgBtnCursor, 255, LV_PART_MAIN | LV_STATE_CHECKED);
  lv_obj_add_event_cb(ui_ImgBtnCursor, ui_event_callback_thunk, LV_EVENT_CLICKED, NULL);

  ui_WiFi_page();
}