/////////////////////////////////////////////////////////////////
/*
  ESP32Berry, "Telegram App" Version 0.2
  For More Information: https://youtu.be/h28_Mvgpe2Y
  Created by Eric N. (ThatProject)
*/
/////////////////////////////////////////////////////////////////
#include "ESP32Berry_AppNote.h"

static AppNote *instance = NULL;

AppNote::AppNote(Display *display, System *system, Network *network, const char *title)
  : AppBase(display, system, network, title) {
  _bodyScreen = display->get_body_screen();
  instance = this;
  menuIdx = 0;
  contents = "";
  this->draw_ui();
}

AppNote::~AppNote() {}

extern "C" void app_note_textarea_event_cb_thunk(lv_event_t *e) {
  instance->_display->textarea_event_cb(e);
}

extern "C" void event_handler_thunk(lv_event_t *e) {
  instance->event_handler(e);
}

extern "C" void file_event_cb_thunk(lv_event_t *e) {
  instance->file_event_cb(e);
}

void AppNote::event_handler(lv_event_t *e) {
  lv_event_code_t code = lv_event_get_code(e);
  lv_obj_t *obj = lv_event_get_target(e);

  if (code == LV_EVENT_CLICKED) {
    if (obj == uiFileListCloseBtn) {
      this->ui_close_file();

    } else if (obj == saveBoxSaveBtn) {

      String newFilename = String(lv_textarea_get_text(saveBoxFileName));
      newFilename.trim();
      if (newFilename.length() == 0) {
        this->ui_message_box(menuItem, "The file name is empty.", false);
      } else {
        String text = String(lv_textarea_get_text(textarea));
        text.trim();
        String filePath = String(NOTE_PATH) + "/" + newFilename;

        if (_system->write_file(filePath.c_str(), text.c_str())) {
          this->ui_message_box(menuItem, "The file created successfully! ", false);
          this->menu_action();
        } else {
          this->ui_message_box(menuItem, "Something Wrong!\nThe file creation failed.", false);
        }
      }

      lv_obj_del(this->saveBox);

    } else if (obj == saveBoxCloseBtn) {
      lv_obj_del(this->saveBox);
    }
  } else if (code == LV_EVENT_VALUE_CHANGED) {
    if (obj == dropDownMenu) {
      lv_dropdown_get_selected_str(obj, menuItem, sizeof(menuItem));
      menuIdx = lv_dropdown_get_selected(obj);
      String newContents = String(lv_textarea_get_text(textarea));
      newContents.trim();
      if (menuIdx == 2) {
        this->ui_save_box();
        return;
      }

      bool needTOSave = newContents != contents && newContents.length() != 0;
      if (needTOSave) {
        this->ui_message_box(menuItem, "Do you want to save it?", true);
      } else {
        this->menu_action();
      }

    } else {

      lv_obj_t *cont = lv_event_get_current_target(e);
      if (obj == cont) return;
      if (lv_msgbox_get_active_btn(cont) == 1) {
        this->ui_save_box();
      } else {
        this->ui_write_textarea("");
        this->ui_reset();
      }

      lv_msgbox_close(msgBox);
    }
  }
}

void AppNote::file_event_cb(lv_event_t *e) {
  lv_event_code_t code = lv_event_get_code(e);
  lv_obj_t *obj = lv_event_get_target(e);

  if (code == LV_EVENT_CLICKED) {

    String selectedFile = lv_list_get_btn_text(uiFileList, obj);
    filename = selectedFile.substring(0, selectedFile.indexOf(" ("));
    String filePath = String(NOTE_PATH) + "/" + filename;
    instance->contents = _system->read_file(filePath.c_str());
    instance->ui_write_textarea(instance->contents);
    instance->ui_close_file();

    lv_label_set_text(currentFileName, filename.c_str());
  }
}


void AppNote::draw_ui() {
  dropDownMenu = lv_dropdown_create(appMain);
  lv_obj_align_to(dropDownMenu, appTitle, LV_ALIGN_OUT_RIGHT_MID, 10, 0);
  lv_dropdown_set_options(dropDownMenu, "New file\n"
                                        "Save\n"
                                        "Save as ...\n"
                                        "Open File\n"
                                        "Exit");

  lv_dropdown_set_text(dropDownMenu, "Menu");
  lv_obj_add_event_cb(dropDownMenu, event_handler_thunk, LV_EVENT_VALUE_CHANGED, NULL);

  currentFileName = lv_label_create(appMain);
  lv_label_set_long_mode(currentFileName, LV_LABEL_LONG_SCROLL_CIRCULAR);
  lv_obj_set_width(currentFileName, 150);
  lv_label_set_text(currentFileName, "Untitled");
  lv_obj_align_to(currentFileName, dropDownMenu, LV_ALIGN_OUT_RIGHT_MID, 10, 0);

  textarea = lv_textarea_create(appMain);
  lv_obj_align(textarea, LV_ALIGN_TOP_MID, 0, 36);
  lv_obj_set_size(textarea, 460, 210);
  lv_obj_add_event_cb(textarea, app_note_textarea_event_cb_thunk, LV_EVENT_FOCUSED, NULL);
  lv_obj_add_event_cb(textarea, app_note_textarea_event_cb_thunk, LV_EVENT_DEFOCUSED, NULL);
}

void AppNote::ui_message_box(const char *title, String msg, bool isSelectable) {
  static const char *multipBtns[] = { "No", "Yes", "" };
  static const char *singleBtns[] = { "Ok", "" };
  msgBox = lv_msgbox_create(appMain, title, msg.c_str(), isSelectable ? multipBtns : singleBtns, true);
  lv_obj_add_event_cb(msgBox, event_handler_thunk, LV_EVENT_VALUE_CHANGED, NULL);
  lv_obj_center(msgBox);
}

void AppNote::ui_save_box() {
  saveBox = lv_obj_create(appMain);
  lv_obj_set_size(saveBox, DISPLAY_WIDTH * 2 / 3, DISPLAY_HEIGHT / 2);
  lv_obj_center(saveBox);

  saveBoxTitle = lv_label_create(saveBox);
  lv_label_set_text(saveBoxTitle, menuItem);
  lv_obj_set_size(saveBoxTitle, DISPLAY_WIDTH * 2 / 3 - 40, 40);
  lv_obj_align(saveBoxTitle, LV_ALIGN_TOP_MID, 0, 0);

  saveBoxFileName = lv_textarea_create(saveBox);
  lv_obj_set_size(saveBoxFileName, DISPLAY_WIDTH * 2 / 3 - 40, 40);
  lv_obj_align_to(saveBoxFileName, saveBoxTitle, LV_ALIGN_TOP_MID, 0, 40);

  if (filename != "" || filename != "Untitled") {
    lv_textarea_set_text(saveBoxFileName, filename.c_str());
  } else {
    lv_textarea_set_placeholder_text(saveBoxFileName, "filename?");
  }

  lv_obj_add_event_cb(saveBoxFileName, app_note_textarea_event_cb_thunk, LV_EVENT_FOCUSED, NULL);
  lv_obj_add_event_cb(saveBoxFileName, app_note_textarea_event_cb_thunk, LV_EVENT_DEFOCUSED, NULL);

  saveBoxSaveBtn = lv_btn_create(saveBox);

  lv_obj_add_event_cb(saveBoxSaveBtn, event_handler_thunk, LV_EVENT_CLICKED, NULL);
  lv_obj_align(saveBoxSaveBtn, LV_ALIGN_BOTTOM_RIGHT, 0, 0);
  lv_obj_t *btnLabel = lv_label_create(saveBoxSaveBtn);
  lv_label_set_text(btnLabel, "Save");
  lv_obj_center(btnLabel);

  saveBoxCloseBtn = lv_btn_create(saveBox);
  lv_obj_add_event_cb(saveBoxCloseBtn, event_handler_thunk, LV_EVENT_CLICKED, NULL);
  lv_obj_align(saveBoxCloseBtn, LV_ALIGN_BOTTOM_LEFT, 0, 0);
  lv_obj_t *btnLabel2 = lv_label_create(saveBoxCloseBtn);
  lv_label_set_text(btnLabel2, "Cancel");
  lv_obj_center(btnLabel2);
}

void AppNote::ui_open_file() {

  uiFileList = lv_list_create(appMain);
  lv_obj_set_size(uiFileList, DISPLAY_WIDTH - 40, DISPLAY_HEIGHT - 80);
  lv_obj_center(uiFileList);
  lv_list_add_text(uiFileList, "/Note");


  uiFileListCloseBtn = lv_btn_create(uiFileList);
  lv_obj_set_size(uiFileListCloseBtn, 30, 30);
  lv_obj_add_flag(uiFileListCloseBtn, LV_OBJ_FLAG_FLOATING);
  lv_obj_align(uiFileListCloseBtn, LV_ALIGN_TOP_RIGHT, 0, 0);
  lv_obj_add_event_cb(uiFileListCloseBtn, event_handler_thunk, LV_EVENT_CLICKED, NULL);

  lv_obj_t *label = lv_label_create(uiFileListCloseBtn);
  lv_label_set_text(label, LV_SYMBOL_CLOSE);
  lv_obj_center(label);

  lv_obj_add_flag(uiFileList, LV_OBJ_FLAG_HIDDEN);
  std::vector<String> noteFileList = _system->list_dir(NOTE_PATH);

  if (noteFileList.size() == 0) return;

  for (std::vector<String>::iterator item = noteFileList.begin(); item != noteFileList.end(); ++item) {

    lv_obj_t *btn = lv_list_add_btn(uiFileList, LV_SYMBOL_FILE, (*item).c_str());
    lv_obj_add_event_cb(btn, file_event_cb_thunk, LV_EVENT_CLICKED, NULL);
  }

  lv_obj_move_foreground(uiFileListCloseBtn);
  lv_obj_clear_flag(uiFileList, LV_OBJ_FLAG_HIDDEN);
}

void AppNote::menu_action() {
  switch (menuIdx) {
    case 0:
      this->ui_write_textarea("");
      this->ui_reset();
      break;
    case 3:
      this->ui_open_file();
      break;
    case 4:
      this->close_app();
      break;
  }
}
void AppNote::ui_write_textarea(String contents) {
  lv_textarea_set_text(textarea, contents.c_str());
}
void AppNote::ui_reset() {
  filename = "";
  lv_label_set_text(currentFileName, "Untitled");
}

void AppNote::ui_close_file() {
  lv_obj_add_flag(uiFileList, LV_OBJ_FLAG_HIDDEN);
  lv_obj_del(uiFileList);
}

void AppNote::close_app() {
  uint32_t child_cnt = lv_obj_get_child_cnt(appMain);
  if (child_cnt > 5) {
    return;
  }
  if (appMain != NULL) {
    lv_obj_del(appMain);
    appMain = NULL;
    delete this;
  }
}