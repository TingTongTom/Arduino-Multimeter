#pragma once

#include <Arduino.h>

enum class ButtonEvent : uint8_t { None, ShortPress, LongPress };
enum class ViewMode : uint8_t { Live, Minimum, Maximum, Relative };

struct ValueHistory {
  float live = 0.0f;
  float minimum = 0.0f;
  float maximum = 0.0f;
  float relativeBase = 0.0f;
  float held = 0.0f;
  bool valid = false;
};
