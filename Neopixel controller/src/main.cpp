// Default library
#include <Arduino.h>
#include <string>
#include "FastLED.h"
#include "LittleFS.h"
#include <ESP8266WiFi.h>
#include <ESPAsyncTCP.h>
#include <ESPAsyncWebServer.h>

// Custom library
#include <Snake.h> // Original by Emanuel Knöpfel (This library have been modified for compatibility. Original lib may malfunction in this code)

using namespace std;

#define WIDTH 6
#define HEIGHT 6
#define SSID "NeoCoat (^˵◕ω◕˵^)"
#define PASSWORD "0903470077"

const unsigned int length = WIDTH * HEIGHT;

CRGB leds[length];

IPAddress apIP(42, 42, 42, 42);

bool state = false;

bool snakeState = false;

// AsyncWebServer on port 80
AsyncWebServer server(80);

// Snake game
Snake snakeGame(6, 6, 10);

// Color convert functions
long stringToHex(string color)
{
  long hexColor = strtol(color.c_str(), NULL, 16);

  return hexColor;
}

// Draw functions
// Hex string
byte drawPixel(int x, int y, string color)
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

    FastLED.show();
  }
  return 0;
}
// RGB value for snake library
byte drawPixel(byte x, byte y, byte r, byte g, byte b)
{
  if (x > 0 && x <= WIDTH && y > 0 && y <= HEIGHT)
  {
    byte position;

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

    leds[position - 1] = CRGB(r, g, b);

    FastLED.show();

    return position;
  }
  return 0;
}

// Functions to adapt snake game. Might overlap existing functions.
void clearScreen()
{
  fill_solid(leds, length, CRGB(3, 3, 3));
  FastLED.show();
}

void changeRGBtoGBR()
{
  for (unsigned int whiteLed = 0; whiteLed < length; whiteLed++)
  {
    leds[whiteLed].setRGB(leds[whiteLed].g, leds[whiteLed].b, leds[whiteLed].r);
  }
}

void snakeLoop()
{
  Snake::pixel *snakeLimbs = snakeGame.getSnakeLimbs();   //this needs to be updated every frame
  Snake::pixel *snakeFood = snakeGame.getFoodPositions(); //this needs to be updated every frame
  clearScreen();
  drawPixel(snakeFood[0].posX + 1, HEIGHT - snakeFood[0].posY, snakeFood[0].pixelColor.r, snakeFood[0].pixelColor.g, snakeFood[0].pixelColor.b); // display the food
  for (int i = 0; i < snakeGame.getSnakeLenght(); i++)
  {
    //display the snake, my setpixel method has x=0, y=0 at the top left, but the library has it at bottom left, so I invert the Y-Axis:
    drawPixel(snakeLimbs[i].posX + 1, HEIGHT - snakeLimbs[i].posY, snakeLimbs[i].pixelColor.r, snakeLimbs[i].pixelColor.g, snakeLimbs[i].pixelColor.b);
  }
  FastLED.show();
  snakeGame.tick();             //main loop for the snake library
  if (snakeGame.wasGameReset()) // if the snake bit itself or the wall, flash a little
  {
    for (int i = 0; i < 30; i++)
    {
      changeRGBtoGBR();
      FastLED.show();
      delay(40);
    }
    snakeState = false;
  }
  else
    delay(30);
}

// Set up
void setup()
{
  // Begin serial
  Serial.begin(9600);

  // Set up fast leds;
  FastLED.addLeds<NEOPIXEL, 1>(leds, length);

  FastLED.clear();
  FastLED.show();

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

  // Set up snake
  snakeGame.setBodyColor(255, 0, 255); //optionally set the color of the snakeparts
  snakeGame.setFoodColor(0, 60, 125);  //optionally set the color of the food
  snakeGame.setHeadColor(225, 20, 60); //optionally set the color of the snakeparts

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

  server.on("/color", HTTP_GET, [](AsyncWebServerRequest *request) {
    if (request->hasParam("x") && request->hasParam("y") && request->hasParam("color"))
    {
      AsyncWebParameter *x = request->getParam("x");
      AsyncWebParameter *y = request->getParam("y");
      AsyncWebParameter *color = request->getParam("color");

      int x_pixel = atoi(x->value().c_str());
      int y_pixel = atoi(y->value().c_str());
      string color_pixel(color->value().c_str());

      drawPixel(x_pixel, y_pixel, color_pixel);

      request->send(200, "text/plain", "Okela");
    }
    else
    {
      request->send(400, "text/plain", "Bad request: Missing arguments");
    }
  });

  server.on("/toggle", HTTP_GET, [](AsyncWebServerRequest *request) {
    state = !state;

    if (state)
    {
      fill_solid(leds, length, CRGB::Gray);
    }
    else
      FastLED.clear();
    FastLED.show();
    request->send(200, "text/plain", "Okela");
  });

  server.on("/snake", HTTP_GET, [](AsyncWebServerRequest *request) {
    snakeState = true;

    request->send(200, "text/plain", "Okela");
  });

  server.on("/command", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send(200, "text/plain", "Okela");

    if (request->hasParam("id"))
    {
      AsyncWebParameter *id_pointer = request->getParam("id");
      int id = atoi(id_pointer->value().c_str());

      Serial.println("Movement:");
      Serial.print(id);

      switch (id)
      {
      case 0:
        snakeState = true;
        break;
      case 1:
        snakeGame.goUp();
        break;
      case 2:
        snakeGame.goRight();
        break;
      case 3:
        snakeGame.goDown();
        break;
      case 4:
        snakeGame.goLeft();
        break;
      case 5:
        snakeState = false;
        fill_solid(leds, length, CRGB::Gray);
        FastLED.show();
        break;
      default:
        break;
      }
    }
    else
    {
      request->send(400, "text/plain", "Missing id");
    }
  });

  server.begin();
}
void loop()
{
  if (state && snakeState)
  {
    snakeLoop();
    Serial.println("Running");
    delay(40);
  }
}

// Code written by Pham Tuan Binh. Feel free to modify it, just credit me in the end.