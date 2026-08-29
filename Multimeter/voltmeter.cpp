#include <Arduino.h>

#include "config.h"
#include "calibration.h"
#include "voltmeter.h"

float readInputVoltage() {
  uint32_t adcSum = 0;
  const uint8_t samples = settingValue(SettingField::Smoothing) == 0 ? 8 :
                          settingValue(SettingField::Smoothing) == 2 ? 64 :
                          VOLTAGE_SAMPLE_COUNT;
  // Eine erste Wandlung verwerfen, damit der Sample-and-Hold-Kondensator nach
  // einem moeglichen Kanalwechsel sicher auf A0 eingeschwungen ist.
  analogRead(VOLTAGE_INPUT_PIN);
  for (uint8_t i = 0; i < samples; ++i) {
    adcSum += analogRead(VOLTAGE_INPUT_PIN);
  }

  const float averageAdc = adcSum / static_cast<float>(samples);
  const float voltageAtA0 = averageAdc *
                            calibrationValue(CalibrationField::AdcReference) /
                            1023.0f;
  const float dividerFactor =
      (DIVIDER_R1_OHM + DIVIDER_R2_OHM) / DIVIDER_R2_OHM;
  return voltageAtA0 * dividerFactor *
         calibrationValue(CalibrationField::VoltageFactor);
}
