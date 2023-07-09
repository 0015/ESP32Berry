/////////////////////////////////////////////////////////////////
/*
  New ESP32Berry Project, The base UI & ChatGPT Client
  For More Information: https://youtu.be/5K6rSw9j5iY
  Created by Eric N. (ThatProject)
*/
/////////////////////////////////////////////////////////////////
#include "ESP32Berry_Display.hpp"

static Display *instance = NULL;

Display::Display(FuncPtrInt callback) {
  instance = this;
  tft = new LGFX();
  menu_event_cb = callback;
  ui_Focused_Obj = NULL;
  initTFT();
}

Display::~Display() {
  delete tft;
}

void Display::initTFT() {
  //Mouse Pin setup
  pinMode(BOARD_BOOT_PIN, INPUT_PULLUP);
  pinMode(BOARD_TBOX_G02, INPUT_PULLUP);
  pinMode(BOARD_TBOX_G01, INPUT_PULLUP);
  pinMode(BOARD_TBOX_G04, INPUT_PULLUP);
  pinMode(BOARD_TBOX_G03, INPUT_PULLUP);

  tft->begin();
  tft->setRotation(1);
  tft->fillScreen(TFT_BLACK);
  this->initLVGL();
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
  uint16_t x, y;
  if (tft->getTouch(&x, &y)) {
    data->state = LV_INDEV_STATE_PR;
    data->point.x = x;
    data->point.y = y;
  } else {
    data->state = LV_INDEV_STATE_REL;
  }
}

void Display::my_mouse_read(lv_indev_drv_t *indev_driver, lv_indev_data_t *data) {
  static int16_t last_x;
  static int16_t last_y;
  bool left_button_down = false;
  const uint8_t dir_pins[5] = { BOARD_TBOX_G02,
                                BOARD_TBOX_G01,
                                BOARD_TBOX_G04,
                                BOARD_TBOX_G03,
                                BOARD_BOOT_PIN };
  static bool last_dir[5];
  uint8_t pos = 10;
  for (int i = 0; i < 5; i++) {
    bool dir = digitalRead(dir_pins[i]);
    if (dir != last_dir[i]) {
      last_dir[i] = dir;
      switch (i) {
        case 0:
          if (last_x < (lv_disp_get_hor_res(NULL) - mouse_cursor_icon.header.w)) {
            last_x += pos;
          }
          break;
        case 1:
          if (last_y > mouse_cursor_icon.header.h) {
            last_y -= pos;
          }
          break;
        case 2:
          if (last_x > mouse_cursor_icon.header.w) {
            last_x -= pos;
          }
          break;
        case 3:
          if (last_y < (lv_disp_get_ver_res(NULL) - mouse_cursor_icon.header.h)) {
            last_y += pos;
          }
          break;
        case 4:
          left_button_down = true;
          break;
        default:
          break;
      }
    }
  }

  data->point.x = last_x;
  data->point.y = last_y;
  data->state = left_button_down ? LV_INDEV_STATE_PRESSED : LV_INDEV_STATE_RELEASED;
}

// Read key value from esp32c3
uint32_t Display::keypad_get_key(void) {
  char key_ch = 0;
  Wire.requestFrom(0x55, 1);
  while (Wire.available() > 0) {
    key_ch = Wire.read();
  }
  return key_ch;
}

/*Will be called by the library to read the mouse*/
void Display::my_key_read(lv_indev_drv_t *indev_drv, lv_indev_data_t *data) {
  static uint32_t last_key = 0;
  uint32_t act_key;
  act_key = keypad_get_key();
  if (act_key != 0) {
    data->state = LV_INDEV_STATE_PR;
    last_key = act_key;
  } else {
    data->state = LV_INDEV_STATE_REL;
  }
  data->key = last_key;
}

void Display::ui_event_callback(lv_event_t *e) {
  lv_event_code_t event_code = lv_event_get_code(e);
  lv_obj_t *target = lv_event_get_target(e);
  if (target == ui_BasePopupCloseBtn && event_code == LV_EVENT_CLICKED) {
    lv_obj_add_flag(ui_BasePopup, LV_OBJ_FLAG_HIDDEN);
  } else if (target == ui_TopPanel && event_code == LV_EVENT_CLICKED) {
    if (lv_obj_has_flag(ui_ControlPanel, LV_OBJ_FLAG_HIDDEN)) lv_obj_clear_flag(ui_ControlPanel, LV_OBJ_FLAG_HIDDEN);
    else lv_obj_add_flag(ui_ControlPanel, LV_OBJ_FLAG_HIDDEN);
  } else if (target == ui_SliderBrightness && event_code == LV_EVENT_VALUE_CHANGED) {
    int sliderValue = lv_slider_get_value(ui_SliderBrightness);
    tft->setBrightness(sliderValue);
  } else if (target == ui_SliderSpeaker && event_code == LV_EVENT_VALUE_CHANGED) {
    int sliderValue = lv_slider_get_value(ui_SliderSpeaker);

  } else if (target == ui_ImgBtnWiFi && event_code == LV_EVENT_CLICKED) {
    if (lv_obj_get_state(ui_ImgBtnWiFi) & LV_STATE_CHECKED) {
      menu_event_cb(WIFI_ON, NULL);
    } else {
      menu_event_cb(WIFI_OFF, NULL);
      lv_obj_clean(ui_WiFiList);
    }
  } else if (target == ui_BtnWiFi && event_code == LV_EVENT_CLICKED) {
    lv_obj_clear_flag(ui_WiFiPanel, LV_OBJ_FLAG_HIDDEN);
    lv_obj_add_flag(ui_ControlPanel, LV_OBJ_FLAG_HIDDEN);
  }
}

void Display::ui_app_btns_callback(lv_event_t *e) {
  lv_event_code_t code = lv_event_get_code(e);
  lv_obj_t *btn = lv_event_get_target(e);
  if (code == LV_EVENT_CLICKED) {
    set_notification("");

    lv_obj_t *label = lv_obj_get_child(btn, 0);
    String appBtnLabel = lv_label_get_text(label);
    switch (appBtnLabel.toInt()) {
      case 0:
        lv_scr_load_anim(ui_Sub_Screen, LV_SCR_LOAD_ANIM_MOVE_LEFT, 100, 0, false);
        menu_event_cb(APP, lv_label_get_text(label));
        break;
      default:
        break;
    }
  }
}

void Display::ui_wifi_event_callback(lv_event_t *e) {
  lv_event_code_t code = lv_event_get_code(e);
  lv_obj_t *btn = lv_event_get_target(e);
  if (code == LV_EVENT_CLICKED) {
    if (btn == ui_WiFiPanelCloseBtn) {
      lv_obj_add_flag(ui_WiFiPanel, LV_OBJ_FLAG_HIDDEN);
    } else if (btn == ui_WiFiMBoxConnectBtn) {
      lv_obj_add_flag(ui_WiFiMBox, LV_OBJ_FLAG_HIDDEN);
      lv_obj_add_flag(ui_WiFiPanel, LV_OBJ_FLAG_HIDDEN);
      char *key = new char[strlen(lv_label_get_text(ui_WiFiMBoxTitle)) + strlen(lv_textarea_get_text(ui_WiFiMBoxPassword)) + 3];
      strcpy(key, lv_label_get_text(ui_WiFiMBoxTitle));
      strcat(key, WIFI_SSID_PW_DELIMITER);
      strcat(key, lv_textarea_get_text(ui_WiFiMBoxPassword));
      menu_event_cb(WIFI_ON, key);
      delete[] key;
      this->show_loading_popup(true);
      lv_textarea_set_text(ui_WiFiMBoxPassword, "");
    } else if (btn == ui_WiFiMBoxCloseBtn) {
      lv_obj_move_background(ui_WiFiMBox);
      lv_obj_add_flag(ui_WiFiMBox, LV_OBJ_FLAG_HIDDEN);
    } else {
      int length = strlen(lv_list_get_btn_text(ui_WiFiList, btn));
      char ssidName[length - 7];
      strncpy(ssidName, lv_list_get_btn_text(ui_WiFiList, btn), length - 8);
      ssidName[length - 8] = '\0';
      lv_label_set_text(ui_WiFiMBoxTitle, ssidName);
      lv_obj_move_foreground(ui_WiFiMBox);
      lv_obj_clear_flag(ui_WiFiMBox, LV_OBJ_FLAG_HIDDEN);
    }
  }
}

void Display::textarea_event_cb(lv_event_t *e) {
  lv_event_code_t code = lv_event_get_code(e);
  lv_obj_t *obj = lv_event_get_target(e);

  if (code == LV_EVENT_FOCUSED) {
    ui_Focused_Obj = obj;
  } else if (code == LV_EVENT_DEFOCUSED) {
    ui_Focused_Obj = NULL;
  }
}

extern "C" void my_disp_flush_thunk(lv_disp_drv_t *drv, const lv_area_t *area, lv_color_t *color_p) {
  instance->my_disp_flush(drv, area, color_p);
}

extern "C" void my_touch_read_thunk(lv_indev_drv_t *indev_driver, lv_indev_data_t *data) {
  instance->my_touch_read(indev_driver, data);
}

extern "C" void my_mouse_read_thunk(lv_indev_drv_t *indev_driver, lv_indev_data_t *data) {
  instance->my_mouse_read(indev_driver, data);
}

extern "C" void my_key_read_thunk(lv_indev_drv_t *indev_driver, lv_indev_data_t *data) {
  instance->my_key_read(indev_driver, data);
}

extern "C" void wifi_event_cb_thunk(lv_event_t *e) {
  instance->ui_wifi_event_callback(e);
}

extern "C" void textarea_event_cb_thunk(lv_event_t *e) {
  instance->textarea_event_cb(e);
}

extern "C" void ui_event_callback_thunk(lv_event_t *e) {
  instance->ui_event_callback(e);
}

extern "C" void ui_app_btns_callback_thunk(lv_event_t *e) {
  instance->ui_app_btns_callback(e);
}

void update_ui_task(void *pvParameters) {
  while (1) {
    xSemaphoreTake(instance->bin_sem, portMAX_DELAY);
    lv_task_handler();
    xSemaphoreGive(instance->bin_sem);
    vTaskDelay(5);
  }
}

void Display::initLVGL() {

  static lv_disp_draw_buf_t draw_buf;

#ifndef BOARD_HAS_PSRAM
#define LVGL_BUFFER_SIZE (TFT_HEIGHT * 100)
  static lv_color_t buf[LVGL_BUFFER_SIZE];
#else
#define LVGL_BUFFER_SIZE (TFT_WIDTH * TFT_HEIGHT * sizeof(lv_color_t))
  static lv_color_t *buf = (lv_color_t *)ps_malloc(LVGL_BUFFER_SIZE);
  if (!buf) {
    Serial.println("menory alloc failed!");
    delay(5000);
    assert(buf);
  }
#endif
  lv_init();
  lv_group_set_default(lv_group_create());
  lv_disp_draw_buf_init(&draw_buf, buf, NULL, LVGL_BUFFER_SIZE);

  static lv_disp_drv_t disp_drv;
  lv_disp_drv_init(&disp_drv);

  disp_drv.hor_res = TFT_HEIGHT;
  disp_drv.ver_res = TFT_WIDTH;
  disp_drv.flush_cb = my_disp_flush_thunk;
  disp_drv.draw_buf = &draw_buf;
#ifdef BOARD_HAS_PSRAM
  disp_drv.full_refresh = 1;
#endif
  lv_disp_drv_register(&disp_drv);

  static lv_indev_drv_t indev_drv;
  lv_indev_drv_init(&indev_drv);
  indev_drv.type = LV_INDEV_TYPE_POINTER;
  indev_drv.read_cb = my_touch_read_thunk;
  lv_indev_drv_register(&indev_drv);

  static lv_indev_drv_t indev_mouse;
  lv_indev_drv_init(&indev_mouse);
  indev_mouse.type = LV_INDEV_TYPE_POINTER;
  indev_mouse.read_cb = my_mouse_read_thunk;
  lv_indev_t *mouse_indev = lv_indev_drv_register(&indev_mouse);
  lv_indev_set_group(mouse_indev, lv_group_get_default());

  lv_obj_t *cursor_obj = lv_img_create(lv_scr_act()); /*Create an image object for the cursor */
  lv_img_set_src(cursor_obj, &mouse_cursor_icon);     /*Set the image source*/
  lv_indev_set_cursor(mouse_indev, cursor_obj);       /*Connect the image  object to the driver*/

  /*Register a keypad input device*/
  static lv_indev_drv_t indev_keypad;
  lv_indev_drv_init(&indev_keypad);
  indev_keypad.type = LV_INDEV_TYPE_KEYPAD;
  indev_keypad.read_cb = my_key_read_thunk;
  lv_indev_t *kb_indev = lv_indev_drv_register(&indev_keypad);
  lv_indev_set_group(kb_indev, lv_group_get_default());

  lv_disp_t *dispp = lv_disp_get_default();
  lv_theme_t *theme = lv_theme_default_init(dispp, lv_color_hex(0xE95622), lv_palette_main(LV_PALETTE_RED), false, &lv_font_montserrat_14);
  lv_disp_set_theme(dispp, theme);

  bin_sem = xSemaphoreCreateMutex();
  ui_main();
  ui_second();
  ui_prep_loading();
  ui_prep_popup_box();

  xTaskCreatePinnedToCore(update_ui_task,
                          "update_ui_task",
                          10000,
                          NULL,
                          1,
                          &lvgl_task_handle,
                          0);

  this->ui_popup_open("Welcome to ESP32Berry Project!", "This project aims to develop useful applications based on the T-Deck device. Let's do a fun project together!\n\n(Version 0.5)");
}


void Display::ui_second() {
  ui_Sub_Screen = lv_obj_create(NULL);

  lv_obj_clear_flag(ui_Sub_Screen, LV_OBJ_FLAG_SCROLLABLE);
  lv_obj_set_style_bg_color(ui_Sub_Screen, lv_color_hex(0x000000), LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_bg_opa(ui_Sub_Screen, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
}

void Display::ui_main() {

  LV_IMG_DECLARE(img_background);
  LV_IMG_DECLARE(icon_brightness);
  LV_IMG_DECLARE(icon_speaker);
  LV_IMG_DECLARE(icon_wifi);
  LV_IMG_DECLARE(icon_chatgpt);

  ui_Main_Screen = lv_obj_create(NULL);
  lv_obj_clear_flag(ui_Main_Screen, LV_OBJ_FLAG_SCROLLABLE);
  lv_obj_set_style_bg_img_src(ui_Main_Screen, &img_background, LV_PART_MAIN | LV_STATE_DEFAULT);

  ui_TopPanel = lv_obj_create(ui_Main_Screen);
  lv_obj_set_width(ui_TopPanel, 318);
  lv_obj_set_height(ui_TopPanel, 50);
  lv_obj_set_x(ui_TopPanel, 0);
  lv_obj_set_y(ui_TopPanel, 1);
  lv_obj_set_align(ui_TopPanel, LV_ALIGN_TOP_MID);
  lv_obj_clear_flag(ui_TopPanel, LV_OBJ_FLAG_SCROLLABLE);
  lv_obj_set_style_bg_color(ui_TopPanel, lv_color_hex(0x000000), LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_bg_opa(ui_TopPanel, 64, LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_border_color(ui_TopPanel, lv_color_hex(0x151515), LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_border_opa(ui_TopPanel, 0, LV_PART_MAIN | LV_STATE_DEFAULT);

  ui_TimeLabel = lv_label_create(ui_TopPanel);
  lv_obj_set_width(ui_TimeLabel, LV_SIZE_CONTENT);
  lv_obj_set_height(ui_TimeLabel, LV_SIZE_CONTENT);
  lv_obj_set_x(ui_TimeLabel, 0);
  lv_obj_set_y(ui_TimeLabel, 10);
  lv_obj_set_align(ui_TimeLabel, LV_ALIGN_BOTTOM_MID);
  lv_label_set_text(ui_TimeLabel, "ESP32Berry");
  lv_obj_set_style_text_color(ui_TimeLabel, lv_color_hex(0xFFFFFF), LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_text_opa(ui_TimeLabel, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_text_font(ui_TimeLabel, &lv_font_montserrat_20, LV_PART_MAIN | LV_STATE_DEFAULT);

  ui_Userlabel = lv_label_create(ui_TopPanel);
  lv_obj_set_width(ui_Userlabel, LV_SIZE_CONTENT);
  lv_obj_set_height(ui_Userlabel, LV_SIZE_CONTENT);
  lv_obj_set_x(ui_Userlabel, 0);
  lv_obj_set_y(ui_Userlabel, -14);
  lv_obj_set_align(ui_Userlabel, LV_ALIGN_TOP_MID);
  lv_label_set_text(ui_Userlabel, USER_NAME);
  lv_obj_set_style_text_color(ui_Userlabel, lv_color_hex(0xFFFFFF), LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_text_opa(ui_Userlabel, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_text_font(ui_Userlabel, &lv_font_montserrat_14, LV_PART_MAIN | LV_STATE_DEFAULT);

  ui_BatteryLabel = lv_label_create(ui_TopPanel);
  lv_obj_set_width(ui_BatteryLabel, LV_SIZE_CONTENT);
  lv_obj_set_height(ui_BatteryLabel, LV_SIZE_CONTENT);
  lv_obj_set_x(ui_BatteryLabel, 0);
  lv_obj_set_y(ui_BatteryLabel, -14);
  lv_label_set_text(ui_BatteryLabel, LV_SYMBOL_BATTERY_FULL);
  lv_obj_set_style_text_color(ui_BatteryLabel, lv_color_hex(0xFFFFFF), LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_text_opa(ui_BatteryLabel, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_text_font(ui_BatteryLabel, &lv_font_montserrat_14, LV_PART_MAIN | LV_STATE_DEFAULT);

  ui_WiFiLabel = lv_label_create(ui_TopPanel);
  lv_obj_set_width(ui_WiFiLabel, LV_SIZE_CONTENT);
  lv_obj_set_height(ui_WiFiLabel, LV_SIZE_CONTENT);
  lv_obj_set_x(ui_WiFiLabel, 0);
  lv_obj_set_y(ui_WiFiLabel, -14);
  lv_obj_set_align(ui_WiFiLabel, LV_ALIGN_TOP_RIGHT);
  lv_label_set_text(ui_WiFiLabel, LV_SYMBOL_WARNING);
  lv_obj_set_style_text_color(ui_WiFiLabel, lv_color_hex(0xFFFFFF), LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_text_opa(ui_WiFiLabel, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_text_font(ui_WiFiLabel, &lv_font_montserrat_14, LV_PART_MAIN | LV_STATE_DEFAULT);

  ui_DateLabel = lv_label_create(ui_TopPanel);
  lv_obj_set_width(ui_DateLabel, LV_SIZE_CONTENT);
  lv_obj_set_height(ui_DateLabel, LV_SIZE_CONTENT);
  lv_obj_set_x(ui_DateLabel, 0);
  lv_obj_set_y(ui_DateLabel, 4);
  lv_label_set_text(ui_DateLabel, "");
  lv_obj_set_style_text_color(ui_DateLabel, lv_color_hex(0xFFFFFF), LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_text_opa(ui_DateLabel, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_text_font(ui_DateLabel, &lv_font_montserrat_14, LV_PART_MAIN | LV_STATE_DEFAULT);

  ui_NotiLabel = lv_label_create(ui_TopPanel);
  lv_obj_set_width(ui_NotiLabel, 80);
  lv_obj_set_height(ui_NotiLabel, LV_SIZE_CONTENT);
  lv_obj_set_x(ui_NotiLabel, 0);
  lv_obj_set_y(ui_NotiLabel, 4);
  lv_obj_set_align(ui_NotiLabel, LV_ALIGN_TOP_RIGHT);
  lv_label_set_text(ui_NotiLabel, "...");
  lv_obj_set_style_text_color(ui_NotiLabel, lv_color_hex(0xFFFFFF), LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_text_opa(ui_NotiLabel, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_text_font(ui_NotiLabel, &lv_font_montserrat_12, LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_label_set_long_mode(ui_NotiLabel, LV_LABEL_LONG_SCROLL_CIRCULAR);

  lv_obj_t *ui_BodyPanel = lv_obj_create(ui_Main_Screen);
  lv_obj_set_width(ui_BodyPanel, 318);
  lv_obj_set_height(ui_BodyPanel, 120);
  lv_obj_set_x(ui_BodyPanel, 0);
  lv_obj_set_y(ui_BodyPanel, 10);
  lv_obj_set_align(ui_BodyPanel, LV_ALIGN_BOTTOM_MID);
  lv_obj_clear_flag(ui_BodyPanel, LV_OBJ_FLAG_SCROLLABLE);
  lv_obj_set_style_bg_color(ui_BodyPanel, lv_color_hex(0x000000), LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_bg_opa(ui_BodyPanel, 64, LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_border_color(ui_BodyPanel, lv_color_hex(0x151515), LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_border_opa(ui_BodyPanel, 0, LV_PART_MAIN | LV_STATE_DEFAULT);

  // App Icons
  lv_obj_t *cont_row = lv_obj_create(ui_BodyPanel);

  lv_obj_set_size(cont_row, 318, 100);
  lv_obj_align(cont_row, LV_ALIGN_TOP_MID, 0, -8);
  lv_obj_set_flex_flow(cont_row, LV_FLEX_FLOW_ROW);

  lv_obj_set_style_bg_opa(cont_row, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_border_opa(cont_row, 0, LV_PART_MAIN | LV_STATE_DEFAULT);


  for (int i = 0; i < 5; i++) {
    lv_obj_t *obj;
    lv_obj_t *label;
    lv_obj_t *ui_btn_icon;

    obj = lv_btn_create(cont_row);
    lv_obj_set_size(obj, 64, 64);
    lv_obj_add_flag(obj, LV_OBJ_FLAG_SCROLL_ON_FOCUS);
    lv_obj_clear_flag(obj, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_style_bg_color(obj, lv_color_hex(0xFFFFFF), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_color(obj, lv_color_hex(0xFFFFFF), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_opa(obj, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(obj, 2, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_width(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_spread(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);

    lv_obj_add_event_cb(obj, ui_app_btns_callback_thunk, LV_EVENT_CLICKED, NULL);

    label = lv_label_create(obj);
    lv_label_set_text_fmt(label, "%" LV_PRIu32 "", i);
    lv_obj_center(label);
    lv_obj_add_flag(label, LV_OBJ_FLAG_HIDDEN);

    if (i == 0) {
      ui_btn_icon = lv_img_create(obj);
      lv_img_set_src(ui_btn_icon, &icon_chatgpt);
      lv_obj_set_width(ui_btn_icon, LV_SIZE_CONTENT);
      lv_obj_set_height(ui_btn_icon, LV_SIZE_CONTENT);
      lv_obj_set_align(ui_btn_icon, LV_ALIGN_CENTER);
      lv_obj_add_flag(ui_btn_icon, LV_OBJ_FLAG_ADV_HITTEST);
      lv_obj_clear_flag(ui_btn_icon, LV_OBJ_FLAG_SCROLLABLE);
    }
  }

  ui_ControlPanel = lv_obj_create(ui_Main_Screen);
  lv_obj_set_height(ui_ControlPanel, 100);
  lv_obj_set_width(ui_ControlPanel, lv_pct(50));
  lv_obj_set_x(ui_ControlPanel, 0);
  lv_obj_set_y(ui_ControlPanel, 20);
  lv_obj_set_align(ui_ControlPanel, LV_ALIGN_TOP_MID);
  lv_obj_add_flag(ui_ControlPanel, LV_OBJ_FLAG_HIDDEN);
  lv_obj_clear_flag(ui_ControlPanel, LV_OBJ_FLAG_SCROLLABLE);
  lv_obj_set_style_bg_color(ui_ControlPanel, lv_color_hex(0xFFFFFF), LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_bg_opa(ui_ControlPanel, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_border_width(ui_ControlPanel, 0, LV_PART_MAIN | LV_STATE_DEFAULT);

  lv_obj_t *ui_IconSpeaker = lv_img_create(ui_ControlPanel);
  lv_img_set_src(ui_IconSpeaker, &icon_speaker);
  lv_obj_set_width(ui_IconSpeaker, LV_SIZE_CONTENT);
  lv_obj_set_height(ui_IconSpeaker, LV_SIZE_CONTENT);
  lv_obj_set_x(ui_IconSpeaker, 0);
  lv_obj_set_y(ui_IconSpeaker, -6);
  lv_obj_add_flag(ui_IconSpeaker, LV_OBJ_FLAG_ADV_HITTEST);
  lv_obj_clear_flag(ui_IconSpeaker, LV_OBJ_FLAG_SCROLLABLE);

  ui_SliderSpeaker = lv_slider_create(ui_ControlPanel);
  lv_slider_set_range(ui_SliderSpeaker, 0, 21);
  lv_obj_set_width(ui_SliderSpeaker, 100);
  lv_obj_set_height(ui_SliderSpeaker, 8);
  lv_obj_set_x(ui_SliderSpeaker, 32);
  lv_obj_set_y(ui_SliderSpeaker, -2);
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

  lv_obj_t *ui_IconSpeaker1 = lv_img_create(ui_ControlPanel);
  lv_img_set_src(ui_IconSpeaker1, &icon_brightness);
  lv_obj_set_width(ui_IconSpeaker1, LV_SIZE_CONTENT);
  lv_obj_set_height(ui_IconSpeaker1, LV_SIZE_CONTENT);
  lv_obj_set_x(ui_IconSpeaker1, 0);
  lv_obj_set_y(ui_IconSpeaker1, 16);
  lv_obj_add_flag(ui_IconSpeaker1, LV_OBJ_FLAG_ADV_HITTEST);
  lv_obj_clear_flag(ui_IconSpeaker1, LV_OBJ_FLAG_SCROLLABLE);

  ui_SliderBrightness = lv_slider_create(ui_ControlPanel);
  lv_slider_set_range(ui_SliderBrightness, 10, 255);
  lv_obj_set_width(ui_SliderBrightness, 100);
  lv_obj_set_height(ui_SliderBrightness, 8);
  lv_obj_set_x(ui_SliderBrightness, 32);
  lv_obj_set_y(ui_SliderBrightness, 20);
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

  ui_WiFi_page();
  lv_disp_load_scr(ui_Main_Screen);
}

void Display::ui_WiFi_page() {
  ui_WiFiPanel = lv_obj_create(ui_Main_Screen);
  lv_obj_set_size(ui_WiFiPanel, tft->width() - 40, tft->height() - 40);
  lv_obj_align(ui_WiFiPanel, LV_ALIGN_TOP_MID, 0, 20);

  lv_obj_t *ui_WiFiPanelLabel = lv_label_create(ui_WiFiPanel);
  lv_label_set_text(ui_WiFiPanelLabel, "Wi-Fi " LV_SYMBOL_SETTINGS);
  lv_obj_align(ui_WiFiPanelLabel, LV_ALIGN_TOP_LEFT, 0, 0);

  ui_WiFiPanelCloseBtn = lv_btn_create(ui_WiFiPanel);
  lv_obj_set_size(ui_WiFiPanelCloseBtn, 30, 30);
  lv_obj_align(ui_WiFiPanelCloseBtn, LV_ALIGN_TOP_RIGHT, 0, -10);
  lv_obj_set_style_bg_color(ui_WiFiPanelCloseBtn, lv_color_hex(0xFFFFFF), LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_bg_opa(ui_WiFiPanelCloseBtn, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_border_opa(ui_WiFiPanelCloseBtn, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_border_width(ui_WiFiPanelCloseBtn, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_shadow_width(ui_WiFiPanelCloseBtn, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_shadow_spread(ui_WiFiPanelCloseBtn, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_add_event_cb(ui_WiFiPanelCloseBtn, wifi_event_cb_thunk, LV_EVENT_CLICKED, NULL);

  lv_obj_t *ui_CloseBtnSymbol = lv_label_create(ui_WiFiPanelCloseBtn);
  lv_label_set_text(ui_CloseBtnSymbol, LV_SYMBOL_CLOSE);
  lv_obj_center(ui_CloseBtnSymbol);
  lv_obj_set_style_text_color(ui_CloseBtnSymbol, lv_color_hex(0x3D3D3D), LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_text_opa(ui_CloseBtnSymbol, 255, LV_PART_MAIN | LV_STATE_DEFAULT);

  ui_WiFiList = lv_list_create(ui_WiFiPanel);
  lv_obj_set_size(ui_WiFiList, tft->width() - 70, tft->height() - 90);
  lv_obj_align_to(ui_WiFiList, ui_WiFiPanelLabel, LV_ALIGN_TOP_LEFT, 0, 20);
  lv_obj_add_flag(ui_WiFiPanel, LV_OBJ_FLAG_HIDDEN);


  ui_WiFiMBox = lv_obj_create(ui_Main_Screen);
  lv_obj_set_size(ui_WiFiMBox, tft->width() - 40, tft->height() - 80);
  lv_obj_center(ui_WiFiMBox);

  lv_obj_t *mboxLabel = lv_label_create(ui_WiFiMBox);
  lv_label_set_text(mboxLabel, "Selected WiFi SSID");
  lv_obj_set_size(mboxLabel, tft->width() - 70, 40);
  lv_obj_align(mboxLabel, LV_ALIGN_TOP_LEFT, 0, 0);

  ui_WiFiMBoxTitle = lv_label_create(ui_WiFiMBox);
  lv_label_set_text(ui_WiFiMBoxTitle, "ThatProject");
  lv_obj_set_size(ui_WiFiMBoxTitle, tft->width() - 70, 40);
  lv_obj_align(ui_WiFiMBoxTitle, LV_ALIGN_TOP_LEFT, 0, 30);

  ui_WiFiMBoxPassword = lv_textarea_create(ui_WiFiMBox);
  lv_textarea_set_cursor_click_pos(ui_WiFiMBoxPassword, false);
  lv_textarea_set_cursor_pos(ui_WiFiMBoxPassword, 0);
  lv_textarea_set_text_selection(ui_WiFiMBoxPassword, false);
  lv_obj_set_size(ui_WiFiMBoxPassword, tft->width() - 70, 40);
  lv_obj_align_to(ui_WiFiMBoxPassword, ui_WiFiMBoxTitle, LV_ALIGN_TOP_LEFT, 0, 20);
  lv_textarea_set_placeholder_text(ui_WiFiMBoxPassword, "Password?");
  lv_textarea_set_max_length(ui_WiFiMBoxPassword, 64);
  lv_obj_add_event_cb(ui_WiFiMBoxPassword, textarea_event_cb_thunk, LV_EVENT_FOCUSED, NULL);
  lv_obj_add_event_cb(ui_WiFiMBoxPassword, textarea_event_cb_thunk, LV_EVENT_DEFOCUSED, NULL);

  ui_WiFiMBoxConnectBtn = lv_btn_create(ui_WiFiMBox);
  lv_obj_add_event_cb(ui_WiFiMBoxConnectBtn, wifi_event_cb_thunk, LV_EVENT_ALL, NULL);
  lv_obj_align(ui_WiFiMBoxConnectBtn, LV_ALIGN_BOTTOM_RIGHT, 0, 0);
  lv_obj_t *btnLabel = lv_label_create(ui_WiFiMBoxConnectBtn);
  lv_label_set_text(btnLabel, "Connect");
  lv_obj_center(btnLabel);

  ui_WiFiMBoxCloseBtn = lv_btn_create(ui_WiFiMBox);
  lv_obj_add_event_cb(ui_WiFiMBoxCloseBtn, wifi_event_cb_thunk, LV_EVENT_ALL, NULL);
  lv_obj_align(ui_WiFiMBoxCloseBtn, LV_ALIGN_BOTTOM_LEFT, 0, 0);
  lv_obj_t *btnLabel2 = lv_label_create(ui_WiFiMBoxCloseBtn);
  lv_label_set_text(btnLabel2, "Cancel");
  lv_obj_center(btnLabel2);

  lv_obj_add_flag(ui_WiFiMBox, LV_OBJ_FLAG_HIDDEN);
}

void Display::ui_prep_popup_box() {

  ui_BasePopup = lv_obj_create(lv_scr_act());
  lv_obj_set_height(ui_BasePopup, 170);
  lv_obj_set_width(ui_BasePopup, lv_pct(93));
  lv_obj_set_align(ui_BasePopup, LV_ALIGN_CENTER);
  lv_obj_clear_flag(ui_BasePopup, LV_OBJ_FLAG_SCROLLABLE);
  lv_obj_set_style_bg_color(ui_BasePopup, lv_color_hex(0x000000), LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_bg_opa(ui_BasePopup, 196, LV_PART_MAIN | LV_STATE_DEFAULT);

  ui_BasePopupTitle = lv_label_create(ui_BasePopup);
  lv_obj_set_width(ui_BasePopupTitle, LV_SIZE_CONTENT);
  lv_obj_set_height(ui_BasePopupTitle, LV_SIZE_CONTENT);
  lv_label_set_text(ui_BasePopupTitle, "");
  lv_obj_set_style_text_color(ui_BasePopupTitle, lv_color_hex(0xFFFFFF), LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_text_opa(ui_BasePopupTitle, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_text_font(ui_BasePopupTitle, &lv_font_montserrat_16, LV_PART_MAIN | LV_STATE_DEFAULT);

  ui_BasePopupMsg = lv_label_create(ui_BasePopup);
  lv_obj_set_width(ui_BasePopupMsg, 270);
  lv_obj_set_height(ui_BasePopupMsg, 110);
  lv_obj_set_x(ui_BasePopupMsg, 0);
  lv_obj_set_y(ui_BasePopupMsg, 30);
  lv_label_set_text(ui_BasePopupMsg, "");
  lv_obj_set_style_text_color(ui_BasePopupMsg, lv_color_hex(0xFFFFFF), LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_text_opa(ui_BasePopupMsg, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_text_font(ui_BasePopupMsg, &lv_font_montserrat_16, LV_PART_MAIN | LV_STATE_DEFAULT);

  ui_BasePopupCloseBtn = lv_btn_create(ui_BasePopup);
  lv_obj_set_width(ui_BasePopupCloseBtn, 50);
  lv_obj_set_height(ui_BasePopupCloseBtn, 34);
  lv_obj_set_align(ui_BasePopupCloseBtn, LV_ALIGN_BOTTOM_RIGHT);
  lv_obj_add_flag(ui_BasePopupCloseBtn, LV_OBJ_FLAG_SCROLL_ON_FOCUS);
  lv_obj_clear_flag(ui_BasePopupCloseBtn, LV_OBJ_FLAG_SCROLLABLE);
  lv_obj_set_style_shadow_width(ui_BasePopupCloseBtn, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_shadow_spread(ui_BasePopupCloseBtn, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_shadow_ofs_x(ui_BasePopupCloseBtn, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_shadow_ofs_y(ui_BasePopupCloseBtn, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_add_event_cb(ui_BasePopupCloseBtn, ui_event_callback_thunk, LV_EVENT_CLICKED, NULL);

  lv_obj_t *ui_BaseOkBtnLabel = lv_label_create(ui_BasePopupCloseBtn);
  lv_obj_set_width(ui_BaseOkBtnLabel, LV_SIZE_CONTENT);
  lv_obj_set_height(ui_BaseOkBtnLabel, LV_SIZE_CONTENT);
  lv_obj_set_align(ui_BaseOkBtnLabel, LV_ALIGN_CENTER);
  lv_label_set_text(ui_BaseOkBtnLabel, "OK");
}

void Display::ui_popup_open(String title, String msg) {
  lv_obj_clear_flag(ui_BasePopup, LV_OBJ_FLAG_HIDDEN);
  lv_label_set_text(ui_BasePopupTitle, title.c_str());
  lv_label_set_text(ui_BasePopupMsg, msg.c_str());
}

void Display::update_ui_network(void *data1, void *data2) {
  lv_port_sem_take();
  if (!lv_obj_has_flag(ui_WiFiMBox, LV_OBJ_FLAG_HIDDEN)) {
    lv_port_sem_give();
    return;
  }

  lv_obj_clear_flag(ui_WiFiList, LV_OBJ_FLAG_CLICKABLE);
  lv_obj_clear_flag(ui_WiFiList, LV_OBJ_FLAG_SCROLLABLE);
  lv_obj_clean(ui_WiFiList);

  int *arraySize = static_cast<int *>(data2);

  std::string *strPtr = static_cast<std::string *>(data1);
  std::vector<std::string> newWifiList(strPtr, strPtr + *arraySize);

  lv_list_add_text(ui_WiFiList, newWifiList.size() > 1 ? "WiFi: Found Networks" : "WiFi: Not Found!");
  for (std::vector<std::string>::iterator item = newWifiList.begin(); item != newWifiList.end(); ++item) {

    lv_obj_t *btn = lv_list_add_btn(ui_WiFiList, LV_SYMBOL_WIFI, (*item).c_str());
    lv_obj_add_event_cb(btn, wifi_event_cb_thunk, LV_EVENT_CLICKED, NULL);
  }
  lv_port_sem_give();

  lv_obj_add_flag(ui_WiFiList, LV_OBJ_FLAG_CLICKABLE);
  lv_obj_add_flag(ui_WiFiList, LV_OBJ_FLAG_SCROLLABLE);
}

void Display::ui_prep_loading() {
  ui_Loading = lv_obj_create(ui_Main_Screen);

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

void Display::show_loading_popup(bool isOn) {
  if (isOn) {
    lv_obj_clear_flag(ui_Loading, LV_OBJ_FLAG_HIDDEN);
  } else {
    lv_obj_add_flag(ui_Loading, LV_OBJ_FLAG_HIDDEN);
  }
}


void Display::update_time(void *timeStruct) {
  lv_port_sem_take();
  char hourMin[10];
  strftime(hourMin, 10, "%H:%M %p", (struct tm *)timeStruct);
  lv_label_set_text(ui_TimeLabel, hourMin);

  char date[12];
  strftime(date, 12, "%a, %b %d", (struct tm *)timeStruct);
  lv_label_set_text(ui_DateLabel, date);
  lv_port_sem_give();
}

void Display::set_notification(const char *msg) {
  lv_label_set_text(ui_NotiLabel, msg);
}

void Display::update_WiFi_label(void *data1) {
  lv_port_sem_take();
  if (data1 != NULL) {
    std::string &s = *(static_cast<std::string *>(data1));
    s.append(" ");
    s.append(LV_SYMBOL_WIFI);
    lv_label_set_text(ui_WiFiLabel, s.c_str());
  } else {
    lv_label_set_text(ui_WiFiLabel, LV_SYMBOL_WARNING);
  }
  lv_port_sem_give();
}

void Display::update_battery(void *data1) {
  int batPercent = *(int *)data1;
  String tempBatString = add_battery_icon(batPercent);
  tempBatString += " ";
  tempBatString += String(batPercent);
  tempBatString += "% ";
  lv_port_sem_take();
  lv_label_set_text(ui_BatteryLabel, tempBatString.c_str());
  lv_port_sem_give();
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
  return ui_Focused_Obj;
}

void Display::set_focused_obj(lv_obj_t *obj) {
  ui_Focused_Obj = obj;
}

lv_obj_t *Display::ui_second_screen() {
  return ui_Sub_Screen;
}

void Display::goback_main_screen() {
  lv_scr_load_anim(ui_Main_Screen, LV_SCR_LOAD_ANIM_MOVE_RIGHT, 100, 0, false);
}

void Display::lv_port_sem_take(void) {
  TaskHandle_t task = xTaskGetCurrentTaskHandle();
  if (lvgl_task_handle != task) {
    xSemaphoreTake(bin_sem, portMAX_DELAY);
  }
}

void Display::lv_port_sem_give(void) {
  TaskHandle_t task = xTaskGetCurrentTaskHandle();
  if (lvgl_task_handle != task) {
    xSemaphoreGive(bin_sem);
  }
}

int Display::get_display_width() {
  return tft->width();
}

int Display::get_display_height() {
  return tft->height();
}