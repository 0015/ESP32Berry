/////////////////////////////////////////////////////////////////
/*
  New ESP32Berry Project, The base UI & ChatGPT Client
  For More Information: https://youtu.be/5K6rSw9j5iY
  Created by Eric N. (ThatProject)
*/
/////////////////////////////////////////////////////////////////
#include "ESP32Berry.hpp"

static ESP32Berry* instance = NULL;

ESP32Berry::ESP32Berry() {
  instance = this;
}

ESP32Berry::~ESP32Berry() {}

void displayEventHandler(Menu_Event_t event, void* param) {
  switch (event) {
    case WIFI_OFF:
      instance->network->WiFiCommend(NETWORK_SCANNING_OFF, param);
      break;

    case WIFI_ON:
      if (param == NULL) {
        instance->network->WiFiCommend(NETWORK_SCANNING_ON, param);
      } else {
        instance->network->WiFiCommend(NETWORK_CONNECTING, param);
      }
      break;

    case APP:
      int menuNum = std::stoi((char*)param);
      switch (menuNum) {
        case 0:
          instance->appChatGPT = new AppChatGPT(instance->display, instance->system, instance->network, "ChatGPT Client");
          break;
      }

      break;
  }
}

void networkResponse(Network_Event_t event, void* data1, void* data2) {
  switch (event) {
    case NETWORK_SCANNING_ON:
      instance->display->update_ui_network(data1, data2);
      break;

    case NETWORK_CONNECTED:
      instance->system->setConfigTzTime();
      instance->display->show_loading_popup(false);
      instance->display->set_notification("[WiFi] Connected!");
      instance->display->update_WiFi_label(data1);
      break;

    case NETWORK_CONNECT_FAILURE:
    case NETWORK_DISCONNECTED:
      instance->display->show_loading_popup(false);
      instance->display->set_notification("[WIFI] Unable to connect to selected WiFi.");
      instance->display->update_WiFi_label(data1);
      break;
  }
}

void systemInfo(System_Event_t event, void* param) {
  switch (event) {
    case SYS_TIME:
      instance->display->update_time(param);
      break;

    case SYS_BATTERY:
      instance->display->update_battery(param);
      break;
  }
}

void ESP32Berry::begin() {
  void (*iptr)(Menu_Event_t, void*) = &displayEventHandler;
  display = new Display(displayEventHandler);

  void (*vptr)(Network_Event_t, void*, void*) = &networkResponse;
  network = new Network(networkResponse);

  void (*sptr)(System_Event_t, void*) = &systemInfo;
  system = new System(systemInfo);
}