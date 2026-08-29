# Frequenzmessung 1 Hz bis 100 kHz

## Festgelegter Messbereich

- Nur massebezogene Kleinspannungs-Rechteck- und Pulssignale von 0 bis 5 V.
- Garantierte Pegel am Eingang: LOW hoechstens 1,0 V, HIGH mindestens 4,0 V.
- Frequenzbereich: 1 Hz bis 100 kHz; Pulsbreite mindestens 5 us.
- Negative Spannungen, Gleichspannungen ueber 5 V, Netzspannung und CAT-Messungen
  sind nicht zulaessig. Die Klemmdioden sind Fehlanschlussschutz, keine
  Erweiterung des Messbereichs.

3,3-V-CMOS-Pegel koennen praktisch funktionieren, sind mit einem 74HC14 an
5 V wegen dessen garantierter HIGH-Schwelle aber nicht zugesichert. Fuer
garantierte 3,3-V-Pegel kann an gleicher Stelle ein 74HCT14 verwendet werden.
Sinus- oder Dreiecksignale sind nur messbar, wenn sie die genannten LOW- und
HIGH-Pegel sicher kreuzen.

## Verdrahtung

| Von | Ueber | Nach |
|---|---|---|
| BNC-Mittelkontakt `F_IN` | R_F 10 kOhm | Schutzknoten `F_PROTECT` |
| `F_PROTECT` | R_PD 100 kOhm | GND |
| GND | BAT43 D_LOW, Anode an GND | Kathode an `F_PROTECT` |
| `F_PROTECT` | BAT43 D_HIGH, Anode am Knoten | Kathode an +5 V |
| `F_PROTECT` | direkte kurze Leitung | 74HC14 Pin 1 (1A) |
| 74HC14 Pin 2 (1Y) | direkte kurze Leitung | Nano D8/ICP1 |
| 74HC14 Pin 14 / Pin 7 | Versorgung | +5 V / GND |
| 74HC14 Pin 14 | C_DEC 100 nF, dicht am IC | Pin 7 |
| BNC-Schirm | direkte Leitung | GND |

Alle unbenutzten 74HC14-Eingaenge (Pins 3, 5, 9, 11 und 13) fest mit GND
verbinden; die zugehoerigen Ausgaenge bleiben offen. Der 100-kOhm-Pulldown
definiert bei offener Buchse LOW. Der 10-kOhm-Serienwiderstand begrenzt den
Strom in die BAT43-Klemmen bei kurzen Fehlanschluessen. Er ersetzt weder
Galvaniktrennung noch einen fuer hoehere Spannungen bemessenen Eingang.

Der grafische Plan liegt in `frequency_meter_schaltplan.svg`.

## Optionale Erweiterung bis 1,6 MHz

Mit einem 74HC4040 und einem zweipoligen Umschalter (DPDT, ON-ON) kann das
aufbereitete Signal wahlweise direkt oder durch 16 geteilt auf D8 gefuehrt
werden. Der zweite Schalterpol meldet den Bereich an D12; die Firmware
multipliziert im Vorteilerbereich automatisch mit 16.

| Verbindung | Anschluss |
|---|---|
| 74HC14 Pin 2 (aufbereitetes Signal) | 74HC4040 Pin 10 (CLOCK) |
| 74HC4040 Pin 11 (RESET) | GND |
| 74HC4040 Pin 16 / Pin 8 | +5 V / GND |
| 100 nF Abblockkondensator | direkt zwischen Pins 16 und 8 |
| DPDT Pol A, gemeinsamer Kontakt | Nano D8/ICP1 |
| DPDT Pol A, Stellung `x1` | 74HC14 Pin 2 |
| DPDT Pol A, Stellung `x16` | 74HC4040 Pin 5 (Q4, geteilt durch 16) |
| DPDT Pol B, gemeinsamer Kontakt | Nano D12 |
| DPDT Pol B, Stellung `x16` | GND |
| DPDT Pol B, Stellung `x1` | offen (interner Pull-up) |

Damit gelten zwei Bereiche: `x1` von 1 Hz bis 100 kHz und `x16` von 16 Hz
bis 1,6 MHz. Moeglichst vor dem Start der Frequenzansicht umschalten, da beim
Umschalten kurzzeitig ein ungueltiger Periodenwert entstehen kann. Der
Vorteiler erweitert nur den Frequenzbereich, nicht den Spannungsbereich:
Weiterhin sind ausschliesslich massebezogene 0-bis-5-V-Signale zulaessig.
Oberhalb 100 kHz muessen die Leitungen kurz sein; die praktische Obergrenze
haengt zusaetzlich von Aufbau und Signalform ab.

Der grafische Zusatzplan liegt in `frequency_meter_extended_schaltplan.svg`.

## Funktion und Test

Timer 1 misst auf D8/ICP1 die Zeit zwischen steigenden Flanken mit 62,5 ns
Zaehlerauflösung. Der digitale Input-Capture-Noise-Canceler ist aktiv. Nach
1,5 s ohne Flanke erscheint `KEIN PULS`; Werte ausserhalb des spezifizierten
Bereichs werden als Bereichsfehler angezeigt.

Die Daempfung bestimmt die Periodenmittelung: `SCHNELL` verwendet eine,
`NORMAL` vier und `RUHIG` acht Perioden. Das Aktualisierungsintervall bestimmt
nur, wie oft die Anzeige neu gezeichnet wird; die Capture-Messung selbst laeuft
interruptgesteuert weiter. D9 und D10 duerfen waehrend der Frequenzansicht
nicht fuer PWM verwendet werden, weil Timer 1 exklusiv belegt ist.

Zum Funktionstest einen massebezogenen Funktionsgenerator zuerst auf 1 kHz,
0 V LOW und 5 V HIGH einstellen. Gemeinsame Masse verbinden, danach 1 Hz,
10 Hz, 1 kHz, 10 kHz und 100 kHz pruefen. Niemals die Frequenzbuchse mit dem
Strompfad oder einer Netzspannungsquelle verbinden.
