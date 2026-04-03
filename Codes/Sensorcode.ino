// Sensor 1
const int trig1 = 7;
const int echo1 = 8;

// Sensor 2
const int trig2 = 2;
const int echo2 = 13;

// RGB LED 1
const int led1R = 3;
const int led1G = 5;
const int led1B = 6;

// RGB LED 2
const int led2R = 9;
const int led2G = 10;
const int led2B = 11;

// Potmeter
const int potPin = A0;

// Maximale meetafstand
const int maxAfstand = 70;

void setup()
{
  Serial.begin(9600);

  pinMode(trig1, OUTPUT);
  pinMode(echo1, INPUT);

  pinMode(trig2, OUTPUT);
  pinMode(echo2, INPUT);

  pinMode(led1R, OUTPUT);
  pinMode(led1G, OUTPUT);
  pinMode(led1B, OUTPUT);

  pinMode(led2R, OUTPUT);
  pinMode(led2G, OUTPUT);
  pinMode(led2B, OUTPUT);
}

void loop()
{
  int potValue = analogRead(potPin);

  // Potmeter bepaalt bereik tussen 70 en 0 cm
  int bereik = map(potValue, 0, 1023, 70, 0);

  // Vermijd problemen met map() als bereik 0 wordt
  if (bereik < 1)
  {
    bereik = 1;
  }

  int afstand1 = meetAfstand(trig1, echo1);
  int afstand2 = meetAfstand(trig2, echo2);

  zetKleur(led1R, led1G, led1B, afstand1, bereik);
  zetKleur(led2R, led2G, led2B, afstand2, bereik);

  Serial.print("Pot: ");
  Serial.print(potValue);
  Serial.print(" | Bereik: ");
  Serial.print(bereik);
  Serial.print(" cm | S1: ");
  Serial.print(afstand1);
  Serial.print(" cm | S2: ");
  Serial.print(afstand2);
  Serial.println(" cm");

  delay(50);
}

int meetAfstand(int trigPin, int echoPin)
{
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);

  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);

  long duration = pulseIn(echoPin, HIGH, 30000);

  // Geen echo = behandel als maximale afstand
  if (duration == 0)
  {
    return maxAfstand;
  }

  int afstand = duration * 0.034 / 2;

  // Nooit meer dan 70 cm teruggeven
  if (afstand > maxAfstand)
  {
    afstand = maxAfstand;
  }

  // Geen negatieve waarden
  if (afstand < 0)
  {
    afstand = 0;
  }

  return afstand;
}

void zetKleur(int rPin, int gPin, int bPin, int afstand, int bereik)
{
  afstand = constrain(afstand, 0, bereik);

  int rood = map(afstand, 0, bereik, 255, 0);
  int groen = map(afstand, 0, bereik, 0, 255);

  setRGB(rPin, gPin, bPin, rood, groen, 0);
}

void setRGB(int rPin, int gPin, int bPin, int r, int g, int b)
{
  analogWrite(rPin, r);
  analogWrite(gPin, g);
  analogWrite(bPin, b);
}