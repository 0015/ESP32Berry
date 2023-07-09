/////////////////////////////////////////////////////////////////
/*
  New ESP32Berry Project, The base UI & ChatGPT Client
  For More Information: https://youtu.be/5K6rSw9j5iY
  Created by Eric N. (ThatProject)
*/
/////////////////////////////////////////////////////////////////
#include "ESP32Berry_System.hpp"

static System* instance = NULL;

void systemTask(void* pvParameters) {
  while (1) {
    instance->update_local_time();
    instance->read_battery();
    vTaskDelay(SYSTEM_UPDATE_INTERVAL);
  }
}

void taskPlayAudio(void* pvParameters) {
  if (SD.exists((char*)pvParameters)) {
    instance->audio->setPinout(BOARD_I2S_BCK, BOARD_I2S_WS, BOARD_I2S_DOUT);
    instance->audio->setVolume(21);
    instance->audio->connecttoFS(SD, (char*)pvParameters);
    while (instance->audio->isRunning()) {
      instance->audio->loop();
    }
    instance->audio->stopSong();
  }
  vTaskDelete(NULL);
}

System::System(FuncPtrString callback) {
  instance = this;
  requestedTimeUpdate = false;
  system_event_cb = callback;
  init();
  xTaskCreate(systemTask, "systemTask", 4096, NULL, 1, NULL);
}

System::~System() {}

void System::init() {
  initADCBAT();
  audio = new Audio();
  isSDCard = initSDCard();
  play_audio(AUDIO_BOOT);
}

void System::initADCBAT() {
  esp_adc_cal_characteristics_t adc_chars;
  esp_adc_cal_value_t val_type = esp_adc_cal_characterize(
    ADC_UNIT_1,
    ADC_ATTEN_DB_11,
    ADC_WIDTH_BIT_12,
    1100,
    &adc_chars);

  if (val_type == ESP_ADC_CAL_VAL_EFUSE_VREF) {
    vRef = adc_chars.vref;
  } else {
    vRef = 1100;
  }
}

void System::setConfigTzTime() {

  const char* ntpServer0 = "time1.google.com";
  const char* ntpServer1 = "pool.ntp.org";
  const char* ntpServer2 = "time.nist.gov";

  esp_netif_init();
  if (sntp_enabled()) {
    sntp_stop();
  }
  sntp_setoperatingmode(SNTP_OPMODE_POLL);
  sntp_setservername(0, (char*)ntpServer0);
  sntp_setservername(1, (char*)ntpServer1);
  sntp_setservername(2, (char*)ntpServer2);
  sntp_init();
  setenv("TZ", TIME_ZONE, 1);
  tzset();
  requestedTimeUpdate = true;
  update_local_time();
}

void System::update_local_time() {

  if (!requestedTimeUpdate) return;

  struct tm timeinfo;
  if (!getLocalTime(&timeinfo)) {
    return;
  }

  system_event_cb(SYS_TIME, &timeinfo);
}

bool System::initSDCard() {
  digitalWrite(BOARD_SDCARD_CS, HIGH);
  if (!SD.begin(BOARD_SDCARD_CS, SPI, 800000U)) {
    return false;
  }

  uint8_t cardType = SD.cardType();
  if (cardType == CARD_NONE) {
    return false;
  }
  return true;
}

void System::play_audio(const char* filename) {
  if (!isSDCard) return;
  xTaskCreate(taskPlayAudio, "play", 1024 * 6, (void*)filename, 2, &audioTaskHandler);
}

std::vector<String> System::listDir(const char* dirname) {
  std::vector<String> fileList;

  File root = storage.open(dirname);
  if (!root) {
    return fileList;
  }
  if (!root.isDirectory()) {
    return fileList;
  }

  File file = root.openNextFile();
  while (file) {
    char fileInfo[128];
    snprintf(fileInfo, sizeof(fileInfo), "%s (%d bytes)", file.name(), file.size());
    fileList.push_back(String(fileInfo));
    file = root.openNextFile();
  }

  return fileList;
}

bool System::createDir(const char* path) {
  if (storage.exists(path)) {
    return true;
  }

  if (storage.mkdir(path)) {
    return true;
  } else {
    return false;
  }
}

bool System::writeFile(const char* path, const char* message) {
  File file = storage.open(path, FILE_WRITE);
  if (!file) {
    file.close();
    return false;
  }
  if (!file.print(message)) {
    file.close();
    return false;
  }
  file.close();
  return true;
}

String System::readFile(const char* path) {
  File file = storage.open(path, FILE_READ);
  if (!file) {

    file.close();
    return "";
  }
  String temp = "";
  while (file.available()) {
    temp += file.readStringUntil('\n');
  }

  file.close();
  return temp;
}

void System::read_battery() {
  uint16_t v = analogRead(BOARD_BAT_ADC);
  double vBat = ((float)v / 4095.0) * 2.0 * 3.3 * (vRef / 1000.0);
  if (vBat > BAT_MAX_VOLT) {
    vBat = BAT_MAX_VOLT;
  }
  int batPercent = map(vBat * 100, 320, 420, 0, 100);
  if (batPercent < 0) {
    batPercent = 0;
  }

  system_event_cb(SYS_BATTERY, &batPercent);
}