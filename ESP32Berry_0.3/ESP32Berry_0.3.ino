/////////////////////////////////////////////////////////////////
/*
  ESP32Berry, "ESP-NOW Chat App" Version 0.3
  For More Information: https://youtu.be/UhIXAp2wqjg
  Created by Eric N. (ThatProject)
*/
/////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////
//ESP32 version 2.0.4
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