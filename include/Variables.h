//
// Created by Hasan Basri on 15/03/22.
//

#ifndef INC_220314_124647_LOLIN32_LITE_VARIABLES_H
#define INC_220314_124647_LOLIN32_LITE_VARIABLES_H

unsigned long previousMillisTimeUpdate = 0;
unsigned long previousMillisScroll = 0;
unsigned long currentMillis = 0;


#define PANEL_RES_X 64 // Number of pixels wide of each INDIVIDUAL panel module.
#define PANEL_RES_Y 32 // Number of pixels tall of each INDIVIDUAL panel module.
#define PANEL_CHAIN 1  // Total number of panels chained one to another

int16_t xposScroll = 64;
uint16_t text_length = 0;
char ssid[] = "Smart Home AP"; // Nama Wifi Anda
char pass[] = "Husna1299";     // Password Wifi Anda
#define TZ (+7 * 60 * 60)       // Timezone

String Message0 = "welcome...";
String apiNews = "<change with your own";
String urlNews = "https://newsapi.org/v2/top-headlines?country=id&apiKey=" + apiNews;

int32_t scrollDelay = 15;
int32_t brightness = 100;

char beritaLastest[38] = {};
int bacaBeritaKe = 0;

// Color definitions
static const uint16_t myBLACK = 0x0000;
static const uint16_t myWHITE = 0xFFFF;
static const uint16_t myNAVY = 0x000F;
static const uint16_t myDARKGREEN = 0x03E0;
static const uint16_t myDARKCYAN = 0x03EF;
static const uint16_t myMAROON = 0x7800;
static const uint16_t myPURPLE = 0x780F;
static const uint16_t myORANGE = 0xFD20;
static const uint16_t myPINK = 0xF81F;
static const uint16_t myOLIVE = 0x7BE0;
static const uint16_t myBLUE = 0x001F;
static const uint16_t myRED = 0xF800;
static const uint16_t myGREEN = 0x07E0;
static const uint16_t myCYAN = 0x07FF;
static const uint16_t myMAGENTA = 0xF81F;
static const uint16_t myYELLOW = 0xFFE0;
static const uint16_t myBRIGHT_RED = 0xF810;
static const uint16_t myLIGHT_GREY = 0xC618;//0x8410;
static const uint16_t myDARK_GREY = 0x7BEF;//0x4208;
static const uint16_t myGREENYELLOW = 0xAFE5;

uint8_t r = 0, g = 0, b = 0;

#endif //INC_220314_124647_LOLIN32_LITE_VARIABLES_H

