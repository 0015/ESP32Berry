/////////////////////////////////////////////////////////////////
/*
  New ESP32Berry Project, The base UI & ChatGPT Client
  For More Information: https://youtu.be/5K6rSw9j5iY
  Created by Eric N. (ThatProject)
*/
/////////////////////////////////////////////////////////////////
#include "ESP32Berry.hpp"
#include <Preferences.h>

Preferences preferences;

static ESP32Berry *instance = NULL;

ESP32Berry::ESP32Berry()
{
  instance = this;
}

ESP32Berry::~ESP32Berry() {}

void displayEventHandler(Menu_Event_t event, void *param)
{
  switch (event)
  {
  case WIFI_OFF:
  {
    instance->network->WiFiCommend(NETWORK_SCANNING_OFF, param);
    break;
  }
  case WIFI_ON:
  {
    instance->network->WiFiCommend(param == NULL ? NETWORK_SCANNING_ON : NETWORK_CONNECTING, param);
    break;
  }
  case APP:
  {
    int menuNum = std::stoi((char *)param);
    switch (menuNum)
    {
    case 0:
      instance->appChatGPT = new AppChatGPT(instance->display, instance->system, instance->network, "ChatGPT Client");
      break;
    }

    break;
  }
  case SET_AUDIO:
  {
    int volume = reinterpret_cast<int>(param);
    instance->system->audio->setVolume(volume);
    instance->display->update_volume_slider(volume);
    preferences.begin("settings", false);
    preferences.putInt("volume", volume);
    preferences.end();
    break;
  }
  case SCREEN_TIMEOUT:
  {
    int timeout = reinterpret_cast<int>(param);
    preferences.begin("settings", false);
    preferences.putInt("screen_timeout", timeout);
    preferences.end();
    instance->display->set_screen_timeout(timeout);
    break;
  }
  }
}

void networkResponse(Network_Event_t event, void *data1, void *data2)
{
  switch (event)
  {
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

void systemInfo(System_Event_t event, void *param)
{
  switch (event)
  {
  case SYS_TIME:
    instance->display->update_time(param);
    break;

  case SYS_BATTERY:
    instance->display->update_battery(param);
    break;
  }
}

void ESP32Berry::begin()
{
  void (*iptr)(Menu_Event_t, void *) = &displayEventHandler;
  display = new Display(displayEventHandler);

  void (*vptr)(Network_Event_t, void *, void *) = &networkResponse;
  network = new Network(networkResponse);

  void (*sptr)(System_Event_t, void *) = &systemInfo;
  system = new System(systemInfo);

  preferences.begin("settings", true);
  int volume = preferences.getInt("volume", 21);
  int screenTimeout = preferences.getInt("screen_timeout", 1);
  preferences.end();
  instance->system->audio->setVolume(volume);
  instance->display->update_volume_slider(volume);
  instance->display->set_screen_timeout(screenTimeout);
  instance->system->play_audio(AUDIO_BOOT);
}