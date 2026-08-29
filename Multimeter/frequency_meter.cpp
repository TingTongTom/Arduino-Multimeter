#include <Arduino.h>
#include <avr/interrupt.h>

#include "config.h"
#include "frequency_meter.h"
#include "calibration.h"

namespace {
volatile uint32_t timer1Overflows = 0;
volatile uint32_t lastCaptureTicks = 0;
volatile uint32_t periodTicks = 0;
volatile uint8_t captureCount = 0;
volatile uint32_t periodSum = 0;
volatile uint8_t averagedPeriods = 0;
volatile uint8_t periodAverageCount = 4;
bool running = false;

uint32_t timer1TicksNow() {
  uint8_t savedSreg = SREG;
  cli();
  uint32_t high = timer1Overflows;
  const uint16_t low = TCNT1;
  if ((TIFR1 & _BV(TOV1)) && low < 0x8000U) ++high;
  SREG = savedSreg;
  return (high << 16) | low;
}
}

ISR(TIMER1_OVF_vect) {
  ++timer1Overflows;
}

ISR(TIMER1_CAPT_vect) {
  uint32_t high = timer1Overflows;
  const uint16_t captured = ICR1;
  // Die Capture-ISR hat Vorrang vor Overflow. Einen gleichzeitig anstehenden
  // Ueberlauf deshalb dem Zeitstempel zurechnen, wenn Capture danach lag.
  if ((TIFR1 & _BV(TOV1)) && captured < 0x8000U) ++high;
  const uint32_t now = (high << 16) | captured;
  if (captureCount != 0) {
    const uint32_t newPeriod = now - lastCaptureTicks;
    // Blockweise Mittelung reduziert die Ziffernunruhe, ohne grosse Puffer.
    periodSum += newPeriod;
    ++averagedPeriods;
    if (averagedPeriods >= periodAverageCount || periodTicks == 0) {
      periodTicks = periodSum / averagedPeriods;
      periodSum = 0;
      averagedPeriods = 0;
    }
  }
  lastCaptureTicks = now;
  if (captureCount < 2) ++captureCount;
}

void beginFrequencyMeter() {
  pinMode(FREQUENCY_INPUT_PIN, INPUT);
  pinMode(FREQUENCY_EXTENDED_RANGE_PIN, INPUT_PULLUP);
}

void startFrequencyMeasurement() {
  uint8_t savedSreg = SREG;
  cli();
  TCCR1A = 0;
  TCCR1B = 0;
  TCNT1 = 0;
  TIFR1 = _BV(ICF1) | _BV(TOV1);
  timer1Overflows = 0;
  lastCaptureTicks = 0;
  periodTicks = 0;
  captureCount = 0;
  const int16_t smoothing = settingValue(SettingField::Smoothing);
  periodAverageCount = smoothing == 0 ? 1 : smoothing == 2 ? 8 : 4;
  periodSum = 0;
  averagedPeriods = 0;
  // Steigende Flanke, digitaler Noise Canceler, Takt F_CPU/1.
  TCCR1B = _BV(ICES1) | _BV(ICNC1) | _BV(CS10);
  TIMSK1 = _BV(ICIE1) | _BV(TOIE1);
  running = true;
  SREG = savedSreg;
}

void stopFrequencyMeasurement() {
  uint8_t savedSreg = SREG;
  cli();
  TIMSK1 = 0;
  TCCR1A = 0;
  TCCR1B = 0;
  running = false;
  SREG = savedSreg;
}

FrequencyMeasurement readFrequency() {
  const bool extendedRange =
      digitalRead(FREQUENCY_EXTENDED_RANGE_PIN) == LOW;
  if (!running) return {0.0f, FrequencyStatus::NoSignal, extendedRange};

  uint32_t capturedPeriod;
  uint32_t capturedAt;
  uint8_t samples;
  noInterrupts();
  capturedPeriod = periodTicks;
  capturedAt = lastCaptureTicks;
  samples = captureCount;
  interrupts();

  const uint32_t timeoutTicks =
      (F_CPU / 1000UL) * FREQUENCY_NO_SIGNAL_TIMEOUT_MS;
  if (samples == 0 || timer1TicksNow() - capturedAt > timeoutTicks) {
    return {0.0f, FrequencyStatus::NoSignal, extendedRange};
  }
  if (samples < 2 || capturedPeriod == 0) {
    return {0.0f, FrequencyStatus::NoSignal, extendedRange};
  }

  float hertz = static_cast<float>(F_CPU) / capturedPeriod;
  if (extendedRange) hertz *= FREQUENCY_EXTENDED_DIVISOR;
  const float minHz = extendedRange ? FREQUENCY_EXTENDED_MIN_HZ
                                    : FREQUENCY_MIN_HZ;
  const float maxHz = extendedRange ? FREQUENCY_EXTENDED_MAX_HZ
                                    : FREQUENCY_MAX_HZ;
  if (hertz < minHz)
    return {hertz, FrequencyStatus::BelowRange, extendedRange};
  if (hertz > maxHz)
    return {hertz, FrequencyStatus::AboveRange, extendedRange};
  return {hertz, FrequencyStatus::Valid, extendedRange};
}
