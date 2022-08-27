/////////////////////////////////////////////////////////////////
/*
  ESP32Berry, "ESP-NOW Chat App" Version 0.3
  For More Information: https://youtu.be/UhIXAp2wqjg
  Created by Eric N. (ThatProject)
*/
/////////////////////////////////////////////////////////////////
#pragma once
#include <vector>
#include <WiFi.h>
#include "ESP32Berry_Config.h"
#include <FastBot.h>
typedef enum {
  NETWORK_DISCONNECTED,
  NETWORK_SCANNING_OFF,
  NETWORK_SCANNING_ON,
  NETWORK_CONNECTING,
  NETWORK_CONNECT_FAILURE,
  NETWORK_CONNECTED,
  TELEGRAM_MESSAGE,
} Network_Event_t;

class Network {
private:
  FastBot *telegram;
  typedef void (*FuncPtrVector)(Network_Event_t, std::vector<String>);
  friend void ntScanTask(void* pvParameters);
  Network_Event_t _networkEvent;
  void WiFiScanner(bool isOn);
  void WiFiScannerStop();
  void WiFiConnector(char* param);

public:
  std::vector<String> WiFiLog;
  TaskHandle_t ntScanTaskHandler, ntConnectTaskHandler;
  FuncPtrVector network_result_cb;
  Network(FuncPtrVector callback);
  ~Network();
  void WiFiCommend(Network_Event_t networkEvent, char* param);
  void WiFiEvent(WiFiEvent_t event, WiFiEventInfo_t info);
  String get_mac_address();
  void set_network_status(Network_Event_t event);
  Network_Event_t get_network_status();

  void telegram_cb(FB_msg& msg);
  void telegram_set_cb_func(void (*handler)(FB_msg& msg));
  void telegram_remove_cb_func();
  void telegram_reset_cb_func();
  void telegram_enable(bool isOn);
  bool telegram_send(String msg);
  void update();

};