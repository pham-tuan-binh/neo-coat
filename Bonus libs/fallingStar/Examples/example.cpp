#include <Arduino.h>
#include "FastLED.h"
#include <fallingStar.h>

#define LENGTH 64
CRGB leds[LENGTH];

starSky ledSky(leds, LENGTH, 50);

void setup()
{
  // Begin serial
  Serial.begin(9600);

  // Set up fast leds;
  FastLED.addLeds<WS2812B, 1>(leds, LENGTH);
  fill_solid(leds, LENGTH, 0x030303);

  // Set up star
  int randomColor = random(0, 255);

  // Add star trail
  ledSky.addStar(0, CHSV(200, 255, 255), random(3, 10));
}

void loop()
{

  // Render star
  ledSky.render();

  FastLED.show();
}