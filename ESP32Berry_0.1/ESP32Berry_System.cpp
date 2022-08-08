/////////////////////////////////////////////////////////////////
/*
  ESP32Berry, "WiFi connection & Simple Note App" Version 0.1
  For More Information: https://youtu.be/wqaxCAcghtk
  Created by Eric N. (ThatProject)
*/
/////////////////////////////////////////////////////////////////
#include "ESP32Berry_System.h"

System::System(FuncPtrString callback) {
  fetchTimer = 0;
  currentTime = "";
  requestedTimeUpdate = false;
  system_event_cb = callback;
}
System::~System() {}

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
}

void System::updateLocalTime() {

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
    system_event_cb(0, currentTime);
  }
}

bool System::initSDCard() {
  if (!SD.begin(5)) {
    return false;
  }
  uint8_t cardType = SD.cardType();

  if (cardType == CARD_NONE) {
    return false;
  }
  return true;
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