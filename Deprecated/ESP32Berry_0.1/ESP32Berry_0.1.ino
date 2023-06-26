/////////////////////////////////////////////////////////////////
/*
  ESP32Berry, "WiFi connection & Simple Note App" Version 0.1
  For More Information: https://youtu.be/wqaxCAcghtk
  Created by Eric N. (ThatProject)
*/
/////////////////////////////////////////////////////////////////

#include "ESP32Berry.h"

ESP32Berry *_ESP32Berry;

void setup() {
  _ESP32Berry = new ESP32Berry();
  _ESP32Berry->begin();
}

void loop() {
  _ESP32Berry->loop();
}