/////////////////////////////////////////////////////////////////
/*
  New ESP32Berry Project, The base UI & ChatGPT Client
  For More Information: https://youtu.be/5K6rSw9j5iY
  Created by Eric N. (ThatProject)
*/
/////////////////////////////////////////////////////////////////
#define LGFX_USE_V1
#include <LovyanGFX.hpp>

static const uint32_t TFT_WIDTH = 240;
static const uint32_t TFT_HEIGHT = 320;

class LGFX : public lgfx::LGFX_Device {
  lgfx::Panel_ST7789 _panel_instance;
  lgfx::Bus_SPI _bus_instance;
  lgfx::Light_PWM _light_instance;
  lgfx::Touch_GT911 _touch_instance;

public:
  LGFX(void) {
    {
      auto cfg = _bus_instance.config();
      cfg.spi_host = SPI2_HOST;
      cfg.spi_mode = 3;
      cfg.freq_write = 80000000;
      cfg.freq_read = 16000000;
      cfg.spi_3wire = false;
      cfg.use_lock = true;
      cfg.dma_channel = SPI_DMA_CH_AUTO;
      cfg.pin_sclk = 40;
      cfg.pin_mosi = 41;
      cfg.pin_miso = 38;
      cfg.pin_dc = 11;
      _bus_instance.config(cfg);
      _panel_instance.setBus(&_bus_instance);
    }

    {
      auto cfg = _panel_instance.config();
      cfg.pin_cs = 12;
      cfg.pin_rst = -1;
      cfg.pin_busy = -1;
      cfg.memory_width = TFT_WIDTH;
      cfg.memory_height = TFT_HEIGHT;
      cfg.panel_width = TFT_WIDTH;
      cfg.panel_height = TFT_HEIGHT;
      cfg.offset_x = 0;
      cfg.offset_y = 0;
      cfg.offset_rotation = 0;
      cfg.dummy_read_pixel = 8;
      cfg.dummy_read_bits = 1;
      cfg.readable = true;
      cfg.invert = true;
      cfg.rgb_order = false;
      cfg.dlen_16bit = false;
      cfg.bus_shared = true;

      _panel_instance.config(cfg);
    }

    {
      auto cfg = _light_instance.config();
      cfg.pin_bl = 42;
      cfg.invert = false;
      cfg.freq = 44100;
      cfg.pwm_channel = 7;

      _light_instance.config(cfg);
      _panel_instance.setLight(&_light_instance);
    }

     {
      auto cfg = _touch_instance.config();
      cfg.i2c_port = 0;
      cfg.i2c_addr = 0x5D;
      cfg.pin_int    = 16;   
      cfg.bus_shared = false; 
      cfg.pin_sda = 18;
      cfg.pin_scl =8;
      cfg.freq = 400000;
      cfg.x_min = 0;
      cfg.x_max = TFT_WIDTH-1;
      cfg.y_min = 0;
      cfg.y_max = TFT_HEIGHT-1;

      _touch_instance.config(cfg);
      _panel_instance.setTouch(&_touch_instance);
    }

    setPanel(&_panel_instance);
  }
};