# Kapazitaetsmessung 100 nF bis 4700 uF

Der Messbereich deckt typische Abblock-, Zeit-, Audio- und
Versorgungskondensatoren in DIY-Elektronik ab. Unter 100 nF dominieren
Streu- und Eingangskapazitaeten; oberhalb 4700 uF werden Mess- und Entladezeit
fuer diese einfache Nano-Schaltung unpraktisch. Elektrolytkondensatoren werden
mit `CX+` am Pluspol und GND am Minuspol angeschlossen.

## Sichere RC-Messschaltung

```text
 D5 --- R_FINE 100k 1% ---+
                           |
 D6 --- R_COARSE 1k 1% ---+--- Messknoten --- A2
                           |        |   |
 D7 -----------------------+    BAT43   BAT43
                           |     nach    nach
 CX+ --- R_PROTECT 1k 1% --+     GND     +5V

 CX- ----------------------------------- GND
```

Die Diode nach GND hat ihre Anode an GND und ihre Kathode am Messknoten. Die
Diode nach +5 V hat ihre Anode am Messknoten und ihre Kathode an +5 V. Beide
BAT43 muessen nahe am Nano sitzen. R_PROTECT liegt vor allen Nano-Verbindungen
und begrenzt daher Lade-, Entlade- und Fehlerstrom. D7 zieht den Messknoten zur
kontrollierten Entladung auf GND. D5 und D6 duerfen niemals direkt miteinander
oder mit CX+ verbunden werden; die jeweiligen Referenzwiderstaende sind
zwingend erforderlich.

| Von | Nach | Bauteil / Hinweis |
|---|---|---|
| Buchse CX+ | Messknoten | R_PROTECT, 1 kOhm, 1 % |
| Buchse CX- | Nano GND | Gemeinsame Masse |
| Nano D5 | Messknoten | R_FINE, 100 kOhm, 1 % |
| Nano D6 | Messknoten | R_COARSE, 1 kOhm, 1 % |
| Nano D7 | Messknoten | Entladeausgang |
| Nano A2 | Messknoten | ADC-Messeingang |
| GND | Messknoten | BAT43: Anode an GND |
| Messknoten | +5 V | BAT43: Kathode an +5 V |

## Messablauf

Beim Oeffnen des Menuepunkts sind alle Digitalpins zuerst hochohmig. A2 erkennt
eine vorhandene Ladung ab etwa 0,1 V und zeigt `Geladen erkannt`. Anschliessend
entlaedt D7 den Kondensator durch R_PROTECT, bis A2 fuer mindestens 50 ms nahezu
0 V misst. Erst dann startet die Ladung bis 63,2 % der Versorgungsspannung.

Zuerst wird ueber 100 kOhm gemessen. Wird die Schwelle nicht innerhalb von
1,2 s erreicht, entlaedt die Software erneut und verwendet den 1-kOhm-Bereich.
Die Berechnung beruecksichtigt R_PROTECT in Reihe mit dem jeweils aktiven
Referenzwiderstand. Nach Messende werden alle drei Digitalpins wieder
hochohmig. Ein Entladevorgang wird nach 30 s sicher abgebrochen.

## Kalibrierung

Die gemessenen Widerstandswerte werden in `config.h` als
`CAPACITANCE_PROTECTION_OHM`, `CAPACITANCE_FINE_REFERENCE_OHM` und
`CAPACITANCE_COARSE_REFERENCE_OHM` eingetragen. Mit bekannten Kondensatoren
nahe 1 uF und 1000 uF lassen sich die beiden Korrekturfaktoren getrennt
abgleichen. Elektrolytkondensatoren haben oft deutlich groessere Toleranzen;
fuer die Kalibrierung sind enge Folien- oder Referenzkondensatoren besser.

## Sicherheitsgrenzen

- Nur ausgebaute, spannungsfreie Kondensatoren aus Kleinspannungsschaltungen
  messen; keine Netzspannungs- oder Hochenergie-Kondensatoren anschliessen.
- Die automatische Erkennung ist eine zweite Schutzstufe, kein Ersatz fuer
  vorheriges externes Entladen. Kondensatoren ueber 5 V vor dem Anschluss mit
  einem geeigneten Leistungswiderstand entladen und nachmessen.
- R_PROTECT begrenzt bei 5 V auf hoechstens etwa 5 mA. Die BAT43 schuetzen vor
  kleinen Restladungen, nicht vor energiereichen oder dauerhaft gespeisten
  Quellen.
- Polaritaet von Elektrolytkondensatoren beachten.

Der grafische Plan liegt in `capacitance_schaltplan.svg`.
