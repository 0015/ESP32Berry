/////////////////////////////////////////////////////////////////
/*
  ESP32Berry Settings App (migrated from ui_settings)
*/
/////////////////////////////////////////////////////////////////
#include "ESP32Berry_AppSettings.hpp"

AppSettings *AppSettings::instance = NULL;

extern "C" void app_settings_event_thunk(lv_event_t *e)
{
    AppSettings::instance->event_handler(e);
}

AppSettings::AppSettings(Display *display, System *system, Network *network, const char *title)
    : AppBase(display, system, network, title)
{
    instance = this;
    draw_ui();
}

AppSettings::~AppSettings() {}

void AppSettings::draw_ui()
{
    LV_IMG_DECLARE(icon_brightness);
    LV_IMG_DECLARE(icon_speaker);
    LV_IMG_DECLARE(icon_wifi);

    lv_obj_t *panel = lv_obj_create(ui_AppPanel);
    lv_obj_set_size(panel, _display->get_display_width() - 40, 100);
    lv_obj_align(panel, LV_ALIGN_TOP_MID, 0, 10);
    lv_obj_clear_flag(panel, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_style_bg_color(panel, lv_color_hex(0xFFFFFF), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(panel, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(panel, 0, LV_PART_MAIN | LV_STATE_DEFAULT);

    lv_obj_t *iconSpeaker = lv_img_create(panel);
    lv_img_set_src(iconSpeaker, &icon_speaker);
    lv_obj_align(iconSpeaker, LV_ALIGN_TOP_LEFT, 0, -6);
    lv_obj_add_flag(iconSpeaker, LV_OBJ_FLAG_ADV_HITTEST);
    lv_obj_clear_flag(iconSpeaker, LV_OBJ_FLAG_SCROLLABLE);

    sliderSpeaker = lv_slider_create(panel);
    lv_slider_set_range(sliderSpeaker, 0, 21);
    lv_obj_set_size(sliderSpeaker, 100, 8);
    lv_obj_align(sliderSpeaker, LV_ALIGN_TOP_LEFT, 32, -2);
    lv_obj_set_style_bg_color(sliderSpeaker, lv_color_hex(0x989898), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(sliderSpeaker, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(sliderSpeaker, lv_color_hex(0xE95622), LV_PART_INDICATOR | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(sliderSpeaker, 255, LV_PART_INDICATOR | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(sliderSpeaker, lv_color_hex(0xFFFFFF), LV_PART_KNOB | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(sliderSpeaker, 255, LV_PART_KNOB | LV_STATE_DEFAULT);
    lv_obj_set_style_border_color(sliderSpeaker, lv_color_hex(0x000000), LV_PART_KNOB | LV_STATE_DEFAULT);
    lv_obj_set_style_border_opa(sliderSpeaker, 255, LV_PART_KNOB | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(sliderSpeaker, 1, LV_PART_KNOB | LV_STATE_DEFAULT);
    // Register with Display so future volume updates propagate
    _display->register_speaker_slider(sliderSpeaker);
    // Initialize to system volume if no pending value was applied in register (pendingVolume becomes -1)
    if (lv_slider_get_value(sliderSpeaker) == 0 || lv_slider_get_value(sliderSpeaker) == 21) // rough heuristic
    {
        uint8_t vol = _system->audio ? _system->audio->getVolume() : 21;
        lv_slider_set_value(sliderSpeaker, vol, LV_ANIM_OFF);
    }
    lv_obj_add_event_cb(sliderSpeaker, app_settings_event_thunk, LV_EVENT_VALUE_CHANGED, NULL);

    lv_obj_t *iconBrightness = lv_img_create(panel);
    lv_img_set_src(iconBrightness, &icon_brightness);
    lv_obj_align(iconBrightness, LV_ALIGN_TOP_LEFT, 0, 16);
    lv_obj_add_flag(iconBrightness, LV_OBJ_FLAG_ADV_HITTEST);
    lv_obj_clear_flag(iconBrightness, LV_OBJ_FLAG_SCROLLABLE);

    sliderBrightness = lv_slider_create(panel);
    lv_slider_set_range(sliderBrightness, 10, 255);
    lv_obj_set_size(sliderBrightness, 100, 8);
    lv_obj_align(sliderBrightness, LV_ALIGN_TOP_LEFT, 32, 20);
    lv_obj_set_style_bg_color(sliderBrightness, lv_color_hex(0x989898), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(sliderBrightness, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(sliderBrightness, lv_color_hex(0xE95622), LV_PART_INDICATOR | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(sliderBrightness, 255, LV_PART_INDICATOR | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(sliderBrightness, lv_color_hex(0xFFFFFF), LV_PART_KNOB | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(sliderBrightness, 255, LV_PART_KNOB | LV_STATE_DEFAULT);
    lv_obj_set_style_border_color(sliderBrightness, lv_color_hex(0x000000), LV_PART_KNOB | LV_STATE_DEFAULT);
    lv_obj_set_style_border_opa(sliderBrightness, 255, LV_PART_KNOB | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(sliderBrightness, 1, LV_PART_KNOB | LV_STATE_DEFAULT);
    lv_slider_set_value(sliderBrightness, 255, LV_ANIM_OFF);
    lv_obj_add_event_cb(sliderBrightness, app_settings_event_thunk, LV_EVENT_VALUE_CHANGED, NULL);

    lv_obj_t *panelWifi = lv_obj_create(panel);
    lv_obj_set_size(panelWifi, 100, 30);
    lv_obj_align(panelWifi, LV_ALIGN_BOTTOM_LEFT, 0, 0);
    lv_obj_clear_flag(panelWifi, LV_OBJ_FLAG_SCROLLABLE);

    btnWiFi = lv_btn_create(panelWifi);
    lv_obj_set_size(btnWiFi, 60, 50);
    lv_obj_align(btnWiFi, LV_ALIGN_LEFT_MID, 16, 0);
    lv_obj_add_event_cb(btnWiFi, app_settings_event_thunk, LV_EVENT_CLICKED, NULL);
    lv_obj_set_style_bg_opa(btnWiFi, 0, LV_PART_MAIN | LV_STATE_DEFAULT);

    lv_obj_t *wifiLabel = lv_label_create(btnWiFi);
    lv_label_set_text(wifiLabel, "WiFi");
    lv_obj_center(wifiLabel);

    imgBtnWiFi = lv_imgbtn_create(panelWifi);
    LV_IMG_DECLARE(icon_wifi);
    lv_imgbtn_set_src(imgBtnWiFi, LV_IMGBTN_STATE_RELEASED, NULL, &icon_wifi, NULL);
    lv_imgbtn_set_src(imgBtnWiFi, LV_IMGBTN_STATE_PRESSED, NULL, &icon_wifi, NULL);
    lv_imgbtn_set_src(imgBtnWiFi, LV_IMGBTN_STATE_CHECKED_PRESSED, NULL, &icon_wifi, NULL);
    lv_imgbtn_set_src(imgBtnWiFi, LV_IMGBTN_STATE_CHECKED_RELEASED, NULL, &icon_wifi, NULL);
    lv_obj_set_size(imgBtnWiFi, 20, 20);
    lv_obj_align(imgBtnWiFi, LV_ALIGN_LEFT_MID, -10, 0);
    lv_obj_add_flag(imgBtnWiFi, LV_OBJ_FLAG_CHECKABLE);
    lv_obj_add_event_cb(imgBtnWiFi, app_settings_event_thunk, LV_EVENT_CLICKED, NULL);

    _display->register_wifi_panel(panelWifi);

    ui_PanelCursor = lv_obj_create(panel);
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
    lv_obj_add_event_cb(ui_ImgBtnCursor, app_settings_event_thunk, LV_EVENT_CLICKED, NULL);
}

void AppSettings::event_handler(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t *target = lv_event_get_target(e);
    if (code == LV_EVENT_VALUE_CHANGED)
    {
        if (target == sliderBrightness)
        {
            int v = lv_slider_get_value(sliderBrightness);
            _display->set_brightness(v);
        }
        else if (target == sliderSpeaker)
        {
            int v = lv_slider_get_value(sliderSpeaker);
            _display->menu_event_cb(SET_AUDIO, reinterpret_cast<void *>(v));
        }
    }
    else if (code == LV_EVENT_CLICKED)
    {
        if (target == imgBtnWiFi)
        {
            if (lv_obj_get_state(imgBtnWiFi) & LV_STATE_CHECKED)
            {
                _display->menu_event_cb((Menu_Event_t)WIFI_ON, NULL);
            }
            else
            {
                _display->menu_event_cb((Menu_Event_t)WIFI_OFF, NULL);
            }
        }
        else if (target == btnWiFi)
        {
            _display->show_wifi_panel();
        }
    }
    else if ((target == ui_ImgBtnCursor && code == LV_EVENT_CLICKED) || (target == ui_PanelCursor && code == LV_EVENT_CLICKED))
    {
        _display->cursor_panel_active = !_display->cursor_panel_active;
        if (_display->cursor_panel_active)
        {
            lv_obj_set_style_bg_color(ui_PanelCursor, lv_color_hex(0x33bd33), LV_PART_MAIN | LV_STATE_DEFAULT); // #33bd33ff
            lv_obj_set_style_bg_color(ui_ImgBtnCursor, lv_color_hex(0x33bd33), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_clear_flag(AppSettings::instance->_display->cursor_obj, LV_OBJ_FLAG_HIDDEN);
        }
        else
        {
            lv_obj_set_style_bg_color(ui_PanelCursor, lv_color_hex(0xFFFFFF), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_bg_color(ui_ImgBtnCursor, lv_color_hex(0xFFFFFF), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_add_flag(AppSettings::instance->_display->cursor_obj, LV_OBJ_FLAG_HIDDEN);
        }
    }
}

void AppSettings::close_app()
{
    _display->goback_main_screen();
    lv_obj_del(_bodyScreen);
    delete this;
}
