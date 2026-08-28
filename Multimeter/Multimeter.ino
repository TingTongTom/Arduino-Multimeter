#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#include "config.h"
#include "resistance.h"
#include "voltmeter.h"

Adafruit_SSD1306 display(OLED_WIDTH, OLED_HEIGHT, &Wire, OLED_RESET);

volatile int16_t encoderDelta = 0;
volatile uint8_t previousEncoderState = 0;

const char *const menuItems[] = {
  "Voltmeter",
  "Kapazitaet",
  "Widerstand",
  "Frequenz",
  "Einstellungen"
};
constexpr uint8_t MENU_COUNT = sizeof(menuItems) / sizeof(menuItems[0]);

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
    display.print(menuItems[item]);
  }
  display.display();
}

void drawDetail() {
  display.clearDisplay();
  display.setTextColor(SSD1306_WHITE);
  display.setTextSize(1);
  display.setCursor(0, 0);
  display.println(menuItems[selectedItem]);
  display.drawLine(0, 10, 127, 10, SSD1306_WHITE);
  display.setCursor(0, 20);
  display.println(F("Messmodul noch nicht"));
  display.println(F("angeschlossen."));
  display.setCursor(0, 52);
  display.println(F("Druecken: zurueck"));
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

void setup() {
  pinMode(ENCODER_A_PIN, INPUT_PULLUP);
  pinMode(ENCODER_B_PIN, INPUT_PULLUP);
  pinMode(ENCODER_BUTTON_PIN, INPUT_PULLUP);
  pinMode(VOLTAGE_INPUT_PIN, INPUT);
  pinMode(RESISTANCE_INPUT_PIN, INPUT);

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
    redraw = true;
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

  if (redraw) {
    if (!detailScreen) {
      drawMenu();
    } else if (selectedItem == 0) {
      drawVoltmeter();
    } else if (selectedItem == 2) {
      drawResistance();
    } else {
      drawDetail();
    }
  }
}
