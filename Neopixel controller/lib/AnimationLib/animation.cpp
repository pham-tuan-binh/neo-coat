#include <animation.h>

// Animation Functions
void Animation::handleUpload(AsyncWebServerRequest *request, String filename, size_t index, uint8_t *data, size_t len, bool final)
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

void Animation::handleFileList(AsyncWebServerRequest *request)
{
    String path = "/";
    // Assuming there are no subdirectories
    fs::Dir dir = SPIFFS.openDir(path);
    String output = "[\"";
    while (dir.next())
    {
        fs::File entry = dir.openFile("r");
        // Separate by comma if there are multiple files
        if (output != "[\"")
            output += "\",\"";
        output += String(entry.name()).substring(1);
        entry.close();
    }
    output += "\"]";
    request->send(200, "text/plain", output);
}

void Animation::handleFileDelete(AsyncWebServerRequest *request)
{
    // make sure we get a file name as a URL argument
    if (request->params() == 0)
        return request->send(400, "text/plain", "Invalid argument");
    String path = request->getParam(0)->value();
    path = "/" + path;
    Serial.println(path);
    // protect root path
    if (path == "/")
        return request->send(400, "text/plain", "BAD PATH!");
    // check if the file exists
    if (!SPIFFS.exists(path))
        return request->send(404, "text/plain", "FILE NOT FOUND!");
    else
    {
        Serial.println("delete");
    }
    SPIFFS.remove(path);
    Serial.println("DELETE: " + path);
    String msg = "deleted file: " + path;
    request->send(200, "text/plain", msg);
}

void Animation::displayJpegMatrix(String path)
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
                drawable->drawPixel(col + 1, row + 1,
                                    pgm_read_byte(&gamma8[r]),
                                    pgm_read_byte(&gamma8[g]),
                                    pgm_read_byte(&gamma8[b]));
            }
        }
        FastLED.show();
    }
}

void Animation::playSequenceCallback()
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

void Animation::handleFilePlay()
{
    *animationState = true;
}

void Animation::handleFileStop()
{
    *animationState = false;
    FastLED.clear();
}