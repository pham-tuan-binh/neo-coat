// Default library
#include <Arduino.h>
#include <string>
#include "FastLED.h"
#include "LittleFS.h"
#include <ESP8266WiFi.h>
#include <ESPAsyncTCP.h>
#include <ESPAsyncWebServer.h>

using namespace std;

#define WIDTH 6
#define HEIGHT 6
#define SSID "NeoCoat - MK1"
#define PASSWORD "123456789"

const unsigned int length = WIDTH * HEIGHT;

CRGB leds[length];

IPAddress apIP(42, 42, 42, 42);

// AsyncWebServer on port 80
AsyncWebServer server(80);

// Color convert functions
long stringToHex(string color)
{
  string preConversion = color.substr(1);
  long hexColor = strtol(preConversion.c_str(), NULL, 16);

  return hexColor;
}

// Draw functions
void drawPixel(int x, int y, string color)
{
  if (x > 0 && x <= WIDTH && y > 0 && y <= HEIGHT)
  {
    int position;

    // Because our matrix data flow is opposite for each concurrent line. We need to change x position based on the line's number.
    // If your matrix is parralel, you must change this part of code.
    // !!!
    if (y % 2 == 0)
    {
      position = y * WIDTH - (x - 1);
    }
    else
    {
      position = (y - 1) * WIDTH + x;
    }

    leds[position - 1] = stringToHex(color);
  }
}

void setup()
{
  // Begin serial
  Serial.begin(9600);

  // Set up wifi access point
  WiFi.mode(WIFI_AP_STA);
  WiFi.softAPConfig(apIP, apIP, IPAddress(255, 255, 255, 0)); // subnet FF FF FF 00

  WiFi.softAP(SSID, PASSWORD);

  // IP for web
  IPAddress myIP = WiFi.softAPIP();
  Serial.print("AP IP address: ");
  Serial.println(myIP);

  // Begin LittleFS
  if (!LittleFS.begin())
  {
    Serial.println("Mount filesystem unsuccessfull");

    // Alarm by showing red color
    FastLED.showColor(0xff0000);
  }
  else
  {
    Serial.println("Mount filesystem successfully");
  }

  // Set up fast leds;
  FastLED.addLeds<NEOPIXEL, 1>(leds, length);

  // Set up server
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send(LittleFS, "/index.html", "text/html");
  });

  server.on("/index.css", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send(LittleFS, "/index.css", "text/css");
  });

  server.on("/index.js", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send(LittleFS, "/index.js", " application/javascript");
  });

  server.begin();
}
void loop()
{

  for (int i = 1; i <= HEIGHT; i++)
  {
    for (int o = 1; o <= WIDTH; o++)
    {
      drawPixel(o, i, "#121212");
    }
  }
  drawPixel(1, 2, "#ff1212");
  drawPixel(2, 3, "#ff1212");
  FastLED.show();
}