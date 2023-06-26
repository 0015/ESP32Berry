/////////////////////////////////////////////////////////////////
/*
  ESP32Berry, "WiFi connection & Simple Note App" Version 0.1
  For More Information: https://youtu.be/wqaxCAcghtk
  Created by Eric N. (ThatProject)
*/
/////////////////////////////////////////////////////////////////

#include "ESP32Berry_Display.h"

static Display *instance = NULL;
static lv_disp_draw_buf_t draw_buf;
static lv_color_t buf[DISPLAY_WIDTH * 10];
static lv_color_t buf2[DISPLAY_WIDTH * 10];

Display::Display(FuncPtrInt callback) {
  instance = this;
  tft = new LGFX();
  menu_event_cb = callback;
  uiTimer = 0;
  focusedObj = NULL;
}

Display::~Display() {
  delete tft;
}

void Display::initTFT() {
  tft->begin();
  tft->setRotation(3);
  tft->fillScreen(TFT_BLACK);

  uint16_t calData[] = { 239, 3926, 233, 265, 3856, 3896, 3714, 308 };
  tft->setTouchCalibrate(calData);
  this->initLVGL();
  this->initKeyPad();
}

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

void Display::initKeyPad() {
  void (*ptr)(char) = &callbackFromKeyPad;
  keypad = new KeyPad(ptr);
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
    if (btn == settingCloseBtn) {
      lv_obj_add_flag(settingPage, LV_OBJ_FLAG_HIDDEN);
    } else if (btn == logoBtn) {
      if (lv_obj_has_flag(menuList, LV_OBJ_FLAG_HIDDEN)) {
        lv_obj_clear_flag(menuList, LV_OBJ_FLAG_HIDDEN);
      } else {
        lv_obj_add_flag(menuList, LV_OBJ_FLAG_HIDDEN);
      }

    } else if (btn == popupBoxCloseBtn) {
      lv_obj_add_flag(popupBox, LV_OBJ_FLAG_HIDDEN);
    } else if (btn == mboxConnectBtn) {
      lv_obj_move_background(mboxConnect);
      lv_obj_add_flag(mboxConnect, LV_OBJ_FLAG_HIDDEN);
      lv_obj_add_flag(menuList, LV_OBJ_FLAG_HIDDEN);
      lv_obj_add_flag(settingPage, LV_OBJ_FLAG_HIDDEN);

      char *key = new char[strlen(lv_label_get_text(mboxTitle)) + strlen(lv_textarea_get_text(mboxPassword)) + 3];
      strcpy(key, lv_label_get_text(mboxTitle));
      strcat(key, WIFI_SSID_PW_DELIMITER);
      strcat(key, lv_textarea_get_text(mboxPassword));
      menu_event_cb(WIFI_ON, key);
      delete[] key;
      this->show_loading_popup(true);
      lv_textarea_set_text(mboxPassword, "");

    } else if (btn == mboxCloseBtn) {
      lv_obj_move_background(mboxConnect);
      lv_obj_add_flag(mboxConnect, LV_OBJ_FLAG_HIDDEN);
    } else if (btn == timeDataBtn) {
      if (lv_obj_has_flag(calendar, LV_OBJ_FLAG_HIDDEN)) {
        lv_obj_clear_flag(calendar, LV_OBJ_FLAG_HIDDEN);
      } else {
        lv_obj_add_flag(calendar, LV_OBJ_FLAG_HIDDEN);
      }

    } else if (btn == appNoteBtn) {
      menu_event_cb(APP_Note, NULL);
    } else {
      lv_obj_add_flag(menuList, LV_OBJ_FLAG_HIDDEN);
      switch (lv_obj_get_child_id(btn)) {
        case 0:
          this->ui_popup_open("ESP32Berry", "ESP32 Lolin32 Lite\nQWERTY keyboard\nILI9488 3.5inch\n\nVersion 0.1 by ThatProject");
          break;
        case 2:
          lv_obj_clear_flag(settingPage, LV_OBJ_FLAG_HIDDEN);
          break;

        case 5:
          //SDCARD
          break;
      }
    }
  } else if (code == LV_EVENT_VALUE_CHANGED) {
    if (btn == settingWiFiSwitch) {
      if (lv_obj_has_state(btn, LV_STATE_CHECKED)) {
        menu_event_cb(WIFI_ON, NULL);
      } else {
        menu_event_cb(WIFI_OFF, NULL);
        lv_obj_clean(wfList);
      }
    }
  }
}

void Display::wifi_event_cb(lv_event_t *e) {
  lv_event_code_t code = lv_event_get_code(e);
  lv_obj_t *obj = lv_event_get_target(e);

  if (code == LV_EVENT_CLICKED) {
    int length = strlen(lv_list_get_btn_text(wfList, obj));
    char ssidName[length - 7];
    strncpy(ssidName, lv_list_get_btn_text(wfList, obj), length - 8);
    ssidName[length - 8] = '\0';
    lv_label_set_text(mboxTitle, ssidName);
    lv_obj_move_foreground(mboxConnect);
    lv_obj_clear_flag(mboxConnect, LV_OBJ_FLAG_HIDDEN);
  }
}

void Display::textarea_event_cb(lv_event_t *e) {
  lv_event_code_t code = lv_event_get_code(e);
  lv_obj_t *obj = lv_event_get_target(e);

  if (code == LV_EVENT_FOCUSED) {
    focusedObj = obj;
  } else if (code == LV_EVENT_DEFOCUSED) {
    focusedObj = NULL;
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

extern "C" void wifi_event_cb_thunk(lv_event_t *e) {
  instance->wifi_event_cb(e);
}

extern "C" void textarea_event_cb_thunk(lv_event_t *e) {
  instance->textarea_event_cb(e);
}

void update_ui_task(void *pvParameters) {
  while (1) {
    if (xSemaphoreTake(instance->bin_sem, portMAX_DELAY) == pdTRUE) {
      lv_timer_handler();
      instance->keypad->checkKeyInput();
      xSemaphoreGive(instance->bin_sem);
    }
    vTaskDelay(5);
  }
}

void Display::initLVGL() {
  lv_init();
  lv_disp_draw_buf_init(&draw_buf, buf, buf2, DISPLAY_WIDTH * 10);

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
  this->ui_menu();
  this->ui_setting();
  this->ui_popup_box();
  this->ui_wifi_conenct_box();
  this->ui_loading();

  bin_sem = xSemaphoreCreateMutex();

  xTaskCreatePinnedToCore(update_ui_task,
                          "update_ui_task",
                          10000,
                          NULL,
                          1,
                          NULL,
                          0);
}

void Display::ui_style() {
  lv_style_init(&title_style);
  lv_style_set_text_font(&title_style, &lv_font_montserrat_24);

  lv_style_init(&border_style);
  lv_style_set_border_width(&border_style, 0);
}

void Display::ui_main() {
  lv_obj_t *statusBar = lv_obj_create(lv_scr_act());
  lv_obj_set_size(statusBar, tft->width() - 50, 40);
  lv_obj_align(statusBar, LV_ALIGN_TOP_RIGHT, 0, 0);
  lv_obj_clear_flag(statusBar, LV_OBJ_FLAG_SCROLLABLE);


  statusLabel = lv_label_create(statusBar);
  lv_obj_set_size(statusLabel, tft->width() - 70, 40);
  lv_label_set_text(statusLabel, "WiFi Not Connected!    " LV_SYMBOL_CLOSE);
  lv_obj_align(statusLabel, LV_ALIGN_LEFT_MID, 0, 10);

  timeDataLabel = lv_label_create(statusBar);
  lv_obj_add_style(timeDataLabel, &border_style, 0);
  lv_obj_set_size(timeDataLabel, 100, 40);
  lv_label_set_text(timeDataLabel, "");
  lv_obj_set_style_text_align(timeDataLabel, LV_TEXT_ALIGN_RIGHT, 0);
  lv_obj_align(timeDataLabel, LV_ALIGN_RIGHT_MID, -30, 3);

  timeDataBtn = lv_btn_create(statusBar);
  lv_obj_set_size(timeDataBtn, 30, 30);
  lv_obj_align(timeDataBtn, LV_ALIGN_RIGHT_MID, 14, 0);
  lv_obj_add_event_cb(timeDataBtn, btn_event_cb_thunk, LV_EVENT_CLICKED, NULL);
  lv_obj_t *label = lv_label_create(timeDataBtn);
  lv_label_set_text(label, LV_SYMBOL_EJECT);
  lv_obj_center(label);
  lv_obj_clear_flag(timeDataBtn, LV_OBJ_FLAG_CLICKABLE);


  bodyScreen = lv_obj_create(lv_scr_act());
  lv_obj_add_style(bodyScreen, &border_style, 0);
  lv_obj_set_size(bodyScreen, tft->width(), tft->height() - 40);
  lv_obj_align(bodyScreen, LV_ALIGN_BOTTOM_MID, 0, 0);

  LV_IMG_DECLARE(ESP32Berry_Icon);
  logoBtn = lv_imgbtn_create(lv_scr_act());
  lv_obj_set_size(logoBtn, 40, 40);
  lv_imgbtn_set_src(logoBtn, LV_IMGBTN_STATE_RELEASED, &ESP32Berry_Icon, NULL, NULL);
  lv_obj_align(logoBtn, LV_ALIGN_TOP_LEFT, 4, 0);
  lv_obj_add_event_cb(logoBtn, btn_event_cb_thunk, LV_EVENT_ALL, NULL);

  LV_IMG_DECLARE(ESP32Berry_BG);
  lv_obj_t *title = lv_img_create(bodyScreen);
  lv_obj_add_style(title, &border_style, 0);
  lv_img_set_src(title, &ESP32Berry_BG);
  lv_obj_center(title);
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
  lv_obj_add_style(icon_frame, &border_style, 0);
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
}

void Display::ui_menu() {
  menuList = lv_list_create(lv_scr_act());
  lv_obj_set_size(menuList, 200, 240);
  lv_obj_align(menuList, LV_ALIGN_TOP_LEFT, 0, 40);

  lv_obj_t *menuListBtn = lv_list_add_btn(menuList, LV_SYMBOL_HOME, "About This");
  lv_obj_add_event_cb(menuListBtn, btn_event_cb_thunk, LV_EVENT_CLICKED, NULL);

  lv_list_add_text(menuList, "");
  menuListBtn = lv_list_add_btn(menuList, LV_SYMBOL_WIFI, "Wi-Fi");
  lv_obj_add_event_cb(menuListBtn, btn_event_cb_thunk, LV_EVENT_CLICKED, NULL);
  menuListBtn = lv_list_add_btn(menuList, LV_SYMBOL_BATTERY_3, "Battery");
  lv_obj_add_event_cb(menuListBtn, btn_event_cb_thunk, LV_EVENT_CLICKED, NULL);
  lv_list_add_text(menuList, "");
  menuListBtn = lv_list_add_btn(menuList, LV_SYMBOL_DRIVE, "SD CARD");
  lv_obj_add_event_cb(menuListBtn, btn_event_cb_thunk, LV_EVENT_CLICKED, NULL);
  lv_list_add_text(menuList, "");
  menuListBtn = lv_list_add_btn(menuList, LV_SYMBOL_POWER, "Restart...");
  lv_obj_add_event_cb(menuListBtn, btn_event_cb_thunk, LV_EVENT_CLICKED, NULL);

  lv_obj_add_flag(menuList, LV_OBJ_FLAG_HIDDEN);
}

void Display::ui_setting() {
  settingPage = lv_obj_create(lv_scr_act());
  lv_obj_set_size(settingPage, tft->width() - 100, tft->height() - 40);
  lv_obj_align(settingPage, LV_ALIGN_TOP_RIGHT, -20, 20);

  settinglabel = lv_label_create(settingPage);
  lv_label_set_text(settinglabel, "Wi-Fi " LV_SYMBOL_SETTINGS);
  lv_obj_align(settinglabel, LV_ALIGN_TOP_LEFT, 0, 0);

  settingCloseBtn = lv_btn_create(settingPage);
  lv_obj_set_size(settingCloseBtn, 30, 30);
  lv_obj_align(settingCloseBtn, LV_ALIGN_TOP_RIGHT, 0, -10);
  lv_obj_add_event_cb(settingCloseBtn, btn_event_cb_thunk, LV_EVENT_CLICKED, NULL);
  lv_obj_t *btnSymbol = lv_label_create(settingCloseBtn);
  lv_label_set_text(btnSymbol, LV_SYMBOL_CLOSE);
  lv_obj_center(btnSymbol);

  settingWiFiSwitch = lv_switch_create(settingPage);
  lv_obj_add_event_cb(settingWiFiSwitch, btn_event_cb_thunk, LV_EVENT_ALL, NULL);
  lv_obj_align_to(settingWiFiSwitch, settinglabel, LV_ALIGN_TOP_RIGHT, 60, -10);

  wfList = lv_list_create(settingPage);
  lv_obj_set_size(wfList, tft->width() - 140, 210);
  lv_obj_align_to(wfList, settinglabel, LV_ALIGN_TOP_LEFT, 0, 30);

  lv_obj_add_flag(settingPage, LV_OBJ_FLAG_HIDDEN);
}

void Display::ui_popup_box() {
  popupBox = lv_obj_create(lv_scr_act());
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
  lv_obj_add_style(popupTitle, &title_style, 0);
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

void Display::ui_wifi_conenct_box() {
  mboxConnect = lv_obj_create(lv_scr_act());
  lv_obj_set_size(mboxConnect, tft->width() * 2 / 3, tft->height() / 2);
  lv_obj_center(mboxConnect);

  mboxTitle = lv_label_create(mboxConnect);
  lv_label_set_text(mboxTitle, "Selected WiFi SSID: ");
  lv_obj_set_size(mboxTitle, tft->width() * 2 / 3 - 40, 40);
  lv_obj_align(mboxTitle, LV_ALIGN_TOP_MID, 0, 0);

  mboxPassword = lv_textarea_create(mboxConnect);
  lv_obj_set_size(mboxPassword, tft->width() * 2 / 3 - 40, 40);
  lv_obj_align_to(mboxPassword, mboxTitle, LV_ALIGN_TOP_MID, 0, 40);
  lv_textarea_set_placeholder_text(mboxPassword, "Password?");
  lv_obj_add_event_cb(mboxPassword, textarea_event_cb_thunk, LV_EVENT_FOCUSED, NULL);
  lv_obj_add_event_cb(mboxPassword, textarea_event_cb_thunk, LV_EVENT_DEFOCUSED, NULL);

  mboxConnectBtn = lv_btn_create(mboxConnect);
  lv_obj_add_event_cb(mboxConnectBtn, btn_event_cb_thunk, LV_EVENT_ALL, NULL);
  lv_obj_align(mboxConnectBtn, LV_ALIGN_BOTTOM_RIGHT, 0, 0);
  lv_obj_t *btnLabel = lv_label_create(mboxConnectBtn);
  lv_label_set_text(btnLabel, "Connect");
  lv_obj_center(btnLabel);

  mboxCloseBtn = lv_btn_create(mboxConnect);
  lv_obj_add_event_cb(mboxCloseBtn, btn_event_cb_thunk, LV_EVENT_ALL, NULL);
  lv_obj_align(mboxCloseBtn, LV_ALIGN_BOTTOM_LEFT, 0, 0);
  lv_obj_t *btnLabel2 = lv_label_create(mboxCloseBtn);
  lv_label_set_text(btnLabel2, "Cancel");
  lv_obj_center(btnLabel2);

  lv_obj_move_background(mboxConnect);
  lv_obj_add_flag(mboxConnect, LV_OBJ_FLAG_HIDDEN);
}

void Display::update_ui_network(std::vector<String> newWifiList) {
  xSemaphoreTake(bin_sem, portMAX_DELAY);
  if (!lv_obj_has_flag(mboxConnect, LV_OBJ_FLAG_HIDDEN)) {
    xSemaphoreGive(bin_sem);
    return;
  }

  lv_obj_clear_flag(wfList, LV_OBJ_FLAG_CLICKABLE);
  lv_obj_clear_flag(wfList, LV_OBJ_FLAG_SCROLLABLE);
  lv_obj_clean(wfList);

  lv_list_add_text(wfList, newWifiList.size() > 1 ? "WiFi: Found Networks" : "WiFi: Not Found!");
  for (std::vector<String>::iterator item = newWifiList.begin(); item != newWifiList.end(); ++item) {

    lv_obj_t *btn = lv_list_add_btn(wfList, LV_SYMBOL_WIFI, (*item).c_str());
    lv_obj_add_event_cb(btn, wifi_event_cb_thunk, LV_EVENT_CLICKED, NULL);
  }
  xSemaphoreGive(bin_sem);

  lv_obj_add_flag(wfList, LV_OBJ_FLAG_CLICKABLE);
  lv_obj_add_flag(wfList, LV_OBJ_FLAG_SCROLLABLE);
}

void Display::ui_loading() {
  popupLoading = lv_obj_create(lv_scr_act());
  lv_obj_set_size(popupLoading, 120, 140);
  lv_obj_t *loading_spinner = lv_spinner_create(popupLoading, 1000, 60);
  lv_obj_set_size(loading_spinner, 80, 80);
  lv_obj_align(loading_spinner, LV_ALIGN_TOP_MID, 0, 0);

  lv_obj_t *loading_label = lv_label_create(popupLoading);
  lv_label_set_text(loading_label, "Loading...");
  lv_obj_align(loading_label, LV_ALIGN_BOTTOM_MID, 0, 0);
  lv_obj_center(popupLoading);
  this->show_loading_popup(false);
}

void Display::show_loading_popup(bool isOn) {
  if (isOn) {
    lv_obj_move_foreground(popupLoading);
    lv_obj_clear_flag(popupLoading, LV_OBJ_FLAG_HIDDEN);
  } else {
    lv_obj_move_background(popupLoading);
    lv_obj_add_flag(popupLoading, LV_OBJ_FLAG_HIDDEN);
  }
}

void Display::update_status_bar(std::vector<String> eventLog) {

  lv_label_set_text(statusLabel, eventLog.back().c_str());
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


lv_obj_t *Display::focused_obj() {
  return focusedObj;
}

void Display::set_focused_obj(lv_obj_t *obj) {
  focusedObj = obj;
}

lv_obj_t *Display::getBodyScreen() {
  return bodyScreen;
}