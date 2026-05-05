# Touchless Soundboard
## Inleiding
Voor het vak Opkomende Technologieën ontwikkelden we een innovatief, touchless soundboard dat muziek maken op een intuïtieve en interactieve manier mogelijk maakt. Met behulp van een Arduino Uno, afstandssensoren en eigen code creëren we een systeem waarbij je door handbewegingen in de lucht verschillende tonen kan bespelen. De afstand tussen je hand en de sensor bepaalt het geluid, waardoor je op een vloeiende manier variatie en expressie in je muziek brengt.

Via een draaiknop kan de gebruiker eenvoudig schakelen tussen verschillende instrumenten uit een ingebouwde bibliotheek, zodat één systeem meerdere muzikale stijlen ondersteunt. Bovendien kan de gevoeligheid van de sensoren aangepast worden, waardoor het instrument volledig af te stemmen is op de voorkeuren van de gebruiker. 
Dit project combineert hardware en software tot een speelse maar krachtige toepassing, waarin technologie en creativiteit samenkomen.

![arduino](imgs/image_2026-05-05_193850016.png)

## Contributors
- Raf Machtelinckx
- Rune Vanhoucke

---
## Project

**Product:** Een soundboard waarmee je zelf je eigen geluid/muziek kunt maken doormiddel van gebaren voor afstandssensoren. Je kan de verschillende tonen kiezen door te scrollen door de instrumentenbibliotheek.

**Doel:**
Gebruikers op een leuke en creatieve manier muziek laten creëeren.

**Uitdaging:**
Door de combinatie te maken met arduino (hardware programmeren) en een python script (software programmeren) leerden we nieuwe paden kennen en gaf het een uitdaging om mee aan de slag te gaan.

### Componenten
Het product bestaat uit een aantal verschillende onderdelen:
- 1× Casing
- 1× Arduino Uno
- 3× HC SR04 ultrasonic distance sensor
- 1× WPI435 Digitale Encoder
- 1× potentiometer
- Jumper kabels
- 1× Breadboard
- 1× LED strip (5V)
- 1× HD44780 1602 LCD-moduledisplaybundel met I2C-interface
- 1× voedingskabel naar computer

### Connectieschema
[Klik hier](docs/fysiek_prototype.md)

*Nog te maken in Wokwi*

## Code
### Input code
[Arduino code](arduino/test_inputs/Sensorcode/Sensorcode.ino)

### Output code
[Python code](arduino/test_outputs/Geluidtest/Geluidtest.py)
