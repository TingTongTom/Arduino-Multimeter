#pragma once

#include <Arduino.h>

enum class CapacitanceStatus {
  Idle,
  Discharging,
  DischargingCharged,
  Measuring,
  Valid,
  BelowRange,
  AboveRange,
  DischargeFailed
};

struct CapacitanceMeasurement {
  float farads;
  CapacitanceStatus status;
  bool wasCharged;
};

void beginCapacitanceMeter();
void startCapacitanceMeasurement();
void updateCapacitanceMeasurement();
void stopCapacitanceMeasurement();
CapacitanceMeasurement getCapacitanceMeasurement();
