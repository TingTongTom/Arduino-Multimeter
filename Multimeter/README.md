# Multimeter

Das Projekt stellt ein bedienbares Multimeter mit Gleichspannungs-,
Kapazitaets-, Widerstands-, Frequenz- und bidirektionaler Gleichstrommessung
bereit. Die Strommessung nutzt ein ACS712-20A-Modul an A3; die Frequenzmessung
nutzt Timer 1 Input Capture an D8/ICP1.

Zielplattform ist ausschliesslich der klassische Arduino Nano mit ATmega328P,
5 V und 16 MHz. ESP32-Unterlagen gehoeren nicht zu diesem Repository. Das
Geraet ist ein DIY-Kleinspannungsmessgeraet ohne CAT-Einstufung, galvanische
Trennung oder Schutzbeschaltung fuer Netzspannung.

## Funktionsumfang und Grenzen

| Messart | Nennbereich | Eingang / Besonderheit |
|---|---:|---|
| Gleichspannung | 0 bis 25 V DC | A0, nur positive und massebezogene Spannung |
| Widerstand | 100 Ohm bis 1 MOhm | A1, Pruefling spannungsfrei |
| Kapazitaet | 100 nF bis 4700 uF | A2, automatische Entladung und Bereichswahl |
| Gleichstrom | -20 A bis +20 A DC | A3, ACS712-20A, Strompfad in Reihe |
| Frequenz x1 | 1 Hz bis 100 kHz | D8/ICP1, aufbereitetes 0-bis-5-V-Pulssignal |
| Frequenz x16 | 16 Hz bis 1,6 MHz | optionaler 74HC4040-Vorteiler, D12 = LOW |

Alle Messansichten bieten `LIVE`, `MIN`, `MAX`, `REL` und `HOLD`. Die
Grenzwert-, Fehler- und Sicherheitsanzeigen bleiben dabei aktiv.

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
| Kapazitaet | Messknoten | A2 | BAT43-Klemmdioden nach GND/5 V |
| Kapazitaet | Fein / Grob / Entladen | D5 / D6 / D7 | 100 kOhm / 1 kOhm / direkt zum geschuetzten Knoten |
| Frequenz | 74HC14-Ausgang Pin 2 | D8/ICP1 | 0...5 V, direkt 1 Hz...100 kHz |
| Frequenz-Erweiterung, optional | 74HC4040 /16 und DPDT-Schalter | D8/ICP1 und D12 | Umschaltbar bis 1,6 MHz |

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
`current_meter_anschlussplan.md`, `capacitance_anschlussplan.md` und
`frequency_meter_anschlussplan.md` samt
jeweiligem SVG-Schaltplan dokumentiert.
Die optionale Vorteilerplatine ist im Zusatzplan
`frequency_meter_extended_schaltplan.svg` dargestellt.

## Benoetigte Arduino-Bibliotheken

Im Bibliotheksverwalter der Arduino IDE installieren:

1. `Adafruit GFX Library`
2. `Adafruit SSD1306`

`Wire` gehoert bereits zur Arduino-Installation.

Die Firmware verwendet ausserdem die zum AVR-Core gehoerenden Bibliotheken
`EEPROM`, `Arduino` und die AVR-Interruptdefinitionen; dafuer ist keine
separate Installation im Bibliotheksverwalter erforderlich.

## Inbetriebnahme

1. Schaltung bei ausgeschalteter Versorgung verdrahten.
2. Sketch `Multimeter.ino` oeffnen.
3. Board **Arduino Nano** und den passenden Prozessor/Bootloader waehlen.
4. Bibliotheken installieren und Sketch hochladen.
5. Im Hauptmenue wechselt Drehen den Menuepunkt; ein kurzer Druck oeffnet die
   Seite. In einer Messansicht schaltet Drehen zwischen `LIVE`, `MIN`, `MAX`
   und `REL` um. Ein kurzer Druck friert die Anzeige mit `HOLD` ein oder gibt
   sie wieder frei; ein langer Druck (etwa 0,7 s) kehrt zum Hauptmenue zurueck.
   Die Kapazitaetsmessung startet beim Oeffnen automatisch mit einer sicheren
   Entladung.
   Die Frequenzmessung startet und stoppt mit ihrer Menueansicht.
6. Unter `Einstellungen` wird mit Drehen ein Eintrag ausgewaehlt. Ein kurzer
   Druck startet die Bearbeitung. Drehen aendert den Wert, ein kurzer Druck
   wechselt zwischen feiner und grober Schrittweite und ein langer Druck
   uebernimmt den Wert. Ein weiterer langer Druck fuehrt zum Hauptmenue.

Beim Wechsel auf `REL` wird der aktuelle Messwert als Nullpunkt uebernommen.
Minimum und Maximum werden beim erneuten Oeffnen einer Messansicht
standardmaessig geloescht.
Die Warn- und Plausibilitaetsanzeigen bleiben auch bei den Zusatzfunktionen
aktiv.

In `MIN` oder `MAX` setzt ein kurzer Druck beide Extremwerte auf den aktuellen
Messwert zurueck. Ob Extremwerte und Hold beim Verlassen erhalten bleiben,
kann in den Einstellungen festgelegt werden.

## Einstellungen

Folgende Werte werden gemeinsam mit Versionskennung und CRC im EEPROM
gespeichert:

- ADC-Referenz und Korrekturfaktoren aller Messarten
- Messwertdaempfung `SCHNELL`, `NORMAL` oder `RUHIG`
- Encoderdrehrichtung und ein, zwei oder vier Zustandswechsel pro Rastung
- Langdruckdauer, Displaykontrast und Aktualisierungsintervall
- Anzahl der Nachkommastellen und automatische Displayabschaltung
- Verhalten von Hold und Min/Max beim erneuten Oeffnen

`Strom jetzt nullen` muss zweimal bestaetigt werden. Dabei darf garantiert
kein Strom durch den ACS712 fliessen. Bei den Referenzkalibrierungen wird ein
bekannter Sollwert eingestellt; langer Druck uebernimmt den aktuell gemessenen
Istwert und berechnet daraus den Korrekturfaktor. Bei Kapazitaet muss vor dem
Uebernehmen `Messung bereit` erscheinen. `Werkwerte laden` verlangt ebenfalls
zwei kurze Tastendruecke.

Die Hauptmenue-Seite `Diagnose` zeigt rohe ADC-Werte, freien SRAM, EEPROM-/CRC-
Status, Firmwareversion und den erkannten Frequenzbereich.

### Einstellbereiche und Werkwerte

| Eintrag | Bereich / Auswahl | Werkwert |
|---|---|---|
| ADC-Referenz | 3,000 bis 5,500 V | 4,320 V |
| Messkorrekturfaktoren | 0,500 bis 1,500 | 1,000 |
| ACS712-Nullpunkt | 1,000 bis 4,000 V | 2,160 V |
| Daempfung | `SCHNELL`, `NORMAL`, `RUHIG` | `NORMAL` |
| Encoder-Richtung | normal / umgekehrt | normal |
| Encoder-Schritte | 1, 2 oder 4 Zustandswechsel/Rastung | 4 |
| Langdruck | 300 bis 2000 ms | 700 ms |
| OLED-Kontrast | 1 bis 255 | 127 |
| Aktualisierung | 100 bis 1000 ms | 200 ms |
| Nachkommastellen | weniger / normal / mehr | normal |
| Display aus | aus oder 1 bis 30 min | aus |
| Hold behalten | ja / nein | nein |
| Min/Max Neustart | ja / nein | ja |

Ein kurzer Druck startet die Bearbeitung und wechselt danach zwischen feiner
und grober Schrittweite. Erst ein langer Druck speichert den bearbeiteten Wert
ins EEPROM. Bei ungueltiger Versionskennung, unplausiblen Daten oder falscher
CRC verwendet die Firmware die Werkwerte aus `config.h`. `Werkwerte laden`
schreibt diese Vorgaben nach der zweiten Bestaetigung wieder ins EEPROM.

### Empfohlene Kalibrierreihenfolge

1. Geraet und Messmodule auf Betriebstemperatur kommen lassen.
2. Reale Nano-5-V-Spannung bestimmen und `ADC-Referenz` einstellen.
3. Spannung und Widerstand mit stabilen, bekannten Referenzen kalibrieren.
4. ACS712 bei garantiert 0 A mit `Strom jetzt nullen` nullen; danach den
   Stromfaktor mit einem sicheren bekannten Strom bestimmen.
5. Kapazitaetsfaktoren getrennt mit einem geeigneten kleinen Kondensator fuer
   den Feinbereich und einem geeigneten grossen Kondensator fuer den
   Grobbereich bestimmen. Nur bei `Messung bereit` uebernehmen.
6. Jeden Messbereich anschliessend mit mindestens einem zweiten Referenzwert
   kontrollieren. Kalibrierung verbessert keine konstruktiv begrenzte
   Aufloesung und ersetzt keine Sicherheitspruefung.

Die abschliessende Pinbelegung, Projektpruefung und gemeinsame
Inbetriebnahme-Checkliste stehen in `PROJEKT_PRUEFUNG.md`.

## Fehlerbehebung

- Bleibt das Display dunkel, Versorgung und I2C-Verdrahtung pruefen. Hat das
  Modul die Adresse `0x3D`, `OLED_ADDRESS` in `config.h` von `0x3C` auf `0x3D`
  aendern und neu kompilieren.
- Reagiert der Encoder pro Rastung gar nicht, mehrfach oder in der falschen
  Richtung, zuerst `Encoder Schritte` beziehungsweise `Encoder Richtung` im
  Einstellungsmenue korrigieren. Ein Vertauschen von D2 und D3 ist nicht
  erforderlich.
- Zeigt die Diagnose `EEPROM ... Werkwerte`, waren die gespeicherten Daten
  ungueltig oder nicht in Version 2 vorhanden. Werte neu kalibrieren und mit
  langem Druck speichern.
- Wacht ein abgeschaltetes OLED nicht auf, Encoder oder Taster betaetigen und
  danach Versorgung sowie Displayverkabelung pruefen.
- Bei unruhiger Anzeige `RUHIG` waehlen; dadurch steigen die ADC-Mittelungen
  und bei Frequenz die Periodenmittelung. Stoerungen in Verdrahtung,
  Massefuehrung und Versorgung muessen trotzdem hardwareseitig behoben werden.

## Sicherheit

Das Gesamtgeraet ist kein CAT-zertifiziertes Messgeraet. Niemals
Netzspannung oder eine unbekannte Spannung an Nano-Pins, Encoder oder OLED
anschliessen.

Kondensatoren vor dem Anschluss extern entladen. Die automatische Erkennung,
der 1-kOhm-Schutzwiderstand und die BAT43-Klemmdioden sind fuer kleine
Restladungen aus Kleinspannungsschaltungen ausgelegt, nicht fuer Netzspannungs-
oder Hochenergie-Kondensatoren.

Die Strommessung muss in Reihe und mit passend dimensionierter Sicherung,
Buchsen und Leitungen aufgebaut werden. Die Softwarewarnung ersetzt niemals
eine Sicherung.
