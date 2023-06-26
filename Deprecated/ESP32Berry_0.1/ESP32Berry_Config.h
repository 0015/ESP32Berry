/////////////////////////////////////////////////////////////////
/*
  ESP32Berry, "WiFi connection & Simple Note App" Version 0.1
  For More Information: https://youtu.be/wqaxCAcghtk
  Created by Eric N. (ThatProject)
*/
/////////////////////////////////////////////////////////////////
#include <Arduino.h>

#define WIFI_SCAN_ITER 5
#define WIFI_SSID_PW_DELIMITER "^_"
#define WIFI_CONNECT_TIMEOUT 10 * 1000

#define TIME_ZONE "PST8PDT,M3.2.0,M11.1.0" // https://github.com/esp8266/Arduino/blob/master/cores/esp8266/TZ.h
#define TIME_UPDATE_INTERVAL 60 * 1000

#define DISPLAY_WIDTH 480
#define DISPLAY_HEIGHT 320

