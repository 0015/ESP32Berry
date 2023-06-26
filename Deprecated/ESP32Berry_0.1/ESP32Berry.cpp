/////////////////////////////////////////////////////////////////
/*
  ESP32Berry, "WiFi connection & Simple Note App" Version 0.1
  For More Information: https://youtu.be/wqaxCAcghtk
  Created by Eric N. (ThatProject)
*/
/////////////////////////////////////////////////////////////////

#include "ESP32Berry.h"

static ESP32Berry *instance = NULL;

ESP32Berry::ESP32Berry() {
  instance = this;
  isSDCardAvailable = false;
  appNote = NULL;
}

ESP32Berry::~ESP32Berry() {}

void displayEventHandler(Menu_Event_t event, char* param) {
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

    case APP_Note:
      if ( instance->appNote != NULL) {
        delete  instance->appNote;
        instance->appNote = NULL;
      }

      instance->appNote = new AppNote(instance->display, instance->system);
      break;
  }
}

void networkResponse(Network_Event_t event, std::vector<String> result) {
  switch (event) {
    case NETWORK_SCANNING_ON:
      instance->display->update_ui_network(result);
      break;

    case NETWORK_CONNECTING:
      instance->display->update_status_bar(result);
      break;

    case NETWORK_CONNECTED:
      instance->display->update_status_bar(result);
      instance->display->show_loading_popup(false);
      instance->system->setConfigTzTime();
      break;

    case NETWORK_CONNECT_FAILURE:
    case NETWORK_DISCONNECTED:
      instance->display->update_status_bar(result);
      instance->display->show_loading_popup(false);
      break;
  }
}

void systemInfo(int event, String param) {
  instance->display->update_time(param);
}

void ESP32Berry::begin() {
  void (*iptr)(Menu_Event_t, char*) = &displayEventHandler;
  display = new Display(displayEventHandler);
  display->initTFT();

  void (*vptr)(Network_Event_t, std::vector<String>) = &networkResponse;
  network = new Network(networkResponse);

  void (*sptr)(int, String) = &systemInfo;
  system = new System(systemInfo);
  isSDCardAvailable = system->initSDCard();
}

void ESP32Berry::loop() {
  system->updateLocalTime();
}