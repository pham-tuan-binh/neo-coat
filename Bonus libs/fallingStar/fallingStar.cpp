#include <fallingStar.h>

void starSky::addStar(int position, CHSV color, int trailLength)
{
    star tempStar(position, color, trailLength);
    starry.push_back(tempStar);
}
void starSky::removeStar()
{
    starry.pop_front();
}
void starSky::drawTrail(star tempStar)
{
    for (int pixels = 0; pixels < tempStar.trailLength; pixels++)
    {
        if (tempStar.position - pixels >= 0 && tempStar.position - pixels < length)
            leds[tempStar.position - pixels] = CHSV(tempStar.color.hue, tempStar.color.sat, pgm_read_byte(&gamma8[tempStar.color.val - pixels * (tempStar.color.val / tempStar.trailLength)]));
    }
}
bool starSky::checkStar(star tempStar)
{
    if (tempStar.position - tempStar.trailLength == length)
    {
        removeStar();
        return false;
    }
    return true;
}
void starSky::moveStar()
{
    for (int pos = 0; pos < starry.size(); pos++)
    {
        starry[pos].position++;
    }
}
void starSky::clearSky()
{
    fill_solid(leds, length, 0x000000);
}
void starSky::render()
{
    for (int pos = 0; pos < starry.size(); pos++)
    {
        if (checkStar(starry[pos]))
        {
            drawTrail(starry[pos]);
        }
    }
    moveStar();
    FastLED.show();
    delay(delayTime);
    clearSky();
}