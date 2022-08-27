/////////////////////////////////////////////////////////////////
/*
  ESP32Berry, "ESP-NOW Chat App" Version 0.3
  For More Information: https://youtu.be/UhIXAp2wqjg
  Created by Eric N. (ThatProject)
*/
/////////////////////////////////////////////////////////////////
#pragma once
#include <vector>
#include "ESP32Berry_Config.h"
#include "ESP32Berry_Display.h"
#include "ESP32Berry_Menu.h"
#include "ESP32Berry_Network.h"
#include "ESP32Berry_System.h"
#include "ESP32Berry_AppNote.h"
#include "ESP32Berry_AppTelegram.h"
#include "ESP32Berry_AppESPNow.h"

class ESP32Berry {
private:

public:
  Display *display;
  Network *network;
  System *system;
  Menu *menu;
  AppNote *appNote;
  AppTelegram *appTelegram;
  AppESPNow *appESPNow;
  bool isSDCardAvailable;

  ESP32Berry();
  ~ESP32Berry();
  void begin();
  void loop();
};