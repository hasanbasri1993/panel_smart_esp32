#include "Arduino.h"
#include "Variables.h"
#include <ESP32-HUB75-MatrixPanel-I2S-DMA.h>
#include "WiFi.h"
#include <Fonts/FreeSerif9pt7b.h>
#include <WebServer.h>
#include <ArduinoJson.h>
#include <FreeRTOS.h>
#include <Preferences.h>
#include <HTTPClient.h>

MatrixPanel_I2S_DMA *dma_display = nullptr;
WebServer server(80);
Preferences preferences;

StaticJsonDocument<250> jsonDocument;

String fetchBerita() {
    HTTPClient http;

    // Your IP address with path or Domain name with URL path
    http.begin("https://newsapi.org/v2/top-headlines?country=id&apiKey=ef589987f29946ba8402cbb50b1d200c");

    // Send HTTP POST request
    int httpResponseCode = http.GET();

    String payload = "{}";

    if (httpResponseCode > 0) {
        Serial.print("HTTP Response code: ");
        Serial.println(httpResponseCode);
        payload = http.getString();
    } else {
        Serial.print("Error code: ");
        Serial.println(httpResponseCode);
    }
    // Free resources
    http.end();

    return payload;
}

void updateTime() {
    if (currentMillis - previousMillisTimeUpdate >= 1000) {
        previousMillisTimeUpdate = currentMillis;
        time_t t;
        static time_t last_t;
        struct tm *tm;
        static const char *const wd[7] = {"Minggu", "Senin", "Selasa", "Rabu", "Kamis", "Jum'at", "Sabtu"};

        t = time(nullptr);
        if (last_t == t)
            return;
        last_t = t;
        int jam, menit, detik;
        int16_t yPos = 13;
        tm = localtime(&t);
        dma_display->fillRect(0, 0, 64, 16, myBLACK);
        jam = tm->tm_hour;
        menit = tm->tm_min;
        detik = tm->tm_sec;
        dma_display->setTextSize(0);
        dma_display->setFont(&FreeSerif9pt7b);
        dma_display->setCursor(1, yPos);
        dma_display->setTextColor(myBRIGHT_RED);
        dma_display->printf("%02d", jam);
        dma_display->setTextColor(MatrixPanel_I2S_DMA::color444(15, 0, 0));
        dma_display->setCursor(19, yPos - 2);
        dma_display->print(":");
        dma_display->setTextColor(myBRIGHT_RED);
        dma_display->setCursor(23, yPos);
        dma_display->printf("%02d", menit);
        dma_display->setTextColor(MatrixPanel_I2S_DMA::color444(0, 15, 15));
        dma_display->setCursor(41, yPos - 2);
        dma_display->print(":");
        dma_display->setTextColor(myBRIGHT_RED);
        dma_display->setCursor(45, yPos);
        dma_display->printf("%02d", detik);
    }
}

void scroll_text(uint8_t ypos, unsigned long scroll_delay, String text) {
    text_length = text.length();
    dma_display->setTextWrap(false);
    if (xposScroll > -(64 + text_length * 14)) {
        if (currentMillis - previousMillisScroll >= scroll_delay) {
            previousMillisScroll = currentMillis;
            xposScroll--;
            int y = r;
            dma_display->setFont();
            dma_display->setCursor(xposScroll, ypos);
            dma_display->fillRect(0, 16, 64, 17, myBLACK);
            for (int x = 0; x < text_length; x++) {
                dma_display->setTextColor(myRED);
                dma_display->print(text[x]);
                y++;
                if (y >= 9) {
                    y = 0;
                }
            }
        }
    } else { xposScroll = 64; }
}

void handlePostMessage() {
    if (!server.hasArg("plain"))
        server.send(200, "application/json", "Please send json format");

    String body = server.arg("plain");
    deserializeJson(jsonDocument, body);

    String red = jsonDocument["message"];
    Message0 = red;
    preferences.putString("message", red);

    // Respond to the client
    server.send(200, "application/json", red);
}

void handlePostScrollDelay() {
    if (!server.hasArg("plain"))
        server.send(200, "application/json", "Please send json format");

    String body = server.arg("plain");
    deserializeJson(jsonDocument, body);

    int32_t red = jsonDocument["delay"];
    scrollDelay = red;
    preferences.putInt("scrollDelay", red);

    // Respond to the client
    server.send(200, "application/json", "success");
}

void handlePostBrightness() {
    if (!server.hasArg("plain"))
        server.send(200, "application/json", "Please send json format");

    String body = server.arg("plain");
    deserializeJson(jsonDocument, body);

    int32_t red = jsonDocument["brightness"];
    brightness = red;
    preferences.putInt("brightness", red);
    //
    // Respond to the client
    server.send(200, "application/json", "success");
}

void setup_routing() {
    server.on("/brightness", HTTP_POST, handlePostBrightness);
    server.on("/delay_scroll", HTTP_POST, handlePostScrollDelay);
    server.on("/text", HTTP_POST, handlePostMessage);
    server.begin();
}

void setup() {
    HUB75_I2S_CFG mxconfig(
            PANEL_RES_X, // module width
            PANEL_RES_Y, // module height
            PANEL_CHAIN  // Chain length
    );

    mxconfig.gpio.e = 18;
    mxconfig.clkphase = false;
    mxconfig.driver = HUB75_I2S_CFG::FM6126A;
    dma_display = new MatrixPanel_I2S_DMA(mxconfig);
    dma_display->begin();
    dma_display->setBrightness8(50); // 0-255
    dma_display->clearScreen();
    dma_display->fillScreen(myWHITE);
    dma_display->clearScreen();
    Serial.begin(115200);
    Serial.print("Attempting to connect to Network named: ");
    Serial.println(ssid);
    WiFi.begin(ssid, pass);
    while (WiFiClass::status() != WL_CONNECTED) {
        Serial.print(".");
        delay(1000);
    }
    Serial.println("");
    IPAddress ip = WiFi.localIP();
    Serial.print("IP Address: ");
    Serial.println(ip);

    setup_routing();

    preferences.begin("gpio", false);
    Message0 = preferences.getString("message", Message0);
    scrollDelay = preferences.getInt("scrollDelay", scrollDelay);
    brightness = preferences.getInt("brightness", brightness);
    Serial.print("Read from: ");
    Serial.print(Message0);
    Serial.println("");
    dma_display->clearScreen();
    dma_display->setTextWrap(true);
    dma_display->setCursor(0, 0);
    dma_display->setTextSize(1);
    dma_display->setFont(&FreeSerif9pt7b);
    dma_display->setTextColor(MatrixPanel_I2S_DMA::color444(0, 15, 15));
    dma_display->print(ip);
    delay(3000);
    dma_display->clearScreen();
    configTime(TZ, 0, "id.pool.ntp.org");
}

void loop() {
    currentMillis = millis();
    server.handleClient();
    dma_display->setBrightness8(brightness);
    updateTime();
    scroll_text(17, scrollDelay, Message0);
}