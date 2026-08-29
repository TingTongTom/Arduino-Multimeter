#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#include "config.h"
#include "capacitance.h"
#include "current_meter.h"
#include "frequency_meter.h"
#include "resistance.h"
#include "voltmeter.h"

Adafruit_SSD1306 display(OLED_WIDTH, OLED_HEIGHT, &Wire, OLED_RESET);

volatile int16_t encoderDelta = 0;
volatile uint8_t previousEncoderState = 0;

constexpr uint8_t MENU_COUNT = 6;

const __FlashStringHelper *menuItemLabel(uint8_t item) {
  switch (item) {
    case 0: return F("Voltmeter");
    case 1: return F("Kapazitaet");
    case 2: return F("Widerstand");
    case 3: return F("Strom");
    case 4: return F("Frequenz");
    default: return F("Einstellungen");
  }
}

int8_t selectedItem = 0;
bool detailScreen = false;
bool lastButtonReading = HIGH;
bool stableButtonState = HIGH;
uint32_t lastButtonChangeMs = 0;

void updateEncoder() {
  // Zustandsfolge fuer einen mechanischen Quadraturencoder.
  static const int8_t transitionTable[16] = {
     0, -1,  1,  0,
     1,  0,  0, -1,
    -1,  0,  0,  1,
     0,  1, -1,  0
  };

  const uint8_t currentState =
      (digitalRead(ENCODER_A_PIN) << 1) | digitalRead(ENCODER_B_PIN);
  const uint8_t transition = (previousEncoderState << 2) | currentState;
  encoderDelta += transitionTable[transition];
  previousEncoderState = currentState;
}

bool buttonWasPressed() {
  const bool reading = digitalRead(ENCODER_BUTTON_PIN);

  if (reading != lastButtonReading) {
    lastButtonChangeMs = millis();
    lastButtonReading = reading;
  }

  if ((millis() - lastButtonChangeMs) >= BUTTON_DEBOUNCE_MS &&
      reading != stableButtonState) {
    stableButtonState = reading;
    return stableButtonState == LOW;
  }
  return false;
}

void drawMenu() {
  display.clearDisplay();
  display.setTextColor(SSD1306_WHITE);
  display.setTextSize(1);
  display.setCursor(0, 0);
  display.println(F("MULTIMETER"));
  display.drawLine(0, 10, 127, 10, SSD1306_WHITE);

  // Drei Eintraege anzeigen; Auswahl bleibt in der Mitte, soweit moeglich.
  int8_t first = selectedItem - 1;
  if (first < 0) first = 0;
  if (first > static_cast<int8_t>(MENU_COUNT) - 3) first = MENU_COUNT - 3;

  for (uint8_t row = 0; row < 3; ++row) {
    const int8_t item = first + row;
    const int16_t y = 16 + row * 15;
    if (item == selectedItem) {
      display.fillRect(0, y - 2, 128, 12, SSD1306_WHITE);
      display.setTextColor(SSD1306_BLACK);
    } else {
      display.setTextColor(SSD1306_WHITE);
    }
    display.setCursor(3, y);
    display.print(menuItemLabel(item));
  }
  display.display();
}

void drawDetail() {
  display.clearDisplay();
  display.setTextColor(SSD1306_WHITE);
  display.setTextSize(1);
  display.setCursor(0, 0);
  display.println(menuItemLabel(selectedItem));
  display.drawLine(0, 10, 127, 10, SSD1306_WHITE);
  display.setCursor(0, 20);
  display.println(F("Messmodul noch nicht"));
  display.println(F("angeschlossen."));
  display.setCursor(0, 52);
  display.println(F("Druecken: zurueck"));
  display.display();
}

void drawCalibration() {
  display.clearDisplay();
  display.setTextColor(SSD1306_WHITE);
  display.setTextSize(1);
  display.setCursor(0, 0);
  display.println(F("KALIBRIERUNG (nur)"));
  display.drawLine(0, 10, 127, 10, SSD1306_WHITE);

  // Reine Anzeige: Hier werden weder RAM-, EEPROM- noch Konfigurationswerte
  // veraendert. Der ACS-Nullpunkt ist der zur Laufzeit verwendete Wert.
  display.setCursor(0, 13);
  display.print(F("ADC-Ref: "));
  display.print(ADC_REFERENCE_VOLTAGE, 3);
  display.println(F(" V"));
  display.print(F("Teiler "));
  display.print(DIVIDER_R1_OHM / 1000.0f, 1);
  display.print(F("k/"));
  display.print(DIVIDER_R2_OHM / 1000.0f, 1);
  display.println(F("k"));
  display.print(F("R-Ref: "));
  display.print(RESISTANCE_REFERENCE_OHM / 1000.0f, 2);
  display.println(F(" kOhm"));
  display.print(F("ACS Null: "));
  display.print(getCurrentZeroVoltage(), 3);
  display.println(F(" V"));
  display.print(F("ACS Sens: "));
  display.print(CURRENT_SENSITIVITY_V_PER_A, 3);
  display.println(F(" V/A"));
  display.display();
}

void drawVoltmeter() {
  const float voltage = readInputVoltage();

  display.clearDisplay();
  display.setTextColor(SSD1306_WHITE);
  display.setTextSize(1);
  display.setCursor(0, 0);
  display.println(F("VOLTMETER DC"));
  display.drawLine(0, 10, 127, 10, SSD1306_WHITE);

  if (voltage >= OVERVOLTAGE_WARNING_THRESHOLD) {
    display.setTextSize(2);
    display.setCursor(4, 17);
    display.println(F("WARNUNG!"));
    display.setTextSize(1);
    display.setCursor(7, 40);
    display.println(F("> 25 V - trennen"));
  } else {
    display.setTextSize(2);
    display.setCursor(12, 22);
    display.print(voltage, 2);
    display.println(F(" V"));
  }

  display.setTextSize(1);
  display.setCursor(0, 55);
  display.println(F("Druecken: zurueck"));
  display.display();
}

void drawResistance() {
  const ResistanceMeasurement measurement = readResistance();

  display.clearDisplay();
  display.setTextColor(SSD1306_WHITE);
  display.setTextSize(1);
  display.setCursor(0, 0);
  display.println(F("WIDERSTAND"));
  display.drawLine(0, 10, 127, 10, SSD1306_WHITE);

  display.setTextSize(2);
  display.setCursor(4, 21);
  if (measurement.status == ResistanceStatus::Open) {
    display.println(F("OFFEN"));
  } else if (measurement.status == ResistanceStatus::BelowRange) {
    display.println(F("< 100 Ohm"));
  } else if (measurement.status == ResistanceStatus::AboveRange) {
    display.println(F("> 1 MOhm"));
  } else if (measurement.ohms >= 1000000.0f) {
    display.print(measurement.ohms / 1000000.0f, 2);
    display.println(F(" MOhm"));
  } else if (measurement.ohms >= 1000.0f) {
    const uint8_t decimals = measurement.ohms < 10000.0f ? 2 : 1;
    display.print(measurement.ohms / 1000.0f, decimals);
    display.println(F(" kOhm"));
  } else {
    display.print(measurement.ohms, 0);
    display.println(F(" Ohm"));
  }

  display.setTextSize(1);
  display.setCursor(0, 55);
  display.println(F("Druecken: zurueck"));
  display.display();
}

void drawCapacitance() {
  const CapacitanceMeasurement measurement = getCapacitanceMeasurement();
  display.clearDisplay();
  display.setTextColor(SSD1306_WHITE);
  display.setTextSize(1);
  display.setCursor(0, 0);
  display.println(F("KAPAZITAET"));
  display.drawLine(0, 10, 127, 10, SSD1306_WHITE);

  display.setCursor(0, 17);
  if (measurement.status == CapacitanceStatus::DischargingCharged) {
    display.println(F("Geladen erkannt"));
    display.println(F("Entlade sicher..."));
  } else if (measurement.status == CapacitanceStatus::Discharging) {
    display.println(F("Entlade..."));
  } else if (measurement.status == CapacitanceStatus::Measuring) {
    display.println(F("Messe RC-Zeit..."));
  } else if (measurement.status == CapacitanceStatus::DischargeFailed) {
    display.setTextSize(2);
    display.println(F("ABBRUCH"));
    display.setTextSize(1);
    display.println(F("Entladen fehlgeschl."));
  } else if (measurement.status == CapacitanceStatus::BelowRange) {
    display.setTextSize(2);
    display.println(F("< 100 nF"));
  } else if (measurement.status == CapacitanceStatus::AboveRange) {
    display.setTextSize(2);
    display.println(F("> 4700 uF"));
  } else if (measurement.status == CapacitanceStatus::Valid) {
    display.setTextSize(2);
    if (measurement.farads >= 1.0e-3f) {
      display.print(measurement.farads * 1.0e3f, 2);
      display.println(F(" mF"));
    } else if (measurement.farads >= 1.0e-6f) {
      display.print(measurement.farads * 1.0e6f, 2);
      display.println(F(" uF"));
    } else {
      display.print(measurement.farads * 1.0e9f, 0);
      display.println(F(" nF"));
    }
  }
  display.setTextSize(1);
  display.setCursor(0, 55);
  display.println(F("Druecken: zurueck"));
  display.display();
}

void drawCurrent() {
  const CurrentMeasurement measurement = readCurrent();

  display.clearDisplay();
  display.setTextColor(SSD1306_WHITE);
  display.setTextSize(1);
  display.setCursor(0, 0);
  display.println(F("STROM DC"));
  display.drawLine(0, 10, 127, 10, SSD1306_WHITE);

  if (measurement.status == CurrentStatus::AdcError) {
    display.setTextSize(2);
    display.setCursor(18, 20);
    display.println(F("FEHLER"));
    display.setTextSize(1);
    display.setCursor(7, 43);
    display.println(F("ADC unplausibel"));
  } else if (measurement.status == CurrentStatus::OutOfRange) {
    display.setTextSize(2);
    display.setCursor(4, 19);
    display.println(F("AUSSERHALB"));
    display.setTextSize(1);
    display.setCursor(14, 43);
    display.println(F("Messbereich 20 A"));
  } else if (measurement.status == CurrentStatus::Warning) {
    display.setTextSize(2);
    display.setCursor(15, 17);
    display.println(F("WARNUNG"));
    display.setTextSize(1);
    display.setCursor(20, 41);
    display.println(F("sofort trennen"));
  } else {
    display.setTextSize(2);
    display.setCursor(5, 17);
    if (measurement.amperes >= 0.0f) display.print('+');
    display.print(measurement.amperes, 1);
    display.println(F(" A"));
    display.setTextSize(1);
    display.setCursor(0, 39);
    display.println(measurement.amperes >= 0.0f
                        ? F("Richtung: IP+ -> IP-")
                        : F("Richtung: IP- -> IP+"));
    if (measurement.status == CurrentStatus::HighLoad) {
      display.setCursor(0, 49);
      display.println(F("HINWEIS: hohe Last"));
    }
  }

  display.setTextSize(1);
  display.setCursor(0, 56);
  display.println(F("Druecken: zurueck"));
  display.display();
}

void drawFrequency() {
  const FrequencyMeasurement measurement = readFrequency();
  display.clearDisplay();
  display.setTextColor(SSD1306_WHITE);
  display.setTextSize(1);
  display.setCursor(0, 0);
  display.println(F("FREQUENZ"));
  display.drawLine(0, 10, 127, 10, SSD1306_WHITE);
  display.setTextSize(2);
  display.setCursor(3, 21);
  if (measurement.status == FrequencyStatus::NoSignal) {
    display.println(F("KEIN PULS"));
  } else if (measurement.status == FrequencyStatus::BelowRange) {
    display.println(measurement.extendedRange ? F("< 16 Hz") : F("< 1 Hz"));
  } else if (measurement.status == FrequencyStatus::AboveRange) {
    display.println(measurement.extendedRange ? F("> 1.6 MHz")
                                              : F("> 100 kHz"));
  } else if (measurement.hertz >= 1000000.0f) {
    display.print(measurement.hertz / 1000000.0f, 3);
    display.println(F(" MHz"));
  } else if (measurement.hertz >= 1000.0f) {
    display.print(measurement.hertz / 1000.0f,
                  measurement.hertz < 10000.0f ? 3 : 2);
    display.println(F(" kHz"));
  } else {
    display.print(measurement.hertz, measurement.hertz < 10.0f ? 2 : 1);
    display.println(F(" Hz"));
  }
  display.setTextSize(1);
  display.setCursor(0, 55);
  display.print(measurement.extendedRange ? F("x16  ") : F("x1   "));
  display.println(F("Druecken: zurueck"));
  display.display();
}

void setup() {
  pinMode(ENCODER_A_PIN, INPUT_PULLUP);
  pinMode(ENCODER_B_PIN, INPUT_PULLUP);
  pinMode(ENCODER_BUTTON_PIN, INPUT_PULLUP);
  pinMode(VOLTAGE_INPUT_PIN, INPUT);
  pinMode(RESISTANCE_INPUT_PIN, INPUT);
  beginCapacitanceMeter();
  beginCurrentMeter();
  beginFrequencyMeter();

  previousEncoderState =
      (digitalRead(ENCODER_A_PIN) << 1) | digitalRead(ENCODER_B_PIN);
  attachInterrupt(digitalPinToInterrupt(ENCODER_A_PIN), updateEncoder, CHANGE);
  attachInterrupt(digitalPinToInterrupt(ENCODER_B_PIN), updateEncoder, CHANGE);

  Wire.begin();
  if (!display.begin(SSD1306_SWITCHCAPVCC, OLED_ADDRESS)) {
    // Ohne Display kann keine Fehlermeldung angezeigt werden.
    while (true) { }
  }

  display.clearDisplay();
  display.setTextColor(SSD1306_WHITE);
  display.setTextSize(1);
  display.setCursor(12, 24);
  display.println(F("System startet..."));
  display.display();
  delay(700);
  drawMenu();
}

void loop() {
  static int16_t accumulatedSteps = 0;
  static uint32_t lastVoltageUpdateMs = 0;
  static uint32_t lastResistanceUpdateMs = 0;
  static uint32_t lastCurrentUpdateMs = 0;
  static uint32_t lastFrequencyUpdateMs = 0;
  bool redraw = false;

  noInterrupts();
  const int16_t movement = encoderDelta;
  encoderDelta = 0;
  interrupts();

  accumulatedSteps += movement;
  // Die meisten KY-040-Encoder liefern vier Zustandswechsel pro Rastung.
  while (accumulatedSteps >= 4) {
    accumulatedSteps -= 4;
    if (!detailScreen) {
      selectedItem = (selectedItem + 1) % MENU_COUNT;
      redraw = true;
    }
  }
  while (accumulatedSteps <= -4) {
    accumulatedSteps += 4;
    if (!detailScreen) {
      selectedItem = (selectedItem - 1 + MENU_COUNT) % MENU_COUNT;
      redraw = true;
    }
  }

  if (buttonWasPressed()) {
    detailScreen = !detailScreen;
    if (selectedItem == 1) {
      if (detailScreen) startCapacitanceMeasurement();
      else stopCapacitanceMeasurement();
    }
    if (selectedItem == 4) {
      if (detailScreen) startFrequencyMeasurement();
      else stopFrequencyMeasurement();
    }
    redraw = true;
  }

  if (detailScreen && selectedItem == 1) {
    const CapacitanceStatus previousStatus =
        getCapacitanceMeasurement().status;
    updateCapacitanceMeasurement();
    const CapacitanceStatus currentStatus =
        getCapacitanceMeasurement().status;
    // Waehrend der RC-Ladung keine langsame OLED-Uebertragung starten. So
    // bleibt auch die 100-nF-Untergrenze zeitlich messbar.
    if (currentStatus != previousStatus &&
        currentStatus != CapacitanceStatus::Measuring &&
        currentStatus != CapacitanceStatus::Discharging) {
      redraw = true;
    }
  }

  if (detailScreen && selectedItem == 0 &&
      (millis() - lastVoltageUpdateMs >= VOLTAGE_UPDATE_MS)) {
    lastVoltageUpdateMs = millis();
    redraw = true;
  }

  if (detailScreen && selectedItem == 2 &&
      (millis() - lastResistanceUpdateMs >= RESISTANCE_UPDATE_MS)) {
    lastResistanceUpdateMs = millis();
    redraw = true;
  }

  if (detailScreen && selectedItem == 3 &&
      (millis() - lastCurrentUpdateMs >= CURRENT_UPDATE_MS)) {
    lastCurrentUpdateMs = millis();
    redraw = true;
  }

  if (detailScreen && selectedItem == 4 &&
      (millis() - lastFrequencyUpdateMs >= FREQUENCY_UPDATE_MS)) {
    lastFrequencyUpdateMs = millis();
    redraw = true;
  }

  if (redraw) {
    if (!detailScreen) {
      drawMenu();
    } else if (selectedItem == 0) {
      drawVoltmeter();
    } else if (selectedItem == 2) {
      drawResistance();
    } else if (selectedItem == 1) {
      drawCapacitance();
    } else if (selectedItem == 3) {
      drawCurrent();
    } else if (selectedItem == 4) {
      drawFrequency();
    } else if (selectedItem == 5) {
      drawCalibration();
    } else {
      drawDetail();
    }
  }
}
