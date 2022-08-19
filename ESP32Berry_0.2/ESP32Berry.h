/////////////////////////////////////////////////////////////////
/*
  ESP32Berry, "Telegram App" Version 0.2
  For More Information: https://youtu.be/h28_Mvgpe2Y
  Created by Eric N. (ThatProject)
*/
/////////////////////////////////////////////////////////////////
#pragma once
#include "ESP32Berry_Config.h"

#include <vector>
#include "ESP32Berry_Display.h"
#include "ESP32Berry_Menu.h"
#include "ESP32Berry_Network.h"
#include "ESP32Berry_System.h"
#include "ESP32Berry_AppNote.h"
#include "ESP32Berry_AppTelegram.h"

class ESP32Berry {
private:

public:
  Display *display;
  Network *network;
  System *system;
  Menu *menu;
  AppNote *appNote;
  AppTelegram *appTelegram;
  bool isSDCardAvailable;

  ESP32Berry();
  ~ESP32Berry();
  void begin();
  void loop();
};