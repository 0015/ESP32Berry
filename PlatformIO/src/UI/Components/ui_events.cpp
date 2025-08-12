/////////////////////////////////////////////////////////////////
/*
  LVGL event handlers and UI updates
*/
/////////////////////////////////////////////////////////////////
#include "UI/ESP32Berry_Display.hpp"
#include <Arduino.h>

extern "C" void wifi_event_cb_thunk(lv_event_t *e);
extern "C" void textarea_event_cb_thunk(lv_event_t *e);
extern "C" void ui_event_callback_thunk(lv_event_t *e);
extern "C" void ui_app_btns_callback_thunk(lv_event_t *e);

void Display::ui_event_callback(lv_event_t *e)
{
    lv_event_code_t event_code = lv_event_get_code(e);
    lv_obj_t *target = lv_event_get_target(e);
    // Any UI event implies user interaction
    register_activity();
    if (target == ui_BasePopupCloseBtn && event_code == LV_EVENT_CLICKED)
    {
        lv_obj_add_flag(ui_BasePopup, LV_OBJ_FLAG_HIDDEN);
    }
    else if (target == ui_TopPanel && event_code == LV_EVENT_CLICKED)
    {
        if (lv_obj_has_flag(ui_ControlPanel, LV_OBJ_FLAG_HIDDEN))
            lv_obj_clear_flag(ui_ControlPanel, LV_OBJ_FLAG_HIDDEN);
        else
            lv_obj_add_flag(ui_ControlPanel, LV_OBJ_FLAG_HIDDEN);
    }
    else if (target == ui_SliderBrightness && event_code == LV_EVENT_VALUE_CHANGED)
    {
        int sliderValue = lv_slider_get_value(ui_SliderBrightness);
        tft->setBrightness(sliderValue);
        // Remember user's chosen brightness so inactivity wake restores this
        previous_brightness = sliderValue;
        if (screen_dimmed && sliderValue > 0)
        {
            // If user moves slider while dimmed, treat as activity and undim immediately
            screen_dimmed = false;
        }
    }
    else if (target == ui_SliderSpeaker && event_code == LV_EVENT_VALUE_CHANGED)
    {
        int sliderValue = lv_slider_get_value(ui_SliderSpeaker);
        menu_event_cb(SET_AUDIO, reinterpret_cast<void *>(sliderValue));
    }
    else if (target == ui_ImgBtnWiFi && event_code == LV_EVENT_CLICKED)
    {
        if (lv_obj_get_state(ui_ImgBtnWiFi) & LV_STATE_CHECKED)
        {
            menu_event_cb(WIFI_ON, NULL);
        }
        else
        {
            menu_event_cb(WIFI_OFF, NULL);
            lv_obj_clean(ui_WiFiList);
        }
    }
    else if (target == ui_BtnWiFi && event_code == LV_EVENT_CLICKED)
    {
        lv_obj_clear_flag(ui_WiFiPanel, LV_OBJ_FLAG_HIDDEN);
        lv_obj_add_flag(ui_ControlPanel, LV_OBJ_FLAG_HIDDEN);
    }
    else if ((target == ui_ImgBtnCursor && event_code == LV_EVENT_CLICKED) || (target == ui_PanelCursor && event_code == LV_EVENT_CLICKED))
    {
        // Toggles each time
        cursor_panel_active = !cursor_panel_active;
        if (cursor_panel_active)
        {
            lv_obj_set_style_bg_color(ui_PanelCursor, lv_color_hex(0x33bd33), LV_PART_MAIN | LV_STATE_DEFAULT); // #33bd33ff
            lv_obj_set_style_bg_color(ui_ImgBtnCursor, lv_color_hex(0x33bd33), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_clear_flag(cursor_obj, LV_OBJ_FLAG_HIDDEN);
        }
        else
        {
            lv_obj_set_style_bg_color(ui_PanelCursor, lv_color_hex(0xFFFFFF), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_bg_color(ui_ImgBtnCursor, lv_color_hex(0xFFFFFF), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_add_flag(cursor_obj, LV_OBJ_FLAG_HIDDEN);
        }
    }
    else if (target == ui_ScreenTimeout && event_code == LV_EVENT_VALUE_CHANGED)
    {
        int sliderValue = lv_slider_get_value(ui_ScreenTimeout);
        String timeoutText = String(sliderValue) + " min";
        lv_label_set_text(ui_LabelScreenTimeoutValue, timeoutText.c_str());
        menu_event_cb(SCREEN_TIMEOUT, reinterpret_cast<void *>(sliderValue));
        set_screen_timeout(sliderValue);
    }
}

void Display::ui_app_btns_callback(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t *btn = lv_event_get_target(e);
    if (code == LV_EVENT_CLICKED)
    {
        set_notification("");

        lv_obj_t *label = lv_obj_get_child(btn, 0);
        String appBtnLabel = lv_label_get_text(label);
        switch (appBtnLabel.toInt())
        {
        case 0:
            lv_scr_load_anim(ui_Sub_Screen, LV_SCR_LOAD_ANIM_MOVE_LEFT, 100, 0, false);
            menu_event_cb(APP, lv_label_get_text(label));
            break;
        default:
            break;
        }
    }
}

void Display::textarea_event_cb(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t *obj = lv_event_get_target(e);

    if (code == LV_EVENT_FOCUSED)
    {
        ui_Focused_Obj = obj;
    }
    else if (code == LV_EVENT_DEFOCUSED)
    {
        ui_Focused_Obj = NULL;
    }
}

void Display::update_ui_network(void *data1, void *data2)
{
    lv_port_sem_take();
    if (!lv_obj_has_flag(ui_WiFiMBox, LV_OBJ_FLAG_HIDDEN))
    {
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
    for (std::vector<std::string>::iterator item = newWifiList.begin(); item != newWifiList.end(); ++item)
    {

        lv_obj_t *btn = lv_list_add_btn(ui_WiFiList, LV_SYMBOL_WIFI, (*item).c_str());
        lv_obj_add_event_cb(btn, wifi_event_cb_thunk, LV_EVENT_CLICKED, NULL);
    }
    lv_port_sem_give();

    lv_obj_add_flag(ui_WiFiList, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_add_flag(ui_WiFiList, LV_OBJ_FLAG_SCROLLABLE);
}

void Display::update_volume_slider(int32_t volume)
{
    if (!ui_SliderSpeaker)
    {
        pendingVolume = volume;
        return;
    }
    lv_slider_set_value(ui_SliderSpeaker, volume, LV_ANIM_OFF);
}

void Display::update_time(void *timeStruct)
{
    lv_port_sem_take();
    char hourMin[10];
    strftime(hourMin, 10, "%H:%M %p", (struct tm *)timeStruct);
    lv_label_set_text(ui_TimeLabel, hourMin);

    char date[12];
    strftime(date, 12, "%a, %b %d", (struct tm *)timeStruct);
    lv_label_set_text(ui_DateLabel, date);
    lv_port_sem_give();
}

void Display::set_notification(const char *msg)
{
    lv_label_set_text(ui_NotiLabel, msg);
}

void Display::update_WiFi_label(void *data1)
{
    lv_port_sem_take();
    if (data1 != NULL)
    {
        std::string &s = *(static_cast<std::string *>(data1));
        s.append(" ");
        s.append(LV_SYMBOL_WIFI);
        lv_label_set_text(ui_WiFiLabel, s.c_str());
    }
    else
    {
        lv_label_set_text(ui_WiFiLabel, LV_SYMBOL_WARNING);
    }
    lv_port_sem_give();
}

void Display::update_battery(void *data1)
{
    int batPercent = *(int *)data1;
    String tempBatString = add_battery_icon(batPercent);
    tempBatString += " ";
    tempBatString += String(batPercent);
    tempBatString += "% ";
    lv_port_sem_take();
    lv_label_set_text(ui_BatteryLabel, tempBatString.c_str());
    lv_port_sem_give();
}

String Display::add_battery_icon(int percentage)
{
    if (percentage >= 90)
    {
        return String(LV_SYMBOL_BATTERY_FULL);
    }
    else if (percentage >= 65 && percentage < 90)
    {
        return String(LV_SYMBOL_BATTERY_3);
    }
    else if (percentage >= 40 && percentage < 65)
    {
        return String(LV_SYMBOL_BATTERY_2);
    }
    else if (percentage >= 15 && percentage < 40)
    {
        return String(LV_SYMBOL_BATTERY_1);
    }
    else
    {
        return String(LV_SYMBOL_BATTERY_EMPTY);
    }
}

lv_obj_t *Display::focused_obj()
{
    return ui_Focused_Obj;
}

void Display::set_focused_obj(lv_obj_t *obj)
{
    ui_Focused_Obj = obj;
}

lv_obj_t *Display::ui_second_screen()
{
    return ui_Sub_Screen;
}

void Display::goback_main_screen()
{
    lv_scr_load_anim(ui_Main_Screen, LV_SCR_LOAD_ANIM_MOVE_RIGHT, 100, 0, false);
}

void Display::lv_port_sem_take(void)
{
    TaskHandle_t task = xTaskGetCurrentTaskHandle();
    if (lvgl_task_handle != task)
    {
        xSemaphoreTake(bin_sem, portMAX_DELAY);
    }
}

void Display::lv_port_sem_give(void)
{
    TaskHandle_t task = xTaskGetCurrentTaskHandle();
    if (lvgl_task_handle != task)
    {
        xSemaphoreGive(bin_sem);
    }
}

int Display::get_display_width()
{
    return tft->width();
}

int Display::get_display_height()
{
    return tft->height();
}
