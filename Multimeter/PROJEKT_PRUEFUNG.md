# Projektpruefung und Inbetriebnahme

Stand der Pruefung: klassischer Arduino Nano mit ATmega328P, 16 MHz und 5 V.
Der Menuepunkt `Einstellungen` erlaubt die Kalibrierung von ADC-Referenz,
Spannungs-, Widerstands- und Stromkorrektur, ACS712-Nullpunkt sowie den beiden
Kapazitaetsbereichen. Bestaetigte Werte werden mit Versionskennung und CRC im
EEPROM gespeichert. Unplausible, beschaedigte oder veraltete EEPROM-Daten
werden ignoriert; dann gelten die Werkwerte aus `config.h`. Die optionale
ACS-Autonullung bleibt in `config.h` standardmaessig doppelt gesperrt.

Alle Messansichten besitzen ohne weitere Hardware `HOLD`, `MIN`, `MAX` und
`REL`. Die Frequenzanzeige mittelt vier aufeinanderfolgende Perioden, um die
Ziffernunruhe zu reduzieren. Eine erste Periode steht weiterhin unmittelbar
zur Verfuegung.

Die Einstellungen enthalten ausserdem drei Daempfungsstufen, Encoder-Richtung
und -Raster, Langdruckdauer, OLED-Kontrast, Aktualisierungsintervall,
Nachkommastellen, Display-Timeout sowie das Verhalten von Hold und Min/Max.
Gefuehrte Referenzkalibrierungen berechnen den jeweiligen Faktor aus Soll- und
Istwert. Stromnullung und Werkreset sind gegen versehentliches Ausloesen durch
eine zweite Bestaetigung geschuetzt. Die Diagnoseansicht zeigt ADC-Rohwerte,
freien SRAM, Firmware-, EEPROM- und Frequenzbereichsstatus.

## Abschliessende Pinbelegung

| Nano-Pin | Funktion | Betriebsart / Hinweis |
|---|---|---|
| D0, D1 | frei / serielle Schnittstelle | Fuer Upload und Diagnose freihalten |
| D2 | Encoder A | `INPUT_PULLUP`, INT0 |
| D3 | Encoder B | `INPUT_PULLUP`, INT1 |
| D4 | Encoder-Taster | `INPUT_PULLUP`, Taster nach GND |
| D5 | Kapazitaet Feinladung | Ueber 100 kOhm zum geschuetzten Messknoten; sonst hochohmig |
| D6 | Kapazitaet Grobladung | Ueber 1 kOhm zum geschuetzten Messknoten; sonst hochohmig |
| D7 | Kapazitaet Entladung | Zum geschuetzten Messknoten; sonst hochohmig |
| D8 | Frequenzeingang | ICP1, Ausgang des 74HC14 beziehungsweise des x16-Umschalters |
| D9, D10 | frei, aber kein PWM bei Frequenzmessung | Timer 1 wird fuer Input Capture exklusiv verwendet |
| D11, D13 | frei | D13 besitzt die Nano-Onboard-LED |
| D12 | Frequenzbereich x1/x16 | `INPUT_PULLUP`; LOW bedeutet x16 |
| A0 | Spannung | Spannungsteiler 56 kOhm / 10 kOhm, Schutzbeschaltung |
| A1 | Widerstand | Messknoten mit 10-kOhm-Referenz und Schutzwiderstand |
| A2 | Kapazitaet | Geschuetzter RC-Messknoten |
| A3 | Strom | ACS712 OUT ueber 1 kOhm, 100 nF nach GND |
| A4 | OLED SDA | I2C |
| A5 | OLED SCL | I2C |
| A6, A7 | frei | Nur Analogeingang, nicht als Digitalausgang nutzbar |

Ergebnis der Konfliktpruefung: Es gibt keine doppelte Pinbelegung. Timer 1
belegt waehrend der Frequenzansicht die Zeitmessung und damit die PWM-Funktion
auf D9/D10; keine andere Projektfunktion benutzt diese Pins. `millis()` und
`micros()` laufen ueber Timer 0 weiter.

## Ressourcen, ADC und Aktualisierungszeiten

- Der Build mit `arduino:avr:nano:cpu=atmega328old` ist massgeblich; die
  konkreten Buildwerte stehen im Abschnitt `Buildnachweis`.
- Der SSD1306-Vollbildpuffer belegt zur Laufzeit zusaetzlich etwa 1024 Byte
  SRAM. Deshalb liegen die Menuebeschriftungen mit `F()` im Flash. Auf dem
  ATmega328P bleibt der RAM-Spielraum begrenzt; keine grossen lokalen Puffer
  oder `String`-Objekte ergaenzen.
- A0 und A1 verwerfen nach jedem moeglichen ADC-Kanalwechsel eine Messung und
  mitteln je nach Daempfung 8, 32 oder 64 folgende Wandlungen. A3 verwirft
  ebenfalls eine Wandlung und mittelt 16, 64 oder 128 Werte. A2 verwirft eine
  Wandlung und verwendet die zweite. Damit ist das Umschalten der
  Sample-and-Hold-Stufe zwischen allen vier Kanaelen beruecksichtigt.
- Spannung, Widerstand, Strom und Frequenz werden mit dem einstellbaren
  Intervall von 100 bis 1000 ms aktualisiert; Werkwert sind 200 ms. Die
  Frequenzmessung laeuft davon unabhaengig per Input Capture und mittelt je
  nach Daempfung 1, 4 oder 8 Perioden. Eine erste vollstaendige Periode steht
  weiterhin unmittelbar zur Verfuegung.
- Die Kapazitaetsmessung ist zustandsbasiert und blockiert nicht. OLED-Zugriffe
  werden waehrend zeitkritischer Lade- und Entladephasen vermieden. Feinladung:
  maximal 1,2 s; Grobladung: maximal 12 s; sichere Entladung: maximal 30 s
  plus 50 ms Stabilitaetspruefung.
- Die Frequenzmessung arbeitet per Timer-1-Input-Capture unabhaengig vom
  OLED-Intervall. `KEIN PULS` erscheint nach 1,5 s ohne verwertbare Flanke.

## EEPROM und Werkwerte

- Das Speicherformat verwendet Kennung `0x4D4D`, Version 2 und eine CRC-16.
- Gespeichert werden sieben Kalibrierwerte und zehn Bedien-/Anzeigeparameter.
- Beim Start werden Kennung, Version, CRC und alle Wertebereiche geprueft.
  Schlaegt eine Pruefung fehl, arbeitet das Geraet mit den Werkwerten aus
  `config.h`; ein automatisches Ueberschreiben des EEPROM findet dabei nicht
  statt.
- Direkt bearbeitete Werte und Referenzkalibrierungen werden erst bei
  Bestaetigung gespeichert. Stromnullung und Werkreset brauchen eine zweite
  Bestaetigung.

## Grenzwertanzeigen und Benennung

| Messart | Anzeigegrenzen |
|---|---|
| Spannung | Ab 25,0 V `WARNUNG` und Aufforderung zum Trennen |
| Widerstand | Unter 100 Ohm, ueber 1 MOhm und offener Eingang ab ADC 1018 getrennt angezeigt |
| Kapazitaet | Unter 100 nF, ueber 4700 uF sowie fehlgeschlagene Entladung getrennt angezeigt |
| Strom | Hohe Last ab 16,0 A mit 15,5-A-Hysterese; Warnung ab 19,5 A mit 19,0-A-Hysterese; ueber 20,0 A ausserhalb; ADC-Schienenwerte als Fehler |
| Frequenz | Kein Puls, unter/ueber Bereich; x1: 1 Hz bis 100 kHz, x16: 16 Hz bis 1,6 MHz |

Die sichtbaren Begriffe sind einheitlich deutsch und die ASCII-Schreibweise
passt zum vorhandenen OLED-Zeichensatz (`Kapazitaet`, `zurueck`). Interne
C++-Bezeichner sind einheitlich englisch und nach Messmodul praefigiert. Die
Konstanten verwenden Grossbuchstaben mit Unterstrichen, Typen PascalCase und
Funktionen lowerCamelCase.

## Zusammengefasste Inbetriebnahme-Checkliste

### Vor dem Einschalten

- [ ] Nano ist ein 5-V-/16-MHz-ATmega328P-Modell; Versorgungspolaritaet stimmt.
- [ ] Alle Massen der Kleinspannungs-Signalseite sind verbunden; kein
      Messanschluss fuehrt Netzspannung oder unbekannte Energie.
- [ ] OLED-Adresse und zulaessige OLED-Versorgung sind geprueft.
- [ ] Encoder liegt an D2/D3/D4, die Kapazitaetspins an D5/D6/D7 und der
      Frequenzeingang ausschliesslich an D8.
- [ ] Schutzwiderstaende, BAT43-Klemmdioden und Abblockkondensatoren sind
      entsprechend den Einzelanschlussplaenen bestueckt.
- [ ] `Einstellungen` zeigt die erwarteten Werte; langer Druck uebernimmt den
      bearbeiteten Wert ins EEPROM.
- [ ] Kurzer Druck schaltet in `LIVE`/`REL` Hold und setzt in `MIN`/`MAX` die
      Extremwerte zurueck; langer Druck kehrt zurueck.

### Spannung 0 bis 25 V DC

- [ ] Reale 5-V-Spannung sowie R1/R2 kontrollieren; zuerst mit einer bekannten
      Kleinspannung testen.
- [ ] VIN- liegt an Nano-GND, Polaritaet stimmt, niemals AC oder Netzspannung.
- [ ] Anzeige bei 0 V und einem Referenzwert pruefen; 25-V-Warnung nur mit
      einer sicheren, strombegrenzten Quelle annaehern.

### Widerstand 100 Ohm bis 1 MOhm

- [ ] Pruefling ist spannungsfrei und mindestens ein Anschluss ist aus der
      Schaltung geloest.
- [ ] Gemessenen Wert von R_REF in `config.h` vergleichen.
- [ ] Offen, ein Wert nahe 10 kOhm sowie ein niedriger und hoher Referenzwert
      pruefen.

### Kapazitaet 100 nF bis 4700 uF

- [ ] Kondensator extern entladen und Spannungsfreiheit nachmessen; bei Elkos
      CX+/CX- richtig polen.
- [ ] Beim Oeffnen der Ansicht Entlade- und anschliessende Messphase beobachten.
- [ ] Je einen bekannten Kondensator im Fein- und Grobbereich pruefen.
- [ ] Nach Abbruch oder Verlassen der Ansicht sind D5/D6/D7 hochohmig.

### Strom -20 A bis +20 A DC

- [ ] Passende Sicherung, Leitungen, Buchsen und Klemmen sind fuer den realen
      Strom ausgelegt; Strommesser nur in Reihe anschliessen.
- [ ] Bei 0 A ACS712-OUT und angezeigten Nullpunkt vergleichen.
- [ ] Mit kleinem bekannten Strom beide Richtungen pruefen; IP+ nach IP- muss
      positiv erscheinen.
- [ ] Hohe Lasten nur kurz und unter kontrollierten Bedingungen testen; die
      Softwarewarnung ersetzt keine Sicherung.

### Frequenz

- [ ] Nur massebezogene 0-bis-5-V-Rechtecksignale zufuehren; gemeinsame Masse
      zuerst verbinden.
- [ ] x1 zunaechst mit 1 kHz testen, danach 1 Hz, 10 Hz, 10 kHz und 100 kHz.
- [ ] Bei optionaler Erweiterung D12: HIGH/offen = x1, LOW = x16; im x16-Bereich
      mit geteiltem Signal und maximal 1,6 MHz Eingang testen.
- [ ] D9/D10-PWM waehrend der Frequenzansicht nicht verwenden.

## Buildnachweis

Arduino CLI kompiliert den Stand mit AVR-Core 1.8.8 und maximalen
Compilerwarnungen fehler- und warnungsfrei. Sowohl
`arduino:avr:nano:cpu=atmega328old` als auch `arduino:avr:nano:cpu=atmega328`
ergeben:

- Flash: 29.608 von 30.720 Byte (96 %), 1.112 Byte frei.
- Statisches SRAM: 580 von 2.048 Byte (28 %), 1.468 Byte vor dynamischen
  Reservierungen frei.
- Nach dem rund 1.024 Byte grossen OLED-Bildpuffer bleiben rechnerisch etwa
  444 Byte, abzueglich geringem Allokations-Overhead, fuer Stack und Laufzeit.
  Das ist ausreichend fuer den aktuellen, pufferarmen Aufbau, aber bewusst
  kein Spielraum fuer grosse lokale Arrays oder dynamische `String`-Objekte.

Benoetigt werden `Adafruit GFX Library` und `Adafruit SSD1306` sowie deren
Abhaengigkeiten. Die angegebenen Groessen gehoeren zum dokumentierten Stand;
nach jeder Codeaenderung muss der Build fuer beide Nano-Bootloadervarianten
erneut ausgefuehrt und dieser Abschnitt aktualisiert werden.

## Dokumentationspruefung

Geprueft wurden README, Bauteilliste, Gesamtpruefung, alle fuenf
Anschlussplaene, beide Frequenzvarianten und der Entwicklungs-Promptkatalog.
Pinbezeichnungen, Messbereiche, Warnschwellen, Bedienung, EEPROM-Version,
Werkwerte, Mittelungsstufen und Sicherheitsgrenzen sind mit dem Quellcode
abgeglichen. Die SVG-Dateien bleiben die grafische Ergaenzung; bei einer
Hardwareaenderung muessen immer Anschlussplan, SVG, Bauteilliste, `config.h`
und diese Gesamtpruefung gemeinsam angepasst werden.
