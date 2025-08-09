/////////////////////////////////////////////////////////////////
/*
  Main Screen construction
*/
/////////////////////////////////////////////////////////////////
#include "UI/ESP32Berry_Display.hpp"
#include "../../Configurations/secrets.h"

extern "C" void ui_app_btns_callback_thunk(lv_event_t *e);
extern "C" void ui_event_callback_thunk(lv_event_t *e);

void Display::ui_main()
{

    LV_IMG_DECLARE(img_background);
    LV_IMG_DECLARE(icon_chatgpt);
    LV_IMG_DECLARE(icon_settings);

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
    lv_obj_add_flag(ui_TopPanel, LV_OBJ_FLAG_CLICKABLE);
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

    for (int i = 0; i < 5; i++)
    {
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

        if (i == 0 || i == 4)
        {
            ui_btn_icon = lv_img_create(obj);
            if (i == 0)
            {
                lv_img_set_src(ui_btn_icon, &icon_chatgpt);
            }
            else
            { // settings button
                lv_img_set_src(ui_btn_icon, &icon_settings);
            }
            lv_obj_set_width(ui_btn_icon, LV_SIZE_CONTENT);
            lv_obj_set_height(ui_btn_icon, LV_SIZE_CONTENT);
            lv_obj_set_align(ui_btn_icon, LV_ALIGN_CENTER);
            lv_obj_add_flag(ui_btn_icon, LV_OBJ_FLAG_ADV_HITTEST);
            lv_obj_clear_flag(ui_btn_icon, LV_OBJ_FLAG_SCROLLABLE);
        }
    }

    lv_disp_load_scr(ui_Main_Screen);
}
