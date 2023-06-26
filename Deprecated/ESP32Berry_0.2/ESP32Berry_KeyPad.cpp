/////////////////////////////////////////////////////////////////
/*
  ESP32Berry, "Telegram App" Version 0.2
  For More Information: https://youtu.be/h28_Mvgpe2Y
  Created by Eric N. (ThatProject)
*/
/////////////////////////////////////////////////////////////////
#include "ESP32Berry_KeyPad.h"

static KeyPad *instance = NULL;
constexpr char KeyPad::kPad[5][11];
constexpr char KeyPad::kSymbol[11];

KeyPad::KeyPad(FuncPtrChar callback) {
  instance = this;
  padTimer = 0;
  isReady = true;
  isShift = false;
  isCapsLock = false;
  released_cb = callback;
}

KeyPad::~KeyPad() {
}

int KeyPad::getKeyRow(int adc) {


  switch (adc) {
    case kMinValue[0]... kMaxValue[0]:
      return 0;
      break;
    case kMinValue[1]... kMaxValue[1]:
      return 1;
      break;
    case kMinValue[2]... kMaxValue[2]:
      return 2;
      break;
    case kMinValue[3]... kMaxValue[3]:
      return 3;
      break;
    case kMinValue[4]... kMaxValue[4]:
      return 4;
      break;
    case kMinValue[5]... kMaxValue[5]:
      return 5;
      break;
    case kMinValue[6]... kMaxValue[6]:
      return 6;
      break;
    case kMinValue[7]... kMaxValue[7]:
      return 7;
      break;
    case kMinValue[8]... kMaxValue[8]:
      return 8;
      break;
    case kMinValue[9]... kMaxValue[9]:
      return 9;
      break;
    case kMinValue[10]... kMaxValue[10]:
      return 10;
      break;
    default:
      return -1;
      break;
  }
}

void KeyPad::checkKeyInput() {

  if (isReady) {
    int sensorValue0 = analogRead(32);
    int sensorValue1 = analogRead(33);
    int sensorValue2 = analogRead(35);
    int sensorValue3 = analogRead(36);
    int sensorValue4 = analogRead(39);

    isShift = false;
    if (sensorValue4 != 4095) {
      int keyRowIdx = getKeyRow(sensorValue4);
      if (keyRowIdx != -1) {
        char key = KeyPad::kPad[4][keyRowIdx];

        if (key == 16) {
          isShift = true;
        } else {

          blockInput();
          released_cb(key);
          Serial.println(key);
        }
      }
    }

    if (sensorValue0 != 4095) {
      int keyRowIdx = getKeyRow(sensorValue0);
      if (keyRowIdx != -1) {
        blockInput();
        char key = KeyPad::kPad[0][keyRowIdx];
        if (key == 27) {
          released_cb(key);
        } else {
          if (isShift) {
            char symbol = KeyPad::kSymbol[keyRowIdx];
            released_cb(symbol);
          } else {
            released_cb(key);
          }
        }
      }
    }

    if (sensorValue1 != 4095) {

      int keyRowIdx = getKeyRow(sensorValue1);
      if (keyRowIdx != -1) {
        blockInput();
        char key = KeyPad::kPad[1][keyRowIdx];
        if (key == 9) {
          released_cb(key);
        } else {
          if (isShift || isCapsLock) {
            released_cb(char(key - 32));
          } else {
            released_cb(key);
          }
        }
      }
    }

    if (sensorValue2 != 4095) {
      int keyRowIdx = getKeyRow(sensorValue2);
      if (keyRowIdx != -1) {
        blockInput();

        char key = KeyPad::kPad[2][keyRowIdx];
        //Special Key
        if (key == 20) {
          isCapsLock = !isCapsLock;
        } else if (key == 8) {
          released_cb(key);
        } else {
          if (isShift || isCapsLock) {
            released_cb(char(key - 32));
          } else {
            released_cb(key);
          }
        }
      }
    }

    if (sensorValue3 != 4095) {
      int keyRowIdx = getKeyRow(sensorValue3);
      if (keyRowIdx != -1) {
        blockInput();
        char key = KeyPad::kPad[3][keyRowIdx];
        if (key == 44 || key == 46 || key == 13) {
          released_cb(key);
        } else {
          if (isShift || isCapsLock) {
            released_cb(char(key - 32));
          } else {
            released_cb(key);
          }
        }
      }
    }
  }

  if (!isReady && millis() - padTimer >= 200) {
    isReady = true;
  }
}

void KeyPad::blockInput() {
  isReady = false;
  padTimer = millis();
}