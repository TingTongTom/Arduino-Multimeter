# Bauteilliste fuer das gesamte Multimeter-Projekt

Diese Liste umfasst die Grundschaltung, den Voltmeter-Eingang von 0 bis 25 V DC
und die Widerstandsmessung von 100 Ohm bis 1 MOhm.

## Gesamte Einkaufsliste

| Anzahl | Bauteil | Wert / Typ | Verwendung |
|---:|---|---|---|
| 1 | Mikrocontroller-Board | Arduino Nano, ATmega328P, 5 V | Zentrale Steuerung und ADC |
| 1 | OLED-Display | 128 x 64 Pixel, I2C, SSD1306, Adresse meist 0x3C | Anzeige |
| 1 | Drehencoder mit Taster | KY-040 oder mechanischer Encoder mit Taster | Menuebedienung |
| 1 | Widerstand | 56 kOhm, 1 %, mindestens 0,25 W | Voltmeter R1 |
| 2 | Widerstand | 10 kOhm, 1 %, mindestens 0,25 W | Voltmeter R2 und Widerstandsmessung R_REF |
| 2 | Widerstand | 1 kOhm, mindestens 0,25 W | Schutzwiderstaende vor A0 und A1 |
| 1 | Keramikkondensator | 100 nF | Filter am Voltmeter-Eingang A0 |
| 1 | Keramikkondensator, optional | 100 nF | Filter am Widerstands-Eingang A1 |
| 2 | Kleinsignal-Schottkydiode | BAT46, bedrahtet (DO-35) | Schutz von A0 gegen GND und 5 V |
| 2 | Messbuchsen | Beruehrungssicher, z. B. 4-mm-Bananenbuchsen | VIN+ und VIN- |
| 2 | Messbuchsen | Passend zu den Messleitungen | RX+ und GND fuer Widerstaende |
| 1 | Experimentierplatine oder Lochrasterplatine | Ausreichend fuer Nano und Eingangsschaltungen | Aufbau der Schaltung |
| 1 Satz | Verbindungsmaterial | Schaltdraht oder Jumper-Kabel, Stiftleisten | Verdrahtung |
| 1 | USB-Kabel | Passend zum verwendeten Nano | Programmierung und Versorgung |

Bei Verwendung eines nackten Drehencoders werden keine externen Pull-up-
Widerstaende benoetigt; der Sketch aktiviert die internen Pull-ups des Nano.

## Grundschaltung

| Anzahl | Bauteil | Anschluss |
|---:|---|---|
| 1 | Arduino Nano | Zentrale Baugruppe |
| 1 | OLED 128 x 64 I2C | 5 V beziehungsweise 3,3 V, GND, A4/SDA, A5/SCL |
| 1 | Drehencoder mit Taster | 5 V, GND, D2/CLK, D3/DT, D4/SW |

Vor dem Anschluss pruefen, ob das konkrete OLED-Modul 5 V vertraegt.

## Voltmeter-Eingang 0 bis 25 V DC

| Bezeichnung | Anzahl | Wert / Typ |
|---|---:|---|
| R1 | 1 | 56 kOhm, 1 %, mindestens 0,25 W |
| R2 | 1 | 10 kOhm, 1 %, mindestens 0,25 W |
| R3 | 1 | 1 kOhm, mindestens 0,25 W |
| C1 | 1 | 100 nF, Keramik |
| D1, D2 | 2 | BAT46, Kleinsignal-Schottkydiode, bedrahtet (DO-35) |
| VIN+, VIN- | 2 | Beruehrungssichere Messbuchsen |

Die BAT46 ist wegen ihres niedrigen Sperrstroms besser fuer den hochohmigen
Spannungsteiler geeignet als Leistungs-Schottkydioden der Reihe 1N5817 bis
1N5819. Fuer eine spaetere SMD-Platine kann anstelle der beiden BAT46 eine
BAT54S-Doppeldiode verwendet werden: Pin 1 an GND, Pin 3 an A0 und Pin 2 an 5 V.

## Widerstandsmessung 100 Ohm bis 1 MOhm

| Bezeichnung | Anzahl | Wert / Typ |
|---|---:|---|
| R_REF | 1 | 10 kOhm, moeglichst 1 % |
| R_PROTECT | 1 | 1 kOhm |
| C1 | 1 optional | 100 nF, Keramik |
| RX+, GND | 2 | Messbuchsen |

`RX` ist das zu messende Bauteil und gehoert nicht fest zur Schaltung.

## Optionale Pruefwiderstaende

Zum Testen und Kalibrieren kann je ein 1-%-Widerstand mit folgenden Werten
bereitgehalten werden: 100 Ohm, 1 kOhm, 2,2 kOhm, 3,3 kOhm, 4,7 kOhm,
10 kOhm, 33 kOhm, 56 kOhm, 100 kOhm, 220 kOhm und 1 MOhm.

## Sicherheit

Das Projekt ist nicht fuer Netzspannung oder CAT-Messungen ausgelegt. Der
Voltmeter-Eingang darf nur fuer positive Gleichspannung bis 25 V verwendet
werden. Widerstaende duerfen nur spannungsfrei gemessen werden.
