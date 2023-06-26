/////////////////////////////////////////////////////////////////
/*
  ESP32Berry, "WiFi connection & Simple Note App" Version 0.1
  For More Information: https://youtu.be/wqaxCAcghtk
  Created by Eric N. (ThatProject)
*/
/////////////////////////////////////////////////////////////////

#pragma once
#include "ESP32Berry_Config.h"

#include <vector>
#include "ESP32Berry_Display.h"
#include "ESP32Berry_Network.h"
#include "ESP32Berry_System.h"
#include "ESP32Berry_App_note.h"

class ESP32Berry {
private:
  bool isSDCardAvailable;

public:
  Display *display;
  Network *network;
  System *system;

  AppNote *appNote;

  ESP32Berry();
  ~ESP32Berry();
  void begin();
  void loop();
};