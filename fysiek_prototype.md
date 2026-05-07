# Fysiek prototype

In dit bestand kun je meer info vinden over hoe alles te verbinden.


## wiring diagram
![Wiring](imgs/image_2026-05-07_223154215.png)

### 1. Sensoren en Invoer
Ultrasone Sensoren (HC-SR04): Er zijn drie sensoren aangesloten om afstanden te meten.  

Sensor 1 (Links): Trigger pin op 12, Echo pin op 13.  
Sensor 2 (Midden): Trigger pin op 9, Echo pin op 11.  
Sensor 3 (Rechts): Trigger pin op 7, Echo pin op 8.  
Rotary Encoder: Wordt gebruikt om door instrumenten te navigeren (Viool, Piano, Trompet, Fluit).  
Verbonden met pinnen 3 (CLK), 4 (DT) en 5 (SW/knop).  
Potentiometer: Aangesloten op analoge pin A0 om de gevoeligheid of het afstandsbereik aan te passen.  

### 2. Uitvoer en Weergave
LCD Scherm (16x2): Werkt via I2C-communicatie (SDA/SCL pinnen) om het gekozen instrument en keuzemenu's te tonen.  
LED-strip (16 LEDs): Aangesloten op digitale pin 6. De strip is verdeeld in drie groepen die van kleur veranderen (van rood naar groen) op basis van de gemeten afstand van de sensoren.  

### 3. Voeding
Alle componenten (sensoren, LCD, encoder en LED-strip) delen een gezamenlijke 5V spanningslijn en GND (aarde) vanaf de Arduino Uno.

## Flowchart
![Wiring](imgs/)
