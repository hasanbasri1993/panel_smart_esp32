#include "Arduino.h"
#include "Variables.h"
#include <ESP32-HUB75-MatrixPanel-I2S-DMA.h>
#include "WiFi.h"
#include <Fonts/FreeSerif9pt7b.h>
#include <Fonts/kongtext4pt7b.h>
#include <WebServer.h>
#include <ArduinoJson.h>
#include <FreeRTOS.h>
#include <Preferences.h>
#include <HTTPClient.h>

MatrixPanel_I2S_DMA *dma_display = nullptr;
WebServer server(80);
Preferences preferences;

StaticJsonDocument<32> jsonRequest;
DynamicJsonDocument beritaJson(18000);

void fetchBerita() {
    HTTPClient http;
    http.begin(urlNews);
    int httpResponseCode = http.GET();
    if (httpResponseCode > 0) {
        beritaJson.clear();
        Serial.print("HTTP Response code: ");
        Serial.print("HTTP Response code: ");
        Serial.println(httpResponseCode);
        DeserializationError error = deserializeJson(beritaJson, http.getString());
        if (error) {
            Serial.print("deserializeJson() failed: ");
            Serial.println(error.c_str());
        }
    } else {
        Serial.print("Error code: ");
        Serial.println(httpResponseCode);
    }
    http.end();
}

void updateTime() {
    if (currentMillis - previousMillisTimeUpdate >= 1000) {
        previousMillisTimeUpdate = currentMillis;
        time_t t;
        static time_t last_t;
        struct tm *tm;
        static const char *const wd[7] = {"Minggu", "Senin", "Selasa", "Rabu", "Kamis", "Jumat", "Sabtu"};
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


        int16_t yPosDate = 30;
        dma_display->fillRect(0, 23, 64, 1, myBRIGHT_RED);
        dma_display->fillRect(0, 24, 64, 10, myBLACK);
        dma_display->setFont(&kongtext4pt7b);
        dma_display->setTextColor(myDARKGREEN);
        dma_display->setCursor(1, yPosDate);
        dma_display->print(wd[tm->tm_wday]);
        dma_display->setCursor(40, yPosDate);
        dma_display->print(", ");
        dma_display->setCursor(45, yPosDate);
        dma_display->print(tm->tm_mday);
        dma_display->setFont();
    }
}

void scroll_text(uint8_t ypos, unsigned long scroll_delay, String text) {
    text_length = text.length();
    dma_display->setTextWrap(false);
    if (xposScroll > -(64 + text_length * 7.5)) {
        if (currentMillis - previousMillisScroll >= scroll_delay) {
            previousMillisScroll = currentMillis;
            xposScroll--;
            int y = r;
            dma_display->setFont();
            dma_display->setCursor(xposScroll, ypos);
            dma_display->fillRect(0, ypos, 64, 8, myBLACK);
            dma_display->setFont(&kongtext4pt7b);
            for (int x = 0; x < text_length; x++) {
                dma_display->setTextColor(myRED);
                dma_display->print(text[x]);
                y++;
                if (y >= 9) {
                    y = 0;
                }
            }
        }
    } else {
        if (bacaBeritaKe > 36) {
            bacaBeritaKe = 0;
            fetchBerita();
        } else bacaBeritaKe++;
        xposScroll = 64;
    }
}

void handlePostMessage() {
    if (!server.hasArg("plain"))
        server.send(200, "application/json", "Please send json format");
    String body = server.arg("plain");
    deserializeJson(jsonRequest, body);
    String red = jsonRequest["message"];
    Message0 = red;
    preferences.putString("message", red);
    server.send(200, "application/json", red);
}

void handlePostScrollDelay() {
    if (!server.hasArg("plain"))
        server.send(200, "application/json", "Please send json format");
    String body = server.arg("plain");
    deserializeJson(jsonRequest, body);
    int32_t red = jsonRequest["delay"];
    scrollDelay = red;
    preferences.putInt("scrollDelay", red);
    server.send(200, "application/json", "success");
}

void handlePostBrightness() {
    if (!server.hasArg("plain"))
        server.send(200, "application/json", "Please send json format");
    String body = server.arg("plain");
    deserializeJson(jsonRequest, body);
    int32_t red = jsonRequest["brightness"];
    brightness = red;
    preferences.putInt("brightness", red);
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
    dma_display->setTextWrap(true);
    dma_display->setCursor(0, 0);
    dma_display->setTextSize(1);
    dma_display->setTextColor(MatrixPanel_I2S_DMA::color444(0, 15, 15));
    dma_display->print("Attempting to connect ...");

    Serial.begin(250000);
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
    fetchBerita();
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
    dma_display->setTextColor(MatrixPanel_I2S_DMA::color444(0, 15, 15));
    dma_display->print(ip);
    dma_display->setCursor(0, 16);
    dma_display->print(ssid);
    delay(3000);
    dma_display->clearScreen();
    configTime(TZ, 0, "id.pool.ntp.org");
}

void loop() {
    currentMillis = millis();
    valueLux = analogRead(ldr); //Reads the Value of LDR(light).
    if (valueLux < threshold) dma_display->setBrightness8(10);
    else dma_display->setBrightness8(brightness);
    server.handleClient();
    updateTime();
    Message0 = beritaJson["articles"][bacaBeritaKe]["description"].as<String>() + " - " +
               beritaJson["articles"][bacaBeritaKe]["source"]["name"].as<String>();
    scroll_text(15, scrollDelay, Message0);
}