/////////////////////////////////////////////////////////////////
/*
  New ESP32Berry Project, The base UI & ChatGPT Client
  For More Information: https://youtu.be/5K6rSw9j5iY
  Created by Eric N. (ThatProject)
*/
/////////////////////////////////////////////////////////////////
#include "ESP32Berry_Network.hpp"

static Network *instance = NULL;

extern "C" void WiFiEvent(WiFiEvent_t event, WiFiEventInfo_t info) {
  instance->WiFiEvent(event, info);
}

extern "C" void WiFiEventOn() {
  WiFi.onEvent(WiFiEvent);
}

Network::Network(FuncPtrVector callback) {
  instance = this;
  ntScanTaskHandler = NULL;
  ntConnectTaskHandler = NULL;
  network_result_cb = callback;
  _networkEvent = NETWORK_DISCONNECTED;
  WiFiEventOn();
}

Network::~Network() {
}

void Network::WiFiEvent(WiFiEvent_t event, WiFiEventInfo_t info) {
  std::string temp;

  switch (event) {
    case SYSTEM_EVENT_STA_DISCONNECTED:
      this->network_result_cb(NETWORK_DISCONNECTED, NULL, NULL);
      break;
    case SYSTEM_EVENT_STA_GOT_IP:
      temp = WiFi.localIP().toString().c_str();
      this->network_result_cb(NETWORK_CONNECTED, static_cast<void*>(&(temp)), NULL);
      break;
    default:
      break;
  }
}

void ntScanTask(void *pvParam) {
  int taskCount = 0;
  std::vector<std::string> foundWifiList;
  while (1) {
    ++taskCount;
    foundWifiList.clear();
    int n = WiFi.scanNetworks();
    vTaskDelay(10);
    for (int i = 0; i < n; ++i) {
      String item = WiFi.SSID(i) + " (" + WiFi.RSSI(i) + ") " + ((WiFi.encryptionType(i) == WIFI_AUTH_OPEN) ? " " : "*");      
      foundWifiList.push_back(item.c_str());
      vTaskDelay(10);
    }

    instance->network_result_cb(NETWORK_SCANNING_ON, static_cast<void*>(foundWifiList.data()), &n);
    vTaskDelay(5000);

    if (taskCount >= WIFI_SCAN_ITER) {
      instance->ntScanTaskHandler = NULL;
      vTaskDelete(NULL);
    }
  }
}

void ntBeginTask(void *pvParameters) {
  WiFi.mode(WIFI_STA);
  vTaskDelay(100);
  WiFi.begin(instance->_ssid.c_str(),instance->_pwd.c_str());
  vTaskDelete(NULL);
}

void Network::WiFiCommend(Network_Event_t networkEvent, void *param) {
  switch (networkEvent) {
    case NETWORK_SCANNING_OFF:
      this->WiFiScanner(false);
      break;
    case NETWORK_SCANNING_ON:
      this->WiFiScanner(true);
      break;
    case NETWORK_CONNECTING:
      this->WiFiConnector(param);
      break;
  }
}

void Network::WiFiScanner(bool isOn) {
  if (isOn) {
    xTaskCreate(ntScanTask,
                "ntScanTask",
                4096,
                NULL,
                1,
                &ntScanTaskHandler);
  } else {
    this->WiFiScannerStop();
    WiFi.disconnect();
  }
}

void Network::WiFiScannerStop() {
  if (ntScanTaskHandler != NULL) {
    vTaskDelete(ntScanTaskHandler);
    ntScanTaskHandler = NULL;
  }
}

void Network::WiFiConnector(void *param) {
  this->WiFiScannerStop();
  String networkInfo = String((char*)param);
  int seperatorIdx = networkInfo.indexOf(WIFI_SSID_PW_DELIMITER);
  _ssid = networkInfo.substring(0, seperatorIdx);
  _pwd = networkInfo.substring(seperatorIdx + 2, networkInfo.length());
  xTaskCreate(ntBeginTask, "ntBeginTask", 4096, NULL, 1, &ntConnectTaskHandler);
}