/////////////////////////////////////////////////////////////////
/*
  New ESP32Berry Project, The base UI & ChatGPT Client
  For More Information: https://youtu.be/5K6rSw9j5iY
  Created by Eric N. (ThatProject)
*/
/////////////////////////////////////////////////////////////////
#pragma once
#include <vector>
#include "ESP32Berry_Config.hpp"
#include <Audio.h>
#include "esp32-hal.h"
#include "lwip/apps/sntp.h"
#include "esp_adc_cal.h"
#include "FS.h"
#include "SD.h"
#include "SPI.h"

typedef enum {
  SYS_TIME,
  SYS_BATTERY,
} System_Event_t;

class System {
private:
  bool requestedTimeUpdate;

  typedef void (*FuncPtrString)(System_Event_t, void*);
  fs::FS &storage = SD;

  int vRef;
  void init();
  bool initSDCard();
  void initADCBAT();
public:
  TaskHandle_t audioTaskHandler;
  FuncPtrString system_event_cb;

  System(FuncPtrString callback);
  ~System();

  Audio* audio;
  bool isSDCard;
  void setConfigTzTime();
  void update_local_time();
  void read_battery();
  void play_audio(const char* filename);
  
  std::vector<String> listDir(const char *dirname);
  bool createDir(const char *path);
  bool writeFile(const char *path, const char *message);
  String readFile(const char *path);
};