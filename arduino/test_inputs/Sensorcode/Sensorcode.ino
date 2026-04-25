#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <FastLED.h>

LiquidCrystal_I2C lcd(0x27, 16, 2);

// LED-strip
const int ledPin = 6;
const int aantalLeds = 16;
CRGB leds[aantalLeds];

// Sensor 1
const int trig1 = 7;
const int echo1 = 8;

// Sensor 2
const int trig2 = 2;
const int echo2 = 13;

// Sensor 3
const int trig3 = 12;
const int echo3 = 11;

// Potmeter
const int potPin = A0;

// Encoder
const int clkPin = 3;
const int dtPin = 4;
const int swPin = 5;

String keuzes[4] = {"Viool", "Piano", "Trompet", "Fluit"};

int index = 0;
int laatsteCLK;

bool geselecteerd = false;
bool knopVorige = HIGH;

String gekozenInstrument = "Viool";

// Debounce
unsigned long laatsteDraaiTijd = 0;
const unsigned long draaiWachttijd = 180;

unsigned long laatsteKnopTijd = 0;
const unsigned long knopWachttijd = 250;

// Sensor timing
unsigned long laatsteSensorPrint = 0;
const unsigned long sensorInterval = 200;

const int maxAfstand = 70;

void setup()
{
  Serial.begin(9600);

  pinMode(trig1, OUTPUT);
  pinMode(echo1, INPUT);

  pinMode(trig2, OUTPUT);
  pinMode(echo2, INPUT);

  pinMode(trig3, OUTPUT);
  pinMode(echo3, INPUT);

  pinMode(clkPin, INPUT);
  pinMode(dtPin, INPUT);
  pinMode(swPin, INPUT_PULLUP);

  laatsteCLK = digitalRead(clkPin);

  lcd.init();
  lcd.backlight();
  toonKeuze();

  FastLED.addLeds<WS2812B, ledPin, GRB>(leds, aantalLeds);
  FastLED.setBrightness(80);
  FastLED.clear();
  FastLED.show();
}

void loop()
{
  leesEncoder();
  leesKnop();

  if (millis() - laatsteSensorPrint >= sensorInterval)
  {
    laatsteSensorPrint = millis();

    int potValue = analogRead(potPin);
    int bereik = map(potValue, 0, 1023, 70, 1);

    int afstand1 = meetAfstand(trig1, echo1);
    int afstand2 = meetAfstand(trig2, echo2);
    int afstand3 = meetAfstand(trig3, echo3);

    updateLedStrip(afstand2, afstand1, afstand3, bereik);

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

void updateLedStrip(int afstand1, int afstand2, int afstand3, int bereik)
{
  // Sensor 1: eerste 5 LEDs
  zetLedGroep(0, 5, afstand1, bereik);

  // Sensor 2: volgende 6 LEDs
  zetLedGroep(5, 6, afstand2, bereik);

  // Sensor 3: laatste 5 LEDs
  zetLedGroep(11, 5, afstand3, bereik);

  FastLED.show();
}

void zetLedGroep(int startLed, int aantal, int afstand, int bereik)
{
  if (bereik < 1)
  {
    bereik = 1;
  }

  afstand = constrain(afstand, 0, bereik);

  // Ver = rood, dichtbij = groen
  int groen = map(afstand, bereik, 0, 0, 255);
  int rood = map(afstand, bereik, 0, 255, 0);

  groen = constrain(groen, 0, 255);
  rood = constrain(rood, 0, 255);

  for (int i = startLed; i < startLed + aantal; i++)
  {
    leds[i] = CRGB(rood, groen, 0);
  }
}

void leesEncoder()
{
  if (geselecteerd)
  {
    return;
  }

  int huidigeCLK = digitalRead(clkPin);

  if (laatsteCLK == HIGH && huidigeCLK == LOW)
  {
    if (millis() - laatsteDraaiTijd > draaiWachttijd)
    {
      if (digitalRead(dtPin) == HIGH)
      {
        index++;
      }
      else
      {
        index--;
      }

      if (index > 3) index = 0;
      if (index < 0) index = 3;

      toonKeuze();

      laatsteDraaiTijd = millis();
    }
  }

  laatsteCLK = huidigeCLK;
}

void leesKnop()
{
  bool knopHuidig = digitalRead(swPin);

  if (knopVorige == HIGH && knopHuidig == LOW)
  {
    if (millis() - laatsteKnopTijd > knopWachttijd)
    {
      geselecteerd = !geselecteerd;

      if (geselecteerd)
      {
        gekozenInstrument = keuzes[index];
        toonGeselecteerd();
      }
      else
      {
        toonKeuze();
      }

      laatsteKnopTijd = millis();
    }
  }

  knopVorige = knopHuidig;
}

void toonKeuze()
{
  lcd.clear();

  lcd.setCursor(0, 0);
  lcd.print("Kies instrument");

  lcd.setCursor(0, 1);
  lcd.print("> ");
  lcd.print(keuzes[index]);
}

void toonGeselecteerd()
{
  lcd.clear();

  lcd.setCursor(0, 0);
  lcd.print("Geselecteerd:");

  lcd.setCursor(0, 1);
  lcd.print(gekozenInstrument);
}

int meetAfstand(int trigPin, int echoPin)
{
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);

  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);

  long duration = pulseIn(echoPin, HIGH, 30000);

  if (duration == 0)
  {
    return maxAfstand;
  }

  int afstand = duration * 0.034 / 2;

  if (afstand > maxAfstand)
  {
    afstand = maxAfstand;
  }

  if (afstand < 0)
  {
    afstand = 0;
  }

  return afstand;
}