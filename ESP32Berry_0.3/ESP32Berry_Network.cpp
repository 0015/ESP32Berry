/////////////////////////////////////////////////////////////////
/*
  ESP32Berry, "ESP-NOW Chat App" Version 0.3
  For More Information: https://youtu.be/UhIXAp2wqjg
  Created by Eric N. (ThatProject)
*/
/////////////////////////////////////////////////////////////////
#include "ESP32Berry_Network.h"

static Network *instance = NULL;

extern "C" void WiFiEvent(WiFiEvent_t event, WiFiEventInfo_t info) {
  instance->WiFiEvent(event, info);
}

extern "C" void WiFiEventOn() {
  WiFi.onEvent(WiFiEvent);
}

Network::Network(FuncPtrVector callback) {
  instance = this;
  telegram = NULL;
  ntScanTaskHandler = NULL;
  ntConnectTaskHandler = NULL;
  network_result_cb = callback;
  _networkEvent = NETWORK_DISCONNECTED;
  WiFiEventOn();
  WiFi.mode(WIFI_AP_STA);
}

Network::~Network() {
}

void Network::WiFiEvent(WiFiEvent_t event, WiFiEventInfo_t info) {
  switch (event) {
    case SYSTEM_EVENT_STA_DISCONNECTED:
      WiFiLog.push_back("Disconnected from WiFi access point.");
      this->set_network_status(NETWORK_DISCONNECTED);
      this->network_result_cb(NETWORK_DISCONNECTED, WiFiLog);
      this->telegram_enable(false);
      break;
    case SYSTEM_EVENT_STA_GOT_IP:
      WiFiLog.push_back("WiFi Connected!");
      this->set_network_status(NETWORK_CONNECTED);
      this->network_result_cb(NETWORK_CONNECTED, WiFiLog);
      this->telegram_enable(true);
      break;

    default:
      break;
  }
}

String Network::get_mac_address() {
  return WiFi.macAddress();
}

void ntScanTask(void *pvParam) {
  int taskCount = 0;
  std::vector<String> foundWifiList;
  while (1) {
    ++taskCount;
    foundWifiList.clear();
    int n = WiFi.scanNetworks();
    vTaskDelay(10);
    for (int i = 0; i < n; ++i) {
      String item = WiFi.SSID(i) + " (" + WiFi.RSSI(i) + ") " + "[" + WiFi.channel(i) + "]" + ((WiFi.encryptionType(i) == WIFI_AUTH_OPEN) ? " " : "*");
      foundWifiList.push_back(item);
      vTaskDelay(10);
    }
    instance->set_network_status(NETWORK_SCANNING_ON);
    instance->network_result_cb(NETWORK_SCANNING_ON, foundWifiList);
    vTaskDelay(5000);

    if (taskCount >= WIFI_SCAN_ITER) {
      instance->ntScanTaskHandler = NULL;
      vTaskDelete(NULL);
    }
  }
}

void ntBeginTask(void *pvParameters) {
  char *_networkInfo = reinterpret_cast<char *>(pvParameters);
  std::string networkInfo = std::string(_networkInfo);

  int seperatorIdx = networkInfo.find(WIFI_SSID_PW_DELIMITER, 0);
  if (seperatorIdx < 3) {
    instance->WiFiLog.push_back("Something Wrong!");
    instance->set_network_status(NETWORK_CONNECT_FAILURE);
    instance->network_result_cb(NETWORK_CONNECT_FAILURE, instance->WiFiLog);
    vTaskDelete(NULL);
  }

  const char *_ssid = networkInfo.substr(0, seperatorIdx).c_str();
  const char *_pwd = networkInfo.substr(seperatorIdx + 2, networkInfo.length() - seperatorIdx + 2).c_str();

  vTaskDelay(500);
  unsigned long startingTime = millis();
  WiFi.disconnect(true);
  WiFi.begin(_ssid, _pwd);
  vTaskDelay(100);
  vTaskDelete(NULL);
}

void Network::WiFiCommend(Network_Event_t networkEvent, char *param) {
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
    WiFi.disconnect(true);
    this->set_network_status(NETWORK_DISCONNECTED);
  }
}

void Network::WiFiScannerStop() {
  if (ntScanTaskHandler != NULL) {
    vTaskDelete(ntScanTaskHandler);
    ntScanTaskHandler = NULL;
  }
}

void Network::WiFiConnector(char *param) {
  this->WiFiScannerStop();
  WiFiLog.push_back("WiFi Connecting!");
  this->set_network_status(NETWORK_CONNECTING);
  this->network_result_cb(NETWORK_CONNECTING, WiFiLog);
  xTaskCreate(ntBeginTask, "ntBeginTask", 2048, (void *)param, 2, &ntConnectTaskHandler);
}

void Network::set_network_status(Network_Event_t event) {
  _networkEvent = event;
}

Network_Event_t Network::get_network_status() {
  return _networkEvent;
}

extern "C" void telegram_cb_thunk(FB_msg &msg) {
  instance->telegram_cb(msg);
}

void Network::telegram_cb(FB_msg &msg) {
  String telegramMsg = "[" + msg.username + "] - " + msg.text;
  WiFiLog.push_back(telegramMsg);
  network_result_cb(TELEGRAM_MESSAGE, WiFiLog);
}

void Network::telegram_enable(bool isOn) {
  if (isOn) {
    telegram = new FastBot(TELEGRAM_BOT_TOKEN);
    telegram->setChatID(TELEGRAM_CHAT_ID);
    telegram_set_cb_func(telegram_cb_thunk);
  } else {
    telegram_remove_cb_func();
    delete telegram;
    telegram = NULL;
  }
}

void Network::telegram_set_cb_func(void (*handler)(FB_msg &msg)) {
  if (telegram != NULL) {
    telegram->attach(handler);
  }
}

void Network::telegram_remove_cb_func() {
  if (telegram != NULL) {
    telegram->detach();
  }
}

void Network::telegram_reset_cb_func() {
  if (telegram != NULL) {
    telegram_set_cb_func(telegram_cb_thunk);
  }
}

bool Network::telegram_send(String msg) {
  if (telegram != NULL) {
    uint8_t result = telegram->sendMessage(msg);
    return result != 0;
  } else {
    return false;
  }
}

void Network::update() {
  if (telegram != NULL) telegram->tick();
}