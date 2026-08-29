#pragma once

enum class CurrentStatus {
  Valid,
  HighLoad,
  Warning,
  AdcError,
  OutOfRange
};

struct CurrentMeasurement {
  float amperes;
  float averageAdc;
  CurrentStatus status;
};

// Initialisiert den Nullpunkt. Eine automatische Bestimmung erfolgt nur,
// wenn CURRENT_AUTO_ZERO_AT_START in config.h bewusst aktiviert wurde.
void beginCurrentMeter();

// Liefert den tatsaechlich verwendeten Nullpunkt (fest konfiguriert oder,
// falls bewusst aktiviert, beim Start sicher automatisch bestimmt).
float getCurrentZeroVoltage();

// Liest A3 mindestens 64-mal und bewertet Betrag, Plausibilitaet und die
// Warnstufen mit Hysterese.
CurrentMeasurement readCurrent();
