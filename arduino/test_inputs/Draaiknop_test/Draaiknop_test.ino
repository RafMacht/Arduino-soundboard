const int clkPin = 2;
const int dtPin = 3;
const int swPin = 4;

int laatsteCLK;
int index = 0;

// Gewoon 1 t.e.m. 4
String waarden[4] = {"Fluit", "Piano", "Trompet", "Viool"};

bool knopVorige = HIGH;

void setup() {
  Serial.begin(9600);

  pinMode(clkPin, INPUT);
  pinMode(dtPin, INPUT);
  pinMode(swPin, INPUT_PULLUP);

  laatsteCLK = digitalRead(clkPin);

  Serial.print("Huidig: ");
  Serial.println(waarden[index]);
}

void loop() {
  int huidigeCLK = digitalRead(clkPin);

  // Alleen reageren op dalende flank (HIGH → LOW)
  if (laatsteCLK == HIGH && huidigeCLK == LOW) {

    if (digitalRead(dtPin) == HIGH) {
      index++;
    } else {
      index--;
    }

    // wrap-around tussen 0 en 3
    if (index > 3) index = 0;
    if (index < 0) index = 3;

    Serial.print("Optie: ");
    Serial.println(waarden[index]);
  }

  laatsteCLK = huidigeCLK;

  // --- DRUKKNOP ---
  bool knopHuidig = digitalRead(swPin);

  if (knopVorige == HIGH && knopHuidig == LOW) {
    Serial.print("GEKOZEN: ");
    Serial.println(waarden[index]);
  }

  knopVorige = knopHuidig;

  delay(2); // kleine stabiliteit
}