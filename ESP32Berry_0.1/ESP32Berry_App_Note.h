/////////////////////////////////////////////////////////////////
/*
  ESP32Berry, "WiFi connection & Simple Note App" Version 0.1
  For More Information: https://youtu.be/wqaxCAcghtk
  Created by Eric N. (ThatProject)
*/
/////////////////////////////////////////////////////////////////
#pragma once
#include <lvgl.h>
#include <vector>
#include "ESP32Berry_Config.h"
#include "ESP32Berry_Display.h"
#include "ESP32Berry_System.h"

class AppNote {

private:
  lv_obj_t *_bodyScreen;
  lv_obj_t *appMain;
  lv_obj_t *currentFileName;
  lv_obj_t *closeBtn;

  lv_obj_t *msgBox;
  lv_obj_t *saveBox;
  lv_obj_t *saveBoxTitle;
  lv_obj_t *saveBoxFileName;
  lv_obj_t *saveBoxSaveBtn;
  lv_obj_t *saveBoxCloseBtn;
  lv_obj_t *dropDownMenu;
  lv_obj_t *uiFileList;
  lv_obj_t *uiFileListCloseBtn;
  String contents;
  String filename;
  uint8_t menuIdx;
  char menuItem[32];
  void ui_app();
  const char *NOTE_PATH = "/Note";
  void ui_open_file();
  void ui_close_file();
  void ui_write_textarea(String contents);
  void menu_action();
  void ui_reset();

public:
  AppNote(Display *display, System *system);
  ~AppNote();
  Display *_display;
  System *_system;
  lv_obj_t *textarea;
  void event_handler(lv_event_t *e);
  void file_event_cb(lv_event_t *e);
  void closeApp();
  void ui_save_box();
  void ui_message_box(const char *title, String msg, bool isSelectable);
};