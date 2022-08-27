/////////////////////////////////////////////////////////////////
/*
  ESP32Berry, "ESP-NOW Chat App" Version 0.3
  For More Information: https://youtu.be/UhIXAp2wqjg
  Created by Eric N. (ThatProject)
*/
/////////////////////////////////////////////////////////////////
#include "ESP32Berry.h"

static ESP32Berry* instance = NULL;

ESP32Berry::ESP32Berry() {
  instance = this;
  isSDCardAvailable = false;
  appNote = NULL;
  appTelegram = NULL;
  appESPNow = NULL;
}

ESP32Berry::~ESP32Berry() {}


void appEventHandler(App_Event_t event, char* param) {
  switch (event) {
    case APP_Note:

      if (!instance->isSDCardAvailable) {
        instance->display->ui_popup_open("Oops!", "Check Your SD Card.");
        return;
      }
      if (instance->appNote != NULL) {
        instance->appNote = NULL;
      }
      instance->appNote = new AppNote(instance->display, instance->system, NULL, "Note App");
      break;

    case APP_Telegram:

      if (instance->network->get_network_status() != NETWORK_CONNECTED) {
        instance->display->ui_popup_open("Oops!", "Check Your Wi-Fi Connection.");
        return;
      }

      if (instance->appTelegram != NULL) {
        instance->appTelegram = NULL;
      }
      instance->appTelegram = new AppTelegram(instance->display, NULL, instance->network, "Telegram App");
      break;

    case APP_ESPNOW:
    
      if (instance->appESPNow != NULL) {
        instance->appESPNow = NULL;
      }
      instance->appESPNow = new AppESPNow(instance->display, instance->system, instance->network, "ESP-NOW App");

      break;
  }
}

void menuEventHandler(Menu_Event_t event, char* param) {
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
  }
}

void networkResponse(Network_Event_t event, std::vector<String> result) {
  switch (event) {
    case NETWORK_SCANNING_ON:
      instance->menu->update_ui_network(result);
      break;

    case NETWORK_CONNECTING:
      instance->display->launch_noti(result.back());
      break;

    case NETWORK_CONNECTED:
      instance->display->show_loading_popup(false);
      instance->display->launch_noti(result.back());
      instance->display->set_wifi_icon(true);
      instance->system->set_config_tz_time();

      break;

    case NETWORK_CONNECT_FAILURE:
    case NETWORK_DISCONNECTED:
      instance->display->show_loading_popup(false);
      instance->display->launch_noti(result.back());
      instance->display->set_wifi_icon(false);
      instance->menu->ui_network_switch(false);
      break;

    case TELEGRAM_MESSAGE:
      instance->display->launch_noti(result.back());
      break;
  }
}

void systemInfo(System_Event_t event, String param) {
  switch (event) {
    case SYS_TIME:
      instance->display->update_time(param);
      break;

    case SYS_BATTERY:
      instance->display->update_battery(param);
      instance->menu->update_battery(param);
      break;
  }
}

void ESP32Berry::begin() {

  void (*iptr)(App_Event_t, char*) = &appEventHandler;
  display = new Display(appEventHandler);

  void (*vptr)(Network_Event_t, std::vector<String>) = &networkResponse;
  network = new Network(networkResponse);

  void (*sptr)(System_Event_t, String) = &systemInfo;
  system = new System(systemInfo);
  isSDCardAvailable = system->init_SDCard();

  void (*mptr)(Menu_Event_t, char*) = &menuEventHandler;
  menu = new Menu(display, system, menuEventHandler);
}

void ESP32Berry::loop() {
  system->update();
  network->update();
}