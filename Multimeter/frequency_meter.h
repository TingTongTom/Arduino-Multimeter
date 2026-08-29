#pragma once

#include <Arduino.h>

enum class FrequencyStatus {
  NoSignal,
  Valid,
  BelowRange,
  AboveRange
};

struct FrequencyMeasurement {
  float hertz;
  FrequencyStatus status;
  bool extendedRange;
};

void beginFrequencyMeter();
void startFrequencyMeasurement();
void stopFrequencyMeasurement();
FrequencyMeasurement readFrequency();
