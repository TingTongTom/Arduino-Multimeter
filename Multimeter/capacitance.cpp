#include <Arduino.h>
#include <math.h>

#include "capacitance.h"
#include "config.h"
#include "calibration.h"

namespace {
CapacitanceMeasurement result = {0.0f, CapacitanceStatus::Idle, false};
uint32_t phaseStartedUs = 0;
uint32_t dischargeStartedMs = 0;
uint32_t dischargedSinceMs = 0;
bool fineRange = true;

void highImpedance(uint8_t pin) {
  digitalWrite(pin, LOW);
  pinMode(pin, INPUT);
}

void isolateOutputs() {
  highImpedance(CAPACITANCE_CHARGE_FINE_PIN);
  highImpedance(CAPACITANCE_CHARGE_COARSE_PIN);
  highImpedance(CAPACITANCE_DISCHARGE_PIN);
}

uint16_t readCapacitanceAdc() {
  analogRead(CAPACITANCE_INPUT_PIN); // erste Wandlung nach Kanalwechsel
  return analogRead(CAPACITANCE_INPUT_PIN);
}

void beginDischarge(bool preserveChargedFlag) {
  highImpedance(CAPACITANCE_CHARGE_FINE_PIN);
  highImpedance(CAPACITANCE_CHARGE_COARSE_PIN);
  digitalWrite(CAPACITANCE_DISCHARGE_PIN, LOW);
  pinMode(CAPACITANCE_DISCHARGE_PIN, OUTPUT);
  dischargeStartedMs = millis();
  dischargedSinceMs = 0;
  result.status = preserveChargedFlag && result.wasCharged
                      ? CapacitanceStatus::DischargingCharged
                      : CapacitanceStatus::Discharging;
}

void beginCharge(bool useFineRange) {
  highImpedance(CAPACITANCE_DISCHARGE_PIN);
  fineRange = useFineRange;
  const uint8_t pin = fineRange ? CAPACITANCE_CHARGE_FINE_PIN
                                : CAPACITANCE_CHARGE_COARSE_PIN;
  digitalWrite(pin, HIGH);
  pinMode(pin, OUTPUT);
  phaseStartedUs = micros();
  result.status = CapacitanceStatus::Measuring;
}

void finishFromElapsed(uint32_t elapsedUs) {
  isolateOutputs();
  const float thresholdRatio = CAPACITANCE_TARGET_ADC / 1023.0f;
  const float logarithm = -log(1.0f - thresholdRatio);
  const float reference = fineRange ? CAPACITANCE_FINE_REFERENCE_OHM
                                    : CAPACITANCE_COARSE_REFERENCE_OHM;
  const float correction = fineRange
      ? calibrationValue(CalibrationField::CapacitanceFineFactor)
      : calibrationValue(CalibrationField::CapacitanceCoarseFactor);
  result.farads = (elapsedUs * 1.0e-6f) /
                  ((reference + CAPACITANCE_PROTECTION_OHM) * logarithm) *
                  correction;
  if (result.farads < CAPACITANCE_MIN_F) {
    result.status = CapacitanceStatus::BelowRange;
  } else if (result.farads > CAPACITANCE_MAX_F) {
    result.status = CapacitanceStatus::AboveRange;
  } else {
    result.status = CapacitanceStatus::Valid;
  }
}
}

void beginCapacitanceMeter() {
  pinMode(CAPACITANCE_INPUT_PIN, INPUT);
  isolateOutputs();
}

void startCapacitanceMeasurement() {
  isolateOutputs();
  result = {0.0f, CapacitanceStatus::Idle, false};
  delayMicroseconds(20);
  result.wasCharged = readCapacitanceAdc() > CAPACITANCE_CHARGED_ADC_THRESHOLD;
  fineRange = true;
  beginDischarge(true);
}

void updateCapacitanceMeasurement() {
  if (result.status == CapacitanceStatus::Discharging ||
      result.status == CapacitanceStatus::DischargingCharged) {
    const uint32_t nowMs = millis();
    if (readCapacitanceAdc() <= CAPACITANCE_DISCHARGED_ADC_THRESHOLD) {
      if (dischargedSinceMs == 0) dischargedSinceMs = nowMs;
      if (nowMs - dischargedSinceMs >= CAPACITANCE_DISCHARGE_STABLE_MS) {
        beginCharge(fineRange);
      }
    } else {
      dischargedSinceMs = 0;
    }
    if (nowMs - dischargeStartedMs >= CAPACITANCE_DISCHARGE_TIMEOUT_MS) {
      isolateOutputs();
      result.status = CapacitanceStatus::DischargeFailed;
    }
    return;
  }

  if (result.status != CapacitanceStatus::Measuring) return;
  const uint32_t elapsedUs = micros() - phaseStartedUs;
  if (readCapacitanceAdc() >= CAPACITANCE_TARGET_ADC) {
    finishFromElapsed(elapsedUs);
    return;
  }

  const uint32_t timeoutUs = fineRange ? CAPACITANCE_FINE_TIMEOUT_US
                                       : CAPACITANCE_COARSE_TIMEOUT_US;
  if (elapsedUs >= timeoutUs) {
    if (fineRange) {
      fineRange = false;
      beginDischarge(false);
    } else {
      isolateOutputs();
      result.status = CapacitanceStatus::AboveRange;
    }
  }
}

void stopCapacitanceMeasurement() {
  isolateOutputs();
  result.status = CapacitanceStatus::Idle;
}

CapacitanceMeasurement getCapacitanceMeasurement() {
  return result;
}
