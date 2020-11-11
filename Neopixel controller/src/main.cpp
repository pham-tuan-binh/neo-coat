// Default library
#include <Arduino.h>
#include <string>
#include "FastLED.h"
#include <ESP8266WiFi.h>
#include <ESPAsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <TaskScheduler.h>

#define FS_NO_GLOBALS //avoids conflicts with JPEGDecoder
#include <FS.h>
#include <JPEGDecoder.h>

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

bool animationState = false;

// AsyncWebServer on port 80
AsyncWebServer server(80);

// Snake game
Snake snakeGame(6, 6, 10);

// hold uploaded file
fs::File fsUploadFile;

// Gamma correction
const uint8_t PROGMEM gamma8[] = {
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1, 1, 2, 2, 2, 2, 2, 2, 2,
    2, 3, 3, 3, 3, 3, 3, 3, 4, 4, 4, 4, 4, 5, 5, 5,
    5, 6, 6, 6, 6, 7, 7, 7, 7, 8, 8, 8, 9, 9, 9, 10,
    10, 10, 11, 11, 11, 12, 12, 13, 13, 13, 14, 14, 15, 15, 16, 16,
    17, 17, 18, 18, 19, 19, 20, 20, 21, 21, 22, 22, 23, 24, 24, 25,
    25, 26, 27, 27, 28, 29, 29, 30, 31, 32, 32, 33, 34, 35, 35, 36,
    37, 38, 39, 39, 40, 41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 50,
    51, 52, 54, 55, 56, 57, 58, 59, 60, 61, 62, 63, 64, 66, 67, 68,
    69, 70, 72, 73, 74, 75, 77, 78, 79, 81, 82, 83, 85, 86, 87, 89,
    90, 92, 93, 95, 96, 98, 99, 101, 102, 104, 105, 107, 109, 110, 112, 114,
    115, 117, 119, 120, 122, 124, 126, 127, 129, 131, 133, 135, 137, 138, 140, 142,
    144, 146, 148, 150, 152, 154, 156, 158, 160, 162, 164, 167, 169, 171, 173, 175,
    177, 180, 182, 184, 186, 189, 191, 193, 196, 198, 200, 203, 205, 208, 210, 213,
    215, 218, 220, 223, 225, 228, 231, 233, 236, 239, 241, 244, 247, 249, 252, 255};

// Color convert functions
long stringToHex(string color)
{
  long hexColor = strtol(color.c_str(), NULL, 16);

  return hexColor;
}

// Draw functions
// Hex string
int positionCalc(int x, int y)
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

    return position;
  }
  return 0;
}

byte drawPixel(int x, int y, string color)
{

  int position = positionCalc(x, y);

  leds[position - 1] = stringToHex(color);

  return 0;
}
// RGB value for snake library
byte drawPixel(byte x, byte y, byte r, byte g, byte b)
{
  byte position = positionCalc(x, y);

  leds[position - 1] = CRGB(r, g, b);

  return position;
}
// CRGB support
byte drawPixel(byte x, byte y, CRGB color)
{
  byte position = positionCalc(x, y);

  leds[position - 1] = color;

  return position;
}

// Animation Functions
void handleUpload(AsyncWebServerRequest *request, String filename, size_t index, uint8_t *data, size_t len, bool final)
{
  if (!index)
  {
    if (!filename.startsWith("/"))
      filename = "/" + filename;
    Serial.print("handleFileUpload Name: ");
    Serial.println(filename);
    fsUploadFile = SPIFFS.open(filename, "w");
  }
  if (fsUploadFile)
    fsUploadFile.write(data, len);
  if (final)
  {
    if (fsUploadFile)
      fsUploadFile.close();
    Serial.print("File upload finish");
  }
}

void handleFileList(AsyncWebServerRequest *request)
{
  String path = "/";
  // Assuming there are no subdirectories
  fs::Dir dir = SPIFFS.openDir(path);
  String output = "[";
  while (dir.next())
  {
    fs::File entry = dir.openFile("r");
    // Separate by comma if there are multiple files
    if (output != "[")
      output += ",";
    output += String(entry.name()).substring(1);
    entry.close();
  }
  output += "]";
  request->send(200, "text/plain", output);
}

void handleFileDelete(AsyncWebServerRequest *request)
{
  // make sure we get a file name as a URL argument
  if (request->params() == 0)
    return request->send(400, "text/plain", "Invalid argument");
  String path = request->getParam(0)->value();
  // protect root path
  if (path == "/")
    return request->send(400, "text/plain", "BAD PATH!");
  // check if the file exists
  if (!SPIFFS.exists(path))
    return request->send(404, "text/plain", "FILE NOT FOUND!");
  SPIFFS.remove(path);
  Serial.println("DELETE: " + path);
  String msg = "deleted file: " + path;
  request->send(200, "text/plain", msg);
}

void displayJpegMatrix(String path)
{
  if (JpegDec.decodeFsFile(path))
  {
    uint32_t mcuPixels = JpegDec.MCUWidth * JpegDec.MCUHeight;
    uint8_t row = 0;
    uint8_t col = 0;
    while (JpegDec.read())
    {
      uint16_t *pImg = JpegDec.pImage;
      for (uint8_t i = 0; i < mcuPixels; i++)
      {
        // Extract the red, green, blue values from each pixel
        uint8_t b = uint8_t((*pImg & 0x001F) << 3);   // 5 LSB for blue
        uint8_t g = uint8_t((*pImg & 0x07C0) >> 3);   // 6 'middle' bits for green
        uint8_t r = uint8_t((*pImg++ & 0xF800) >> 8); // 5 MSB for red
        // Calculate the matrix index (column and row)
        col = JpegDec.MCUx * 8 + i % 8;
        row = JpegDec.MCUy * 8 + i / 8;
        // Set the matrix pixel to the RGB value
        drawPixel(col + 1, row + 1,
                  pgm_read_byte(&gamma8[r]),
                  pgm_read_byte(&gamma8[g]),
                  pgm_read_byte(&gamma8[b]));
      }
    }
    FastLED.show();
  }
}

void playSequenceCallback()
{
  String path = "/";
  // Assuming there are no subdirectories
  fs::Dir dir = SPIFFS.openDir(path);
  while (dir.next())
  {
    fs::File entry = dir.openFile("r");
    String filename = entry.name();
    if (filename.endsWith(".jpg"))
    {
      displayJpegMatrix(filename);
      delay(20);
    }
    entry.close();
  }
}

// Task
Task playSequence(0, TASK_FOREVER, &playSequenceCallback);
Scheduler runner;

void handleFilePlay()
{
  animationState = true;
}

void handleFileStop()
{
  FastLED.clear();
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

  // Set up snake
  snakeGame.setBodyColor(255, 0, 255); //optionally set the color of the snakeparts
  snakeGame.setFoodColor(0, 60, 125);  //optionally set the color of the food
  snakeGame.setHeadColor(225, 20, 60); //optionally set the color of the snakeparts

  // Set up server
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send(SPIFFS, "/index.html", "text/html");
  });

  server.on("/index.css", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send(SPIFFS, "/index.css", "text/css");
  });

  server.on("/index.js", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send(SPIFFS, "/index.js", " application/javascript");
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
      fill_solid(leds, length, CRGB::Black);
    FastLED.show();
    request->send(200, "text/plain", "Okela");
  });

  server.on("/animation", HTTP_GET, [](AsyncWebServerRequest *request) {
    if (request->hasParam("id"))
    {
      AsyncWebParameter *id_pointer = request->getParam("id");
      int id = atoi(id_pointer->value().c_str());

      switch (id)
      {
      case 0:
        Serial.println("Play");
        animationState = true;
        snakeState = false;
        break;
      case 1:
        Serial.println("Stop");
        animationState = false;
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
  if (state && snakeState && !animationState)
  {
    snakeLoop();
  }
  if (state && !snakeState && animationState)
  {
    runner.execute();
  }
  delay(40);
  FastLED.show();
}

// Code written by Pham Tuan Binh. Feel free to modify it, just credit me in the end.