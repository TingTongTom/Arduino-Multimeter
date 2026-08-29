# Bauteilliste fuer das gesamte Multimeter-Projekt

Stand: Grundgeraet sowie Spannungs-, Widerstands-, Strom-, Kapazitaets- und
Frequenzmessung sind in der Firmware umgesetzt und dokumentiert. Ein
AZ-Delivery ACS712-20A ist bereits vorhanden. Die optionale
Frequenzbereichserweiterung mit 74HC4040 bleibt ein Hardware-Zusatz.

## Vorhandenes Bauteil

| Anzahl | Bauteil | Status / Verwendung |
|---:|---|---|
| 1 | AZ-Delivery ACS712-20A Stromsensormodul | Vorhanden; Strommessung an Nano A3 |

## Zusammengefasste Einkaufsliste fuer alle Ausbaustufen

| Anzahl | Bauteil | Wert / Typ | Verwendung / Status |
|---:|---|---|---|
| 1 | Mikrocontroller-Board | Arduino Nano, ATmega328P, 5 V | Grundgeraet |
| 1 | OLED-Display | 128 x 64, I2C, SSD1306, meist 0x3C | Grundgeraet |
| 1 | Drehencoder mit Taster | KY-040 oder mechanischer Encoder | Grundgeraet |
| 1 | Widerstand | 56 kOhm, 1 %, mindestens 0,25 W | Voltmeter |
| 3 | Widerstand | 10 kOhm, 1 %, mindestens 0,25 W | Spannung, Widerstand und Frequenz |
| 4 | Widerstand | 1 kOhm, 1 %, mindestens 0,25 W | A0, A1, A3 und Kapazitaetseingang |
| 2 | Widerstand | 100 kOhm, 1 %, mindestens 0,25 W | Kapazitaet und Frequenz |
| 6 | Kleinsignal-Schottkydiode | BAT43, bedrahtet, DO-35 | Je zwei fuer Spannung, Kapazitaet und Frequenz |
| 6 | Keramikkondensator | 100 nF, mindestens 25 V | Filter und Abblockung; einer davon optional an A1 |
| 1 | Elektrolytkondensator, optional | 10 uF, mindestens 10 V | Stuetzung am ACS712-Modul |
| 1 | Schmitt-Trigger-IC | 74HC14, DIP-14 | Frequenzeingang |
| 1 | IC-Sockel, empfohlen | DIP-14 | 74HC14 austauschbar montieren |
| 1 | Sicherungshalter | Fuer Kfz-Flachsicherung, mindestens 20 A | Strommesspfad |
| 2 | Sicherungen | 20 A, eine davon als Ersatz | Strommesspfad; Softwarewarnung ersetzt sie nicht |
| 2 | Hochstrombuchsen | Beruehrungsgeschuetzt, 4 mm, mindestens 20 A | Stromeingang und Stromausgang |
| 6 | Messbuchsen | Beruehrungsgeschuetzt, 4 mm | Spannung, Widerstand und Kapazitaet |
| 1 | Eingangsbuchse | Isolierte BNC-Buchse | Frequenzeingang |
| ca. 0,5 m | Hochstromleitung | Mindestens 2,5 mm2, flexibel und kurz | ACS712-Strompfad |
| 1 | Lochrasterplatine oder eigene Leiterplatte | Ausreichende Groesse und Abstaende | Aufbau |
| 1 Satz | Verbindungsmaterial | Schaltdraht, Jumper, Stiftleisten, Schrumpfschlauch | Verdrahtung |
| 1 | USB-Kabel | Passend zum Nano | Programmierung und Versorgung |
| 1 | Kunststoffgehaeuse | Mit Abdeckung des Hochstrompfads | Mechanischer Schutz |
| 1 Satz | Abstandshalter und Schrauben | Passend zu Platinen und Gehaeuse | Montage |

Die Mengen enthalten den vollstaendigen dokumentierten Aufbau einschliesslich
der separat gekennzeichneten optionalen Erweiterungen. Vor der Bestellung
Gehaeuseabmessungen, Buchsendurchmesser, reale Modulbauformen und die
Strombelastbarkeit aller Hochstromteile am konkreten Aufbau pruefen.

## Grundschaltung

| Anzahl | Bauteil | Anschluss |
|---:|---|---|
| 1 | Arduino Nano | Zentrale Baugruppe |
| 1 | OLED 128 x 64 I2C | 5 V oder laut Modul 3,3 V, GND, A4/SDA, A5/SCL |
| 1 | Drehencoder mit Taster | 5 V, GND, D2/CLK, D3/DT, D4/SW |

Bei einem nackten mechanischen Encoder werden keine externen Pull-ups
benoetigt; der Sketch aktiviert die internen Pull-up-Widerstaende des Nano.

## Voltmeter-Eingang 0 bis 25 V DC

| Bezeichnung | Anzahl | Wert / Typ |
|---|---:|---|
| R1 | 1 | 56 kOhm, 1 %, mindestens 0,25 W |
| R2 | 1 | 10 kOhm, 1 %, mindestens 0,25 W |
| R3 | 1 | 1 kOhm, 1 %, mindestens 0,25 W |
| C1 | 1 | 100 nF, Keramik |
| D1, D2 | 2 | BAT43, Kleinsignal-Schottkydiode, DO-35 |
| VIN+, VIN- | 2 | Beruehrungsgeschuetzte 4-mm-Messbuchsen |

Die BAT43 ist fuer diesen 0-bis-5-V-Klemmeingang die bevorzugte Diode. Sie hat
einen niedrigen Sperrstrom und ihre Sperrspannungsfestigkeit von 30 V reicht
fuer die Spannung direkt am ADC-Eingang aus. Die BAT46 bleibt eine geeignete
Alternative, ist aber nicht zwingend besser. Leistungsdioden der Reihe 1N5817
bis 1N5819 sind fuer den hochohmigen Teiler weniger guenstig.

## Widerstandsmessung 100 Ohm bis 1 MOhm

| Bezeichnung | Anzahl | Wert / Typ |
|---|---:|---|
| R_REF | 1 | 10 kOhm, 1 % |
| R_PROTECT | 1 | 1 kOhm, 1 % |
| C1 | 1 optional | 100 nF, Keramik |
| RX+, GND | 2 | Beruehrungsgeschuetzte 4-mm-Messbuchsen |

`RX` ist das zu messende Bauteil und gehoert nicht fest zur Schaltung.

## Strommessung mit vorhandenem ACS712-20A

| Bezeichnung | Anzahl | Wert / Typ |
|---|---:|---|
| Stromsensor | 1 vorhanden | AZ-Delivery ACS712-20A, 5 V, typisch 100 mV/A |
| R_PROTECT | 1 | 1 kOhm zwischen OUT und Nano A3 |
| C_FILTER | 1 | 100 nF zwischen A3 und GND |
| C_VCC | 1 | 100 nF direkt zwischen Modul-VCC und GND |
| C_BULK | 1 optional | 10 uF zwischen Modul-VCC und GND |
| F1 | 1 plus Ersatz | 20-A-Kfz-Flachsicherung mit Halter |
| I+, I- | 2 | Beruehrungsgeschuetzte Hochstrombuchsen, mindestens 20 A |
| Leitung | nach Bedarf | Mindestens 2,5 mm2, kurz und mechanisch geschuetzt |

Der Strompfad wird in Reihe angeschlossen. Unter 16 A liegt der vorgesehene
Arbeitsbereich. Ab 16 A soll die Software auf hohe Last hinweisen, ab 19,5 A
zum sofortigen Trennen auffordern. 20 A ist nur die kurze Messbereichsgrenze,
nicht der vorgesehene Dauerbetrieb. Sicherung, Buchsen, Leitungen, Klemmen und
Leiterbahnen muessen unabhaengig von der Software ausreichend belastbar sein.

## Kapazitaetsmessung 100 nF bis 4700 uF

| Anzahl | Bauteil | Wert / Typ |
|---:|---|---|
| je 1 | Referenzwiderstand | 1 kOhm und 100 kOhm, jeweils 1 % |
| 1 | Schutzwiderstand | 1 kOhm, 1 % |
| 2 | Schutzdiode | BAT43 |
| 2 | Messbuchsen | CX+ und GND |

D5 und D6 waehlen die RC-Bereiche automatisch, D7 entlaedt kontrolliert. Der
1-kOhm-Schutzwiderstand liegt zwischen CX+ und dem gemeinsamen Messknoten an
A2. Zwei BAT43 klemmen diesen Knoten gegen GND und 5 V. Details und
Sicherheitsgrenzen stehen in `capacitance_anschlussplan.md`.

## Frequenzmessung 1 Hz bis 100 kHz

| Anzahl | Bauteil | Wert / Typ |
|---:|---|---|
| 1 | Schmitt-Trigger | 74HC14, DIP-14 |
| 1 | IC-Sockel | DIP-14, empfohlen |
| je 1 | Widerstand | 10 kOhm und 100 kOhm, 1 % |
| 2 | Schutzdiode | BAT43 |
| 1 | Abblockkondensator | 100 nF |
| 1 | Eingangsbuchse | Isolierte BNC-Buchse |

Der 10-kOhm-Serienwiderstand liegt zwischen BNC-Mitte und Schutzknoten. Zwei
BAT43 klemmen den Knoten nach GND und +5 V; 100 kOhm ziehen ihn bei offener
Buchse definiert auf LOW. Der Knoten speist 74HC14 Pin 1, Pin 2 geht an D8/ICP1.
Der 100-nF-Abblockkondensator gehoert direkt zwischen Pins 14 und 7. Details
und die verbindlichen Pegelgrenzen stehen in `frequency_meter_anschlussplan.md`.

### Optionale Frequenzbereichserweiterung bis 1,6 MHz

| Anzahl | Bauteil | Wert / Typ |
|---:|---|---|
| 1 | Binaerer Teiler | 74HC4040, DIP-16 |
| 1 | IC-Sockel | DIP-16, empfohlen |
| 1 | Umschalter | DPDT, ON-ON |
| 1 | Abblockkondensator | 100 nF, Keramik |

Der Umschalter waehlt zwischen dem direkten Signal und Q4 des 74HC4040
(Teilung durch 16). Sein zweiter Pol zieht D12 im erweiterten Bereich nach
GND. Details stehen in `frequency_meter_anschlussplan.md`.

## Optionale Kalibrier- und Pruefteile

Zum Testen der Widerstandsmessung kann je ein 1-%-Widerstand mit folgenden
Werten bereitgehalten werden: 100 Ohm, 1 kOhm, 2,2 kOhm, 3,3 kOhm, 4,7 kOhm,
10 kOhm, 33 kOhm, 56 kOhm, 100 kOhm, 220 kOhm und 1 MOhm.

Fuer den ACS712 werden eine einstellbare Kleinspannungsquelle, eine geeignete
Last und ein verlaessliches Referenzmultimeter benoetigt. Diese Werkzeuge sind
nicht Bestandteil des fertigen Geraets.

Zusaetzlich empfohlen sind stabile Referenzkondensatoren fuer beide
Kapazitaetsbereiche, ein massebezogener 0-bis-5-V-Funktionsgenerator fuer den
Frequenzeingang und ein I2C-Adressscanner fuer die OLED-Inbetriebnahme.

## Sicherheit und Projektgrenzen

- Nur fuer Kleinspannungs-DIY- und Mikrocontrollerprojekte verwenden.
- Keine Netzspannung und keine CAT-Messungen durchfuehren.
- Der Voltmeter-Eingang ist nur fuer positive Gleichspannung bis 25 V gedacht.
- Der Frequenzeingang ist nur fuer massebezogene 0-bis-5-V-Signale gedacht.
- Widerstaende und Kondensatoren nur spannungsfrei anschliessen; Kondensatoren
  vorher vollstaendig entladen.
- Die Strommessung immer in Reihe und mit Sicherung aufbauen.
- Eine OLED-Warnung ersetzt weder Sicherung noch ausreichend dimensionierte
  Buchsen, Leitungen, Leiterbahnen und Klemmen.
