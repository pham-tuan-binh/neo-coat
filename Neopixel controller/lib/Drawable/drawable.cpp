#include <drawable.h>

// Color convert functions
long Drawable::stringToHex(String color)
{
    long hexColor = strtol(color.c_str(), NULL, 16);

    return hexColor;
}

// Draw functions
// Hex String
int Drawable::positionCalc(int x, int y)
{
    if (x > 0 && x <= _WIDTH && y > 0 && y <= _HEIGHT)
    {
        int position;

        // Because our matrix data flow is opposite for each concurrent line. We need to change x position based on the line's number.
        // If your matrix is parralel, you must change this part of code.
        // !!!
        // if (y % 2 == 0)
        // {
        //     position = y * _WIDTH - (x - 1);
        // }
        // else
        // {
        position = (y - 1) * _WIDTH + x;
        // }

        return position;
    }
    return 0;
}

byte Drawable::drawPixel(int x, int y, String color)
{

    int position = positionCalc(x, y);

    leds[position - 1] = stringToHex(color);

    return 0;
}
// RGB value for snake library
byte Drawable::drawPixel(byte x, byte y, byte r, byte g, byte b)
{
    byte position = positionCalc(x, y);

    leds[position - 1] = CRGB(r, g, b);

    return position;
}
// CRGB support
byte Drawable::drawPixel(byte x, byte y, CRGB color)
{
    byte position = positionCalc(x, y);

    leds[position - 1] = color;

    return position;
}

void Drawable::clearScreen()
{
    fill_solid(leds, length, CRGB(3, 3, 3));
    FastLED.show();
}

void Drawable::changeRGBtoGBR()
{
    for (unsigned int whiteLed = 0; whiteLed < length; whiteLed++)
    {
        leds[whiteLed].setRGB(leds[whiteLed].g, leds[whiteLed].b, leds[whiteLed].r);
    }
}