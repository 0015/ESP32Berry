/////////////////////////////////////////////////////////////////
/*
  ESP32 VNC Viewer
  For More Information: https://youtu.be/WuPIX3qxg4k
  Created by Eric N. (ThatProject)
*/
/////////////////////////////////////////////////////////////////

#include <Arduino.h>
#include <WiFi.h>
#include <VNC.h>
#include "LovyanGFX_VNCDriver.h"
#include "ESP32Berry_KeyPad.h"

const char* vnc_ip = "192.168.1.12";
const uint16_t vnc_port = 5900;
const char* vnc_pass = "12345678";

const char* ssid = "your-ssid";
const char* password = "your-password";

// ESP32Berry KeyPad
KeyPad* keyPad;

int touch_x = 0;
int touch_y = 0;
bool hadTouchEvent = false;

LGFX _lgfx;

// VNC Graphic Driver with LovyanGFX for ILI9488
// https://github.com/lovyan03/LovyanGFX
VNCDriver* lcd = new VNCDriver(&_lgfx);

// Forked Version of Arduino VNC
// https://github.com/0015/arduinoVNC/tree/0015
arduinoVNC vnc = arduinoVNC(lcd);

void callbackFromKeyPad(char key) {

  if (vnc.connected()) {

    int intKey = key;
    Serial.printf("intKey: %d\n", intKey);

    if (intKey == 10) {
      intKey = 0xff0d;  // ENTER
    } else if (key == 9) {
      intKey = 0xff09;  // Tab
    } else if (key == 8) {
      intKey = 0xff08;  // BackSpace
    } else if (key == 27) {
      intKey = 0xff1b;  // ESC
    } else if (key == 1) {
      intKey = 0xff51;  // Left
    } else if (key == 2) {
      intKey = 0xff54;  // Down
    } else if (key == 3) {
      intKey = 0xff52;  // Up
    } else if (key == 4) {
      intKey = 0xff53;  // Right
    }

    vnc.keyEvent(intKey, 0b001);
    vnc.keyEvent(intKey, 0b000);
  }
}


void setup(void) {

  Serial.begin(115200);
  _lgfx.init();

  void (*ptr)(char) = &callbackFromKeyPad;
  keyPad = new KeyPad(ptr);

  Serial.print("Connecting to ");
  Serial.println(ssid);

  lcd->print_screen("Connecting to ", ssid, TFT_YELLOW);

  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
    lcd->print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());

  lcd->print_screen("WiFi connected!", WiFi.localIP().toString(), TFT_GREEN);
  Serial.println(F("[SETUP] VNC..."));

  vnc.begin(vnc_ip, vnc_port);
  vnc.setPassword(vnc_pass);  // check for vnc server settings

  xTaskCreatePinnedToCore(vnc_task,
                          "vnc_task",
                          10000,
                          NULL,
                          1,
                          NULL,
                          0);
}

void loop() {}

void vnc_task(void* pvParameters) {
  while (1) {
    if (WiFi.status() != WL_CONNECTED) {
      lcd->print_screen("WiFi Disconnected!", "Check your WiFi", TFT_RED);
      vnc.reconnect();
      vTaskDelay(100);
    } else {
      vnc.loop();
      if (!vnc.connected()) {
        lcd->print_screen("Connecting VNC", getVNCAddr(), TFT_GREEN);
        vTaskDelay(5000);
      } else {
        keyPad->checkKeyInput();
        touchEvent();
      }
    }
    vTaskDelay(1);
  }
}

void touchEvent() {
  uint16_t x, y;
  if (_lgfx.getTouch(&x, &y)) {
    hadTouchEvent = true;
    touch_x = x;
    touch_y = y;
    vnc.mouseEvent(touch_x, touch_y, 0b001);

  } else if (hadTouchEvent) {
    hadTouchEvent = false;
    vnc.mouseEvent(touch_x, touch_y, 0b000);
  }
}

String getVNCAddr() {
  return String(vnc_ip) + String(":") + String(vnc_port);
}