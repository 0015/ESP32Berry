/**
 * Updated: 05-23-2024
 * Unit Test based on LvoaynGFX instead of TFT_eSPI
  
 *Note*
 * https://github.com/Xinyuan-LilyGO/T-Deck
 * Refer to the official GitHub and prepare the necessary libraries first, then try this.

 * ESP32: 2.0.17
 * https://github.com/espressif/arduino-esp32

 * LovaynGFX: 1.1.12
 * https://github.com/lovyan03/LovyanGFX

 * LVGL: 8.4.0
 * https://github.com/lvgl/lvgl
 * lv_conf.h 
 # Line#15, #if 1
 * #define LV_COLOR_16_SWAP 1
 * #define LV_TICK_CUSTOM 1
 * #define LV_FONT_MONTSERRAT_28 1

 * Arduino Setting, Menu Tools ->
 * Board:"ESP32S3 Dev Module"
 * USB CDC On Boot:"Enable"
 * USB DFU On Boot:"Disable"
 * Flash Size : "16MB(128Mb)"
 * Flash Mode"QIO 80MHz
 * Partition Scheme:"16M Flash(3M APP/9.9MB FATFS)"
 * PSRAM:"OPI PSRAM"
 * Upload Mode:"UART0/Hardware CDC"
 * USB Mode:"Hardware CDC and JTAG"
*/

/**
 * @file      UnitTest.ino
 * @author    Lewis He (lewishe@outlook.com)
 * @license   MIT
 * @copyright Copyright (c) 2023  Shenzhen Xin Yuan Electronic Technology Co., Ltd
 * @date      2023-04-11
 * @note      Arduino Setting
 *            Tools ->
 * Board:"ESP32S3 Dev Module"
 * USB CDC On Boot:"Enable"
 * USB DFU On Boot:"Disable"
 * Flash Size : "16MB(128Mb)"
 * Flash Mode"QIO 80MHz
 * Partition Scheme:"16M Flash(3M APP/9.9MB FATFS)"
 * PSRAM:"OPI PSRAM"
 * Upload Mode:"UART0/Hardware CDC"
 * USB Mode:"Hardware CDC and JTAG"
 */

#include <Arduino.h>
#include <SPI.h>
#include <RadioLib.h>
#include <lvgl.h>
#include <SD.h>
#include "es7210.h"
#include <Audio.h>
#include <driver/i2s.h>
#include "LGFX_T-Deck.h"

// By default, the audio pass-through speaker is used for testing, and esp_sr can also be used for noise detection.
// #define USE_ESP_VAD

#define TOUCH_MODULES_GT911
#include "TouchLib.h"
#include "utilities.h"

#ifndef BOARD_HAS_PSRAM
#error "Detected that PSRAM is not turned on. Please set PSRAM to OPI PSRAM in ArduinoIDE"
#endif

#ifndef RADIO_FREQ
#define RADIO_FREQ 868.0
#endif

#define DEFAULT_COLOR (lv_color_make(252, 218, 72))
#define MIC_I2S_SAMPLE_RATE 16000
#define MIC_I2S_PORT I2S_NUM_1
#define SPK_I2S_PORT I2S_NUM_0
#define VAD_SAMPLE_RATE_HZ 16000
#define VAD_FRAME_LENGTH_MS 30
#define VAD_BUFFER_LENGTH (VAD_FRAME_LENGTH_MS * VAD_SAMPLE_RATE_HZ / 1000)
#define LVGL_BUFFER_SIZE (TFT_WIDTH * TFT_HEIGHT * sizeof(lv_color_t))

LV_IMG_DECLARE(image_output);

LV_IMG_DECLARE(image);
LV_IMG_DECLARE(image1);
LV_IMG_DECLARE(image2);
LV_IMG_DECLARE(image3);
LV_IMG_DECLARE(image4);
LV_IMG_DECLARE(image_emoji);


enum DemoEvent {
  DEMO_TX_BTN_CLICK_EVENT,
  DEMO_RX_BTN_CLICK_EVENT,
  DEMO_CLEAN_BTN_CLICK_EVENT,
  DEMO_VAD_BTN_CLICK_EVENT,
  DEMO_PLAY_BTN_CLICK_EVENT,
  DEMO_SLEEP_BTN_CLICK_EVENT
};


static const DemoEvent event[] = {
  DEMO_TX_BTN_CLICK_EVENT,
  DEMO_RX_BTN_CLICK_EVENT,
  DEMO_CLEAN_BTN_CLICK_EVENT,
  DEMO_VAD_BTN_CLICK_EVENT,
  DEMO_PLAY_BTN_CLICK_EVENT,
  DEMO_SLEEP_BTN_CLICK_EVENT,
};

#ifdef USE_ESP_VAD
#include <esp_vad.h>
int16_t *vad_buff;
vad_handle_t vad_inst;
const size_t vad_buffer_size = VAD_BUFFER_LENGTH * sizeof(short);
#else
uint16_t loopbackBuffer[3200] = { 0 };
#endif

TouchLib *touch = NULL;
SX1262 radio = new Module(RADIO_CS_PIN, RADIO_DIO1_PIN, RADIO_RST_PIN, RADIO_BUSY_PIN);
LGFX tft;
Audio audio;
size_t bytes_read;
uint8_t status;
TaskHandle_t playHandle = NULL;
TaskHandle_t radioHandle = NULL;

static lv_obj_t *vad_btn_label;
static uint32_t vad_detected_counter = 0;
static TaskHandle_t vadTaskHandler;
bool transmissionFlag = true;
bool enableInterrupt = true;
int transmissionState;
bool hasRadio = false;
bool touchDected = false;
bool kbDected = false;
bool sender = true;
bool enterSleep = false;
uint32_t sendCount = 0;
uint32_t runningMillis = 0;
uint8_t touchAddress = GT911_SLAVE_ADDRESS2;

lv_indev_t *kb_indev = NULL;
lv_indev_t *mouse_indev = NULL;
lv_indev_t *touch_indev = NULL;
lv_group_t *kb_indev_group;
lv_obj_t *radio_ta;
lv_obj_t *main_count;
SemaphoreHandle_t xSemaphore = NULL;

void setupLvgl();

// LilyGo  T-Deck  control backlight chip has 16 levels of adjustment range
// The adjustable range is 0~15, 0 is the minimum brightness, 15 is the maximum brightness
void setBrightness(uint8_t value) {
  static uint8_t level = 0;
  static uint8_t steps = 16;
  if (value == 0) {
    digitalWrite(BOARD_BL_PIN, 0);
    delay(3);
    level = 0;
    return;
  }
  if (level == 0) {
    digitalWrite(BOARD_BL_PIN, 1);
    level = steps;
    delayMicroseconds(30);
  }
  int from = steps - level;
  int to = steps - value;
  int num = (steps + to - from) % steps;
  for (int i = 0; i < num; i++) {
    digitalWrite(BOARD_BL_PIN, 0);
    digitalWrite(BOARD_BL_PIN, 1);
  }
  level = value;
}

void setFlag(void) {
  // check if the interrupt is enabled
  if (!enableInterrupt) {
    return;
  }
  // we got a packet, set the flag
  transmissionFlag = true;
}

void scanDevices(TwoWire *w) {
  uint8_t err, addr;
  int nDevices = 0;
  uint32_t start = 0;
  for (addr = 1; addr < 127; addr++) {
    start = millis();
    w->beginTransmission(addr);
    delay(2);
    err = w->endTransmission();
    if (err == 0) {
      nDevices++;
      Serial.print("I2C device found at address 0x");
      if (addr < 16) {
        Serial.print("0");
      }
      Serial.print(addr, HEX);
      Serial.println(" !");

      if (addr == GT911_SLAVE_ADDRESS2) {
        touchAddress = GT911_SLAVE_ADDRESS2;
        Serial.println("Find GT911 Drv Slave address: 0x14");
      } else if (addr == GT911_SLAVE_ADDRESS1) {
        touchAddress = GT911_SLAVE_ADDRESS1;
        Serial.println("Find GT911 Drv Slave address: 0x5D");
      }
    } else if (err == 4) {
      Serial.print("Unknow error at address 0x");
      if (addr < 16) {
        Serial.print("0");
      }
      Serial.println(addr, HEX);
    }
  }
  if (nDevices == 0)
    Serial.println("No I2C devices found\n");
}

bool setupRadio() {
  digitalWrite(BOARD_SDCARD_CS, HIGH);
  digitalWrite(RADIO_CS_PIN, HIGH);
  digitalWrite(BOARD_TFT_CS, HIGH);
  SPI.end();
  SPI.begin(BOARD_SPI_SCK, BOARD_SPI_MISO, BOARD_SPI_MOSI);  //SD

  int state = radio.begin(RADIO_FREQ);
  if (state == RADIOLIB_ERR_NONE) {
    Serial.println("Start Radio success!");
  } else {
    Serial.print("Start Radio failed,code:");
    Serial.println(state);
    return false;
  }

  hasRadio = true;

  // set carrier frequency to 868.0 MHz
  if (radio.setFrequency(RADIO_FREQ) == RADIOLIB_ERR_INVALID_FREQUENCY) {
    Serial.println(F("Selected frequency is invalid for this module!"));
    return false;
  }

  // set bandwidth to 125 kHz
  if (radio.setBandwidth(125.0) == RADIOLIB_ERR_INVALID_BANDWIDTH) {
    Serial.println(F("Selected bandwidth is invalid for this module!"));
    return false;
  }

  // set spreading factor to 10
  if (radio.setSpreadingFactor(10) == RADIOLIB_ERR_INVALID_SPREADING_FACTOR) {
    Serial.println(F("Selected spreading factor is invalid for this module!"));
    return false;
  }

  // set coding rate to 6
  if (radio.setCodingRate(6) == RADIOLIB_ERR_INVALID_CODING_RATE) {
    Serial.println(F("Selected coding rate is invalid for this module!"));
    return false;
  }

  // set LoRa sync word to 0xAB
  if (radio.setSyncWord(0xAB) != RADIOLIB_ERR_NONE) {
    Serial.println(F("Unable to set sync word!"));
    return false;
  }

  // set output power to 10 dBm (accepted range is -17 - 22 dBm)
  if (radio.setOutputPower(17) == RADIOLIB_ERR_INVALID_OUTPUT_POWER) {
    Serial.println(F("Selected output power is invalid for this module!"));
    return false;
  }

  // set over current protection limit to 140 mA (accepted range is 45 - 140 mA)
  // NOTE: set value to 0 to disable overcurrent protection
  if (radio.setCurrentLimit(140) == RADIOLIB_ERR_INVALID_CURRENT_LIMIT) {
    Serial.println(F("Selected current limit is invalid for this module!"));
    return false;
  }

  // set LoRa preamble length to 15 symbols (accepted range is 0 - 65535)
  if (radio.setPreambleLength(15) == RADIOLIB_ERR_INVALID_PREAMBLE_LENGTH) {
    Serial.println(F("Selected preamble length is invalid for this module!"));
    return false;
  }

  // disable CRC
  if (radio.setCRC(false) == RADIOLIB_ERR_INVALID_CRC_CONFIGURATION) {
    Serial.println(F("Selected CRC is invalid for this module!"));
    return false;
  }

  // set the function that will be called
  // when new packet is received
  radio.setDio1Action(setFlag);
  return true;
}

bool setupSD() {
  digitalWrite(BOARD_SDCARD_CS, HIGH);
  digitalWrite(RADIO_CS_PIN, HIGH);
  digitalWrite(BOARD_TFT_CS, HIGH);

  if (SD.begin(BOARD_SDCARD_CS, SPI, 800000U)) {
    uint8_t cardType = SD.cardType();
    if (cardType == CARD_NONE) {
      Serial.println("No SD_MMC card attached");
      return false;
    } else {
      Serial.print("SD_MMC Card Type: ");
      if (cardType == CARD_MMC) {
        Serial.println("MMC");
      } else if (cardType == CARD_SD) {
        Serial.println("SDSC");
      } else if (cardType == CARD_SDHC) {
        Serial.println("SDHC");
      } else {
        Serial.println("UNKNOWN");
      }
      uint32_t cardSize = SD.cardSize() / (1024 * 1024);
      uint32_t cardTotal = SD.totalBytes() / (1024 * 1024);
      uint32_t cardUsed = SD.usedBytes() / (1024 * 1024);
      Serial.printf("SD Card Size: %lu MB\n", cardSize);
      Serial.printf("Total space: %lu MB\n", cardTotal);
      Serial.printf("Used space: %lu MB\n", cardUsed);
      return true;
    }
  }
  return false;
}

bool setupCoder() {
  uint32_t ret_val = ESP_OK;

  Wire.beginTransmission(ES7210_ADDR);
  uint8_t error = Wire.endTransmission();
  if (error != 0) {
    Serial.println("ES7210 address not found");
    return false;
  }

  audio_hal_codec_config_t cfg = {
    .adc_input = AUDIO_HAL_ADC_INPUT_ALL,
    .codec_mode = AUDIO_HAL_CODEC_MODE_ENCODE,
    .i2s_iface = {
      .mode = AUDIO_HAL_MODE_SLAVE,
      .fmt = AUDIO_HAL_I2S_NORMAL,
      .samples = AUDIO_HAL_16K_SAMPLES,
      .bits = AUDIO_HAL_BIT_LENGTH_16BITS,
    },
  };

  ret_val |= es7210_adc_init(&Wire, &cfg);
  ret_val |= es7210_adc_config_i2s(cfg.codec_mode, &cfg.i2s_iface);
  ret_val |= es7210_adc_set_gain(
    (es7210_input_mics_t)(ES7210_INPUT_MIC1 | ES7210_INPUT_MIC2),
    (es7210_gain_value_t)GAIN_6DB);
  ret_val |= es7210_adc_ctrl_state(cfg.codec_mode, AUDIO_HAL_CTRL_START);
  return ret_val == ESP_OK;
}

void taskplaySong(void *p) {
  while (1) {
    if (xSemaphoreTake(xSemaphore, portMAX_DELAY) == pdTRUE) {
      playTTS("hello.mp3");
      xSemaphoreGive(xSemaphore);
    }
    vTaskSuspend(NULL);
  }
}

void loopRadio() {
  if (!hasRadio) {
    // lv_textarea_set_text(radio_ta, "Radio not online !");
    return;
  }
  if (xSemaphoreTake(xSemaphore, portMAX_DELAY) == pdTRUE) {
    digitalWrite(BOARD_SDCARD_CS, HIGH);
    digitalWrite(RADIO_CS_PIN, HIGH);
    digitalWrite(BOARD_TFT_CS, HIGH);

    char buf[256];

    if (strlen(lv_textarea_get_text(radio_ta)) >= lv_textarea_get_max_length(radio_ta)) {
      lv_textarea_set_text(radio_ta, "");
    }

    if (sender) {
      // Send data every 200 ms
      if (millis() - runningMillis > 1000) {
        // check if the previous transmission finished
        if (transmissionFlag) {
          // disable the interrupt service routine while
          // processing the data
          enableInterrupt = false;
          // reset flag
          transmissionFlag = false;

          if (transmissionState == RADIOLIB_ERR_NONE) {
            // packet was successfully sent
            Serial.println(F("transmission finished!"));
            // NOTE: when using interrupt-driven transmit method,
            //       it is not possible to automatically measure
            //       transmission data rate using getDataRate()
          } else {
            Serial.print(F("failed, code "));
            Serial.println(transmissionState);
          }

          snprintf(buf, 256, "[ %u ]TX %u finished\n", millis() / 1000, sendCount);
          lv_textarea_set_text(radio_ta, buf);
          lv_obj_add_state(radio_ta, LV_STATE_FOCUSED);

          Serial.println(buf);

          // you can also transmit byte array up to 256 bytes long
          transmissionState = radio.startTransmit(String(sendCount++).c_str());

          // we're ready to send more packets,
          // enable interrupt service routine
          enableInterrupt = true;
        }
        // snprintf(dispSenderBuff, sizeof(dispSenderBuff), "TX: %u", sendCount);

        runningMillis = millis();
      }
    } else {

      String recv;

      // check if the flag is set
      if (transmissionFlag) {
        // disable the interrupt service routine while
        // processing the data
        enableInterrupt = false;

        // reset flag
        transmissionFlag = false;

        // you can read received data as an Arduino String
        // int state = radio.readData(recv);

        // you can also read received data as byte array
        /*
                */
        int state = radio.readData(recv);
        if (state == RADIOLIB_ERR_NONE) {


          // packet was successfully received
          Serial.print(F("[RADIO] Received packet!"));

          // print data of the packet
          Serial.print(F(" Data:"));
          Serial.print(recv);

          // print RSSI (Received Signal Strength Indicator)
          Serial.print(F(" RSSI:"));
          Serial.print(radio.getRSSI());
          Serial.print(F(" dBm"));
          // snprintf(dispRecvicerBuff[1], sizeof(dispRecvicerBuff[1]), "RSSI:%.2f dBm", radio.getRSSI());

          // print SNR (Signal-to-Noise Ratio)
          Serial.print(F("  SNR:"));
          Serial.print(radio.getSNR());
          Serial.println(F(" dB"));


          snprintf(buf, 256, "RX:%s RSSI:%.2f SNR:%.2f\n", recv.c_str(), radio.getRSSI(), radio.getSNR());

          lv_textarea_set_text(radio_ta, buf);
          lv_obj_add_state(radio_ta, LV_STATE_FOCUSED);

        } else if (state == RADIOLIB_ERR_CRC_MISMATCH) {
          // packet was received, but is malformed
          Serial.println(F("CRC error!"));

        } else {
          // some other error occurred
          Serial.print(F("failed, code "));
          Serial.println(state);
        }
        // put module back to listen mode
        radio.startReceive();

        // we're ready to receive more packets,
        // enable interrupt service routine
        enableInterrupt = true;
      }
    }
    xSemaphoreGive(xSemaphore);
  }
}

void serialToScreen(lv_obj_t *parent, String string, bool result) {
  lv_obj_t *cont = lv_obj_create(parent);
  lv_obj_set_scroll_dir(cont, LV_DIR_NONE);
  lv_obj_set_size(cont, LV_PCT(100), lv_font_get_line_height(&lv_font_montserrat_28) + 2);

  lv_obj_t *label1 = lv_label_create(cont);
  lv_label_set_recolor(label1, true);
  lv_label_set_text(label1, string.c_str());
  lv_obj_align(label1, LV_ALIGN_LEFT_MID, 0, 0);

  lv_obj_t *label = lv_label_create(cont);
  lv_label_set_recolor(label, true);
  lv_label_set_text(label, result ? "#FFFFFF [# #00ff00 PASS# #FFFFFF ]#" : "#FFFFFF [# #ff0000  FAIL# #FFFFFF ]#");
  lv_obj_align(label, LV_ALIGN_RIGHT_MID, 0, 0);

  lv_obj_scroll_to_y(parent, lv_disp_get_ver_res(NULL), LV_ANIM_ON);

  int i = 200;
  while (i--) {
    lv_task_handler();
    delay(1);
  }
}

static bool getTouch(int16_t &x, int16_t &y);

bool checkKb() {
  int retry = 3;
  do {
    Wire.requestFrom(0x55, 1);
    if (Wire.read() != -1) {
      return true;
    }
  } while (retry--);
  return false;
}

#ifdef USE_ESP_VAD
void vadTask(void *params) {
  Serial.println("vadTask(void *params)");

  vTaskSuspend(NULL);
  while (1) {
    size_t read_len = 0;
    if (i2s_read(MIC_I2S_PORT, (char *)vad_buff, vad_buffer_size, &read_len, portMAX_DELAY) == ESP_OK) {
      // if (watch.readMicrophone((char *) vad_buff, vad_buffer_size, &read_len)) {
      // Feed samples to the VAD process and get the result
#if ESP_IDF_VERSION_VAL(4, 4, 1) == ESP_IDF_VERSION
      vad_state_t vad_state = vad_process(vad_inst, vad_buff);
#elif ESP_IDF_VERSION >= ESP_IDF_VERSION_VAL(4, 4, 1) && ESP_IDF_VERSION < ESP_IDF_VERSION_VAL(5, 0, 0)
      vad_state_t vad_state = vad_process(vad_inst, vad_buff, MIC_I2S_SAMPLE_RATE, VAD_FRAME_LENGTH_MS);
#else
#error "ESP VAD Not support Version > V5.0.0 , please use IDF V4.4.4"
#endif
      if (vad_state == VAD_SPEECH) {
        Serial.print(millis());
        Serial.println(" -> Noise detected!!!");
        if (vad_btn_label) {
          lv_label_set_text_fmt(vad_btn_label, "Noise %u", vad_detected_counter++);
        }
      }
    }
    delay(5);
  }
}

#else

void audioLoopbackTask(void *params) {
  vTaskSuspend(NULL);
  while (1) {
    delay(5);
    size_t bytes_read = 0, bytes_write = 0;
    memset(loopbackBuffer, 0, sizeof(loopbackBuffer));
    i2s_read(MIC_I2S_PORT, loopbackBuffer, sizeof(loopbackBuffer), &bytes_read, 15);
    if (bytes_read) {
      i2s_write(SPK_I2S_PORT, loopbackBuffer, bytes_read, &bytes_write, 15);
    }
  }
  vTaskDelete(NULL);
}

#endif

void setupMicrophoneI2S(i2s_port_t i2s_ch) {
  i2s_config_t i2s_config = {
    .mode = (i2s_mode_t)(I2S_MODE_MASTER | I2S_MODE_RX),
    .sample_rate = MIC_I2S_SAMPLE_RATE,
    .bits_per_sample = I2S_BITS_PER_SAMPLE_16BIT,
    .channel_format = I2S_CHANNEL_FMT_ALL_LEFT,
    .communication_format = I2S_COMM_FORMAT_STAND_I2S,
    .intr_alloc_flags = ESP_INTR_FLAG_LEVEL1,
    .dma_buf_count = 8,
    .dma_buf_len = 64,
    .use_apll = false,
    .tx_desc_auto_clear = true,
    .fixed_mclk = 0,
    .mclk_multiple = I2S_MCLK_MULTIPLE_256,
    .bits_per_chan = I2S_BITS_PER_CHAN_16BIT,
    .chan_mask = (i2s_channel_t)(I2S_TDM_ACTIVE_CH0 | I2S_TDM_ACTIVE_CH1 | I2S_TDM_ACTIVE_CH2 | I2S_TDM_ACTIVE_CH3),
    .total_chan = 4,
  };
  i2s_pin_config_t pin_config = { 0 };
  pin_config.data_in_num = BOARD_ES7210_DIN;
  pin_config.mck_io_num = BOARD_ES7210_MCLK;
  pin_config.bck_io_num = BOARD_ES7210_SCK;
  pin_config.ws_io_num = BOARD_ES7210_LRCK;
  pin_config.data_out_num = -1;
  i2s_driver_install(i2s_ch, &i2s_config, 0, NULL);
  i2s_set_pin(i2s_ch, &pin_config);
  i2s_zero_dma_buffer(i2s_ch);

#ifdef USE_ESP_VAD
  // Initialize esp-sr vad detected
#if ESP_IDF_VERSION_VAL(4, 4, 1) == ESP_IDF_VERSION
  vad_inst = vad_create(VAD_MODE_0, MIC_I2S_SAMPLE_RATE, VAD_FRAME_LENGTH_MS);
#elif ESP_IDF_VERSION >= ESP_IDF_VERSION_VAL(4, 4, 1)
  vad_inst = vad_create(VAD_MODE_0);
#else
#error "No support this version."
#endif
  vad_buff = (int16_t *)ps_malloc(vad_buffer_size);
  if (vad_buff == NULL) {
    while (1) {
      Serial.println("Memory allocation failed!");
      delay(1000);
    }
  }
  xTaskCreate(vadTask, "vad", 8 * 1024, NULL, 12, &vadTaskHandler);
#else
  xTaskCreate(audioLoopbackTask, "vad", 8 * 1024, NULL, 12, &vadTaskHandler);
#endif
}

void playTTS(const char *filename) {
  vTaskSuspend(vadTaskHandler);
#ifdef USE_ESP_VAD
  lv_label_set_text(vad_btn_label, "VAD detect");
#else
  lv_label_set_text(vad_btn_label, "loopback");
#endif

  bool findMp3 = false;
  if (SD.exists("/" + String(filename))) {
    findMp3 = audio.connecttoFS(SD, filename);
  } else if (SPIFFS.exists("/" + String(filename))) {
    findMp3 = audio.connecttoFS(SPIFFS, filename);
  }
  if (findMp3) {
    while (audio.isRunning()) {
      audio.loop();
      delay(3);
    }
  }
}

void setupAmpI2S(i2s_port_t i2s_ch) {
  audio.setPinout(BOARD_I2S_BCK, BOARD_I2S_WS, BOARD_I2S_DOUT);
  audio.setVolume(21);
}

void lv_button_event_cb(lv_event_t *e) {
  static uint8_t btn_index = 0;
  lv_event_code_t code = lv_event_get_code(e);
  lv_obj_t *obj = lv_event_get_target(e);
  if (code == LV_EVENT_CLICKED) {
    DemoEvent event = *(DemoEvent *)lv_event_get_user_data(e);
    switch (event) {
      case DEMO_TX_BTN_CLICK_EVENT:
        {
          if (!hasRadio) {
            lv_textarea_set_text(radio_ta, "Radio is not online");
            return;
          }
          lv_textarea_set_text(radio_ta, "");
          // send the first packet on this node
          Serial.print(F("[Radio] Sending first packet ... "));
          transmissionState = radio.startTransmit("Hello World!");
          sender = true;
        }

        break;
      case DEMO_RX_BTN_CLICK_EVENT:
        Serial.println("DEMO_RX_BTN_CLICK_EVENT");
        if (!hasRadio) {
          lv_textarea_set_text(radio_ta, "Radio is not online");
          return;
        }
        {
          lv_textarea_set_text(radio_ta, "");
          Serial.print(F("[Radio] Starting to listen ... "));
          int state = radio.startReceive();
          if (state == RADIOLIB_ERR_NONE) {
            Serial.println(F("success!"));
          } else {
            Serial.print(F("failed, code "));
            Serial.println(state);
          }
          sender = false;
        }
        break;
      case DEMO_CLEAN_BTN_CLICK_EVENT:
        Serial.println("DEMO_CLEAN_BTN_CLICK_EVENT");
        lv_textarea_set_text(radio_ta, "");
        radio.standby();
        break;
      case DEMO_VAD_BTN_CLICK_EVENT:
        Serial.println("DEMO_VAD_BTN_CLICK_EVENT");
        {
          lv_state_t state = lv_obj_get_state(obj);
          if (state == 2) {
            vTaskSuspend(vadTaskHandler);
#ifdef USE_ESP_VAD
            lv_label_set_text(vad_btn_label, "VAD detect");
#else
            lv_label_set_text(vad_btn_label, "loopback");
#endif
          } else {
            vad_detected_counter = 0;
            vTaskResume(vadTaskHandler);
          }
        }
        break;
      case DEMO_PLAY_BTN_CLICK_EVENT:
        Serial.println("DEMO_PLAY_BTN_CLICK_EVENT");
        if (playHandle) {
          vTaskResume(playHandle);
        }
        break;
      case DEMO_SLEEP_BTN_CLICK_EVENT:
        enterSleep = true;
        break;
      default:
        break;
    }
  }
}


// !!! LVGL !!!
// !!! LVGL !!!
// !!! LVGL !!!
static void disp_flush(lv_disp_drv_t *disp, const lv_area_t *area, lv_color_t *color_p) {
  uint32_t w = (area->x2 - area->x1 + 1);
  uint32_t h = (area->y2 - area->y1 + 1);
  if (xSemaphoreTake(xSemaphore, portMAX_DELAY) == pdTRUE) {
    tft.startWrite();
    tft.setAddrWindow(area->x1, area->y1, w, h);
    tft.pushColors((uint16_t *)&color_p->full, w * h, false);
    tft.endWrite();
    lv_disp_flush_ready(disp);
    xSemaphoreGive(xSemaphore);
  }
}

static bool getTouch(int16_t &x, int16_t &y) {
  uint8_t rotation = tft.getRotation();
  if (!touch->read()) {
    return false;
  }
  TP_Point t = touch->getPoint(0);
  switch (rotation) {
    case 1:
      x = t.y;
      y = tft.height() - t.x;
      break;
    case 2:
      x = tft.width() - t.x;
      y = tft.height() - t.y;
      break;
    case 3:
      x = tft.width() - t.y;
      y = t.x;
      break;
    case 0:
    default:
      x = t.x;
      y = t.y;
  }
  Serial.printf("R:%d X:%d Y:%d\n", rotation, x, y);
  return true;
}

static void mouse_read(lv_indev_drv_t *indev, lv_indev_data_t *data) {
  static int16_t last_x;
  static int16_t last_y;
  bool left_button_down = false;
  const uint8_t dir_pins[5] = { BOARD_TBOX_G02,
               BOARD_TBOX_G01,
               BOARD_TBOX_G04,
               BOARD_TBOX_G03,
               BOARD_BOOT_PIN };
  static bool last_dir[5];
  uint8_t pos = 10;
  for (int i = 0; i < 5; i++) {
    bool dir = digitalRead(dir_pins[i]);
    if (dir != last_dir[i]) {
      last_dir[i] = dir;
      switch (i) {
        case 0:
          if (last_x < (lv_disp_get_hor_res(NULL) - image_emoji.header.w)) {
            last_x += pos;
          }
          break;
        case 1:
          if (last_y > image_emoji.header.h) {
            last_y -= pos;
          }
          break;
        case 2:
          if (last_x > image_emoji.header.w) {
            last_x -= pos;
          }
          break;
        case 3:
          if (last_y < (lv_disp_get_ver_res(NULL) - image_emoji.header.h)) {
            last_y += pos;
          }
          break;
        case 4:
          left_button_down = true;
          break;
        default:
          break;
      }
    }
  }
  // Serial.printf("indev:X:%04d  Y:%04d \n", last_x, last_y);
  /*Store the collected data*/
  data->point.x = last_x;
  data->point.y = last_y;
  data->state = left_button_down ? LV_INDEV_STATE_PRESSED : LV_INDEV_STATE_RELEASED;
}

/*Read the touchpad*/
static void touchpad_read(lv_indev_drv_t *indev_driver, lv_indev_data_t *data) {
  data->state = getTouch(data->point.x, data->point.y) ? LV_INDEV_STATE_PR : LV_INDEV_STATE_REL;
}

// Read key value from esp32c3
static uint32_t keypad_get_key(void) {
  char key_ch = 0;
  Wire.requestFrom(0x55, 1);
  while (Wire.available() > 0) {
    key_ch = Wire.read();
  }
  return key_ch;
}

/*Will be called by the library to read the mouse*/
static void keypad_read(lv_indev_drv_t *indev_drv, lv_indev_data_t *data) {
  static uint32_t last_key = 0;
  uint32_t act_key;
  act_key = keypad_get_key();
  if (act_key != 0) {
    data->state = LV_INDEV_STATE_PR;
    Serial.printf("Key pressed : 0x%x\n", act_key);
    last_key = act_key;
  } else {
    data->state = LV_INDEV_STATE_REL;
  }
  data->key = last_key;
}

void setupLvgl() {
  static lv_disp_draw_buf_t draw_buf;
  static lv_color_t *buf = (lv_color_t *)ps_malloc(LVGL_BUFFER_SIZE);
  if (!buf) {
    Serial.println("menory alloc failed!");
    delay(5000);
    assert(buf);
  }
  String LVGL_Arduino = "Hello Arduino! ";
  LVGL_Arduino += String('V') + lv_version_major() + "." + lv_version_minor() + "." + lv_version_patch();
  Serial.println(LVGL_Arduino);
  Serial.println("I am LVGL_Arduino");

  lv_init();

  lv_group_set_default(lv_group_create());

  lv_disp_draw_buf_init(&draw_buf, buf, NULL, LVGL_BUFFER_SIZE);

  /*Initialize the display*/
  static lv_disp_drv_t disp_drv;
  lv_disp_drv_init(&disp_drv);

  /*Change the following line to your display resolution*/
  disp_drv.hor_res = TFT_HEIGHT;
  disp_drv.ver_res = TFT_WIDTH;
  disp_drv.flush_cb = disp_flush;
  disp_drv.draw_buf = &draw_buf;
  disp_drv.full_refresh = 1;
  lv_disp_drv_register(&disp_drv);

  /*Initialize the  input device driver*/

  /*Register a touchscreen input device*/
  if (touchDected) {
    static lv_indev_drv_t indev_touchpad;
    lv_indev_drv_init(&indev_touchpad);
    indev_touchpad.type = LV_INDEV_TYPE_POINTER;
    indev_touchpad.read_cb = touchpad_read;
    touch_indev = lv_indev_drv_register(&indev_touchpad);
  }

  /*Register a mouse input device*/
  static lv_indev_drv_t indev_mouse;
  lv_indev_drv_init(&indev_mouse);
  indev_mouse.type = LV_INDEV_TYPE_POINTER;
  indev_mouse.read_cb = mouse_read;
  mouse_indev = lv_indev_drv_register(&indev_mouse);
  lv_indev_set_group(mouse_indev, lv_group_get_default());

  lv_obj_t *cursor_obj;
  cursor_obj = lv_img_create(lv_scr_act());     /*Create an image object for the cursor */
  lv_img_set_src(cursor_obj, &image_emoji);     /*Set the image source*/
  lv_indev_set_cursor(mouse_indev, cursor_obj); /*Connect the image  object to the driver*/

  if (kbDected) {
    Serial.println("Keyboard registered!!");
    /*Register a keypad input device*/
    static lv_indev_drv_t indev_keypad;
    lv_indev_drv_init(&indev_keypad);
    indev_keypad.type = LV_INDEV_TYPE_KEYPAD;
    indev_keypad.read_cb = keypad_read;
    kb_indev = lv_indev_drv_register(&indev_keypad);
    lv_indev_set_group(kb_indev, lv_group_get_default());
  }
}

void setup() {
  Serial.begin(115200);
  Serial.println("T-DECK factory");

  //! The board peripheral power control pin needs to be set to HIGH when using the peripheral
  pinMode(BOARD_POWERON, OUTPUT);
  digitalWrite(BOARD_POWERON, HIGH);

  //! Set CS on all SPI buses to high level during initialization
  pinMode(BOARD_SDCARD_CS, OUTPUT);
  pinMode(RADIO_CS_PIN, OUTPUT);
  pinMode(BOARD_TFT_CS, OUTPUT);

  digitalWrite(BOARD_SDCARD_CS, HIGH);
  digitalWrite(RADIO_CS_PIN, HIGH);
  digitalWrite(BOARD_TFT_CS, HIGH);

  pinMode(BOARD_SPI_MISO, INPUT_PULLUP);
  SPI.begin(BOARD_SPI_SCK, BOARD_SPI_MISO, BOARD_SPI_MOSI);  //SD

  pinMode(BOARD_BOOT_PIN, INPUT_PULLUP);
  pinMode(BOARD_TBOX_G02, INPUT_PULLUP);
  pinMode(BOARD_TBOX_G01, INPUT_PULLUP);
  pinMode(BOARD_TBOX_G04, INPUT_PULLUP);
  pinMode(BOARD_TBOX_G03, INPUT_PULLUP);

  //Wakeup touch chip
  pinMode(BOARD_TOUCH_INT, OUTPUT);
  digitalWrite(BOARD_TOUCH_INT, HIGH);

  //Add mutex to allow multitasking access
  xSemaphore = xSemaphoreCreateBinary();
  assert(xSemaphore);
  xSemaphoreGive(xSemaphore);

  tft.begin();
  tft.setRotation(1);
  tft.fillScreen(TFT_BLACK);
  Wire.begin(BOARD_I2C_SDA, BOARD_I2C_SCL);


  // Set touch int input
  pinMode(BOARD_TOUCH_INT, INPUT);
  delay(20);

  // Two touch screens, the difference between them is the device address,
  // use ScanDevices to get the existing I2C address
  scanDevices(&Wire);

  touch = new TouchLib(Wire, BOARD_I2C_SDA, BOARD_I2C_SCL, touchAddress);

  touch->init();

  Wire.beginTransmission(touchAddress);
  touchDected = Wire.endTransmission() == 0;

  kbDected = checkKb();

  setupLvgl();

  SPIFFS.begin();

  setupSD();

  setupRadio();

  setupCoder();

  setupAmpI2S(SPK_I2S_PORT);

  setupMicrophoneI2S(MIC_I2S_PORT);

  // Test screen
  lv_obj_t *label;

  const lv_img_dsc_t *img_src[4] = { &image1, &image2, &image3, &image4 };
  lv_obj_t *img = lv_img_create(lv_scr_act());
  lv_img_set_src(img, (void *)(img_src[3]));

  // Adjust backlight
  pinMode(BOARD_BL_PIN, OUTPUT);
  //T-Deck control backlight chip has 16 levels of adjustment range
  for (int i = 0; i < 16; ++i) {
    setBrightness(i);
    lv_task_handler();
    delay(30);
  }
  delay(4000);

  int i = 2;
  while (i >= 0) {
    lv_img_set_src(img, (void *)(img_src[i]));
    lv_task_handler();
    i--;
    delay(2000);
  }

  lv_obj_del(img);



  main_count = lv_obj_create(lv_scr_act());
  lv_obj_set_style_bg_img_src(main_count, &image_output, LV_PART_MAIN);
  lv_obj_set_style_border_opa(main_count, LV_OPA_100, 0);
  lv_obj_set_style_radius(main_count, 0, 0);
  lv_obj_set_size(main_count, LV_PCT(100), LV_PCT(100));
  lv_obj_set_flex_flow(main_count, LV_FLEX_FLOW_COLUMN);
  lv_obj_center(main_count);

  // Show device state
  serialToScreen(main_count, "Keyboard C3", kbDected);

  serialToScreen(main_count, "Capacitive Touch", touchDected);
  serialToScreen(main_count, "Radio SX1262", hasRadio);
  if (SD.cardType() != CARD_NONE) {
    serialToScreen(main_count, "Mass storage #FFFFFF [# #00ff00  " + String(SD.cardSize() / 1024 / 1024.0) + "MB# #FFFFFF ]#", true);
  } else {
    serialToScreen(main_count, "Mass storage", false);
  }

  uint32_t endTime = millis() + 5000;
  while (millis() < endTime) {
    lv_task_handler();
    delay(1);
  }

  lv_obj_clean(main_count);
  lv_obj_set_scroll_dir(main_count, LV_DIR_NONE);
  lv_obj_set_scrollbar_mode(main_count, LV_SCROLLBAR_MODE_OFF);


  // Simple GUI for factory test
  lv_obj_t *win_ui = lv_obj_create(main_count);
  lv_obj_set_style_border_width(win_ui, 0, LV_PART_MAIN);
  lv_obj_set_style_bg_opa(win_ui, LV_OPA_TRANSP, LV_PART_MAIN);
  lv_obj_set_style_pad_all(win_ui, 0, LV_PART_MAIN);
  lv_obj_set_size(win_ui, LV_PCT(100), LV_PCT(55));

  radio_ta = lv_textarea_create(win_ui);
  lv_obj_set_style_bg_opa(radio_ta, LV_OPA_50, 0);
  lv_textarea_set_cursor_click_pos(radio_ta, false);
  lv_textarea_set_text_selection(radio_ta, false);
  lv_obj_set_size(radio_ta, LV_PCT(100), LV_PCT(100));
  lv_textarea_set_text(radio_ta, "");
  lv_textarea_set_max_length(radio_ta, 1024);

  lv_obj_t *btn_ui = lv_obj_create(main_count);
  lv_obj_set_style_bg_opa(btn_ui, LV_OPA_TRANSP, LV_PART_MAIN);
  lv_obj_set_size(btn_ui, LV_PCT(100), LV_PCT(20));
  lv_obj_set_flex_flow(btn_ui, LV_FLEX_FLOW_ROW);
  lv_obj_align_to(btn_ui, radio_ta, LV_ALIGN_OUT_BOTTOM_MID, 0, 0);
  lv_obj_set_style_pad_top(btn_ui, 1, LV_PART_MAIN);
  lv_obj_set_style_pad_bottom(btn_ui, 1, LV_PART_MAIN);

  lv_obj_t *btn1 = lv_btn_create(btn_ui);
  lv_obj_set_size(btn1, LV_PCT(21), LV_PCT(100));
  lv_obj_add_event_cb(btn1, lv_button_event_cb, LV_EVENT_CLICKED, (void *)&event[0]);
  label = lv_label_create(btn1);
  lv_label_set_text(label, "Tx");
  lv_obj_set_user_data(btn1, label);
  lv_obj_center(label);

  lv_obj_t *btn2 = lv_btn_create(btn_ui);
  lv_obj_set_size(btn2, LV_PCT(21), LV_PCT(100));
  lv_obj_add_event_cb(btn2, lv_button_event_cb, LV_EVENT_CLICKED, (void *)&event[1]);
  label = lv_label_create(btn2);
  lv_label_set_text(label, "Rx");
  lv_obj_center(label);

  lv_obj_t *btn3 = lv_btn_create(btn_ui);
  lv_obj_set_size(btn3, LV_PCT(21), LV_PCT(100));
  lv_obj_add_event_cb(btn3, lv_button_event_cb, LV_EVENT_CLICKED, (void *)&event[2]);
  label = lv_label_create(btn3);
  lv_label_set_text(label, "Clean");
  lv_obj_center(label);

  lv_obj_t *sleep = lv_btn_create(btn_ui);
  lv_obj_set_size(sleep, LV_PCT(25), LV_PCT(100));
  lv_obj_add_event_cb(sleep, lv_button_event_cb, LV_EVENT_CLICKED, (void *)&event[5]);
  label = lv_label_create(sleep);
  lv_label_set_text(label, "Sleep");
  lv_obj_center(label);

  lv_obj_t *btn_ui2 = lv_obj_create(main_count);
  lv_obj_set_style_bg_opa(btn_ui2, LV_OPA_TRANSP, 0);
  lv_obj_set_size(btn_ui2, LV_PCT(100), LV_PCT(20));
  lv_obj_set_flex_flow(btn_ui2, LV_FLEX_FLOW_ROW);
  lv_obj_align_to(btn_ui2, btn_ui, LV_ALIGN_OUT_BOTTOM_MID, 0, 0);
  lv_obj_set_style_pad_top(btn_ui2, 1, LV_PART_MAIN);
  lv_obj_set_style_pad_bottom(btn_ui2, 1, LV_PART_MAIN);

  lv_obj_t *btn4 = lv_btn_create(btn_ui2);
  lv_obj_set_size(btn4, LV_PCT(45), LV_PCT(100));
  lv_obj_add_flag(btn4, LV_OBJ_FLAG_CHECKABLE);
  lv_obj_add_event_cb(btn4, lv_button_event_cb, LV_EVENT_CLICKED, (void *)&event[3]);
  vad_btn_label = lv_label_create(btn4);
#ifdef USE_ESP_VAD
  lv_label_set_text(vad_btn_label, "Noise detection");
#else
  lv_label_set_text(vad_btn_label, "loopback");
#endif
  lv_obj_center(vad_btn_label);

  lv_obj_t *btn5 = lv_btn_create(btn_ui2);
  lv_obj_set_size(btn5, LV_PCT(45), LV_PCT(100));
  lv_obj_add_event_cb(btn5, lv_button_event_cb, LV_EVENT_CLICKED, (void *)&event[4]);
  label = lv_label_create(btn5);
  lv_label_set_text(label, "Play MP3");
  lv_obj_center(label);


  xTaskCreate(taskplaySong, "play", 1024 * 4, NULL, 10, &playHandle);
}

void loop() {
  if (enterSleep) {

    lv_obj_clean(main_count);
    lv_obj_t *label = lv_label_create(main_count);
    lv_label_set_text(label, "Sleep");
    lv_obj_center(label);

    //LilyGo T-Deck control backlight chip has 16 levels of adjustment range
    for (int i = 16; i > 0; --i) {
      setBrightness(i);
      lv_task_handler();
      delay(30);
    }

    delay(1000);

    //If you need other peripherals to maintain power, please set the IO port to hold
    // gpio_hold_en((gpio_num_t)BOARD_POWERON);
    // gpio_deep_sleep_hold_en();

    // When sleeping, set the touch and display screen to sleep, and all other peripherals will be powered off
    pinMode(BOARD_TOUCH_INT, OUTPUT);
    digitalWrite(BOARD_TOUCH_INT, LOW);  //Before touch to set sleep, it is necessary to set INT to LOW
    touch->enableSleep();                //set touchpad enter sleep mode
    tft.writecommand(0x10);              //set disaplay enter sleep mode
    SPI.end();
    Wire.end();
    esp_sleep_enable_ext1_wakeup(1ull << BOARD_BOOT_PIN, ESP_EXT1_WAKEUP_ALL_LOW);
    esp_deep_sleep_start();
    //Deep sleep consumes approximately 240uA of current
  }

  loopRadio();
  lv_task_handler();
  delay(1);
}