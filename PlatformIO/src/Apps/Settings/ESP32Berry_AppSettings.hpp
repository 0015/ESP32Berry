/////////////////////////////////////////////////////////////////
/*
  ESP32Berry Settings App (migrated from ui_settings)
*/
/////////////////////////////////////////////////////////////////
#pragma once

#include "Apps/ESP32Berry_AppBase.hpp"
#include "UI/ESP32Berry_Display.hpp"

class AppSettings : public AppBase
{
public:
    static AppSettings *instance; // for C thunks

private:
    lv_obj_t *sliderSpeaker;
    lv_obj_t *sliderBrightness;
    lv_obj_t *btnWiFi;
    lv_obj_t *imgBtnWiFi;
    lv_obj_t *ui_ImgBtnCursor;
    lv_obj_t *ui_PanelCursor;
    void draw_ui();

public:
    AppSettings(Display *display, System *system, Network *network, const char *title);
    ~AppSettings();
    void event_handler(lv_event_t *e);
    void close_app();
};

extern "C" void app_settings_event_thunk(lv_event_t *e);
