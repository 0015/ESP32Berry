/////////////////////////////////////////////////////////////////
/*
  ESP32Berry, "WiFi connection & Simple Note App" Version 0.1
  For More Information: https://youtu.be/wqaxCAcghtk
  Created by Eric N. (ThatProject)
*/
/////////////////////////////////////////////////////////////////
#pragma once
#include <vector>
#include "ESP32Berry_Config.h"
#include "esp32-hal.h"
#include "lwip/apps/sntp.h"
#include "esp_netif.h"
#include "FS.h"
#include "SD.h"
#include "SPI.h"

class System {
private:
  long fetchTimer;
  String currentTime;
  bool requestedTimeUpdate;

  typedef void (*FuncPtrString)(int, String);
  fs::FS &storage = SD;

public:
  FuncPtrString system_event_cb;

  System(FuncPtrString callback);
  ~System();

  void setConfigTzTime();
  void updateLocalTime();

  bool initSDCard();
  std::vector<String> listDir(const char *dirname);
  bool createDir(const char *path);
  bool writeFile(const char *path, const char *message);
  String readFile(const char *path);
};