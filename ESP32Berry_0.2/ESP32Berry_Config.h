/////////////////////////////////////////////////////////////////
/*
  ESP32Berry, "Telegram App" Version 0.2
  For More Information: https://youtu.be/h28_Mvgpe2Y
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

#define BAT_MAX_VOLT 4.2
#define BAT_PIN 34
#define BAT_CONV_FACTOR 1.725
#define BAT_READS 24

#define TELEGRAM_BOT_TOKEN "<YOUR_BOT_TOKEN>" // https://github.com/GyverLibs/FastBot
#define TELEGRAM_CHAT_ID "<YOUR_CHAT_ID>"