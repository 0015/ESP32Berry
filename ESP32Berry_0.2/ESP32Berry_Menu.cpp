/////////////////////////////////////////////////////////////////
/*
  ESP32Berry, "Telegram App" Version 0.2
  For More Information: https://youtu.be/h28_Mvgpe2Y
  Created by Eric N. (ThatProject)
*/
/////////////////////////////////////////////////////////////////
#include "ESP32Berry_Menu.h"
static Menu *instance = NULL;

Menu::Menu(Display *display, System *system, FuncPtrInt callback) {
  instance = this;
  _system = system;
  _display = display;
  _uiRoot = display->get_ui_root();
  _bodyScreen = display->get_body_screen();
  menu_event_cb = callback;
  xSemaphoreTake(_display->get_mutex(), portMAX_DELAY);
  this->ui_menu_items();
  this->ui_menu();
  xSemaphoreGive(_display->get_mutex());
}

Menu::~Menu() {}

extern "C" void menu_event_handler_thunk(lv_event_t *e) {
  instance->menu_event_handler(e);
}

extern "C" void wifi_event_cb_thunk(lv_event_t *e) {
  instance->wifi_event_cb(e);
}

extern "C" void menu_textarea_event_cb_thunk(lv_event_t *e) {
  instance->_display->textarea_event_cb(e);
}

void Menu::menu_event_handler(lv_event_t *e) {
  lv_event_code_t code = lv_event_get_code(e);
  lv_obj_t *obj = lv_event_get_target(e);

  if (code == LV_EVENT_CLICKED) {
    if (obj == uiMenuCloseBtn) {
      lv_obj_add_flag(uiMenuSDCard, LV_OBJ_FLAG_HIDDEN);
    } else if (obj == logoBtn) {
      if (lv_obj_has_flag(menuList, LV_OBJ_FLAG_HIDDEN)) {
        this->ui_menu_open(true);
      } else {
        this->ui_menu_open(false);
      }

    } else if (obj == mboxConnectBtn) {
      lv_obj_move_background(mboxConnect);
      lv_obj_add_flag(mboxConnect, LV_OBJ_FLAG_HIDDEN);
      lv_obj_add_flag(menuList, LV_OBJ_FLAG_HIDDEN);
      lv_obj_add_flag(uiMenuWiFI, LV_OBJ_FLAG_HIDDEN);

      char *key = new char[strlen(lv_label_get_text(mboxTitle)) + strlen(lv_textarea_get_text(mboxPassword)) + 3];
      strcpy(key, lv_label_get_text(mboxTitle));
      strcat(key, WIFI_SSID_PW_DELIMITER);
      strcat(key, lv_textarea_get_text(mboxPassword));
      menu_event_cb(WIFI_ON, key);
      delete[] key;
      _display->show_loading_popup(true);
      lv_textarea_set_text(mboxPassword, "");

    } else if (obj == mboxCloseBtn) {
      lv_obj_move_background(mboxConnect);
      lv_obj_add_flag(mboxConnect, LV_OBJ_FLAG_HIDDEN);

    } else if (obj == uiWiFiCloseBtn) {
      lv_obj_add_flag(uiMenuWiFI, LV_OBJ_FLAG_HIDDEN);
    } else if (obj == uiMenuCloseBtn) {
      lv_obj_add_flag(uiMenuSDCard, LV_OBJ_FLAG_HIDDEN);
    } else if (obj == uiBatteryCloseBtn) {
      lv_obj_add_flag(uiMenuBattery, LV_OBJ_FLAG_HIDDEN);
    } else {
      lv_obj_add_flag(menuList, LV_OBJ_FLAG_HIDDEN);
      switch (lv_obj_get_child_id(obj)) {
        case 0:
          _display->ui_popup_open("ESP32Berry v0.2", "ESP v2.0.4\nLVGL v8.3.1\nLovyanGFX v0.4.18\nFastBot v2.22\n\nby ThatProject");
          break;
        case 2:
          lv_obj_clear_flag(uiMenuWiFI, LV_OBJ_FLAG_HIDDEN);
          break;
        case 3:
          lv_obj_clear_flag(uiMenuBattery, LV_OBJ_FLAG_HIDDEN);
          break;
        case 5:
          this->open_menu_sdcard();
          break;
        case 7:
          Serial.println("RESTART");
          _system->restart();
          break;
      }
    }
  } else if (code == LV_EVENT_VALUE_CHANGED) {
    if (obj == uiWiFiSwitch) {
      if (lv_obj_has_state(obj, LV_STATE_CHECKED)) {
        menu_event_cb(WIFI_ON, NULL);
      } else {
        menu_event_cb(WIFI_OFF, NULL);
        lv_obj_clean(uiWiFiList);
      }
    }
  }
}

void Menu::wifi_event_cb(lv_event_t *e) {
  lv_event_code_t code = lv_event_get_code(e);
  lv_obj_t *obj = lv_event_get_target(e);

  if (code == LV_EVENT_CLICKED) {
    int length = strlen(lv_list_get_btn_text(uiWiFiList, obj));
    char ssidName[length - 7];
    strncpy(ssidName, lv_list_get_btn_text(uiWiFiList, obj), length - 8);
    ssidName[length - 8] = '\0';
    lv_label_set_text(mboxTitle, ssidName);
    lv_obj_move_foreground(mboxConnect);
    lv_obj_clear_flag(mboxConnect, LV_OBJ_FLAG_HIDDEN);
  }
}

void Menu::ui_menu() {
  LV_IMG_DECLARE(ESP32Berry_Icon);
  logoBtn = lv_imgbtn_create(_uiRoot);
  lv_obj_set_size(logoBtn, 40, 40);
  lv_imgbtn_set_src(logoBtn, LV_IMGBTN_STATE_RELEASED, &ESP32Berry_Icon, NULL, NULL);
  lv_obj_align(logoBtn, LV_ALIGN_TOP_LEFT, 4, 0);
  lv_obj_add_event_cb(logoBtn, menu_event_handler_thunk, LV_EVENT_CLICKED, NULL);

  menuList = lv_list_create(_uiRoot);
  lv_obj_set_size(menuList, 200, 240);
  lv_obj_align(menuList, LV_ALIGN_TOP_LEFT, 0, 40);

  lv_obj_t *menuListBtn = lv_list_add_btn(menuList, LV_SYMBOL_HOME, "About This");
  lv_obj_add_event_cb(menuListBtn, menu_event_handler_thunk, LV_EVENT_CLICKED, NULL);

  lv_list_add_text(menuList, "");
  menuListBtn = lv_list_add_btn(menuList, LV_SYMBOL_WIFI, "Wi-Fi");
  lv_obj_add_event_cb(menuListBtn, menu_event_handler_thunk, LV_EVENT_CLICKED, NULL);
  menuListBtn = lv_list_add_btn(menuList, LV_SYMBOL_BATTERY_3, "Battery");
  lv_obj_add_event_cb(menuListBtn, menu_event_handler_thunk, LV_EVENT_CLICKED, NULL);
  lv_list_add_text(menuList, "");
  menuListBtn = lv_list_add_btn(menuList, LV_SYMBOL_SD_CARD, "SD CARD");
  lv_obj_add_event_cb(menuListBtn, menu_event_handler_thunk, LV_EVENT_CLICKED, NULL);
  lv_list_add_text(menuList, "");
  menuListBtn = lv_list_add_btn(menuList, LV_SYMBOL_POWER, "Restart...");
  lv_obj_add_event_cb(menuListBtn, menu_event_handler_thunk, LV_EVENT_CLICKED, NULL);

  this->ui_menu_open(false);
}

void Menu::ui_menu_open(bool isOn) {
  if (isOn) {
    lv_obj_clear_flag(menuList, LV_OBJ_FLAG_HIDDEN);
  } else {
    lv_obj_add_flag(menuList, LV_OBJ_FLAG_HIDDEN);
  }
}

void Menu::ui_menu_items() {
  this->ui_menu_wifi();
  this->ui_wifi_conenct_box();
  this->ui_menu_battery();
  this->ui_menu_sdcard();
}

void Menu::ui_menu_wifi() {
  uiMenuWiFI = lv_obj_create(_uiRoot);
  lv_obj_add_flag(uiMenuWiFI, LV_OBJ_FLAG_HIDDEN);
  lv_obj_set_size(uiMenuWiFI, DISPLAY_WIDTH - 100, DISPLAY_HEIGHT - 40);
  lv_obj_align(uiMenuWiFI, LV_ALIGN_TOP_MID, 0, 20);

  lv_obj_t *titleLabel = lv_label_create(uiMenuWiFI);
  lv_label_set_text(titleLabel, "Wi-Fi " LV_SYMBOL_WIFI);
  lv_obj_set_style_text_font(titleLabel, &lv_font_montserrat_20, LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_align(titleLabel, LV_ALIGN_TOP_LEFT, 0, 0);

  uiWiFiCloseBtn = lv_btn_create(uiMenuWiFI);
  lv_obj_set_size(uiWiFiCloseBtn, 30, 30);
  lv_obj_align(uiWiFiCloseBtn, LV_ALIGN_TOP_RIGHT, 0, -10);
  lv_obj_add_event_cb(uiWiFiCloseBtn, menu_event_handler_thunk, LV_EVENT_CLICKED, NULL);
  lv_obj_t *btnSymbol = lv_label_create(uiWiFiCloseBtn);
  lv_label_set_text(btnSymbol, LV_SYMBOL_CLOSE);
  lv_obj_center(btnSymbol);

  uiWiFiSwitch = lv_switch_create(uiMenuWiFI);
  lv_obj_add_event_cb(uiWiFiSwitch, menu_event_handler_thunk, LV_EVENT_VALUE_CHANGED, NULL);
  lv_obj_align_to(uiWiFiSwitch, titleLabel, LV_ALIGN_TOP_RIGHT, 60, -5);

  uiWiFiList = lv_list_create(uiMenuWiFI);
  lv_obj_set_size(uiWiFiList, DISPLAY_WIDTH - 140, 210);
  lv_obj_align_to(uiWiFiList, titleLabel, LV_ALIGN_TOP_LEFT, 0, 30);
}

void Menu::ui_wifi_conenct_box() {
  mboxConnect = lv_obj_create(_uiRoot);
  lv_obj_set_size(mboxConnect, DISPLAY_WIDTH * 2 / 3, DISPLAY_HEIGHT / 2);
  lv_obj_center(mboxConnect);

  mboxTitle = lv_label_create(mboxConnect);
  lv_label_set_text(mboxTitle, "Selected WiFi SSID: ");
  lv_obj_set_size(mboxTitle, DISPLAY_WIDTH * 2 / 3 - 40, 40);
  lv_obj_align(mboxTitle, LV_ALIGN_TOP_MID, 0, 0);

  mboxPassword = lv_textarea_create(mboxConnect);
  lv_obj_set_size(mboxPassword, DISPLAY_WIDTH * 2 / 3 - 40, 40);
  lv_obj_align_to(mboxPassword, mboxTitle, LV_ALIGN_TOP_MID, 0, 40);
  lv_textarea_set_placeholder_text(mboxPassword, "Password?");
  lv_obj_add_event_cb(mboxPassword, menu_textarea_event_cb_thunk, LV_EVENT_FOCUSED, NULL);
  lv_obj_add_event_cb(mboxPassword, menu_textarea_event_cb_thunk, LV_EVENT_DEFOCUSED, NULL);

  mboxConnectBtn = lv_btn_create(mboxConnect);
  lv_obj_add_event_cb(mboxConnectBtn, menu_event_handler_thunk, LV_EVENT_ALL, NULL);
  lv_obj_align(mboxConnectBtn, LV_ALIGN_BOTTOM_RIGHT, 0, 0);
  lv_obj_t *btnLabel = lv_label_create(mboxConnectBtn);
  lv_label_set_text(btnLabel, "Connect");
  lv_obj_center(btnLabel);

  mboxCloseBtn = lv_btn_create(mboxConnect);
  lv_obj_add_event_cb(mboxCloseBtn, menu_event_handler_thunk, LV_EVENT_ALL, NULL);
  lv_obj_align(mboxCloseBtn, LV_ALIGN_BOTTOM_LEFT, 0, 0);
  lv_obj_t *btnLabel2 = lv_label_create(mboxCloseBtn);
  lv_label_set_text(btnLabel2, "Cancel");
  lv_obj_center(btnLabel2);

  lv_obj_move_background(mboxConnect);
  lv_obj_add_flag(mboxConnect, LV_OBJ_FLAG_HIDDEN);
}


void Menu::update_ui_network(std::vector<String> newWifiList) {

  xSemaphoreTake(_display->get_mutex(), portMAX_DELAY);
  if (!lv_obj_has_flag(mboxConnect, LV_OBJ_FLAG_HIDDEN)) {
    xSemaphoreGive(_display->get_mutex());
    return;
  }

  lv_obj_clear_flag(uiWiFiList, LV_OBJ_FLAG_CLICKABLE);
  lv_obj_clear_flag(uiWiFiList, LV_OBJ_FLAG_SCROLLABLE);
  lv_obj_clean(uiWiFiList);

  lv_list_add_text(uiWiFiList, newWifiList.size() > 1 ? "WiFi: Found Networks" : "WiFi: Not Found!");
  for (std::vector<String>::iterator item = newWifiList.begin(); item != newWifiList.end(); ++item) {

    lv_obj_t *btn = lv_list_add_btn(uiWiFiList, LV_SYMBOL_WIFI, (*item).c_str());
    lv_obj_add_event_cb(btn, wifi_event_cb_thunk, LV_EVENT_CLICKED, NULL);
  }

  lv_obj_add_flag(uiWiFiList, LV_OBJ_FLAG_CLICKABLE);
  lv_obj_add_flag(uiWiFiList, LV_OBJ_FLAG_SCROLLABLE);

  xSemaphoreGive(_display->get_mutex());
}

void Menu::ui_network_switch(bool isOn) {
  if (!isOn) {
    lv_obj_clear_state(uiWiFiSwitch, LV_STATE_CHECKED);
  }
}

void Menu::ui_menu_battery() {
  lv_style_init(&batteryStyle);
  lv_style_set_bg_opa(&batteryStyle, LV_OPA_COVER);
  lv_style_set_bg_color(&batteryStyle, lv_palette_main(LV_PALETTE_GREEN));
  lv_style_set_bg_grad_color(&batteryStyle, lv_palette_main(LV_PALETTE_GREY));
  lv_style_set_bg_grad_dir(&batteryStyle, LV_GRAD_DIR_VER);
  lv_style_set_radius(&batteryStyle, 16);
  lv_style_set_border_width(&batteryStyle, 0);

  lv_style_init(&borderStyle);
  lv_style_set_radius(&borderStyle, 16);
  lv_style_set_border_width(&borderStyle, 4);
  lv_style_set_border_color(&borderStyle, lv_color_black());

  uiMenuBattery = lv_obj_create(_uiRoot);
  lv_obj_add_flag(uiMenuBattery, LV_OBJ_FLAG_HIDDEN);
  lv_obj_set_size(uiMenuBattery, DISPLAY_WIDTH - 100, DISPLAY_HEIGHT - 40);
  lv_obj_align(uiMenuBattery, LV_ALIGN_TOP_MID, 0, 20);

  lv_obj_t *titleLabel = lv_label_create(uiMenuBattery);
  lv_label_set_text(titleLabel, "Battery " LV_SYMBOL_BATTERY_FULL);
  lv_obj_set_style_text_font(titleLabel, &lv_font_montserrat_20, LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_align(titleLabel, LV_ALIGN_TOP_LEFT, 0, 0);

  uiBatteryCloseBtn = lv_btn_create(uiMenuBattery);
  lv_obj_set_size(uiBatteryCloseBtn, 30, 30);
  lv_obj_align(uiBatteryCloseBtn, LV_ALIGN_TOP_RIGHT, 0, -10);
  lv_obj_add_event_cb(uiBatteryCloseBtn, menu_event_handler_thunk, LV_EVENT_CLICKED, NULL);
  lv_obj_t *btnSymbol = lv_label_create(uiBatteryCloseBtn);
  lv_label_set_text(btnSymbol, LV_SYMBOL_CLOSE);
  lv_obj_center(btnSymbol);

  lv_obj_t *batteryTop = lv_obj_create(uiMenuBattery);
  lv_obj_remove_style_all(batteryTop);
  lv_obj_add_style(batteryTop, &borderStyle, 0);
  lv_obj_set_size(batteryTop, 36, 36);
  lv_obj_align(batteryTop, LV_ALIGN_TOP_MID, 0, 0);

  lv_obj_t *batteryFrame = lv_obj_create(uiMenuBattery);
  lv_obj_add_style(batteryFrame, &borderStyle, 0);
  lv_obj_set_size(batteryFrame, 116, 216);
  lv_obj_center(batteryFrame);

  batteryMeter = lv_bar_create(uiMenuBattery);
  lv_obj_remove_style_all(batteryMeter);
  lv_obj_add_style(batteryMeter, &batteryStyle, LV_PART_INDICATOR);
  lv_obj_set_size(batteryMeter, 100, 200);
  lv_obj_center(batteryMeter);
  lv_bar_set_range(batteryMeter, 0, 100);

  uiBatteryVolts = lv_label_create(uiMenuBattery);
  lv_obj_set_width(uiBatteryVolts, 150);
  lv_label_set_text(uiBatteryVolts, "-.-- v");
  lv_obj_align(uiBatteryVolts, LV_ALIGN_RIGHT_MID, 0, 0);
  lv_obj_set_style_text_align(uiBatteryVolts, LV_TEXT_ALIGN_RIGHT, 0);
  lv_obj_set_style_text_font(uiBatteryVolts, &lv_font_montserrat_20, LV_PART_MAIN | LV_STATE_DEFAULT);

  uiBatteryPercent = lv_label_create(uiMenuBattery);
  lv_obj_set_width(uiBatteryPercent, 150);
  lv_label_set_text(uiBatteryPercent, "");
  lv_obj_align(uiBatteryPercent, LV_ALIGN_LEFT_MID, 10, 0);
  lv_obj_set_style_text_align(uiBatteryPercent, LV_TEXT_ALIGN_LEFT, 0);
  lv_obj_set_style_text_font(uiBatteryPercent, &lv_font_montserrat_20, LV_PART_MAIN | LV_STATE_DEFAULT);
}

void Menu::update_battery(String info) {

  int delimiterIdx = info.indexOf(",");
  if (delimiterIdx < 1) {
    return;
  }

  String _volt = info.substring(0, delimiterIdx);
  String _percent = info.substring(delimiterIdx + 1, info.length());

  xSemaphoreTake(_display->get_mutex(), portMAX_DELAY);
  lv_bar_set_value(batteryMeter, _percent.toInt(), LV_ANIM_ON);

  _percent += " %";
  lv_label_set_text(uiBatteryPercent, _percent.c_str());

  _volt += " v";
  lv_label_set_text(uiBatteryVolts, _volt.c_str());

  xSemaphoreGive(_display->get_mutex());
}


void Menu::ui_menu_sdcard() {
  uiMenuSDCard = lv_obj_create(_uiRoot);
  lv_obj_add_flag(uiMenuSDCard, LV_OBJ_FLAG_HIDDEN);
  lv_obj_set_size(uiMenuSDCard, DISPLAY_WIDTH - 100, DISPLAY_HEIGHT - 40);
  lv_obj_align(uiMenuSDCard, LV_ALIGN_TOP_MID, 0, 20);

  lv_obj_t *titleLabel = lv_label_create(uiMenuSDCard);
  lv_label_set_text(titleLabel, "SDCARD " LV_SYMBOL_SD_CARD);
  lv_obj_set_style_text_font(titleLabel, &lv_font_montserrat_20, LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_align(titleLabel, LV_ALIGN_TOP_LEFT, 0, 0);

  uiMenuCloseBtn = lv_btn_create(uiMenuSDCard);
  lv_obj_set_size(uiMenuCloseBtn, 30, 30);
  lv_obj_align(uiMenuCloseBtn, LV_ALIGN_TOP_RIGHT, 0, -10);
  lv_obj_add_event_cb(uiMenuCloseBtn, menu_event_handler_thunk, LV_EVENT_CLICKED, NULL);
  lv_obj_t *btnSymbol = lv_label_create(uiMenuCloseBtn);
  lv_label_set_text(btnSymbol, LV_SYMBOL_CLOSE);
  lv_obj_center(btnSymbol);

  uiArc = lv_arc_create(uiMenuSDCard);
  lv_obj_set_size(uiArc, 150, 150);
  lv_arc_set_rotation(uiArc, 135);
  lv_arc_set_bg_angles(uiArc, 0, 270);
  lv_arc_set_value(uiArc, 0);
  lv_obj_center(uiArc);
  lv_obj_clear_flag(uiArc, LV_OBJ_FLAG_CLICKABLE);

  uiUsedBytes = lv_label_create(uiMenuSDCard);
  lv_obj_set_width(uiUsedBytes, 150);
  lv_label_set_text(uiUsedBytes, "");
  lv_obj_align(uiUsedBytes, LV_ALIGN_LEFT_MID, 10, 0);
  lv_obj_set_style_text_align(uiUsedBytes, LV_TEXT_ALIGN_LEFT, 0);
  lv_obj_set_style_text_font(uiUsedBytes, &lv_font_montserrat_20, LV_PART_MAIN | LV_STATE_DEFAULT);

  uiTotalBytes = lv_label_create(uiMenuSDCard);
  lv_obj_set_width(uiTotalBytes, 150);
  lv_label_set_text(uiTotalBytes, "");
  lv_obj_align(uiTotalBytes, LV_ALIGN_RIGHT_MID, 0, 0);
  lv_obj_set_style_text_align(uiTotalBytes, LV_TEXT_ALIGN_RIGHT, 0);
  lv_obj_set_style_text_font(uiTotalBytes, &lv_font_montserrat_20, LV_PART_MAIN | LV_STATE_DEFAULT);

  uiPercentage = lv_label_create(uiMenuSDCard);
  lv_obj_set_width(uiPercentage, 150);
  lv_label_set_text(uiPercentage, "");
  lv_obj_center(uiPercentage);
  lv_obj_set_style_text_align(uiPercentage, LV_TEXT_ALIGN_CENTER, 0);
  lv_obj_set_style_text_font(uiPercentage, &lv_font_montserrat_20, LV_PART_MAIN | LV_STATE_DEFAULT);
}

void Menu::get_sdcard_info() {
  unsigned long totalBytes;
  unsigned long usedBytes;
  _system->sdcard_info(&totalBytes, &usedBytes);

  float percentage = usedBytes / totalBytes * 100;
  String strPercentage = String(percentage);
  strPercentage += "%";

  lv_arc_set_value(uiArc, int(percentage));
  lv_label_set_text(uiPercentage, strPercentage.c_str());

  String strDummy = "Total:\n";
  strDummy += String(totalBytes);
  strDummy += "\nMB";
  lv_label_set_text(uiTotalBytes, strDummy.c_str());

  strDummy = "Used:\n";
  strDummy += String(usedBytes);
  strDummy += "\nMB";
  lv_label_set_text(uiUsedBytes, strDummy.c_str());
}

void Menu::open_menu_sdcard() {
  this->get_sdcard_info();
  lv_obj_clear_flag(uiMenuSDCard, LV_OBJ_FLAG_HIDDEN);
}