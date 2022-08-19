/////////////////////////////////////////////////////////////////
/*
  ESP32Berry, "Telegram App" Version 0.2
  For More Information: https://youtu.be/h28_Mvgpe2Y
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

typedef enum {
  SYS_TIME,
  SYS_BATTERY,
} System_Event_t;

class System {
private:
  long fetchTimer;
  long batteryTimer;
  String currentTime;
  bool requestedTimeUpdate;
  typedef void (*FuncPtrString)(System_Event_t, String);
  fs::FS &storage = SD;

public:
  FuncPtrString system_event_cb;

  System(FuncPtrString callback);
  ~System();

  void set_config_tz_time();
  void update_local_time();

  bool init_SDCard();
  std::vector<String> list_dir(const char *dirname);
  bool create_dir(const char *path);
  bool write_file(const char *path, const char *message);
  String read_file(const char *path);
  void sdcard_info(unsigned long *totalBytes, unsigned long *usedBytes);
  void read_battery();
  void update();
  void restart();
};