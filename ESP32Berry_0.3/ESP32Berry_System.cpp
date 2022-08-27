/////////////////////////////////////////////////////////////////
/*
  ESP32Berry, "ESP-NOW Chat App" Version 0.3
  For More Information: https://youtu.be/UhIXAp2wqjg
  Created by Eric N. (ThatProject)
*/
/////////////////////////////////////////////////////////////////
#include "ESP32Berry_System.h"

System::System(FuncPtrString callback) {
  fetchTimer = 0;
  currentTime = "";
  isSDCardAvailable = false;
  requestedTimeUpdate = false;
  system_event_cb = callback;
  batteryTimer = 0;
}
System::~System() {}

void System::set_config_tz_time() {

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
}

void System::update_local_time() {

  if (!requestedTimeUpdate) return;

  if (fetchTimer == 0 || millis() - fetchTimer >= TIME_UPDATE_INTERVAL) {
    fetchTimer = millis();

    struct tm timeinfo;
    if (!getLocalTime(&timeinfo)) {
      return;
    }

    char hourMin[24];
    strftime(hourMin, 24, "%a %H:%M\n%D", &timeinfo);
    currentTime = String(hourMin);
    system_event_cb(SYS_TIME, currentTime);
  }
}

String System::get_local_time_for_msg() {
  struct tm timeinfo;
  if (!getLocalTime(&timeinfo)) {
    return "";
  }

  char dateTime[10];
  strftime(dateTime, 10, "%T", &timeinfo);
  return String(dateTime);
}

bool System::init_SDCard() {

  if (!SD.begin(5)) {
    return false;
  }
  uint8_t cardType = SD.cardType();

  if (cardType == CARD_NONE) {
    return false;
  }
  isSDCardAvailable = true;
  return true;
}

std::vector<String> System::list_dir(const char* dirname) {
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

bool System::create_dir(const char* path) {
  if (storage.exists(path)) {
    return true;
  }

  if (storage.mkdir(path)) {
    return true;
  } else {
    return false;
  }
}

bool System::write_file(const char* path, const char* message) {
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

int System::read_file_size(const char* path) {
  File file = storage.open(path, FILE_READ);
  if (!file) {

    file.close();
    return 0;
  }

  return file.size();
}

String System::read_file(const char* path) {
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

void System::sdcard_info(unsigned long* totalBytes, unsigned long* usedBytes) {
  *totalBytes = SD.totalBytes() / (1024 * 1024);
  *usedBytes = SD.usedBytes() / (1024 * 1024);
}

bool System::is_SDcard_available() {
  return isSDCardAvailable;
}

void System::read_battery() {
  if (millis() - batteryTimer >= 1000) {
    batteryTimer = millis();

    int totalValue = 0;
    int averageValue = 0;
    for (int i = 0; i < BAT_READS; i++) {
      totalValue += analogRead(BAT_PIN);
    }
    averageValue = totalValue / BAT_READS;
    double batVolts = averageValue * BAT_CONV_FACTOR / 1000;
    if (batVolts > BAT_MAX_VOLT) {
      batVolts = BAT_MAX_VOLT;
    }
    int batPercent = map(batVolts * 100, 320, 420, 0, 100);
    if (batPercent < 0) {
      return;
    }

    String batData = String(batVolts);
    batData += ",";
    batData += String(batPercent);
    system_event_cb(SYS_BATTERY, batData);
  }
}

void System::update() {
  this->update_local_time();
  this->read_battery();
}

void System::restart() {
  ESP.restart();
}