#ifndef DRAWABLE
#define DRAWABLE

#include <Arduino.h>
#include <string>
#include "FastLED.h"

class Drawable
{
private:
    int _WIDTH;
    int _HEIGHT;
    int length;
    CRGB *leds;

public:
    Drawable(int width, int height, CRGB *leds) : _WIDTH(width), _HEIGHT(height), leds(leds), length(width * height)
    {
    }
    // Color convert functions
    long stringToHex(std::string color);

    // Draw functions
    // Hex string
    int positionCalc(int x, int y);
    byte drawPixel(int x, int y, std::string color);

    // RGB value for snake library
    byte drawPixel(byte x, byte y, byte r, byte g, byte b);

    // CRGB support
    byte drawPixel(byte x, byte y, CRGB color);

    void clearScreen();

    void changeRGBtoGBR();
};

#endif