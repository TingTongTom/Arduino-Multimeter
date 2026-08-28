#include <Arduino.h>

#include "config.h"
#include "voltmeter.h"

float readInputVoltage() {
  uint32_t adcSum = 0;
  // Eine erste Wandlung verwerfen, damit der Sample-and-Hold-Kondensator nach
  // einem moeglichen Kanalwechsel sicher auf A0 eingeschwungen ist.
  analogRead(VOLTAGE_INPUT_PIN);
  for (uint8_t i = 0; i < VOLTAGE_SAMPLE_COUNT; ++i) {
    adcSum += analogRead(VOLTAGE_INPUT_PIN);
  }

  const float averageAdc = adcSum / static_cast<float>(VOLTAGE_SAMPLE_COUNT);
  const float voltageAtA0 = averageAdc * ADC_REFERENCE_VOLTAGE / 1023.0f;
  const float dividerFactor =
      (DIVIDER_R1_OHM + DIVIDER_R2_OHM) / DIVIDER_R2_OHM;
  return voltageAtA0 * dividerFactor * VOLTAGE_CORRECTION_FACTOR;
}
