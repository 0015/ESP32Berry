/////////////////////////////////////////////////////////////////
/*
  New ESP32Berry Project, The base UI & ChatGPT Client
  For More Information: https://youtu.be/5K6rSw9j5iY
  Created by Eric N. (ThatProject)
*/
/////////////////////////////////////////////////////////////////
#pragma once
#include <lvgl.h>
#include <Wire.h>
#include <vector>
#include "Configurations/LGFX_T-Deck.h"
#include "Configurations/ESP32Berry_Config.hpp"

typedef enum
{
    WIFI_OFF,
    WIFI_ON,
    APP,
    SET_AUDIO,
    SCREEN_TIMEOUT
} Menu_Event_t;

LV_IMG_DECLARE(mouse_cursor_icon);

class Display
{
private:
    int pendingVolume = -1;

    TaskHandle_t lvgl_task_handle;
    SemaphoreHandle_t bin_sem;
    friend void update_ui_task(void *pvParameters);
    LGFX *tft;

    bool cursor_panel_active = false; // Only initialized once!

    lv_indev_drv_t indev_mouse;
    lv_obj_t *cursor_obj;

    lv_obj_t *ui_SliderSpeaker;
    lv_obj_t *ui_SliderBrightness;
    lv_obj_t *ui_ImgBtnWiFi;
    lv_obj_t *ui_BtnWiFi;
    lv_obj_t *ui_ImgBtnCursor;
    lv_obj_t *ui_PanelCursor;

    lv_obj_t *ui_Main_Screen;
    lv_obj_t *ui_TopPanel;
    lv_obj_t *ui_ControlPanel;
    lv_obj_t *_bodyScreen;
    lv_obj_t *ui_AppPanel;
    lv_obj_t *ui_AppTitle;
    lv_obj_t *ui_AppCloseBtn;
    lv_obj_t *ui_WiFiPanel;
    lv_obj_t *ui_WiFiPanelCloseBtn;
    lv_obj_t *ui_WiFiLabel;
    lv_obj_t *ui_Userlabel;
    lv_obj_t *ui_TimeLabel;
    lv_obj_t *ui_DateLabel;
    lv_obj_t *ui_BatteryLabel;
    lv_obj_t *ui_Sub_Screen;
    lv_obj_t *ui_Focused_Obj;
    lv_obj_t *ui_Loading;
    lv_obj_t *ui_WiFiList;
    lv_obj_t *ui_WiFiMBox;
    lv_obj_t *ui_WiFiMBoxTitle;
    lv_obj_t *ui_WiFiMBoxPassword;
    lv_obj_t *ui_WiFiMBoxConnectBtn;
    lv_obj_t *ui_WiFiMBoxCloseBtn;
    lv_obj_t *ui_BasePopup;
    lv_obj_t *ui_BasePopupCloseBtn;
    lv_obj_t *ui_BasePopupTitle;
    lv_obj_t *ui_BasePopupMsg;
    lv_obj_t *ui_LabelScreenTimeoutValue;
    lv_obj_t *ui_ScreenTimeout;

    // Inactivity / screen timeout tracking
    uint32_t last_input_tick = 0;      // FreeRTOS tick of last user interaction
    int screen_timeout_minutes = 5;    // Configured timeout (minutes); 0 means disabled
    bool screen_dimmed = false;        // Whether we've turned the backlight off due to inactivity
    uint8_t previous_brightness = 240; // Brightness to restore after wake

    uint32_t keypad_get_key();
    void initLVGL();
    void ui_main();
    void ui_second();
    void ui_settings();
    void ui_prep_loading();
    void ui_prep_popup_box();
    void ui_popup_open(String title, String msg);
    void HandleKeyboardShortcuts(uint32_t key);
    void inactivity_task_loop(); // Called by RTOS task wrapper
    void fade_backlight_to(uint8_t target, uint8_t step, uint16_t delay_ms);
    static void inactivity_task(void *param);
    void register_activity(); // Mark activity + wake if needed

    String add_battery_icon(int percentage);
    typedef void (*FuncPtrInt)(Menu_Event_t, void *);

public:
    lv_obj_t *ui_NotiLabel;
    FuncPtrInt menu_event_cb;
    TaskHandle_t uiNotiTaskHandler;
    Display(FuncPtrInt callback);
    ~Display();
    void initTFT();
    void ui_WiFi_page();
    void my_disp_flush(lv_disp_drv_t *disp, const lv_area_t *area, lv_color_t *color_p);
    void my_touch_read(lv_indev_drv_t *indev_driver, lv_indev_data_t *data);
    void my_mouse_read(lv_indev_drv_t *indev_driver, lv_indev_data_t *data);
    void my_key_read(lv_indev_drv_t *indev_driver, lv_indev_data_t *data);
    void ui_wifi_event_callback(lv_event_t *e);
    void textarea_event_cb(lv_event_t *e);
    void ui_event_callback(lv_event_t *e);
    void ui_app_btns_callback(lv_event_t *e);
    lv_obj_t *focused_obj();
    void set_focused_obj(lv_obj_t *obj);
    void update_ui_network(void *data1, void *data2);
    void show_loading_popup(bool isOn);
    void update_volume_slider(int32_t volume);
    void update_time(void *timeStruct);
    void set_notification(const char *msg);
    void update_WiFi_label(void *data1);
    void update_battery(void *data1);
    lv_obj_t *ui_second_screen();
    void goback_main_screen();
    void lv_port_sem_take(void);
    void lv_port_sem_give(void);
    int get_display_width();
    int get_display_height();

    // Public API for screen timeout
    void set_screen_timeout(int minutes); // Update timeout config (minutes)
};

// Global instance pointer used by C thunks to forward LVGL callbacks
extern Display *g_display_instance;
