#pragma once

#include <Arduino.h>

// Hardware: klassischer Arduino Nano (ATmega328P, 5 V)
// OLED: 128x64, I2C, meist Adresse 0x3C
constexpr uint8_t OLED_WIDTH = 128;
constexpr uint8_t OLED_HEIGHT = 64;
constexpr uint8_t OLED_ADDRESS = 0x3C;
constexpr int8_t OLED_RESET = -1;

constexpr uint8_t ENCODER_A_PIN = 2;
constexpr uint8_t ENCODER_B_PIN = 3;
constexpr uint8_t ENCODER_BUTTON_PIN = 4;
constexpr uint16_t BUTTON_DEBOUNCE_MS = 30;

constexpr uint8_t VOLTAGE_INPUT_PIN = A0;
constexpr uint8_t RESISTANCE_INPUT_PIN = A1;

// Voltmeter: R1 von VIN+ zum Teilerknoten, R2 vom Teilerknoten nach GND.
// Die Widerstandswerte koennen durch gemessene Werte ersetzt werden.
// ADC_REFERENCE_VOLTAGE mit einem Multimeter an 5V kalibrieren.
constexpr float DIVIDER_R1_OHM = 56000.0f;
constexpr float DIVIDER_R2_OHM = 10000.0f;
constexpr float ADC_REFERENCE_VOLTAGE = 4.320f;
constexpr float VOLTAGE_CORRECTION_FACTOR = 1.000f;
constexpr float MAX_INPUT_VOLTAGE = 25.0f;
constexpr float OVERVOLTAGE_WARNING_THRESHOLD = MAX_INPUT_VOLTAGE;
constexpr uint8_t VOLTAGE_SAMPLE_COUNT = 32;
constexpr uint16_t VOLTAGE_UPDATE_MS = 200;

// Widerstandsmessung: 5 V -- R_REF -- Messknoten/A1 -- R_X -- GND.
// RESISTANCE_REFERENCE_OHM durch den gemessenen Wert des 10-kOhm-Widerstands
// ersetzen. Der Korrekturfaktor erlaubt eine zusaetzliche Feinabstimmung.
constexpr float RESISTANCE_REFERENCE_OHM = 10000.0f;
constexpr float RESISTANCE_CORRECTION_FACTOR = 1.000f;
constexpr float MIN_RESISTANCE_OHM = 100.0f;
constexpr float MAX_RESISTANCE_OHM = 1000000.0f;
constexpr uint16_t RESISTANCE_OPEN_ADC_THRESHOLD = 1018;
constexpr uint8_t RESISTANCE_SAMPLE_COUNT = 32;
constexpr uint16_t RESISTANCE_UPDATE_MS = 200;

// Strommessung: ACS712-20A an A3. Der Sensor liefert bei 0 A etwa die halbe
// Versorgungsspannung; das Vorzeichen ergibt sich aus der Stromrichtung.
constexpr uint8_t CURRENT_INPUT_PIN = A3;
constexpr float CURRENT_SENSITIVITY_V_PER_A = 0.100f;
constexpr float CURRENT_ZERO_VOLTAGE = 2.160f;
constexpr float CURRENT_CORRECTION_FACTOR = 1.000f;
constexpr uint8_t CURRENT_SAMPLE_COUNT = 64;
constexpr uint16_t CURRENT_UPDATE_MS = 200;

// Nur auf true setzen, wenn beim Einschalten hardwareseitig garantiert kein
// Strom durch den ACS712 fliesst. Andernfalls gilt CURRENT_ZERO_VOLTAGE.
constexpr bool CURRENT_AUTO_ZERO_AT_START = false;
constexpr bool CURRENT_ZERO_CURRENT_GUARANTEED = false;
constexpr uint8_t CURRENT_ZERO_SAMPLE_COUNT = 64;

constexpr float CURRENT_MAX_ABS_A = 20.0f;
constexpr float CURRENT_HIGH_THRESHOLD_A = 16.0f;
constexpr float CURRENT_HIGH_CLEAR_A = 15.5f;
constexpr float CURRENT_WARNING_THRESHOLD_A = 19.5f;
constexpr float CURRENT_WARNING_CLEAR_A = 19.0f;

// Werte nahe den Versorgungsschienen sind fuer das 20-A-Modul unplausibel
// und deuten auf Unterbrechung, Kurzschluss oder falsche Verdrahtung hin.
constexpr uint16_t CURRENT_PLAUSIBLE_ADC_MIN = 5;
constexpr uint16_t CURRENT_PLAUSIBLE_ADC_MAX = 1018;
