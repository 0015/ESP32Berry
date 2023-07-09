/////////////////////////////////////////////////////////////////
/*
  New ESP32Berry Project, The base UI & ChatGPT Client
  For More Information: https://youtu.be/5K6rSw9j5iY
  Created by Eric N. (ThatProject)
*/
/////////////////////////////////////////////////////////////////
#pragma once
#include <vector>
#include <WiFiClientSecure.h>
#include "ESP32Berry_Config.hpp"
typedef enum {
  NETWORK_DISCONNECTED,
  NETWORK_SCANNING_OFF,
  NETWORK_SCANNING_ON,
  NETWORK_CONNECTING,
  NETWORK_CONNECT_FAILURE,
  NETWORK_CONNECTED,
} Network_Event_t;

class Network {
private:
  typedef void (*FuncPtrVector)(Network_Event_t, void*, void*);
  friend void ntScanTask(void* pvParameters);
  Network_Event_t _networkEvent;
  void WiFiScanner(bool isOn);
  void WiFiScannerStop();
  void WiFiConnector(void* param);
public:
  String _ssid, _pwd;
  std::vector<String> WiFiLog;
  TaskHandle_t ntScanTaskHandler, ntConnectTaskHandler;
  FuncPtrVector network_result_cb;
  Network(FuncPtrVector callback);
  ~Network();
  void WiFiCommend(Network_Event_t networkEvent, void* param);
  void WiFiEvent(WiFiEvent_t event, WiFiEventInfo_t info);
};