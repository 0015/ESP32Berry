/////////////////////////////////////////////////////////////////
/*
  ESP32 VNC Viewer
  For More Information: https://youtu.be/WuPIX3qxg4k
  Created by Eric N. (ThatProject)
*/
/////////////////////////////////////////////////////////////////

#pragma once
#include "VNC_config.h"
#include "VNC.h"
#define LGFX_USE_V1
#include <LovyanGFX.hpp>

class LGFX : public lgfx::LGFX_Device {
  lgfx::Panel_ILI9488 _panel_instance;
  lgfx::Bus_SPI _bus_instance;
  lgfx::Touch_XPT2046 _touch_instance;

public:
  LGFX(void) {
    {
      auto cfg = _bus_instance.config();
      cfg.spi_host = VSPI_HOST;
      cfg.spi_mode = 0;
      cfg.freq_write = 40000000;
      cfg.freq_read = 16000000;
      cfg.spi_3wire = false;
      cfg.use_lock = true;
      cfg.dma_channel = 1;
      cfg.pin_sclk = 18;
      cfg.pin_mosi = 23;
      cfg.pin_miso = -1;
      cfg.pin_dc = 14;
      _bus_instance.config(cfg);
      _panel_instance.setBus(&_bus_instance);
    }

    {
      auto cfg = _panel_instance.config();

      cfg.pin_cs = 13;
      cfg.pin_rst = 12;
      cfg.pin_busy = -1;
      cfg.memory_width = 320;
      cfg.memory_height = 480;
      cfg.panel_width = 320;
      cfg.panel_height = 480;
      cfg.offset_x = 0;
      cfg.offset_y = 0;
      cfg.offset_rotation = 2;
      cfg.dummy_read_pixel = 8;
      cfg.dummy_read_bits = 1;
      cfg.readable = true;
      cfg.invert = false;
      cfg.rgb_order = false;
      cfg.dlen_16bit = false;
      cfg.bus_shared = true;

      _panel_instance.config(cfg);
    }
    {
      auto cfg = _touch_instance.config();
      cfg.x_min = 0;
      cfg.x_max = 319;
      cfg.y_min = 0;
      cfg.y_max = 479;
      cfg.pin_int = -1;
      cfg.bus_shared = true;
      cfg.offset_rotation = 0;
      cfg.spi_host = VSPI_HOST;
      cfg.freq = 1000000;
      cfg.pin_sclk = 18;
      cfg.pin_mosi = 23;
      cfg.pin_miso = 19;
      cfg.pin_cs = 4;
      _touch_instance.config(cfg);
      _panel_instance.setTouch(&_touch_instance);
    }

    setPanel(&_panel_instance);
  }
};

class VNCDriver : public VNCdisplay {
public:

  VNCDriver(LGFX* lgfx);
  ~VNCDriver();

  bool hasCopyRect(void);
  uint32_t getHeight(void);
  uint32_t getWidth(void);
  void draw_area(uint32_t x, uint32_t y, uint32_t w, uint32_t h, uint8_t* data);
  void draw_rect(uint32_t x, uint32_t y, uint32_t w, uint32_t h, uint16_t color);
  void copy_rect(uint32_t src_x, uint32_t src_y, uint32_t dest_x, uint32_t dest_y, uint32_t w, uint32_t h);
  void area_update_start(uint32_t x, uint32_t y, uint32_t w, uint32_t h);
  void area_update_data(char* data, uint32_t pixel);
  void area_update_end(void);

  void vnc_options_override(dfb_vnc_options* opt);
  void print_screen(String title, String msg, int color);
  void print(String text);

private:
  LGFX* _lcd;
};