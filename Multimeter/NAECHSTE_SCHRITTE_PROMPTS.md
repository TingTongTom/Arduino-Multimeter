# Entwicklungsarchiv: urspruengliche Projekt-Prompts

Diese Prompts dokumentieren die urspruengliche Ausbauplanung. Die Schritte 1
bis 6 sind inzwischen umgesetzt und duerfen nicht mehr als offene Aufgaben
oder aktuelle technische Spezifikation gelesen werden. Verbindlich fuer den
heutigen Stand sind `README.md`, `PROJEKT_PRUEFUNG.md`, `config.h`, die
modulspezifischen Anschlussplaene und der Quellcode. Abweichungen in diesem
Archiv bleiben zur Nachvollziehbarkeit der Entwicklung erhalten.

## 1. Strommessung mit ACS712-20A implementieren

```text
Erweitere das Arduino-Nano-Multimeter um eine bidirektionale Gleichstrommessung
mit dem vorhandenen AZ-Delivery ACS712-20A an A3. Das Modul wird mit 5 V und
GND des Nano versorgt; OUT fuehrt ueber 1 kOhm zu A3, mit 100 nF von A3 nach
GND. Fuege den Menuepunkt "Strom" hinzu und strukturiere die Implementierung
wie die vorhandenen Module voltmeter.cpp/.h und resistance.cpp/.h in eigenen
Dateien current_meter.cpp/.h.

Anforderungen:
- Messbereich -20,0 A bis +20,0 A; fuer Grenzwerte den Betrag verwenden.
- Typischer Startwert der Empfindlichkeit: 0,100 V/A.
- Nullpunkt beim Start nur dann automatisch aus mehreren Messungen bestimmen,
  wenn garantiert kein Strom fliesst; andernfalls einen kalibrierbaren Wert
  aus config.h verwenden.
- Erste ADC-Wandlung nach Kanalwechsel verwerfen und mindestens 64 Messungen
  mitteln.
- Unter 16 A den Strom normal anzeigen.
- Ab 16,0 A deutlich "HINWEIS: hohe Last" anzeigen.
- Ab 19,5 A deutlich "WARNUNG: sofort trennen" anzeigen. 19,5 A wird als
  Vorwarnschwelle verwendet, damit ADC- und Sensortoleranzen die Warnung nicht
  erst oberhalb des Nennbereichs ausloesen.
- Mit Hysterese arbeiten: Hinweis erst unter 15,5 A und Warnung erst unter
  19,0 A zuruecknehmen, damit die Anzeige an der Grenze nicht flackert.
- Messwerte ausserhalb des plausiblen ADC-Bereichs als Fehler anzeigen.
- Konstanten fuer Pin, Empfindlichkeit, Nullpunkt, Korrekturfaktor,
  Mittelungszahl, Aktualisierungszeit und Grenzwerte in config.h ablegen.
- Auf dem OLED Stromrichtung und Einheit A anzeigen.
- Niemals behaupten, dass die Softwarewarnung eine Sicherung ersetzt.
- Bestehende Spannungs- und Widerstandsmessung duerfen nicht beeintraechtigt
  werden.

Erstelle beziehungsweise aktualisiere auch Anschlussdokumentation und einen
SVG-Schaltplan. Kompiliere den Sketch anschliessend und behebe alle Fehler.
```

## 2. ACS712 kalibrieren und pruefen

```text
Erstelle fuer die ACS712-20A-Erweiterung eine sichere, nachvollziehbare
Kalibrieranleitung fuer DIY-Kleinspannungsprojekte. Die Anleitung soll zuerst
den Nullpunkt bei 0 A bestimmen und danach die Empfindlichkeit mit einem
bekannten Strom und einem verlaesslichen Referenzmultimeter kalibrieren.
Beschreibe die Berechnung fuer CURRENT_ZERO_VOLTAGE und
CURRENT_CORRECTION_FACTOR in config.h. Verwende mehrere Pruefpunkte in beiden
Stromrichtungen, soweit der Aufbau das sicher erlaubt. Weise auf Eigenerwaermung,
Temperaturdrift, ADC-Aufloesung und die begrenzte Genauigkeit bei kleinen
Stroemen hin. Tests nahe 20 A duerfen nur sehr kurz erfolgen; fuer den
Dauerbetrieb ist der Bereich unter 16 A vorgesehen. Ergaenze eine Tabelle zum
Eintragen von Sollwert, Anzeige, Abweichung und Bauteiltemperatur.
```

## 3. Schutz und mechanischen Strompfad dokumentieren

```text
Erstelle einen Anschluss- und Sicherheitsplan fuer den Hochstrompfad des
DIY-Multimeters mit ACS712-20A. Plane beruehrungsgeschuetzte 4-mm-Buchsen,
einen Sicherungshalter mit maximal 20-A-Sicherung, kurze Leitungen mit
mindestens 2,5 mm2 und eine mechanische Abdeckung der Schraubklemmen ein.
Kennzeichne klar, dass der Stromanschluss in Reihe erfolgt. Lege 16 A als
empfohlenes Ende des laenger nutzbaren Bereichs fest und 20 A nur als kurze
Messbereichsgrenze. Die konkrete zulaessige Dauerbelastung muss sich nach
Sicherung, Buchsen, Leitungen, Leiterbahnen, Klemmen und Erwaermung richten.
Das Geraet bleibt auf Kleinspannungs-DIY-Projekte begrenzt und ist nicht fuer
Netzspannung oder CAT-Messungen vorgesehen. Aktualisiere Bauteilliste und
Schaltplan entsprechend.
```

## 4. Kapazitaetsmessung planen und implementieren

```text
Plane und implementiere die im Menue vorgesehene Kapazitaetsmessung des
Arduino-Nano-Multimeters. Verwende A2 als Messeingang und freie Digitalpins zum
kontrollierten Laden und vollstaendigen Entladen. Definiere zuerst einen
realistischen Messbereich fuer DIY-Elektronik und entwirf danach eine sichere
RC-Messschaltung mit 1-%-Referenzwiderstaenden, 1-kOhm-Schutzwiderstand,
BAT43-Klemmdioden und einer kontrollierten Entladung. Geladene Kondensatoren
muessen erkannt beziehungsweise vor der Messung entladen werden. Trenne die
Software in capacitance.cpp/.h, lege alle Kalibrierwerte in config.h ab,
aktualisiere Menue, Anschlussdokumentation, Bauteilliste und SVG-Schaltplan.
Bestehende Messarten muessen weiterhin funktionieren. Kompiliere und pruefe
den vollstaendigen Sketch.
```

## 5. Frequenzmessung planen und implementieren

```text
Plane und implementiere die im Menue vorgesehene Frequenzmessung fuer
Kleinspannungs-DIY-Signale. Nutze nach Moeglichkeit den Timer-1-Input-Capture-
Pin D8/ICP1 des Arduino Nano und einen 74HC14-Schmitt-Trigger zur sauberen
Signalformung. Entwirf einen geschuetzten Eingang mit Serienwiderstand,
BAT43-Klemmdioden, definierter Eingangsvorspannung beziehungsweise Pulldown und
100-nF-Abblockkondensator am 74HC14. Lege den erlaubten Spannungs- und
Frequenzbereich vor der Implementierung eindeutig fest. Erstelle
frequency_meter.cpp/.h, aktualisiere Menue, config.h, Anschlussdokumentation,
Bauteilliste und SVG-Schaltplan. Zeige Frequenz automatisch passend in Hz,
kHz oder MHz und erkenne ein fehlendes Signal. Kompiliere und teste den Sketch,
ohne Spannung, Widerstand oder Strommessung zu beeintraechtigen.
```

## 6. Einstellungen und Gesamtpruefung

```text
Baue den vorhandenen Menuepunkt "Einstellungen" zu einer sicheren
Kalibrieranzeige aus. Zeige zunaechst nur die aktuell verwendeten Parameter
fuer ADC-Referenz, Spannungsteiler, Widerstandsreferenz, ACS712-Nullpunkt und
ACS712-Empfindlichkeit an; veraendere gespeicherte Werte nicht automatisch.
Pruefe danach das gesamte Projekt auf Pin-Konflikte, RAM- und Flash-Verbrauch,
ADC-Kanalwechsel, Aktualisierungszeiten, Grenzwertanzeigen und einheitliche
Benennung. Erstelle eine abschliessende Pinbelegung und eine zusammengefasste
Inbetriebnahme-Checkliste fuer alle Messarten. Kompiliere den Sketch fuer den
klassischen Arduino Nano mit ATmega328P.
```

## Festgelegte Stromgrenzen

| Betrag des Stroms | Anzeige | Bedeutung |
|---:|---|---|
| unter 16,0 A | normale Anzeige | vorgesehener Arbeitsbereich |
| 16,0 A bis unter 19,5 A | Hinweis: hohe Last | nur kurz beziehungsweise Erwaermung beobachten |
| ab 19,5 A | Warnung: sofort trennen | unmittelbare Naehe der 20-A-Messgrenze |

Die Warnschwelle liegt absichtlich unter 20 A. Sensor-, ADC- und
Kalibriertoleranzen koennen sonst dazu fuehren, dass eine exakt auf 20 A
gesetzte Warnung zu spaet erscheint.
