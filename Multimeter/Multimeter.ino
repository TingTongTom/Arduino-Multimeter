#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#include "config.h"
#include "calibration.h"
#include "capacitance.h"
#include "current_meter.h"
#include "frequency_meter.h"
#include "resistance.h"
#include "voltmeter.h"
#include "ui_types.h"

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
uint32_t buttonPressedMs = 0;

ViewMode viewMode = ViewMode::Live;
bool measurementHeld = false;
ValueHistory measurementHistory;
uint8_t calibrationItem = 0;
bool calibrationEditing = false;

ValueHistory &activeHistory() {
  return measurementHistory;
}

void updateHistory(float value) {
  ValueHistory &history = activeHistory();
  history.live = value;
  if (!history.valid) {
    history.minimum = history.maximum = value;
    history.valid = true;
  } else {
    if (value < history.minimum) history.minimum = value;
    if (value > history.maximum) history.maximum = value;
  }
}

float displayedValue() {
  const ValueHistory &history = activeHistory();
  if (measurementHeld) return history.held;
  if (viewMode == ViewMode::Minimum) return history.minimum;
  if (viewMode == ViewMode::Maximum) return history.maximum;
  if (viewMode == ViewMode::Relative) return history.live - history.relativeBase;
  return history.live;
}

const __FlashStringHelper *viewModeLabel() {
  if (measurementHeld) return F("HOLD");
  if (viewMode == ViewMode::Minimum) return F("MIN");
  if (viewMode == ViewMode::Maximum) return F("MAX");
  if (viewMode == ViewMode::Relative) return F("REL");
  return F("LIVE");
}

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

ButtonEvent readButtonEvent() {
  const bool reading = digitalRead(ENCODER_BUTTON_PIN);

  if (reading != lastButtonReading) {
    lastButtonChangeMs = millis();
    lastButtonReading = reading;
  }

  if ((millis() - lastButtonChangeMs) >= BUTTON_DEBOUNCE_MS &&
      reading != stableButtonState) {
    stableButtonState = reading;
    if (stableButtonState == LOW) {
      buttonPressedMs = millis();
    } else {
      return millis() - buttonPressedMs >= BUTTON_LONG_PRESS_MS
                 ? ButtonEvent::LongPress : ButtonEvent::ShortPress;
    }
  }
  return ButtonEvent::None;
}

void drawMeasurementFooter() {
  display.setTextSize(1);
  display.setCursor(0, 55);
  display.print(viewModeLabel());
  display.setCursor(43, 55);
  display.print(F("Lang: zurueck"));
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
  display.println(F("KALIBRIERUNG"));
  display.drawLine(0, 10, 127, 10, SSD1306_WHITE);

  display.setCursor(0, 15);
  if (calibrationItem == 0) display.println(F("ADC-Referenz"));
  else if (calibrationItem == 1) display.println(F("Spannung Faktor"));
  else if (calibrationItem == 2) display.println(F("Widerst. Faktor"));
  else if (calibrationItem == 3) display.println(F("Strom Nullpunkt"));
  else if (calibrationItem == 4) display.println(F("Strom Faktor"));
  else if (calibrationItem == 5) display.println(F("Kap. fein Faktor"));
  else if (calibrationItem == 6) display.println(F("Kap. grob Faktor"));
  else display.println(F("Werkwerte laden"));

  if (calibrationItem < 7) {
    display.setTextSize(2);
    display.setCursor(8, 29);
    display.print(calibrationValue(static_cast<CalibrationField>(calibrationItem)), 3);
    if (calibrationItem == 0 || calibrationItem == 3) display.print(F(" V"));
  } else {
    display.setCursor(8, 30);
    display.print(F("Druecken"));
  }
  display.setTextSize(1);
  display.setCursor(0, 55);
  display.print(calibrationEditing ? F("Drehen, Klick speich.")
                                   : F("Klick:aendern Lang:zur"));
  display.display();
}

void drawVoltmeter() {
  updateHistory(readInputVoltage());
  const float voltage = displayedValue();

  display.clearDisplay();
  display.setTextColor(SSD1306_WHITE);
  display.setTextSize(1);
  display.setCursor(0, 0);
  display.println(F("VOLTMETER DC"));
  display.drawLine(0, 10, 127, 10, SSD1306_WHITE);

  if (activeHistory().live >= OVERVOLTAGE_WARNING_THRESHOLD) {
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

  drawMeasurementFooter();
  display.display();
}

void drawResistance() {
  static ResistanceMeasurement measurement = {0.0f, 0.0f, ResistanceStatus::Open};
  measurement = readResistance();
  if (measurement.status == ResistanceStatus::Valid) updateHistory(measurement.ohms);
  const float shownOhms = activeHistory().valid ? displayedValue() : measurement.ohms;

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
  } else if (shownOhms >= 1000000.0f) {
    display.print(shownOhms / 1000000.0f, 2);
    display.println(F(" MOhm"));
  } else if (shownOhms >= 1000.0f) {
    const uint8_t decimals = shownOhms < 10000.0f ? 2 : 1;
    display.print(shownOhms / 1000.0f, decimals);
    display.println(F(" kOhm"));
  } else {
    display.print(shownOhms, 0);
    display.println(F(" Ohm"));
  }

  drawMeasurementFooter();
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
    updateHistory(measurement.farads);
    const float shownFarads = displayedValue();
    display.setTextSize(2);
    if (fabs(shownFarads) >= 1.0e-3f) {
      display.print(shownFarads * 1.0e3f, 2);
      display.println(F(" mF"));
    } else if (fabs(shownFarads) >= 1.0e-6f) {
      display.print(shownFarads * 1.0e6f, 2);
      display.println(F(" uF"));
    } else {
      display.print(shownFarads * 1.0e9f, 0);
      display.println(F(" nF"));
    }
  }
  drawMeasurementFooter();
  display.display();
}

void drawCurrent() {
  static CurrentMeasurement measurement = {0.0f, 0.0f, CurrentStatus::Valid};
  measurement = readCurrent();
  if (measurement.status != CurrentStatus::AdcError &&
      measurement.status != CurrentStatus::OutOfRange)
    updateHistory(measurement.amperes);
  const float shownAmperes = activeHistory().valid
                                 ? displayedValue() : measurement.amperes;

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
    if (shownAmperes >= 0.0f) display.print('+');
    display.print(shownAmperes, 1);
    display.println(F(" A"));
    display.setTextSize(1);
    display.setCursor(0, 39);
    display.println(shownAmperes >= 0.0f
                        ? F("Richtung: IP+ -> IP-")
                        : F("Richtung: IP- -> IP+"));
    if (measurement.status == CurrentStatus::HighLoad) {
      display.setCursor(0, 49);
      display.println(F("HINWEIS: hohe Last"));
    }
  }

  drawMeasurementFooter();
  display.display();
}

void drawFrequency() {
  static FrequencyMeasurement measurement = {0.0f, FrequencyStatus::NoSignal, false};
  measurement = readFrequency();
  if (measurement.status == FrequencyStatus::Valid) updateHistory(measurement.hertz);
  const float shownHertz = activeHistory().valid
                               ? displayedValue() : measurement.hertz;
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
  } else if (fabs(shownHertz) >= 1000000.0f) {
    display.print(shownHertz / 1000000.0f, 3);
    display.println(F(" MHz"));
  } else if (fabs(shownHertz) >= 1000.0f) {
    display.print(shownHertz / 1000.0f,
                  fabs(shownHertz) < 10000.0f ? 3 : 2);
    display.println(F(" kHz"));
  } else {
    display.print(shownHertz, fabs(shownHertz) < 10.0f ? 2 : 1);
    display.println(F(" Hz"));
  }
  drawMeasurementFooter();
  display.display();
}

void handleDetent(int8_t direction) {
  if (!detailScreen) {
    selectedItem = (selectedItem + direction + MENU_COUNT) % MENU_COUNT;
    return;
  }
  if (selectedItem == 5) {
    if (!calibrationEditing) {
      calibrationItem = (calibrationItem + direction + 8) % 8;
    } else if (calibrationItem < 7) {
      const CalibrationField field = static_cast<CalibrationField>(calibrationItem);
      setCalibrationValue(field, calibrationValue(field) + direction * 0.001f);
    }
    return;
  }
  int8_t mode = static_cast<int8_t>(viewMode) + direction;
  if (mode < 0) mode = 3;
  if (mode > 3) mode = 0;
  viewMode = static_cast<ViewMode>(mode);
  if (viewMode == ViewMode::Relative && activeHistory().valid)
    activeHistory().relativeBase = activeHistory().live;
}

void leaveDetailScreen() {
  if (selectedItem == 1) stopCapacitanceMeasurement();
  if (selectedItem == 4) stopFrequencyMeasurement();
  if (selectedItem == 5 && calibrationEditing) {
    saveCalibration();
    calibrationEditing = false;
  }
  detailScreen = false;
  measurementHeld = false;
}

void setup() {
  pinMode(ENCODER_A_PIN, INPUT_PULLUP);
  pinMode(ENCODER_B_PIN, INPUT_PULLUP);
  pinMode(ENCODER_BUTTON_PIN, INPUT_PULLUP);
  pinMode(VOLTAGE_INPUT_PIN, INPUT);
  pinMode(RESISTANCE_INPUT_PIN, INPUT);
  beginCalibration();
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
    handleDetent(1);
    redraw = true;
  }
  while (accumulatedSteps <= -4) {
    accumulatedSteps += 4;
    handleDetent(-1);
    redraw = true;
  }

  const ButtonEvent buttonEvent = readButtonEvent();
  if (buttonEvent == ButtonEvent::ShortPress) {
    if (!detailScreen) {
      detailScreen = true;
      viewMode = ViewMode::Live;
      measurementHeld = false;
      if (selectedItem < 5) measurementHistory = ValueHistory();
      if (selectedItem == 1) startCapacitanceMeasurement();
      if (selectedItem == 4) startFrequencyMeasurement();
    } else if (selectedItem == 5) {
      if (calibrationItem == 7) {
        resetCalibration();
      } else {
        calibrationEditing = !calibrationEditing;
        if (!calibrationEditing) saveCalibration();
      }
    } else {
      if (!measurementHeld && activeHistory().valid)
        activeHistory().held = displayedValue();
      measurementHeld = !measurementHeld;
    }
    redraw = true;
  } else if (buttonEvent == ButtonEvent::LongPress && detailScreen) {
    leaveDetailScreen();
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
