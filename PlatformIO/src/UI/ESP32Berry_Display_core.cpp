/////////////////////////////////////////////////////////////////
/*
  Display core: init, flush, input devices, tasks
*/
/////////////////////////////////////////////////////////////////
#include "UI/ESP32Berry_Display.hpp"
#include <Arduino.h>
#include "../Configurations/secrets.h"
#include "../Utils/BusLock.hpp"

Display *g_display_instance = NULL;
static Display *instance = NULL;
extern "C" void my_mouse_read_thunk(lv_indev_drv_t *indev_driver, lv_indev_data_t *data);
extern "C" void wifi_event_cb_thunk(lv_event_t *e)
{
    instance->ui_wifi_event_callback(e);
}
extern "C" void textarea_event_cb_thunk(lv_event_t *e)
{
    instance->textarea_event_cb(e);
}
extern "C" void ui_event_callback_thunk(lv_event_t *e)
{
    instance->ui_event_callback(e);
}
extern "C" void ui_app_btns_callback_thunk(lv_event_t *e)
{
    instance->ui_app_btns_callback(e);
}

Display::Display(FuncPtrInt callback)
{
    instance = this;
    g_display_instance = this;
    spi_bus_init();
    tft = new LGFX();
    menu_event_cb = callback;
    ui_Focused_Obj = NULL;
    initTFT();
    last_input_tick = xTaskGetTickCount();
}

Display::~Display()
{
    delete tft;
}

void Display::initTFT()
{
    // Mouse Pin setup
    pinMode(BOARD_BOOT_PIN, INPUT_PULLUP);
    pinMode(BOARD_TBOX_G02, INPUT_PULLUP);
    pinMode(BOARD_TBOX_G01, INPUT_PULLUP);
    pinMode(BOARD_TBOX_G04, INPUT_PULLUP);
    pinMode(BOARD_TBOX_G03, INPUT_PULLUP);

    tft->begin();
    tft->setRotation(1);
    tft->fillScreen(TFT_BLACK);
    tft->setBrightness(240);
    this->initLVGL();
}

extern "C" void my_disp_flush_thunk(lv_disp_drv_t *drv, const lv_area_t *area, lv_color_t *color_p)
{
    instance->my_disp_flush(drv, area, color_p);
}

void Display::my_disp_flush(lv_disp_drv_t *disp, const lv_area_t *area, lv_color_t *color_p)
{
    uint32_t w = (area->x2 - area->x1 + 1);
    uint32_t h = (area->y2 - area->y1 + 1);

    // Try to acquire the SPI bus briefly; if busy (e.g., SD), wait a bit to avoid corruption
    if (!spi_bus_try_lock(pdMS_TO_TICKS(50)))
    {
        spi_bus_lock();
    }
    tft->startWrite();
    tft->setAddrWindow(area->x1, area->y1, w, h);
    tft->writePixels((lgfx::rgb565_t *)&color_p->full, w * h);
    tft->endWrite();
    spi_bus_unlock();

    lv_disp_flush_ready(disp);
}

void Display::my_touch_read(lv_indev_drv_t *indev_driver, lv_indev_data_t *data)
{
    uint16_t x, y;
    bool touched = false;
    if (spi_bus_try_lock(pdMS_TO_TICKS(10)))
    {
        touched = tft->getTouch(&x, &y);
        spi_bus_unlock();
    }
    else
    {
        touched = tft->getTouch(&x, &y);
    }
    if (touched)
    {
        data->state = LV_INDEV_STATE_PR;
        data->point.x = x;
        data->point.y = y;
        register_activity();
    }
    else
    {
        data->state = LV_INDEV_STATE_REL;
    }
}

bool isSetBrightnessRunning = false;
void Display::my_mouse_read(lv_indev_drv_t *indev_driver, lv_indev_data_t *data)
{
    if (cursor_panel_active == false)
        return;

    static int16_t last_x;
    static int16_t last_y;
    bool left_button_down = false;
    const uint8_t dir_pins[5] = {BOARD_TBOX_G02,
                                 BOARD_TBOX_G01,
                                 BOARD_TBOX_G04,
                                 BOARD_TBOX_G03,
                                 BOARD_BOOT_PIN};
    static bool last_dir[5];
    uint8_t pos = 10;
    for (int i = 0; i < 5; i++)
    {
        bool dir = digitalRead(dir_pins[i]);
        if (dir != last_dir[i])
        {
            last_dir[i] = dir;
            register_activity();
            switch (i)
            {
            case 0:
                if (last_x < (lv_disp_get_hor_res(NULL) - mouse_cursor_icon.header.w))
                {
                    last_x += pos;
                }
                break;
            case 1:
                if (last_y > mouse_cursor_icon.header.h)
                {
                    last_y -= pos;
                }
                break;
            case 2:
                if (last_x > mouse_cursor_icon.header.w)
                {
                    last_x -= pos;
                }
                break;
            case 3:
                if (last_y < (lv_disp_get_ver_res(NULL) - mouse_cursor_icon.header.h))
                {
                    last_y += pos;
                }
                break;
            case 4:

                if (tft->getBrightness() == 0 && !isSetBrightnessRunning)
                {
                    isSetBrightnessRunning = true;
                    for (int i = 0; i < 255; ++i)
                    {
                        tft->setBrightness(i);
                        lv_task_handler();
                        delay(1);
                    }
                    isSetBrightnessRunning = false;
                }
                else
                {
                    left_button_down = true;
                }
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
uint32_t Display::keypad_get_key(void)
{
    char key_ch = 0;
    Wire.requestFrom(0x55, 2);
    while (Wire.available() > 0)
    {
        key_ch = Wire.read();
    }

    return key_ch;
}

/*Will be called by the library to read the mouse*/
void Display::my_key_read(lv_indev_drv_t *indev_drv, lv_indev_data_t *data)
{
    static uint32_t last_key = 0;
    uint32_t act_key;
    act_key = keypad_get_key();
    if (act_key != 0)
    {
        Serial.println(data->key);
        Serial.println(data->continue_reading);

        data->state = LV_INDEV_STATE_PR;
        last_key = act_key;
        Serial.printf("Key: %d\n", last_key);
        HandleKeyboardShortcuts(last_key);
        register_activity();
    }
    else
    {
        data->state = LV_INDEV_STATE_REL;
    }
    data->key = last_key;
}

void Display::HandleKeyboardShortcuts(uint32_t key)
{
    if (key == 224) // sym+shift+space/shift+mic turn off display
    {
        for (int i = tft->getBrightness(); i >= 0; --i)
        {
            tft->setBrightness(i);
            lv_task_handler();
            delay(5);
        }
        screen_dimmed = true;
    }
    if (key == 4) // shift + speaker  toggle sound
    {
        int sliderValue = lv_slider_get_value(ui_SliderSpeaker);
        int volume = sliderValue > 0 ? 0 : 21;
        menu_event_cb(SET_AUDIO, reinterpret_cast<void *>(volume));
    }
}

extern "C" void my_touch_read_thunk(lv_indev_drv_t *indev_driver, lv_indev_data_t *data)
{
    instance->my_touch_read(indev_driver, data);
}

extern "C" void my_mouse_read_thunk(lv_indev_drv_t *indev_driver, lv_indev_data_t *data)
{
    instance->my_mouse_read(indev_driver, data);
}

extern "C" void my_key_read_thunk(lv_indev_drv_t *indev_driver, lv_indev_data_t *data)
{
    instance->my_key_read(indev_driver, data);
}

void update_ui_task(void *pvParameters)
{
    while (1)
    {
        xSemaphoreTake(instance->bin_sem, portMAX_DELAY);
        lv_task_handler();
        xSemaphoreGive(instance->bin_sem);
        vTaskDelay(5);
    }
}

void Display::initLVGL()
{

    static lv_disp_draw_buf_t draw_buf;

#ifndef BOARD_HAS_PSRAM
#define LVGL_BUFFER_SIZE (TFT_HEIGHT * 100)
    static lv_color_t buf[LVGL_BUFFER_SIZE];
#else
#define LVGL_BUFFER_SIZE (TFT_WIDTH * TFT_HEIGHT * sizeof(lv_color_t))
    static lv_color_t *buf = (lv_color_t *)ps_malloc(LVGL_BUFFER_SIZE);
    if (!buf)
    {
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
    // Wire the trackball read callback so LVGL can receive cursor movement
    indev_mouse.read_cb = my_mouse_read_thunk;
    lv_indev_t *mouse_indev = lv_indev_drv_register(&indev_mouse);
    lv_indev_set_group(mouse_indev, lv_group_get_default());

    cursor_obj = lv_img_create(lv_scr_act());
    lv_img_set_src(cursor_obj, &mouse_cursor_icon);
    lv_indev_set_cursor(mouse_indev, cursor_obj);
    lv_obj_add_flag(cursor_obj, LV_OBJ_FLAG_HIDDEN);

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
    ui_settings();
    ui_prep_loading();
    // ui_prep_popup_box();

    xTaskCreatePinnedToCore(update_ui_task,
                            "update_ui_task",
                            10000,
                            NULL,
                            1,
                            &lvgl_task_handle,
                            0);

    // Inactivity monitor task
    xTaskCreatePinnedToCore(Display::inactivity_task,
                            "inactivity_task",
                            4096,
                            this,
                            1,
                            NULL,
                            1);

    // this->ui_popup_open("Welcome to ESP32Berry Project!", "This project aims to develop useful applications based on the T-Deck device. Let's do a fun project together!\n\n(Version 0.5)");
}

// === Inactivity / Screen Timeout Implementation ===

void Display::register_activity()
{
    last_input_tick = xTaskGetTickCount();
    if (screen_dimmed)
    {
        // Wake display gradually
        fade_backlight_to(previous_brightness, 8, 4);
        screen_dimmed = false;
    }
}

void Display::set_screen_timeout(int minutes)
{
    if (minutes < 0)
        minutes = 0;
    screen_timeout_minutes = minutes;
    register_activity();
}

void Display::fade_backlight_to(uint8_t target, uint8_t step, uint16_t delay_ms)
{
    uint8_t current = tft->getBrightness();
    if (current == target)
        return;
    if (current < target)
    {
        for (int v = current; v <= target; v += step)
        {
            tft->setBrightness(v);
            vTaskDelay(pdMS_TO_TICKS(delay_ms));
        }
    }
    else
    {
        for (int v = current; v >= (int)target; v -= step)
        {
            tft->setBrightness(v);
            vTaskDelay(pdMS_TO_TICKS(delay_ms));
        }
    }
    tft->setBrightness(target);
}

void Display::inactivity_task(void *param)
{
    Display *self = static_cast<Display *>(param);
    const TickType_t check_delay = pdMS_TO_TICKS(1000);
    while (true)
    {
        vTaskDelay(check_delay);
        int timeout = self->screen_timeout_minutes;
        if (timeout <= 0)
            continue; // Disabled
        if (self->screen_dimmed)
            continue; // Already dimmed
        uint32_t now = xTaskGetTickCount();
        uint32_t elapsed_ms = (now - self->last_input_tick) * portTICK_PERIOD_MS;
        if (elapsed_ms >= (uint32_t)timeout * 60000UL)
        {
            self->previous_brightness = self->tft->getBrightness();
            self->fade_backlight_to(0, 4, 8);
            self->screen_dimmed = true;
        }
    }
}
