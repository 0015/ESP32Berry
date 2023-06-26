/////////////////////////////////////////////////////////////////
/*
  ESP32Berry, "ESP-NOW Chat App" Version 0.3
  For More Information: https://youtu.be/UhIXAp2wqjg
  Created by Eric N. (ThatProject)
*/
/////////////////////////////////////////////////////////////////
#include "ESP32Berry_Display.h"

static Display *instance = NULL;
static lv_disp_draw_buf_t draw_buf;
static lv_color_t buf[DISPLAY_WIDTH * 10];

Display::Display(FuncPtrInt callback) {
  instance = this;
  tft = new LGFX();
  app_event_cb = callback;
  uiTimer = 0;
  focusedObj = NULL;
  this->init_tft();
}

Display::~Display() {
  delete tft;
}

void Display::init_tft() {
  tft->begin();
  tft->setRotation(3);
  tft->fillScreen(TFT_BLACK);
#ifdef ESP32BERRY
  uint16_t calData[] = { 239, 3926, 233, 265, 3856, 3896, 3714, 308 };
  tft->setTouchCalibrate(calData);
#endif
  this->init_lvgl();
  this->init_keypad();
}

#ifdef ESP32BERRY
void callbackFromKeyPad(char key) {
  if (instance->focused_obj() == NULL) return;

  if (key == 8) {
    lv_textarea_del_char(instance->focused_obj());
  } else if (key == 1) {
    lv_textarea_cursor_left(instance->focused_obj());
  } else if (key == 2) {
    lv_textarea_cursor_down(instance->focused_obj());
  } else if (key == 3) {
    lv_textarea_cursor_up(instance->focused_obj());
  } else if (key == 4) {
    lv_textarea_cursor_right(instance->focused_obj());
  } else {
    lv_textarea_add_char(instance->focused_obj(), key);
  }
}
#endif

void Display::init_keypad() {
#ifdef ESP32BERRY
  void (*ptr)(char) = &callbackFromKeyPad;
  keypad = new KeyPad(ptr);
#else
  kb = lv_keyboard_create(uiRoot);
  lv_obj_add_flag(kb, LV_OBJ_FLAG_HIDDEN);
#endif
}

void Display::my_disp_flush(lv_disp_drv_t *disp, const lv_area_t *area, lv_color_t *color_p) {
  uint32_t w = (area->x2 - area->x1 + 1);
  uint32_t h = (area->y2 - area->y1 + 1);

  tft->startWrite();
  tft->setAddrWindow(area->x1, area->y1, w, h);
  tft->writePixels((lgfx::rgb565_t *)&color_p->full, w * h);
  tft->endWrite();

  lv_disp_flush_ready(disp);
}

void Display::my_touch_read(lv_indev_drv_t *indev_driver, lv_indev_data_t *data) {
  uint16_t touchX, touchY;
  bool touched = tft->getTouch(&touchX, &touchY);
  if (!touched) {
    data->state = LV_INDEV_STATE_REL;
  } else {
    data->state = LV_INDEV_STATE_PR;
    data->point.x = touchX;
    data->point.y = touchY;
  }
}

void Display::btn_event_cb(lv_event_t *e) {
  lv_event_code_t code = lv_event_get_code(e);
  lv_obj_t *btn = lv_event_get_target(e);
  if (code == LV_EVENT_CLICKED) {
    if (btn == popupBoxCloseBtn) {
      lv_obj_add_flag(popupBox, LV_OBJ_FLAG_HIDDEN);
    } else if (btn == timeDataBtn) {
      if (lv_obj_has_flag(calendar, LV_OBJ_FLAG_HIDDEN)) {
        lv_obj_clear_flag(calendar, LV_OBJ_FLAG_HIDDEN);
      } else {
        lv_obj_add_flag(calendar, LV_OBJ_FLAG_HIDDEN);
      }
    } else if (btn == appNoteBtn) {
      app_event_cb(APP_Note, NULL);
    } else if (btn == appTelegramBtn) {
      app_event_cb(APP_Telegram, NULL);
    }else if (btn == appESPNOWBtn) {
      app_event_cb(APP_ESPNOW, NULL);
    }
  }
}

void Display::textarea_event_cb(lv_event_t *e) {
  lv_event_code_t code = lv_event_get_code(e);
  lv_obj_t *obj = lv_event_get_target(e);

  if (code == LV_EVENT_FOCUSED) {
    focusedObj = obj;
#ifndef ESP32BERRY
    lv_keyboard_set_textarea(kb, obj);
    lv_obj_clear_flag(kb, LV_OBJ_FLAG_HIDDEN);
    lv_obj_move_foreground(kb);
#endif

  } else if (code == LV_EVENT_DEFOCUSED) {
    focusedObj = NULL;

#ifndef ESP32BERRY
    lv_keyboard_set_textarea(kb, NULL);
    lv_obj_add_flag(kb, LV_OBJ_FLAG_HIDDEN);
#endif
  }
}


extern "C" void my_disp_flush_thunk(lv_disp_drv_t *drv, const lv_area_t *area, lv_color_t *color_p) {
  instance->my_disp_flush(drv, area, color_p);
}

extern "C" void my_touch_read_thunk(lv_indev_drv_t *indev_driver, lv_indev_data_t *data) {
  instance->my_touch_read(indev_driver, data);
}

extern "C" void btn_event_cb_thunk(lv_event_t *e) {
  instance->btn_event_cb(e);
}

extern "C" void textarea_event_cb_thunk(lv_event_t *e) {
  instance->textarea_event_cb(e);
}

extern "C" void anim_event_cb_thunk(void *var, int32_t v) {
  instance->anim_x_cb(var, v);
}

void Display::anim_x_cb(void *var, int32_t v) {
  lv_obj_set_x((lv_obj_t *)var, v);
}

void update_ui_task(void *pvParameters) {
  while (1) {
    if (xSemaphoreTake(instance->bin_sem, portMAX_DELAY) == pdTRUE) {
      lv_timer_handler();
#ifdef ESP32BERRY
      instance->keypad->checkKeyInput();
#endif
      xSemaphoreGive(instance->bin_sem);
    }
    vTaskDelay(5);
  }
}

void Display::init_lvgl() {
  lv_init();
  lv_disp_draw_buf_init(&draw_buf, buf, NULL, DISPLAY_WIDTH * 10);

  static lv_disp_drv_t disp_drv;
  lv_disp_drv_init(&disp_drv);

  disp_drv.hor_res = DISPLAY_WIDTH;
  disp_drv.ver_res = DISPLAY_HEIGHT;
  disp_drv.flush_cb = my_disp_flush_thunk;
  disp_drv.draw_buf = &draw_buf;
  lv_disp_drv_register(&disp_drv);

  static lv_indev_drv_t indev_drv;
  lv_indev_drv_init(&indev_drv);
  indev_drv.type = LV_INDEV_TYPE_POINTER;
  indev_drv.read_cb = my_touch_read_thunk;
  lv_indev_drv_register(&indev_drv);

  lv_disp_t *disp = lv_disp_get_default();
  lv_theme_t *th = lv_theme_default_init(disp,
                                         lv_color_hex(0xffa500), lv_color_hex(0xa9a9a9),
                                         false,
                                         &lv_font_montserrat_14);

  lv_disp_set_theme(disp, th);


  this->ui_style();
  this->ui_main();
  this->ui_apps();
  this->ui_calendar();
  this->ui_popup_box();
  this->ui_noti();

  bin_sem = xSemaphoreCreateMutex();

  xTaskCreatePinnedToCore(update_ui_task,
                          "update_ui_task",
                          10000,
                          NULL,
                          10,
                          NULL,
                          0);
}

void Display::ui_style() {
  lv_style_init(&titleStyle);
  lv_style_set_text_font(&titleStyle, &lv_font_montserrat_20);

  lv_style_init(&borderStyle);
  lv_style_set_border_width(&borderStyle, 0);

  lv_style_init(&notiStyle);
  lv_style_set_text_font(&notiStyle, &lv_font_montserrat_20);
  lv_style_set_border_width(&notiStyle, 2);

  lv_style_set_radius(&notiStyle, 8);
  lv_style_set_bg_color(&notiStyle, lv_color_black());
  lv_style_set_text_color(&notiStyle, lv_color_white());
  lv_style_set_outline_width(&notiStyle, 2);
  lv_style_set_outline_color(&notiStyle, lv_palette_main(LV_PALETTE_ORANGE));
  lv_style_set_outline_pad(&notiStyle, 8);
}

void Display::ui_main() {
  uiRoot = lv_scr_act();
  lv_obj_clear_flag(uiRoot, LV_OBJ_FLAG_SCROLLABLE);

  lv_obj_t *statusBar = lv_obj_create(uiRoot);
  lv_obj_set_size(statusBar, tft->width() - 50, 40);
  lv_obj_align(statusBar, LV_ALIGN_TOP_RIGHT, 0, 0);
  lv_obj_clear_flag(statusBar, LV_OBJ_FLAG_SCROLLABLE);


  batLabel = lv_label_create(statusBar);
  lv_obj_remove_style_all(batLabel);
  lv_label_set_text(batLabel, "100% " LV_SYMBOL_BATTERY_FULL);
  lv_obj_set_style_text_align(batLabel, LV_TEXT_ALIGN_CENTER, 0);
  lv_obj_align(batLabel, LV_ALIGN_RIGHT_MID, 0, 0);

  networkIcon = lv_label_create(statusBar);
  lv_obj_remove_style_all(networkIcon);
  lv_label_set_text(networkIcon, LV_SYMBOL_WARNING);
  lv_obj_set_style_text_align(networkIcon, LV_TEXT_ALIGN_CENTER, 0);
  lv_obj_align_to(networkIcon, batLabel, LV_ALIGN_OUT_LEFT_MID, -10, 0);

  timeDataBtn = lv_btn_create(statusBar);
  lv_obj_remove_style_all(timeDataBtn);
  lv_obj_set_size(timeDataBtn, 100, 40);
  lv_obj_align_to(timeDataBtn, networkIcon, LV_ALIGN_OUT_LEFT_MID, -10, 0);
  lv_obj_add_event_cb(timeDataBtn, btn_event_cb_thunk, LV_EVENT_CLICKED, NULL);
  lv_obj_clear_flag(timeDataBtn, LV_OBJ_FLAG_CLICKABLE);

  timeDataLabel = lv_label_create(timeDataBtn);
  lv_obj_remove_style_all(timeDataLabel);
  lv_obj_set_size(timeDataLabel, 90, 30);
  lv_label_set_text(timeDataLabel, "");
  lv_obj_set_style_text_align(timeDataLabel, LV_TEXT_ALIGN_RIGHT, 0);
  lv_obj_center(timeDataLabel);

  bodyScreen = lv_obj_create(uiRoot);
  lv_obj_add_style(bodyScreen, &borderStyle, 0);
  lv_obj_set_size(bodyScreen, tft->width(), tft->height() - 40);
  lv_obj_align(bodyScreen, LV_ALIGN_BOTTOM_MID, 0, 0);
  lv_obj_clear_flag(bodyScreen, LV_OBJ_FLAG_SCROLLABLE);

  LV_IMG_DECLARE(ESP32Berry_BG);
  lv_obj_t *bg = lv_img_create(bodyScreen);
  lv_obj_remove_style_all(bg);
  lv_img_set_src(bg, &ESP32Berry_BG);
  lv_obj_center(bg);
  lv_obj_clear_flag(bodyScreen, LV_OBJ_FLAG_SCROLLABLE);
}

void Display::ui_calendar() {
  calendar = lv_calendar_create(bodyScreen);
  lv_obj_set_size(calendar, tft->height() - 50, tft->height() - 50);
  lv_obj_align(calendar, LV_ALIGN_TOP_RIGHT, 10, -10);
  lv_obj_add_flag(calendar, LV_OBJ_FLAG_HIDDEN);
}

void Display::ui_apps() {
  lv_obj_t *icon_frame = lv_obj_create(bodyScreen);
  lv_obj_set_size(icon_frame, 80, 80);
  lv_obj_align(icon_frame, LV_ALIGN_TOP_LEFT, 12, 8);
  lv_obj_add_style(icon_frame, &borderStyle, 0);
  lv_obj_clear_flag(icon_frame, LV_OBJ_FLAG_SCROLLABLE);

  LV_IMG_DECLARE(ESP32Berry_Icon_Note);
  appNoteBtn = lv_imgbtn_create(icon_frame);
  lv_obj_set_size(appNoteBtn, 64, 64);
  lv_imgbtn_set_src(appNoteBtn, LV_IMGBTN_STATE_RELEASED, &ESP32Berry_Icon_Note, NULL, NULL);
  lv_obj_align(appNoteBtn, LV_ALIGN_CENTER, 0, -8);
  lv_obj_add_event_cb(appNoteBtn, btn_event_cb_thunk, LV_EVENT_CLICKED, NULL);

  lv_obj_t *appTitle = lv_label_create(icon_frame);
  lv_label_set_text(appTitle, "Note");
  lv_obj_align(appTitle, LV_ALIGN_BOTTOM_MID, 0, 12);

  lv_obj_t *icon_frame2 = lv_obj_create(bodyScreen);
  lv_obj_set_size(icon_frame2, 80, 80);
  lv_obj_align_to(icon_frame2, icon_frame, LV_ALIGN_OUT_RIGHT_MID, 36, 0);
  lv_obj_add_style(icon_frame2, &borderStyle, 0);
  lv_obj_clear_flag(icon_frame2, LV_OBJ_FLAG_SCROLLABLE);

  LV_IMG_DECLARE(ESP32Berry_Icon_Telegram);
  appTelegramBtn = lv_imgbtn_create(icon_frame2);
  lv_obj_set_size(appTelegramBtn, 64, 64);
  lv_imgbtn_set_src(appTelegramBtn, LV_IMGBTN_STATE_RELEASED, &ESP32Berry_Icon_Telegram, NULL, NULL);
  lv_obj_align(appTelegramBtn, LV_ALIGN_CENTER, 0, -8);
  lv_obj_add_event_cb(appTelegramBtn, btn_event_cb_thunk, LV_EVENT_CLICKED, NULL);

  appTitle = lv_label_create(icon_frame2);
  lv_label_set_text(appTitle, "Telegram");
  lv_obj_align(appTitle, LV_ALIGN_BOTTOM_MID, 0, 12);

  lv_obj_t *icon_frame3 = lv_obj_create(bodyScreen);
  lv_obj_set_size(icon_frame3, 80, 80);
  lv_obj_align_to(icon_frame3, icon_frame2, LV_ALIGN_OUT_RIGHT_MID, 36, 0);
  lv_obj_add_style(icon_frame3, &borderStyle, 0);
  lv_obj_clear_flag(icon_frame3, LV_OBJ_FLAG_SCROLLABLE);

  LV_IMG_DECLARE(ESP32Berry_Icon_ESPNow);
  appESPNOWBtn = lv_imgbtn_create(icon_frame3);
  lv_obj_set_size(appESPNOWBtn, 64, 64);
  lv_imgbtn_set_src(appESPNOWBtn, LV_IMGBTN_STATE_RELEASED, &ESP32Berry_Icon_ESPNow, NULL, NULL);
  lv_obj_align(appESPNOWBtn, LV_ALIGN_CENTER, 0, -8);
  lv_obj_add_event_cb(appESPNOWBtn, btn_event_cb_thunk, LV_EVENT_CLICKED, NULL);

  appTitle = lv_label_create(icon_frame3);
  lv_label_set_text(appTitle, "ESP-NOW");
  lv_obj_align(appTitle, LV_ALIGN_BOTTOM_MID, 0, 12);
}

void Display::ui_popup_box() {
  popupBox = lv_obj_create(uiRoot);
  lv_obj_set_size(popupBox, tft->width() * 2 / 3, tft->height() / 2);
  lv_obj_center(popupBox);
  lv_obj_add_flag(popupBox, LV_OBJ_FLAG_HIDDEN);
}

void Display::ui_popup_open(String title, String msg) {
  lv_obj_clear_flag(popupBox, LV_OBJ_FLAG_HIDDEN);
  lv_obj_clean(popupBox);

  LV_IMG_DECLARE(ESP32Berry_Icon);
  lv_obj_t *logo = lv_img_create(popupBox);
  lv_img_set_src(logo, &ESP32Berry_Icon);
  lv_obj_set_size(logo, 40, 40);
  lv_obj_align(logo, LV_ALIGN_TOP_RIGHT, -8, 4);

  lv_obj_t *popupTitle = lv_label_create(popupBox);
  lv_obj_add_style(popupTitle, &titleStyle, 0);
  lv_label_set_text(popupTitle, title.c_str());
  lv_obj_set_width(popupTitle, tft->width() * 2 / 3 - 50);
  lv_obj_align(popupTitle, LV_ALIGN_TOP_LEFT, 0, 0);

  lv_obj_t *popupMSG = lv_label_create(popupBox);
  lv_obj_set_width(popupMSG, tft->width() * 2 / 3 - 50);
  lv_label_set_text(popupMSG, msg.c_str());
  lv_obj_align(popupMSG, LV_ALIGN_TOP_LEFT, 0, 40);

  popupBoxCloseBtn = lv_btn_create(popupBox);
  lv_obj_add_event_cb(popupBoxCloseBtn, btn_event_cb_thunk, LV_EVENT_ALL, NULL);
  lv_obj_align(popupBoxCloseBtn, LV_ALIGN_BOTTOM_RIGHT, 0, 0);
  lv_obj_t *btnLabel = lv_label_create(popupBoxCloseBtn);
  lv_label_set_text(btnLabel, "OK");
  lv_obj_center(btnLabel);
}

void Display::show_loading_popup(bool isOn) {
  if (isOn) {
    popupLoading = lv_obj_create(uiRoot);
    lv_obj_set_size(popupLoading, 120, 140);
    lv_obj_t *loading_spinner = lv_spinner_create(popupLoading, 1000, 60);
    lv_obj_set_size(loading_spinner, 80, 80);
    lv_obj_align(loading_spinner, LV_ALIGN_TOP_MID, 0, 0);

    lv_obj_t *loading_label = lv_label_create(popupLoading);
    lv_label_set_text(loading_label, "Loading...");
    lv_obj_align(loading_label, LV_ALIGN_BOTTOM_MID, 0, 0);
    lv_obj_center(popupLoading);
  } else {

    if (popupLoading != NULL) {
      xSemaphoreTake(bin_sem, portMAX_DELAY);
      lv_obj_del(popupLoading);
      xSemaphoreGive(bin_sem);
      popupLoading = NULL;
    }
  }
}

void Display::set_wifi_icon(bool isConnected) {
  lv_label_set_text(networkIcon, isConnected ? LV_SYMBOL_WIFI : LV_SYMBOL_MINUS);
}

void Display::ui_noti() {
  notiBtn = lv_btn_create(lv_scr_act());
  lv_obj_set_size(notiBtn, DISPLAY_WIDTH / 2, 60);
  lv_obj_align(notiBtn, LV_ALIGN_OUT_TOP_RIGHT, 20, 50);
  lv_obj_add_style(notiBtn, &notiStyle, 0);
  notiLabel = lv_label_create(notiBtn);
  lv_label_set_long_mode(notiLabel, LV_LABEL_LONG_WRAP);
  lv_label_set_text(notiLabel, "Welcome Back!");
  lv_obj_set_size(notiLabel, DISPLAY_WIDTH / 2, 60);
  lv_obj_align(notiLabel, LV_ALIGN_LEFT_MID, 2, 4);
  ui_noti_anim();
}

void Display::ui_noti_anim() {
  lv_obj_move_foreground(notiBtn);
  lv_anim_t a;
  lv_anim_init(&a);
  lv_anim_set_var(&a, notiBtn);
  lv_anim_set_values(&a, 20, 50);
  lv_anim_set_time(&a, 800);
  lv_anim_set_playback_delay(&a, 1200);
  lv_anim_set_playback_time(&a, 300);
  lv_anim_set_path_cb(&a, lv_anim_path_ease_in_out);
  lv_anim_set_exec_cb(&a, anim_event_cb_thunk);
  lv_anim_set_values(&a, DISPLAY_WIDTH + 50, DISPLAY_WIDTH / 2);
  lv_anim_start(&a);
}

void Display::launch_noti(String msg) {
  lv_label_set_text(notiLabel, msg.c_str());
  ui_noti_anim();
}

void Display::update_time(String time) {
  lv_label_set_text(timeDataLabel, time.c_str());

  if (!lv_obj_has_flag(timeDataBtn, LV_OBJ_FLAG_CLICKABLE)) {
    lv_obj_add_flag(timeDataBtn, LV_OBJ_FLAG_CLICKABLE);
  }

  int year = time.substring(time.length() - 2, time.length()).toInt() + 2000;
  int day = time.substring(time.length() - 5, time.length() - 3).toInt();
  int month = time.substring(time.length() - 8, time.length() - 6).toInt();

  lv_calendar_set_today_date(calendar, year, month, day);
  lv_calendar_set_showed_date(calendar, year, month);
}

void Display::update_battery(String info) {
  int delimiterIdx = info.indexOf(",");
  if (delimiterIdx < 1) {
    return;
  }

  String _percent = info.substring(delimiterIdx + 1, info.length());
  xSemaphoreTake(bin_sem, portMAX_DELAY);
  _percent += "% ";
  _percent += add_battery_icon(_percent.toInt());
  lv_label_set_text(batLabel, _percent.c_str());
  xSemaphoreGive(bin_sem);
}

String Display::add_battery_icon(int percentage) {
  if (percentage >= 90) {
    return String(LV_SYMBOL_BATTERY_FULL);
  } else if (percentage >= 65 && percentage < 90) {
    return String(LV_SYMBOL_BATTERY_3);
  } else if (percentage >= 40 && percentage < 65) {
    return String(LV_SYMBOL_BATTERY_2);
  } else if (percentage >= 15 && percentage < 40) {
    return String(LV_SYMBOL_BATTERY_1);
  } else {
    return String(LV_SYMBOL_BATTERY_EMPTY);
  }
}

lv_obj_t *Display::focused_obj() {
  return focusedObj;
}

void Display::set_focused_obj(lv_obj_t *obj) {
  focusedObj = obj;
}

lv_obj_t *Display::get_ui_root() {
  return uiRoot;
}

lv_obj_t *Display::get_body_screen() {
  return bodyScreen;
}

SemaphoreHandle_t Display::get_mutex() {
  return bin_sem;
}

LGFX * Display::get_lgfx() {
  return tft;
}