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
