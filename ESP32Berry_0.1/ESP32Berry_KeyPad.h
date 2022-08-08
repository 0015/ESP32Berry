/////////////////////////////////////////////////////////////////
/*
  ESP32Berry, "WiFi connection & Simple Note App" Version 0.1
  For More Information: https://youtu.be/wqaxCAcghtk
  Created by Eric N. (ThatProject)
*/
/////////////////////////////////////////////////////////////////
#pragma once
#include "ESP32Berry_Config.h"

class KeyPad {
private:
  static constexpr int kMinValue[] = { 3766, 3643, 3516, 3247, 2970, 1870, 1131, 828, 500, 191, 0 };
  static constexpr int kMaxValue[] = { 3876, 3753, 3626, 3357, 3080, 1980, 1241, 938, 610, 301, 80 };
  static constexpr char kPad[5][11] = {
    { 27, 49, 50, 51, 52, 53, 54, 55, 56, 57, 48 },
    { 9, 113, 119, 101, 114, 116, 121, 117, 105, 111, 112 },
    { 20, 97, 115, 100, 102, 103, 104, 106, 107, 108, 8 },
    { 17, 122, 120, 99, 118, 98, 110, 109, 44, 46, 13 },
    { 16, 18, 93, 126, 32, 32, 63, 1, 2, 3, 4 }
  };
  static constexpr char kSymbol[11] = {
    27, 33, 64, 35, 36, 37, 94, 38, 42, 40, 41
  };


  typedef void (*FuncPtrChar)(char);
  long padTimer;
  bool isReady;
  bool isShift;
  bool isCapsLock;
  void blockInput();
  int getKeyRow(int adc);

public:
  FuncPtrChar released_cb;
  KeyPad(FuncPtrChar callback);
  ~KeyPad();
  void checkKeyInput();
};