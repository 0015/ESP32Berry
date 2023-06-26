/////////////////////////////////////////////////////////////////
/*
  ESP32Berry, "WiFi connection & Simple Note App" Version 0.1
  For More Information: https://youtu.be/wqaxCAcghtk
  Created by Eric N. (ThatProject)
*/
/////////////////////////////////////////////////////////////////
#pragma once
#include <vector>
#include <WiFi.h>
#include "ESP32Berry_Config.h"
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
};