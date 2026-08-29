# Bidirektionale Gleichstrommessung mit ACS712-20A

Die Strommessung verwendet das vorhandene AZ-Delivery-ACS712-20A-Modul an A3.
Sie misst Gleichstrom in beiden Richtungen von -20,0 A bis +20,0 A. Der
Hochstrompfad des Sensors ist galvanisch vom Signalausgang getrennt, die
Nano-Seite teilt sich jedoch 5 V und GND mit dem Sensor-Modul.

## Anschlusstabelle

| Von | Nach | Hinweis |
|---|---|---|
| Nano 5V | ACS712 VCC | Modulversorgung 5 V |
| Nano GND | ACS712 GND | Gemeinsame Signalmasse |
| ACS712 OUT | R1, 1 kOhm | Schutzwiderstand |
| R1 | Nano A3 | ADC-Eingang |
| C1, 100 nF | A3 und Nano GND | Filter, nahe am Nano |
| Stromquelle/Versorgung | Sicherung, dann ACS712 IP+ | Reihenschaltung |
| ACS712 IP- | Last | Diese Richtung wird positiv angezeigt |
| Last | Rueckleiter der Stromquelle | Stromkreis schliessen |

```text
Signalteil:
Nano 5V  ---------------- ACS712 VCC
Nano GND ---------------- ACS712 GND
ACS712 OUT --- R1 1k ---+--- Nano A3
                        |
                      C1 100nF
                        |
Nano GND ---------------+

Hochstrompfad (in Reihe):
Quelle + --- Sicherung --- IP+ [ ACS712 ] IP- --- Last --- Quelle -
```

Die Klemmenbezeichnung am konkreten Modul pruefen. Strom von IP+ nach IP- wird
als positiver Wert und die Gegenrichtung als negativer Wert angezeigt. Die
Hochstromleitungen kurz, ausreichend dimensioniert und beruehrungsgeschuetzt
ausfuehren. Der grafische Plan liegt in `current_meter_schaltplan.svg`.

## Kalibrierung

Der typische Startwert `CURRENT_SENSITIVITY_V_PER_A` ist 0,100 V/A. Reale
ACS712-Module weichen davon ab. Zuerst bei garantiert stromlosem Hochstrompfad
`Einstellungen > Strom jetzt nullen` zweimal bestaetigen. Die Firmware bildet
den Nullpunkt aus dem ADC-Rohwert und der eingestellten ADC-Referenz und
speichert ihn im EEPROM. Der Werkwert `CURRENT_ZERO_VOLTAGE` von 2,160 V gilt
nur ohne gueltigen gespeicherten Wert beziehungsweise nach Werkreset.

`CURRENT_AUTO_ZERO_AT_START` und `CURRENT_ZERO_CURRENT_GUARANTEED` bleiben
normalerweise `false`. Beide duerfen nur auf `true` gesetzt werden, wenn die
Hardware garantiert, dass waehrend des Einschaltens kein Strom fliesst.
Andernfalls wuerde ein vorhandener Strom als Nullpunkt gespeichert und alle
folgenden Messungen verfaelschen. Ist nur einer der Schalter aktiv, verwendet
die Software weiterhin den kalibrierten `CURRENT_ZERO_VOLTAGE`.

Danach einen bekannten Strom in beiden Richtungen messen. Unter `Strom
Referenz` den Betrag des bekannten Stroms einstellen und bei stabiler Messung
mit langem Druck uebernehmen. Die Firmware passt den Stromkorrekturfaktor an;
`CURRENT_CORRECTION_FACTOR` bleibt dessen Werkwert. Die Software verwirft nach
jedem moeglichen ADC-Kanalwechsel die erste Wandlung und mittelt je nach
Daempfung 16, 64 oder 128 weitere Messungen.

## Anzeige und Grenzwerte

- Unter 16,0 A Betrag: normaler Messwert mit Vorzeichen, Richtung und Einheit A.
- Ab 16,0 A: `HINWEIS: hohe Last`; Ruecknahme erst unter 15,5 A.
- Ab 19,5 A: `WARNUNG: sofort trennen`; Ruecknahme erst unter 19,0 A.
- Ueber 20,0 A oder bei unplausiblem ADC-Wert: Fehler beziehungsweise
  Messbereichsueberschreitung.

Die Schwelle 19,5 A ist bewusst unterhalb der Nennbereichsgrenze angeordnet,
damit ADC- und Sensortoleranzen die Warnung nicht erst oberhalb 20 A ausloesen.

## Sicherheit

- Nur an geeigneten Kleinspannungs-Gleichstromkreisen einsetzen; niemals an
  Netzspannung oder fuer CAT-Messungen.
- Den Strommesser immer in Reihe anschliessen. Paralleles Anschliessen erzeugt
  einen Kurzschluss.
- Sicherung, Halter, Buchsen, Leitungen, Leiterbahnen und Klemmen muessen fuer
  den erwarteten Strom ausgelegt sein. 20 A ist kein empfohlener Dauerstrom.
- Die Softwarewarnung und die OLED-Anzeige ersetzen niemals eine Sicherung
  oder andere Hardware-Schutzmassnahmen.
- Vor Umverdrahtung den Messkreis abschalten und spannungsfrei machen.
