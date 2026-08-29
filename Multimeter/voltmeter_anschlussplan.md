# Voltmeter 0 bis 25 V DC

Die Erweiterung misst ausschliesslich positive Gleichspannungen. `VIN-` muss
mit `GND` des Arduino Nano verbunden sein. Der bestehende Schaltplan
`schaltplan.svg` bleibt unveraendert; die Voltmeter-Erweiterung ist in
`voltmeter_schaltplan.svg` dargestellt.

## Bauteilliste

| Bezeichnung | Wert / Typ | Aufgabe |
|---|---|---|
| R1 | 56 kOhm, 1 %, mindestens 0,25 W | Oberer Widerstand des Spannungsteilers |
| R2 | 10 kOhm, 1 %, mindestens 0,25 W | Unterer Widerstand des Spannungsteilers |
| R3 | 1 kOhm | Begrenzt den Strom in A0 und die Schutzdioden |
| C1 | 100 nF, Keramik | Filter von A0 nach GND |
| D1, D2 | 2 x BAT43, Kleinsignal-Schottky, DO-35 | Begrenzung von A0 gegen GND und 5 V |
| Eingang | Beruehrungssichere Buchsen fuer VIN+ und VIN- | Anschluss der Messspannung |

## Anschlusstabelle

| Von | Nach | Hinweis |
|---|---|---|
| VIN+ | R1 Anschluss 1 | Positive Messspannung |
| R1 Anschluss 2 | Teilerknoten | Verbindung mit R2 und R3 |
| R2 Anschluss 1 | Teilerknoten | Unterer Teilerwiderstand |
| R2 Anschluss 2 | Nano GND | Gemeinsame Masse |
| Teilerknoten | R3 Anschluss 1 | Schutzwiderstand vor A0 |
| R3 Anschluss 2 | Nano A0 | ADC-Eingang |
| C1 | zwischen A0 und GND | Moeglichst nahe am Nano |
| D1 (Anode/Kathode) | GND / A0 | Klemmt negative Spannung |
| D2 (Anode/Kathode) | A0 / 5V | Klemmt positive Ueberspannung |
| VIN- | Nano GND | Zwingend verbinden |

## Verdrahtung

```text
VIN+ --- R1 56k ---+--- R3 1k ---+--- A0
                   |             |
                 R2 10k        C1 100nF
                   |             |
VIN- --------------+-------------+--- GND

Schutzdioden direkt an A0:
  D1: GND --|>|-- A0       (Anode an GND, Kathode an A0)
  D2: A0  --|>|-- 5V       (Anode an A0, Kathode an 5V)
```

Bei der BAT43 kennzeichnet der Ring die Kathode. Fuer Breadboard und
Lochraster werden zwei BAT43 empfohlen. Ihr niedriger Sperrstrom beeinflusst
den hochohmigen Spannungsteiler weniger als Leistungs-Schottkydioden wie
1N5817, 1N5818 oder 1N5819. Die BAT46 ist ebenfalls geeignet, fuer diesen
Klemmeingang aber nicht eindeutig besser.

Auf einer spaeteren SMD-Platine kann eine BAT54S-Doppeldiode beide einzelnen
Dioden ersetzen. Anschluss: Pin 1 an GND, Pin 3 an A0 und Pin 2 an 5 V. Vor der
Bestueckung ist die Pinbelegung im Datenblatt des konkreten Herstellers zu
pruefen.

R1, R2 und R3 nahe am Eingang beziehungsweise A0 anordnen. C1 und beide
Schottky-Dioden sollen kurze Leitungen zum Nano haben. Die Schutzdioden sind
eine Zusatzbegrenzung, ersetzen aber weder eine Sicherung noch galvanische
Trennung oder einen professionellen Eingangsschutz.

## Berechnung des Spannungsteilers

Ohne nennenswerte ADC-Eingangslast gilt:

```text
U_A0 = U_IN * R2 / (R1 + R2)
     = U_IN * 10 kOhm / (56 kOhm + 10 kOhm)
     = U_IN / 6,6

bei U_IN = 25 V: U_A0 = 25 V / 6,6 = 3,788 V
```

Der 1-kOhm-Schutzwiderstand R3 liegt nicht im belasteten Teilerpfad und
veraendert den Gleichspannungs-Teilfaktor wegen des hochohmigen ADC-Eingangs
praktisch nicht. Bei der aktuell gemessenen Referenzspannung von 4,320 V wird
der ADC theoretisch erst bei etwa 28,51 V Eingangsspannung voll ausgesteuert.
Dieser Spielraum ist kein zulaessiger erweiterter Messbereich.

Der Sketch verwirft nach einem moeglichen Kanalwechsel die erste Wandlung und
mittelt danach je nach Daempfung 8 (`SCHNELL`), 32 (`NORMAL`) oder 64
(`RUHIG`) ADC-Wandlungen. Aus dem ADC-Mittelwert wird gerechnet:

```text
U_IN = ADC / 1023 * U_REF * 6,6 * Korrekturfaktor
```

## Kalibrierung

1. Die Nano-5-V-Spannung mit einem verlaesslichen Multimeter zwischen `5V` und
   `GND` messen und unter `Einstellungen > ADC-Referenz` eintragen.
2. Eine stabile, bekannte Gleichspannung (zweckmaessig 15 bis 20 V) anlegen.
3. Unter `Spannung Referenz` den bekannten Sollwert einstellen und bei
   stabiler Anzeige mit langem Druck uebernehmen. Die Firmware berechnet den
   Korrekturfaktor nach folgender Formel:

   `Korrekturfaktor_neu = Korrekturfaktor_alt * Referenzwert / Anzeigewert`

4. Bei mehreren Spannungen pruefen. Fuer beste Genauigkeit koennen ausserdem
   die real gemessenen Werte von R1 und R2 in `DIVIDER_R1_OHM` und
   `DIVIDER_R2_OHM` eingetragen werden.

Die Menuekalibrierung wird mit CRC im EEPROM gespeichert. Die Konstanten
`ADC_REFERENCE_VOLTAGE` und `VOLTAGE_CORRECTION_FACTOR` in `config.h` bleiben
die Werkwerte fuer ein leeres, ungueltiges oder zurueckgesetztes EEPROM.

Die Warnanzeige erscheint fuer berechnete Messwerte ueber 25 V. Wegen
Bauteiltoleranzen und Kalibrierung ist sie keine zertifizierte
Ueberspannungsabschaltung.

Bis D1 und D2 eingebaut sind, fehlen die externen Klemmdioden. In diesem
Zwischenzustand nicht ueber 25 V testen und Verpolung unbedingt vermeiden.

## Sicherheit

- Nur positive Gleichspannung von 0 bis 25 V messen.
- Niemals Netzspannung, Wechselspannung oder unbekannte Spannungen anschliessen.
- `VIN-` immer mit Nano-GND verbinden; die Messung ist nicht galvanisch getrennt.
- Messleitungen nur bei ausgeschalteter Schaltung aufbauen oder veraendern.
- Eine falsch gepolte oder zu hohe Eingangsspannung kann Nano, Computer und
  angeschlossene USB-Geraete beschaedigen. Bei Warnanzeige sofort trennen.
- Bei Versorgung des Nano ueber USB sind PC-Masse und `VIN-` miteinander
  verbunden. Keine Schaltung messen, deren Masse nicht sicher verbunden werden darf.
