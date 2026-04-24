// Sensor 1
const int trig1 = 7;
const int echo1 = 8;

// Sensor 2
const int trig2 = 2;
const int echo2 = 13;

// Sensor 3
const int trig3 = A1;
const int echo3 = A2;

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

// Encoder
const int clkPin = A3;
const int dtPin = A4;
const int swPin = A5;

int laatsteCLK;
int index = 0;
String waarden[4] = {"Fluit", "Piano", "Trompet", "Viool"};
bool knopVorige = HIGH;
String gekozenInstrument = "Fluit";

// Maximale meetafstand
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

  pinMode(led1R, OUTPUT);
  pinMode(led1G, OUTPUT);
  pinMode(led1B, OUTPUT);

  pinMode(led2R, OUTPUT);
  pinMode(led2G, OUTPUT);
  pinMode(led2B, OUTPUT);

  pinMode(clkPin, INPUT);
  pinMode(dtPin, INPUT);
  pinMode(swPin, INPUT_PULLUP);

  laatsteCLK = digitalRead(clkPin);
}

void loop()
{
  leesEncoder();

  int potValue = analogRead(potPin);
  int bereik = map(potValue, 0, 1023, 70, 1);

  int afstand1 = meetAfstand(trig1, echo1);
  int afstand2 = meetAfstand(trig2, echo2);
  int afstand3 = meetAfstand(trig3, echo3);

  zetKleur(led1R, led1G, led1B, afstand1, bereik);
  zetKleur(led2R, led2G, led2B, afstand2, bereik);

  // Formaat:
  // pot,bereik,sensor1,sensor2,sensor3,instrument
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

  delay(50);
}

void leesEncoder()
{
  int huidigeCLK = digitalRead(clkPin);

  if (laatsteCLK == HIGH && huidigeCLK == LOW)
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
  }

  laatsteCLK = huidigeCLK;

  bool knopHuidig = digitalRead(swPin);

  if (knopVorige == HIGH && knopHuidig == LOW)
  {
    gekozenInstrument = waarden[index];
  }

  knopVorige = knopHuidig;
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