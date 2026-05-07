```cpp
#include <FastLED.h>

// LED-strip
const int ledPin = 6;
const int aantalLeds = 16;

CRGB leds[aantalLeds];

void setup()
{
  FastLED.addLeds<WS2812B, ledPin, GRB>(leds, aantalLeds);
  FastLED.setBrightness(80);

  // Zet alle LEDs rood
  for (int i = 0; i < aantalLeds; i++)
  {
    leds[i] = CRGB::Red;
  }

  FastLED.show();
}

void loop()
{
  // Niets nodig, de LED-strip blijft rood aan
}
```
