#include <Arduino.h>

#include "config.h"
#include "calibration.h"
#include "resistance.h"

ResistanceMeasurement readResistance() {
  uint32_t adcSum = 0;
  const uint8_t samples = settingValue(SettingField::Smoothing) == 0 ? 8 :
                          settingValue(SettingField::Smoothing) == 2 ? 64 :
                          RESISTANCE_SAMPLE_COUNT;

  // Erste Wandlung nach einem moeglichen Kanalwechsel von A0 verwerfen.
  analogRead(RESISTANCE_INPUT_PIN);
  for (uint8_t i = 0; i < samples; ++i) {
    adcSum += analogRead(RESISTANCE_INPUT_PIN);
  }

  const float averageAdc =
      adcSum / static_cast<float>(samples);

  if (averageAdc >= RESISTANCE_OPEN_ADC_THRESHOLD) {
    return {0.0f, averageAdc, ResistanceStatus::Open};
  }

  // R_X liegt unterhalb des Messknotens gegen GND.
  const float resistance = RESISTANCE_REFERENCE_OHM * averageAdc /
                           (1023.0f - averageAdc) *
                           calibrationValue(CalibrationField::ResistanceFactor);

  if (resistance < MIN_RESISTANCE_OHM) {
    return {resistance, averageAdc, ResistanceStatus::BelowRange};
  }
  if (resistance > MAX_RESISTANCE_OHM) {
    return {resistance, averageAdc, ResistanceStatus::AboveRange};
  }
  return {resistance, averageAdc, ResistanceStatus::Valid};
}
