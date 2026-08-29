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

constexpr uint8_t MENU_COUNT = 7;
constexpr uint8_t SETTINGS_COUNT = 24;

const __FlashStringHelper *menuItemLabel(uint8_t item) {
  switch (item) {
    case 0: return F("Voltmeter");
    case 1: return F("Kapazitaet");
    case 2: return F("Widerstand");
    case 3: return F("Strom");
    case 4: return F("Frequenz");
    case 5: return F("Einstellungen");
    default: return F("Diagnose");
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
int8_t historyOwner = -1;
uint8_t calibrationItem = 0;
bool calibrationEditing = false;
bool calibrationCoarse = false;
bool resetPending = false;
float referenceTarget = 1.0f;
uint32_t lastInteractionMs = 0;
bool displaySleeping = false;

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
      return millis() - buttonPressedMs >= static_cast<uint32_t>(
                         settingValue(SettingField::LongPressMs))
                 ? ButtonEvent::LongPress : ButtonEvent::ShortPress;
    }
  }
  return ButtonEvent::None;
}

uint8_t displayDecimals(uint8_t normal) {
  const int16_t mode = settingValue(SettingField::DecimalMode);
  if (mode == 0 && normal > 0) return normal - 1;
  if (mode == 2 && normal < 4) return normal + 1;
  return normal;
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

  display.setCursor(0, 14);
  if (calibrationItem == 0) display.println(F("ADC-Referenz"));
  else if (calibrationItem == 1) display.println(F("Spannung Faktor"));
  else if (calibrationItem == 2) display.println(F("Widerst. Faktor"));
  else if (calibrationItem == 3) display.println(F("Strom Nullpunkt"));
  else if (calibrationItem == 4) display.println(F("Strom Faktor"));
  else if (calibrationItem == 5) display.println(F("Kap. fein Faktor"));
  else if (calibrationItem == 6) display.println(F("Kap. grob Faktor"));
  else if (calibrationItem == 7) display.println(F("Daempfung"));
  else if (calibrationItem == 8) display.println(F("Encoder Richtung"));
  else if (calibrationItem == 9) display.println(F("Encoder Schritte"));
  else if (calibrationItem == 10) display.println(F("Langdruck Zeit"));
  else if (calibrationItem == 11) display.println(F("Display Kontrast"));
  else if (calibrationItem == 12) display.println(F("Aktualisierung"));
  else if (calibrationItem == 13) display.println(F("Nachkommastellen"));
  else if (calibrationItem == 14) display.println(F("Display Aus"));
  else if (calibrationItem == 15) display.println(F("Hold behalten"));
  else if (calibrationItem == 16) display.println(F("Min/Max Neustart"));
  else if (calibrationItem == 17) display.println(F("Strom jetzt nullen"));
  else if (calibrationItem == 18) display.println(F("Spannung Referenz"));
  else if (calibrationItem == 19) display.println(F("Widerst. Referenz"));
  else if (calibrationItem == 20) display.println(F("Strom Referenz"));
  else if (calibrationItem == 21) display.println(F("Kap. fein Referenz"));
  else if (calibrationItem == 22) display.println(F("Kap. grob Referenz"));
  else display.println(F("Werkwerte laden"));

  display.setTextSize(2);
  display.setCursor(5, 28);
  if (calibrationItem < 7) {
    display.print(calibrationValue(static_cast<CalibrationField>(calibrationItem)), 3);
  } else if (calibrationItem <= 16) {
    const int16_t value = settingValue(static_cast<SettingField>(calibrationItem - 7));
    if (calibrationItem == 7) display.print(value == 0 ? F("SCHNELL") : value == 1 ? F("NORMAL") : F("RUHIG"));
    else if (calibrationItem == 8) display.print(value < 0 ? F("UMGEKEHRT") : F("NORMAL"));
    else if (calibrationItem == 13) display.print(value == 0 ? F("WENIG") : value == 1 ? F("NORMAL") : F("MEHR"));
    else if (calibrationItem == 14) { if (value == 0) display.print(F("AUS")); else { display.print(value); display.print(F(" min")); } }
    else if (calibrationItem >= 15) display.print(value ? F("JA") : F("NEIN"));
    else { display.print(value); if (calibrationItem == 10 || calibrationItem == 12) display.print(F(" ms")); }
  } else if (calibrationItem <= 22 && calibrationEditing) {
    display.print(referenceTarget, calibrationItem == 19 ? 0 : 3);
    if (calibrationItem == 18) display.print(F(" V"));
    else if (calibrationItem == 19) display.print(F(" Ohm"));
    else if (calibrationItem == 20) display.print(F(" A"));
    else display.print(F(" uF"));
  } else {
    display.print(resetPending ? F("SICHER?") : F("DRUECKEN"));
  }
  display.setTextSize(1);
  if (calibrationEditing && calibrationItem >= 21 && calibrationItem <= 22) {
    display.setCursor(0, 45);
    display.print(getCapacitanceMeasurement().status == CapacitanceStatus::Valid
                      ? F("Messung bereit") : F("Messung laeuft"));
  }
  display.setCursor(0, 55);
  display.print(calibrationEditing ? (calibrationCoarse ? F("GROB Klick:fein Lang:OK")
                                                       : F("FEIN Klick:grob Lang:OK"))
                                   : F("Klick:wahl Lang:zurueck"));
  display.display();
}

int freeRam() {
  extern int __heap_start, *__brkval;
  int local;
  return reinterpret_cast<int>(&local) -
         reinterpret_cast<int>(__brkval ? __brkval : &__heap_start);
}

void drawDiagnostics() {
  display.clearDisplay();
  display.setTextColor(SSD1306_WHITE);
  display.setTextSize(1);
  display.setCursor(0, 0);
  display.print(F("DIAGNOSE FW "));
  display.println(F("2.0"));
  display.drawLine(0, 10, 127, 10, SSD1306_WHITE);
  display.setCursor(0, 14);
  display.print(F("ADC "));
  display.print(analogRead(A0)); display.print('/');
  display.print(analogRead(A1)); display.print('/');
  display.print(analogRead(A2)); display.print('/');
  display.println(analogRead(A3));
  display.print(F("SRAM frei: ")); display.println(freeRam());
  display.print(F("EEPROM v")); display.print(calibrationStorageVersion());
  display.println(calibrationStorageValid() ? F(" CRC OK") : F(" Werkwerte"));
  display.print(F("Freq: "));
  display.println(digitalRead(FREQUENCY_EXTENDED_RANGE_PIN) ? F("x1") : F("x16"));
  display.setCursor(0, 55);
  display.println(F("Lang: zurueck"));
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
    display.print(voltage, displayDecimals(2));
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
    display.print(shownOhms / 1000000.0f, displayDecimals(2));
    display.println(F(" MOhm"));
  } else if (shownOhms >= 1000.0f) {
    const uint8_t decimals = shownOhms < 10000.0f ? 2 : 1;
    display.print(shownOhms / 1000.0f, displayDecimals(decimals));
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
      display.print(shownFarads * 1.0e3f, displayDecimals(2));
      display.println(F(" mF"));
    } else if (fabs(shownFarads) >= 1.0e-6f) {
      display.print(shownFarads * 1.0e6f, displayDecimals(2));
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
    display.print(shownAmperes, displayDecimals(1));
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
    display.print(shownHertz / 1000000.0f, displayDecimals(3));
    display.println(F(" MHz"));
  } else if (fabs(shownHertz) >= 1000.0f) {
    display.print(shownHertz / 1000.0f,
                  displayDecimals(fabs(shownHertz) < 10000.0f ? 3 : 2));
    display.println(F(" kHz"));
  } else {
    display.print(shownHertz,
                  displayDecimals(fabs(shownHertz) < 10.0f ? 2 : 1));
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
      calibrationItem = (calibrationItem + direction + SETTINGS_COUNT) % SETTINGS_COUNT;
      resetPending = false;
    } else if (calibrationItem < 7) {
      const CalibrationField field = static_cast<CalibrationField>(calibrationItem);
      setCalibrationValue(field, calibrationValue(field) +
                          direction * (calibrationCoarse ? 0.010f : 0.001f));
    } else if (calibrationItem <= 16) {
      const SettingField field = static_cast<SettingField>(calibrationItem - 7);
      int16_t step = 1;
      if (calibrationItem == 10) step = calibrationCoarse ? 100 : 50;
      if (calibrationItem == 11) step = calibrationCoarse ? 16 : 4;
      if (calibrationItem == 12) step = calibrationCoarse ? 100 : 50;
      if (calibrationItem == 14) step = calibrationCoarse ? 5 : 1;
      int16_t value = settingValue(field);
      if (calibrationItem == 8) value = -value;
      else if (calibrationItem == 9) value = direction > 0 ? value * 2 : value / 2;
      else if (calibrationItem == 15 || calibrationItem == 16) value = !value;
      else value += direction * step;
      setSettingValue(field, value);
      if (calibrationItem == 11) {
        display.ssd1306_command(SSD1306_SETCONTRAST);
        display.ssd1306_command(settingValue(field));
      }
    } else if (calibrationItem <= 22) {
      float step = calibrationCoarse ? 1.0f : 0.1f;
      if (calibrationItem == 19) step = calibrationCoarse ? 1000.0f : 100.0f;
      if (calibrationItem >= 21) step = calibrationCoarse ? 10.0f : 1.0f;
      referenceTarget = max(0.001f, referenceTarget + direction * step);
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

void applyReferenceCalibration() {
  float measured = 0.0f;
  CalibrationField field = CalibrationField::VoltageFactor;
  if (calibrationItem == 18) {
    measured = readInputVoltage();
    field = CalibrationField::VoltageFactor;
  } else if (calibrationItem == 19) {
    const ResistanceMeasurement result = readResistance();
    if (result.status == ResistanceStatus::Valid) measured = result.ohms;
    field = CalibrationField::ResistanceFactor;
  } else if (calibrationItem == 20) {
    const CurrentMeasurement result = readCurrent();
    if (result.status != CurrentStatus::AdcError) measured = fabs(result.amperes);
    field = CalibrationField::CurrentFactor;
  } else {
    const CapacitanceMeasurement result = getCapacitanceMeasurement();
    if (result.status == CapacitanceStatus::Valid) measured = result.farads * 1.0e6f;
    field = calibrationItem == 21 ? CalibrationField::CapacitanceFineFactor
                                  : CalibrationField::CapacitanceCoarseFactor;
  }
  if (measured > 0.000001f) {
    setCalibrationValue(field, calibrationValue(field) * referenceTarget / measured);
    saveCalibration();
  }
  if (calibrationItem >= 21) stopCapacitanceMeasurement();
}

void leaveDetailScreen() {
  if (selectedItem == 1) stopCapacitanceMeasurement();
  if (selectedItem == 4) stopFrequencyMeasurement();
  if (selectedItem == 5 && calibrationEditing) {
    saveCalibration();
    calibrationEditing = false;
  }
  detailScreen = false;
  if (!settingValue(SettingField::KeepHold)) measurementHeld = false;
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
  display.ssd1306_command(SSD1306_SETCONTRAST);
  display.ssd1306_command(settingValue(SettingField::DisplayContrast));
  delay(700);
  lastInteractionMs = millis();
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

  accumulatedSteps += movement * settingValue(SettingField::EncoderDirection);
  const int16_t encoderSteps = settingValue(SettingField::EncoderSteps);
  while (accumulatedSteps >= encoderSteps) {
    accumulatedSteps -= encoderSteps;
    handleDetent(1);
    redraw = true;
  }
  while (accumulatedSteps <= -encoderSteps) {
    accumulatedSteps += encoderSteps;
    handleDetent(-1);
    redraw = true;
  }

  const ButtonEvent buttonEvent = readButtonEvent();
  if (buttonEvent == ButtonEvent::ShortPress) {
    if (!detailScreen) {
      detailScreen = true;
      viewMode = ViewMode::Live;
      if (!settingValue(SettingField::KeepHold) || historyOwner != selectedItem ||
          settingValue(SettingField::ResetMinMax))
        measurementHeld = false;
      if (selectedItem < 5 &&
          (settingValue(SettingField::ResetMinMax) || historyOwner != selectedItem))
        measurementHistory = ValueHistory();
      if (selectedItem < 5) historyOwner = selectedItem;
      if (selectedItem == 1) startCapacitanceMeasurement();
      if (selectedItem == 4) startFrequencyMeasurement();
    } else if (selectedItem == 5) {
      if (calibrationItem == 23) {
        if (resetPending) { resetCalibration(); resetPending = false; }
        else resetPending = true;
      } else if (calibrationItem == 17) {
        if (!resetPending) {
          resetPending = true;
        } else {
          const CurrentMeasurement result = readCurrent();
          setCalibrationValue(CalibrationField::CurrentZero,
              result.averageAdc * calibrationValue(CalibrationField::AdcReference) / 1023.0f);
          saveCalibration();
          resetPending = false;
        }
      } else if (!calibrationEditing) {
        calibrationEditing = true;
        calibrationCoarse = false;
        if (calibrationItem == 18) referenceTarget = 5.0f;
        else if (calibrationItem == 19) referenceTarget = 10000.0f;
        else if (calibrationItem == 20) referenceTarget = 1.0f;
        else if (calibrationItem == 21) { referenceTarget = 1.0f; startCapacitanceMeasurement(); }
        else if (calibrationItem == 22) { referenceTarget = 1000.0f; startCapacitanceMeasurement(); }
      } else {
        calibrationCoarse = !calibrationCoarse;
      }
    } else {
      if ((viewMode == ViewMode::Minimum || viewMode == ViewMode::Maximum) &&
          activeHistory().valid) {
        activeHistory().minimum = activeHistory().maximum = activeHistory().live;
      } else {
        if (!measurementHeld && activeHistory().valid)
          activeHistory().held = displayedValue();
        measurementHeld = !measurementHeld;
      }
    }
    redraw = true;
  } else if (buttonEvent == ButtonEvent::LongPress && detailScreen) {
    if (selectedItem == 5 && calibrationEditing) {
      if (calibrationItem >= 18 && calibrationItem <= 22)
        applyReferenceCalibration();
      else
        saveCalibration();
      calibrationEditing = false;
    } else {
      leaveDetailScreen();
    }
    redraw = true;
  }

  if (movement != 0 || buttonEvent != ButtonEvent::None) {
    lastInteractionMs = millis();
    if (displaySleeping) {
      display.ssd1306_command(SSD1306_DISPLAYON);
      displaySleeping = false;
      redraw = true;
    }
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

  if (detailScreen && selectedItem == 5 && calibrationEditing &&
      calibrationItem >= 21 && calibrationItem <= 22) {
    const CapacitanceStatus previousStatus = getCapacitanceMeasurement().status;
    updateCapacitanceMeasurement();
    const CapacitanceStatus currentStatus = getCapacitanceMeasurement().status;
    if (currentStatus != previousStatus &&
        currentStatus != CapacitanceStatus::Measuring &&
        currentStatus != CapacitanceStatus::Discharging)
      redraw = true;
  }

  if (detailScreen && selectedItem == 0 &&
      (millis() - lastVoltageUpdateMs >= static_cast<uint32_t>(
          settingValue(SettingField::UpdateIntervalMs)))) {
    lastVoltageUpdateMs = millis();
    redraw = true;
  }

  if (detailScreen && selectedItem == 2 &&
      (millis() - lastResistanceUpdateMs >= static_cast<uint32_t>(
          settingValue(SettingField::UpdateIntervalMs)))) {
    lastResistanceUpdateMs = millis();
    redraw = true;
  }

  if (detailScreen && selectedItem == 3 &&
      (millis() - lastCurrentUpdateMs >= static_cast<uint32_t>(
          settingValue(SettingField::UpdateIntervalMs)))) {
    lastCurrentUpdateMs = millis();
    redraw = true;
  }

  if (detailScreen && selectedItem == 4 &&
      (millis() - lastFrequencyUpdateMs >= static_cast<uint32_t>(
          settingValue(SettingField::UpdateIntervalMs)))) {
    lastFrequencyUpdateMs = millis();
    redraw = true;
  }

  const CapacitanceStatus settingsCapStatus = getCapacitanceMeasurement().status;
  const bool capTimingCritical = detailScreen && selectedItem == 5 &&
      calibrationEditing && calibrationItem >= 21 && calibrationItem <= 22 &&
      (settingsCapStatus == CapacitanceStatus::Measuring ||
       settingsCapStatus == CapacitanceStatus::Discharging);
  if (redraw && !capTimingCritical) {
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
    } else if (selectedItem == 6) {
      drawDiagnostics();
    } else {
      drawDetail();
    }
  }

  const int16_t timeoutMin = settingValue(SettingField::DisplayTimeoutMin);
  if (!displaySleeping && timeoutMin > 0 &&
      millis() - lastInteractionMs >= static_cast<uint32_t>(timeoutMin) * 60000UL) {
    display.ssd1306_command(SSD1306_DISPLAYOFF);
    displaySleeping = true;
  }
}
