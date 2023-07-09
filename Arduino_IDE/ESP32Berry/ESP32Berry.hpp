/////////////////////////////////////////////////////////////////
/*
  New ESP32Berry Project, The base UI & ChatGPT Client
  For More Information: https://youtu.be/5K6rSw9j5iY
  Created by Eric N. (ThatProject)
*/
/////////////////////////////////////////////////////////////////

#pragma once
#include "ESP32Berry_Config.hpp"

#include <vector>
#include "ESP32Berry_Display.hpp"
#include "ESP32Berry_Network.hpp"
#include "ESP32Berry_System.hpp"
#include "ESP32Berry_AppChatGPT.hpp"

class ESP32Berry {
private:

public:
  Display *display;
  Network *network;
  System *system;
  AppChatGPT *appChatGPT;

  ESP32Berry();
  ~ESP32Berry();
  void begin();
};