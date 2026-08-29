#include <Arduino.h>

#include "config.h"
#include "calibration.h"
#include "current_meter.h"

namespace {
float zeroVoltage = CURRENT_ZERO_VOLTAGE;
CurrentStatus latchedStatus = CurrentStatus::Valid;

float readAverageAdc(uint8_t sampleCount) {
  uint32_t adcSum = 0;
  // Erste Wandlung nach einem moeglichen Kanalwechsel verwerfen.
  analogRead(CURRENT_INPUT_PIN);
  for (uint8_t i = 0; i < sampleCount; ++i) {
    adcSum += analogRead(CURRENT_INPUT_PIN);
  }
  return adcSum / static_cast<float>(sampleCount);
}
}

void beginCurrentMeter() {
  pinMode(CURRENT_INPUT_PIN, INPUT);
  zeroVoltage = calibrationValue(CalibrationField::CurrentZero);
  if (CURRENT_AUTO_ZERO_AT_START && CURRENT_ZERO_CURRENT_GUARANTEED) {
    const float averageAdc = readAverageAdc(CURRENT_ZERO_SAMPLE_COUNT);
    if (averageAdc >= CURRENT_PLAUSIBLE_ADC_MIN &&
        averageAdc <= CURRENT_PLAUSIBLE_ADC_MAX) {
      zeroVoltage = averageAdc *
                    calibrationValue(CalibrationField::AdcReference) / 1023.0f;
    }
  }
}

float getCurrentZeroVoltage() {
  return zeroVoltage;
}

CurrentMeasurement readCurrent() {
  if (!(CURRENT_AUTO_ZERO_AT_START && CURRENT_ZERO_CURRENT_GUARANTEED))
    zeroVoltage = calibrationValue(CalibrationField::CurrentZero);
  const float averageAdc = readAverageAdc(CURRENT_SAMPLE_COUNT);
  if (averageAdc < CURRENT_PLAUSIBLE_ADC_MIN ||
      averageAdc > CURRENT_PLAUSIBLE_ADC_MAX) {
    return {0.0f, averageAdc, CurrentStatus::AdcError};
  }

  const float sensorVoltage =
      averageAdc * calibrationValue(CalibrationField::AdcReference) / 1023.0f;
  const float amperes = (sensorVoltage - zeroVoltage) /
                       CURRENT_SENSITIVITY_V_PER_A *
                       calibrationValue(CalibrationField::CurrentFactor);
  const float absoluteCurrent = fabs(amperes);

  if (absoluteCurrent > CURRENT_MAX_ABS_A) {
    return {amperes, averageAdc, CurrentStatus::OutOfRange};
  }

  if (latchedStatus == CurrentStatus::Warning) {
    if (absoluteCurrent < CURRENT_WARNING_CLEAR_A) {
      latchedStatus = absoluteCurrent >= CURRENT_HIGH_THRESHOLD_A
                          ? CurrentStatus::HighLoad
                          : CurrentStatus::Valid;
    }
  } else if (absoluteCurrent >= CURRENT_WARNING_THRESHOLD_A) {
    latchedStatus = CurrentStatus::Warning;
  } else if (latchedStatus == CurrentStatus::HighLoad) {
    if (absoluteCurrent < CURRENT_HIGH_CLEAR_A) {
      latchedStatus = CurrentStatus::Valid;
    }
  } else if (absoluteCurrent >= CURRENT_HIGH_THRESHOLD_A) {
    latchedStatus = CurrentStatus::HighLoad;
  }

  return {amperes, averageAdc, latchedStatus};
}
