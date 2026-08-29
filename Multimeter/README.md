# Multimeter

Das Projekt stellt ein bedienbares Multimeter mit Gleichspannungs-,
Widerstands- und bidirektionaler Gleichstrommessung bereit. Die Strommessung
nutzt ein ACS712-20A-Modul an A3.

## Anschlussplan

| Bauteil | Anschluss | Arduino Nano | Hinweis |
|---|---|---|---|
| OLED 128x64 I2C | VCC | 5V | Nur wenn das Modul 5 V vertraegt; sonst 3.3V |
| OLED 128x64 I2C | GND | GND | Gemeinsame Masse |
| OLED 128x64 I2C | SCL | A5 | I2C-Takt |
| OLED 128x64 I2C | SDA | A4 | I2C-Daten |
| Drehencoder | `+` / VCC | 5V | Bei einem KY-040-Modul |
| Drehencoder | GND | GND | Gemeinsame Masse |
| Drehencoder | CLK / A | D2 | Interrupt INT0 |
| Drehencoder | DT / B | D3 | Interrupt INT1 |
| Drehencoder | SW | D4 | Taster gegen GND, interner Pull-up aktiv |
| ACS712-20A | VCC / GND | 5V / GND | Sensorversorgung |
| ACS712-20A | OUT | ueber 1 kOhm an A3 | 100 nF von A3 nach GND |

Bei einem nackten mechanischen Encoder wird der gemeinsame Kontakt an GND
gelegt. A kommt an D2, B an D3 und der Taster zwischen D4 und GND. Die internen
Pull-up-Widerstaende des Nano werden vom Programm eingeschaltet.

## Vereinfachter Schaltplan

```text
                         Arduino Nano
                    +--------------------+
          +5 V  ----| 5V              A5 |---- SCL  OLED
          GND   ----| GND             A4 |---- SDA  OLED
                    |                    |
 Encoder CLK/A  ----| D2                 |
 Encoder DT/B   ----| D3                 |
 Encoder SW     ----| D4                 |
                    +--------------------+

 OLED:     VCC -> 5 V*       GND -> GND
 Encoder:  VCC -> 5 V        GND -> GND

 * Vorher die Spannungsangabe des OLED-Moduls pruefen. Viele SSD1306-Module
   akzeptieren 3,3 bis 5 V, aber nicht jedes Modul ist gleich aufgebaut.
```

Der grafische Grundplan liegt in `schaltplan.svg`. Die Messmodule sind in
`voltmeter_anschlussplan.md`, `resistance_anschlussplan.md` und
`current_meter_anschlussplan.md` samt jeweiligem SVG-Schaltplan dokumentiert.

## Benoetigte Arduino-Bibliotheken

Im Bibliotheksverwalter der Arduino IDE installieren:

1. `Adafruit GFX Library`
2. `Adafruit SSD1306`

`Wire` gehoert bereits zur Arduino-Installation.

## Inbetriebnahme

1. Schaltung bei ausgeschalteter Versorgung verdrahten.
2. Sketch `Multimeter.ino` oeffnen.
3. Board **Arduino Nano** und den passenden Prozessor/Bootloader waehlen.
4. Bibliotheken installieren und Sketch hochladen.
5. Drehen wechselt den Menuepunkt; Druecken oeffnet oder schliesst die Seite.

Bleibt das Display dunkel, ist seine I2C-Adresse eventuell `0x3D`. Dann im
Sketch `OLED_ADDRESS` von `0x3C` auf `0x3D` aendern. Reagiert eine Rastung zu
langsam oder doppelt, muss der Schwellwert `4` im Hauptprogramm passend zum
Encoder auf `2` oder `1` geaendert werden. Sind Drehrichtungen vertauscht,
werden D2 und D3 miteinander getauscht.

## Sicherheit

Diese Grundschaltung ist keine geschuetzte Messeingangsschaltung. Niemals
Netzspannung oder eine unbekannte Spannung an Nano-Pins, Encoder oder OLED
anschliessen.

Die Strommessung muss in Reihe und mit passend dimensionierter Sicherung,
Buchsen und Leitungen aufgebaut werden. Die Softwarewarnung ersetzt niemals
eine Sicherung.
