#include <Wire.h>                  // I2C communicatie (voor LCD)
#include <LiquidCrystal_I2C.h>    // LCD aansturen via I2C
#include <FastLED.h>              // LED-strip aansturen

LiquidCrystal_I2C lcd(0x27, 16, 2); // LCD op adres 0x27, 16 kolommen, 2 rijen

// LED-strip instellingen
const int ledPin = 6;             // Pin waarop LED-strip zit
const int aantalLeds = 16;        // Aantal LEDs
CRGB leds[aantalLeds];            // Array met LED-kleuren

// Sensor 1 pins
const int trig1 = 12;
const int echo1 = 13;

// Sensor 2 pins
const int trig2 = 9;
const int echo2 = 11;

// Sensor 3 pins
const int trig3 = 7;
const int echo3 = 8;

// Potmeter (voor gevoeligheid/afstand)
const int potPin = A0;

// Encoder (voor instrumentkeuze)
const int clkPin = 3;
const int dtPin = 4;
const int swPin = 5;

// Lijst met instrumenten
String keuzes[4] = {"Viool", "Piano", "Trompet", "Fluit"};

int index = 0;                    // Huidige selectie in lijst
int laatsteCLK;                   // Vorige status encoder

bool geselecteerd = false;        // Of instrument vastgezet is
bool knopVorige = HIGH;           // Vorige knopstatus

String gekozenInstrument = "Viool"; // Standaard instrument

// Debounce voor encoder draaien (voorkomt te snel tellen)
unsigned long laatsteDraaiTijd = 0;
const unsigned long draaiWachttijd = 180;

// Debounce voor knop indrukken
unsigned long laatsteKnopTijd = 0;
const unsigned long knopWachttijd = 250;

// Timing voor sensor uitlezen
unsigned long laatsteSensorPrint = 0;
const unsigned long sensorInterval = 200;

const int maxAfstand = 70;        // Maximale meetafstand (cm)

void setup()
{
  Serial.begin(9600);             // Seriële communicatie starten

  // Sensor pins instellen
  pinMode(trig1, OUTPUT);
  pinMode(echo1, INPUT);

  pinMode(trig2, OUTPUT);
  pinMode(echo2, INPUT);

  pinMode(trig3, OUTPUT);
  pinMode(echo3, INPUT);

  // Encoder pins instellen
  pinMode(clkPin, INPUT);
  pinMode(dtPin, INPUT);
  pinMode(swPin, INPUT_PULLUP);

  laatsteCLK = digitalRead(clkPin); // Startwaarde encoder

  // LCD initialiseren
  lcd.init();
  lcd.backlight();
  toonKeuze();                   // Eerste scherm tonen

  // LED-strip initialiseren
  FastLED.addLeds<WS2812B, ledPin, GRB>(leds, aantalLeds);
  FastLED.setBrightness(80);
  FastLED.clear();
  FastLED.show();
}

void loop()
{
  leesEncoder();   // Check draaien encoder
  leesKnop();      // Check drukknop

  // Sensoren lezen op vaste interval
  if (millis() - laatsteSensorPrint >= sensorInterval)
  {
    laatsteSensorPrint = millis();

    int potValue = analogRead(potPin);             // Potmeter uitlezen
    int bereik = map(potValue, 0, 1023, 70, 1);    // Omzetten naar afstandsbereik

    // Afstanden meten met sensoren
    int afstand1 = meetAfstand(trig1, echo1);
    int afstand2 = meetAfstand(trig2, echo2);
    int afstand3 = meetAfstand(trig3, echo3);

    // LED-strip aanpassen volgens afstand
    updateLedStrip(afstand2, afstand1, afstand3, bereik);

    // Data naar computer sturen (voor Python/audio)
    Serial.print(potValue);
    Serial.print(",");
    Serial.print(bereik);
    Serial.print(",");
    Serial.print(afstand2);
    Serial.print(",");
    Serial.print(afstand1);
    Serial.print(",");
    Serial.print(afstand3);
    Serial.print(",");
    Serial.println(gekozenInstrument);
  }
}

// Update alle LED-groepen volgens sensoren
void updateLedStrip(int afstand1, int afstand2, int afstand3, int bereik)
{
  zetLedGroep(0, 5, afstand1, bereik);   // Sensor 1 → eerste LEDs
  zetLedGroep(5, 6, afstand2, bereik);   // Sensor 2 → midden LEDs
  zetLedGroep(11, 5, afstand3, bereik);  // Sensor 3 → laatste LEDs

  FastLED.show(); // Toon wijzigingen
}

// Zet kleur voor een groep LEDs (rood → ver, groen → dichtbij)
void zetLedGroep(int startLed, int aantal, int afstand, int bereik)
{
  if (bereik < 1) bereik = 1;

  afstand = constrain(afstand, 0, bereik);

  int groen = map(afstand, bereik, 0, 0, 255);
  int rood = map(afstand, bereik, 0, 255, 0);

  groen = constrain(groen, 0, 255);
  rood = constrain(rood, 0, 255);

  for (int i = startLed; i < startLed + aantal; i++)
  {
    leds[i] = CRGB(rood, groen, 0); // Kleur instellen
  }
}

// Encoder uitlezen om door instrumenten te scrollen
void leesEncoder()
{
  if (geselecteerd) return; // Niet aanpassen als gekozen

  int huidigeCLK = digitalRead(clkPin);

  // Detecteer draai
  if (laatsteCLK == HIGH && huidigeCLK == LOW)
  {
    if (millis() - laatsteDraaiTijd > draaiWachttijd)
    {
      if (digitalRead(dtPin) == HIGH)
        index++;   // Rechts draaien
      else
        index--;   // Links draaien

      // Grenzen van lijst
      if (index > 3) index = 0;
      if (index < 0) index = 3;

      toonKeuze(); // Toon nieuwe keuze

      laatsteDraaiTijd = millis();
    }
  }

  laatsteCLK = huidigeCLK;
}

// Knop uitlezen om instrument te selecteren
void leesKnop()
{
  bool knopHuidig = digitalRead(swPin);

  if (knopVorige == HIGH && knopHuidig == LOW)
  {
    if (millis() - laatsteKnopTijd > knopWachttijd)
    {
      geselecteerd = !geselecteerd; // Toggle selectie

      if (geselecteerd)
      {
        gekozenInstrument = keuzes[index];
        toonGeselecteerd(); // Toon gekozen instrument
      }
      else
      {
        toonKeuze(); // Ga terug naar kiezen
      }

      laatsteKnopTijd = millis();
    }
  }

  knopVorige = knopHuidig;
}

// Toon huidige keuze op LCD
void toonKeuze()
{
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Kies instrument");

  lcd.setCursor(0, 1);
  lcd.print("> ");
  lcd.print(keuzes[index]);
}

// Toon geselecteerd instrument
void toonGeselecteerd()
{
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Geselecteerd:");

  lcd.setCursor(0, 1);
  lcd.print(gekozenInstrument);
}

// Meet afstand met ultrasone sensor
int meetAfstand(int trigPin, int echoPin)
{
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);

  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);

  long duration = pulseIn(echoPin, HIGH, 30000); // Tijd meten

  if (duration == 0)
    return maxAfstand; // Geen signaal → max afstand

  int afstand = duration * 0.034 / 2; // Omzetten naar cm

  // Grenzen toepassen
  if (afstand > maxAfstand) afstand = maxAfstand;
  if (afstand < 0) afstand = 0;

  return afstand;
}
