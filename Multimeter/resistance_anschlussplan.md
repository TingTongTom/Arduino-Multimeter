# Widerstandsmessung 100 Ohm bis 1 MOhm

Die Widerstandsmessung verwendet A1 und ist damit unabhaengig vom Voltmeter an
A0, dem Encoder an D2 bis D4 und dem OLED an A4/A5. Die vorhandenen Dateien
`schaltplan.svg` und `voltmeter_schaltplan.svg` bleiben unveraendert. Der neue
grafische Plan liegt in `resistance_schaltplan.svg`.

## Bauteilliste

| Bezeichnung | Wert / Typ | Funktion |
|---|---:|---|
| R_REF | 10 kOhm, moeglichst 1 % | Referenzwiderstand des Spannungsteilers |
| R_PROTECT | 1 kOhm | Strombegrenzung vor A1 |
| C1 (optional) | 100 nF Keramik | Beruhigt den Messwert an A1 |
| RX | 100 Ohm bis 1 MOhm | Zu messender, spannungsfreier Widerstand |

Der 10-kOhm-Referenzwiderstand ist fuer die vorhandenen Werte 1 kOhm, 2,2 kOhm,
3,3 kOhm, 4,7 kOhm, 10 kOhm, 33 kOhm, 56 kOhm, 100 kOhm und 220 kOhm ein
sinnvoller Kompromiss. Der zusaetzliche 1-kOhm-Widerstand liegt nur im praktisch
stromlosen ADC-Zweig und geht deshalb nicht in die Teilerformel ein.

## Anschlusstabelle

| Von | Nach | Hinweis |
|---|---|---|
| Nano 5V | R_REF Anschluss 1 | Messspeisung |
| R_REF Anschluss 2 | Messknoten RX+ | Oberer Teil des Teilers |
| Messknoten RX+ | R_PROTECT Anschluss 1 | ADC-Schutz |
| R_PROTECT Anschluss 2 | Nano A1 | Widerstands-Messeingang |
| C1 (optional) | A1 und GND | Nahe am Nano anordnen |
| RX Anschluss 1 | Messknoten RX+ | Unbekannter Widerstand |
| RX Anschluss 2 | Nano GND | Unterer Teil des Teilers |

## Verdrahtung

```text
Nano 5V ---- R_REF 10k ----+---- RX ---- Nano GND
                            |
                            +---- R_PROTECT 1k ---- A1
                                                    |
                                           C1 100nF (optional)
                                                    |
                                                Nano GND
```

RX nur bei ausgeschaltetem Geraet anschliessen oder wechseln. Fuer lose
Widerstaende eignen sich zwei Messbuchsen zwischen Messknoten und GND.

## Berechnungsformel

Mit `N` als gemitteltem ADC-Wert von 0 bis 1023 gilt:

```text
U_A1 = U_5V * RX / (R_REF + RX)
RX   = R_REF * N / (1023 - N)
```

Da ADC-Referenz und Spannungsteiler dieselbe 5-V-Versorgung verwenden, kuerzt
sich deren exakter Spannungswert aus der Formel heraus. Die Software verwirft
nach einem Kanalwechsel die erste Wandlung und mittelt danach 32 Messungen.

## Messbereich und Aufloesung

Der technisch ausgewertete Bereich ist 100 Ohm bis 1 MOhm. Darunter zeigt das
OLED `< 100 Ohm`, darueber `> 1 MOhm`. Ein ADC-Wert ab 1018 wird als `OFFEN`
bewertet. Damit wird eine nicht angeschlossene Messleitung von einem
Widerstand oberhalb des Messbereichs getrennt erkannt. Mit R_REF = 10 kOhm
liegt der besonders sinnvolle Bereich ungefaehr zwischen 1 kOhm und 220 kOhm;
alle vorgegebenen Widerstandswerte liegen darin.

Die theoretische Schrittweite ist nicht ueberall gleich. In der Naehe von
100 Ohm entspricht ein ADC-Schritt etwa 10 Ohm, bei 1 kOhm etwa 12 Ohm, bei
10 kOhm etwa 39 Ohm, bei 220 kOhm etwa 5,3 kOhm und bei 1 MOhm bereits rund
100 kOhm. Bauteiltoleranz, Kontaktwiderstand, ADC-Fehler und Stoerungen
begrenzen die praktische Genauigkeit. Werte nahe 1 MOhm sind daher nur eine
grobe Anzeige. Fuer hohe Widerstaende waere eine umschaltbare Referenz genauer;
sie ist fuer die vorgegebenen Widerstaende nicht erforderlich.

## Kalibrierung

1. R_REF spannungsfrei mit einem verlaesslichen Multimeter messen.
2. Den Messwert in `config.h` als `RESISTANCE_REFERENCE_OHM` eintragen.
3. Einen bekannten Widerstand nahe 10 kOhm anschliessen und die Anzeige pruefen.
4. Falls noetig `RESISTANCE_CORRECTION_FACTOR` geringfuegig anpassen. Beispiel:
   Sollwert 10,00 kOhm, Anzeige 9,90 kOhm: Faktor `10.00 / 9.90 = 1.0101`.
5. Anschliessend mindestens einen niedrigen und einen hohen vorhandenen Wert
   kontrollieren.

## Sicherheitshinweise

- Niemals Netzspannung messen.
- Nur spannungsfreie Widerstaende und Schaltungen messen.
- Kondensatoren in der zu pruefenden Schaltung vorher sicher entladen.
- Fuer verlaessliche Werte mindestens einen Anschluss des Widerstands aus der
  Schaltung loesen; parallele Strompfade verfaelschen sonst das Ergebnis.
- Keine externe Spannung an Messknoten, A1, 5V oder GND anlegen.
- Diese einfache Eingangsschaltung ist kein CAT-zertifiziertes Messgeraet und
  besitzt keinen Schutz fuer unbekannte oder energiereiche Quellen.
