#pragma once

enum class ResistanceStatus {
  Valid,
  BelowRange,
  AboveRange,
  Open
};

struct ResistanceMeasurement {
  float ohms;
  float averageAdc;
  ResistanceStatus status;
};

// Liest A1 mehrfach, mittelt die ADC-Werte und wertet den Spannungsteiler aus.
ResistanceMeasurement readResistance();
