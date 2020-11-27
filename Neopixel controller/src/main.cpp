// Default library
#include <Arduino.h>
#include "FastLED.h"
#include <ESP8266WiFi.h>
#include <ESPAsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <TaskScheduler.h>
#include <ArduinoJson.h>

#define FS_NO_GLOBALS //avoids conflicts with JPEGDecoder
#include <FS.h>

// Custom library
#include <Snake.h> // Original by Emanuel Knöpfel (This library have been modified for compatibility. Original lib may malfunction in this code)

// Structure library
// These libraries consist of functions previously was in main.cpp and was written by me for the program.
// In compatibility mode, they are stripped out for ease of reading and modification.
#include <animation.h>
#include <drawable.h>

using namespace std;

#define WIDTH 8
#define HEIGHT 8
#define SSID "NeoCoat (^˵◕ω◕˵^)"
#define PASSWORD "0903470077"

const unsigned int LENGTH = WIDTH * HEIGHT;

// Color temperature
// FF4827 for TPC coat
int Temperature = 0xFFFFFF;

CRGB leds[LENGTH];

IPAddress apIP(1, 1, 1, 1);

// States of the program
bool state = false;

bool snakeState = false;

bool animationState = false;

// AsyncWebServer on port 80
AsyncWebServer server(80);

// Snake game
Snake snakeGame(WIDTH, HEIGHT, 10);

// Drawable (This is the core of the program)
Drawable drawable(WIDTH, HEIGHT, leds);

// Animation object
Animation animation(&animationState, &drawable);

// Task
Task playSequence(0, TASK_FOREVER, []() {
  animation.playSequenceCallback();
});

Scheduler runner;

//Config
String convertCRGBtoString(CRGB color)
{
  String colorString = String(color.r) + "-" + String(color.g) + "-" + String(color.b);
  return colorString;
}

CRGB convertStringtoCRGB(String color)
{
  String r, g, b;
  unsigned int index = color.indexOf("-");
  r = color.substring(0, index);
  g = color.substring(index + 1, color.indexOf("-", index + 1));
  index = color.indexOf("-", index + 1);
  b = color.substring(index + 1, color.indexOf("-", index + 1));

  CRGB convertedColor = CRGB(r.toInt(), g.toInt(), b.toInt());

  return convertedColor;
}

void saveLedsState()
{
  const size_t capacity = JSON_ARRAY_SIZE(LENGTH) + 860;
  DynamicJsonDocument ledsJson(capacity);

  for (unsigned int index = 0; index < LENGTH; index++)
  {
    ledsJson.add(convertCRGBtoString(leds[index]));
  }

  fs::File jsonFile = SPIFFS.open("/ledsJson.json", "w");

  serializeJson(ledsJson, jsonFile);

  jsonFile.close();
}

bool applyLedsState()
{
  if (SPIFFS.exists("/ledsJson.json"))
  {
    fs::File jsonFile = SPIFFS.open("/ledsJson.json", "r");

    const size_t capacity = JSON_ARRAY_SIZE(LENGTH) + 860;
    DynamicJsonDocument ledsJson(capacity);

    // Deserialize the JSON document
    DeserializationError error = deserializeJson(ledsJson, jsonFile);
    if (error)
      Serial.println(F("Failed to read file, using default configuration"));

    for (unsigned int index = 0; index < LENGTH; index++)
    {
      leds[index] = convertStringtoCRGB(ledsJson[index]);
    }

    return true;
  }
  else
  {
    fs::File jsonFile = SPIFFS.open("/ledsJson.json", "w");
    return false;
  }
}

// Animation Functions
// In c++ this is a way of passing callbacks thar are bounded functions
// I don't know if there are other optimized ways but contact me if you have it
void handleUpload(AsyncWebServerRequest *request, String filename, size_t index, uint8_t *data, size_t len, bool final)
{
  animation.handleUpload(request, filename, index, data, len, final);
}

void handleFileList(AsyncWebServerRequest *request)
{
  animation.handleFileList(request);
}

void handleFileDelete(AsyncWebServerRequest *request)
{
  animation.handleFileDelete(request);
}

void displayJpegMatrix(String path)
{
  animation.displayJpegMatrix(path);
}

void handleFilePlay()
{
  animation.handleFilePlay();
}

void handleFileStop()
{
  animation.handleFileStop();
}

void snakeLoop()
{
  Snake::pixel *snakeLimbs = snakeGame.getSnakeLimbs();   //this needs to be updated every frame
  Snake::pixel *snakeFood = snakeGame.getFoodPositions(); //this needs to be updated every frame
  drawable.clearScreen();
  drawable.drawPixel(snakeFood[0].posX + 1, HEIGHT - snakeFood[0].posY, snakeFood[0].pixelColor.r, snakeFood[0].pixelColor.g, snakeFood[0].pixelColor.b); // display the food
  for (int i = 0; i < snakeGame.getSnakeLenght(); i++)
  {
    //display the snake, my setpixel method has x=0, y=0 at the top left, but the library has it at bottom left, so I invert the Y-Axis:
    drawable.drawPixel(snakeLimbs[i].posX + 1, HEIGHT - snakeLimbs[i].posY, snakeLimbs[i].pixelColor.r, snakeLimbs[i].pixelColor.g, snakeLimbs[i].pixelColor.b);
  }
  FastLED.show();
  snakeGame.tick();             //main loop for the snake library
  if (snakeGame.wasGameReset()) // if the snake bit itself or the wall, flash a little
  {
    for (int i = 0; i < 30; i++)
    {
      drawable.changeRGBtoGBR();
      FastLED.show();
      delay(40);
    }
    snakeState = false;
  }
  else
    delay(30);
}

void colorPixel(AsyncWebServerRequest *request)
{
  if (request->hasParam("x") && request->hasParam("y") && request->hasParam("color"))
  {
    AsyncWebParameter *x = request->getParam("x");
    AsyncWebParameter *y = request->getParam("y");
    AsyncWebParameter *color = request->getParam("color");

    int x_pixel = atoi(x->value().c_str());
    int y_pixel = atoi(y->value().c_str());
    String color_pixel(color->value().c_str());

    drawable.drawPixel(x_pixel, y_pixel, color_pixel);

    request->send(200, "text/plain", "Okela");
  }
  else
  {
    request->send(400, "text/plain", "Bad request: Missing arguments");
  }
}

void changeTemperature(AsyncWebServerRequest *request)
{
  if (request->hasParam("color"))
  {
    AsyncWebParameter *color = request->getParam("color");

    String color_temp(color->value().c_str());

    Temperature = drawable.stringToHex(color_temp);

    request->send(200, "text/plain", "Okela");
  }
  else
  {
    request->send(400, "text/plain", "Bad request: Missing arguments");
  }
}

void toggle(AsyncWebServerRequest *request)
{
  state = !state;

  if (state)
  {
    if (!applyLedsState())
    {
      fill_solid(leds, LENGTH, CRGB::Gray);
    }
  }
  else
  {
    saveLedsState();
    fill_solid(leds, LENGTH, CRGB::Black);
  }
  FastLED.show();
  request->send(200, "text/plain", "Okela");
}

void animate(AsyncWebServerRequest *request)
{
  if (request->hasParam("id"))
  {
    request->send(200, "text/plain", "Okela");

    AsyncWebParameter *id_pointer = request->getParam("id");
    int id = atoi(id_pointer->value().c_str());

    switch (id)
    {
    case 0:
      playSequence.enable();
      Serial.println("Play");
      animationState = true;
      snakeState = false;
      break;
    case 1:
      playSequence.disable();
      Serial.println("Stop");
      animationState = false;
      snakeState = false;
      if (!applyLedsState())
      {
        fill_solid(leds, LENGTH, CRGB::Gray);
      }
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
}

void command(AsyncWebServerRequest *request)
{
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
      animationState = false;
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
      animationState = false;
      if (!applyLedsState())
      {
        fill_solid(leds, LENGTH, CRGB::Gray);
      }
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
}

// Set up
void setup()
{
  // Begin serial
  Serial.begin(9600);

  // Begin SPIFFS
  if (!SPIFFS.begin())
  {
    Serial.println("Mount filesystem unsuccessfull");

    // Alarm by showing red color
    FastLED.showColor(0xff0000);
  }
  else
  {
    Serial.println("Mount filesystem successfully");
  }

  // Set up ticker
  runner.addTask(playSequence);
  playSequence.enable();

  // Set up fast leds;
  FastLED.addLeds<NEOPIXEL, 1>(leds, LENGTH);

  FastLED.setTemperature(Temperature);

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

  // Set up snake
  snakeGame.setBodyColor(255, 0, 255); //optionally set the color of the snakeparts
  snakeGame.setFoodColor(0, 60, 125);  //optionally set the color of the food
  snakeGame.setHeadColor(225, 20, 60); //optionally set the color of the snakeparts

  // Set up server
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send(SPIFFS, "/index.html", "text/html");
  });

  server.on("/json", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send(SPIFFS, "/ledsJson.json", "text/plain");
  });

  server.on("/index.css", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send(SPIFFS, "/index.css", "text/css");
  });

  server.on("/index.js", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send(SPIFFS, "/index.js", " application/javascript");
  });

  server.on("/color", HTTP_GET, colorPixel);

  server.on("/toggle", HTTP_GET, toggle);

  server.on("/animation", HTTP_GET, animate);

  server.on("/temperture", HTTP_GET, changeTemperature);

  // list available files
  server.on("/list", HTTP_GET, handleFileList);
  // delete file
  server.on("/delete", HTTP_DELETE, handleFileDelete);
  // handle file upload
  server.on(
      "/upload", HTTP_POST, [](AsyncWebServerRequest *request) {
        request->send(200, "text/plain", "{\"success\":1}");
      },
      handleUpload);

  server.on("/snake", HTTP_GET, [](AsyncWebServerRequest *request) {
    snakeState = true;
    request->send(200, "text/plain", "Okela");
  });

  server.on("/command", HTTP_GET, command);

  server.begin();
}
void loop()
{
  if (state && snakeState && !animationState)
  {
    snakeLoop();
  }
  if (state && !snakeState && animationState)
  {
    runner.execute();
  }
  FastLED.setTemperature(Temperature);
  delay(40);
  FastLED.show();
}

// Code written by Pham Tuan Binh. Feel free to modify it, just credit me in the end.