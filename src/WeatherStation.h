#ifndef _WEATHER_STATION_H

#define _WEATHER_STATION_H

#include <Arduino.h>
#include <WiFiManager.h>
#include <WebServer.h>
#include <TFT_eSPI.h>
#include <ArduinoJson.h>
#include <HTTPClient.h>
#include <ESP32Time.h>
#include "ArialRounded14.h"
#include "ArialRounded36.h"
#include "weatherIcons.h"

#define VW_Version "v. 1.00"

struct sensorData {
  float t = 0.0;                  // temp
  float h = 0.0;                  // humidity
  float b = 0.0;                  // battery voltage
  bool is_update = false;         // if true, data has been updated
};

// Objects to manipulate display
TFT_eSPI tft = TFT_eSPI();
TFT_eSprite sprite = TFT_eSprite(&tft);

// Network and Web app related
WiFiManager wifiManager;
String staHostname = "WeatherStation";      // Not sure it"s usefull to configure hostname.
// [Please CHANGE THIS] - Custom Static adress for the Weather Station
char static_ip[16] = "192.168.3.10";
char static_gw[16] = "192.168.3.1";
char static_mask[16] = "255.255.255.0";
char static_dns[16] = "192.168.3.1";
// [Please CHANGE THIS] - TimeZone/Town/Latitude/longitude
const String timeZone = "Europe%2FParis";
const String townName = "Toulouse";
const String townLat = "43.603951";
const String townLon = "1.444510";

WebServer server(80);

// API Web server for accurate time getTime.
String timeServer = "https://timeapi.io/api/time/current/zone?timeZone=" + timeZone;
String weatherServer = "https://api.open-meteo.com/v1/forecast?latitude=" + townLat + "&longitude=" + townLon
                       + "&current=weather_code&daily=weather_code,temperature_2m_max,temperature_2m_min,precipitation_probability_max&forecast_days=1";

const char *days[] = {"DIM","LUN","MAR","MER","JEU","VEN","SAM"};
const char *months[] = {"JAN","FEV","MAR","AVR","MAI","JUN","JUL","AOU","SEP","OCT","NOV","DEC"};
ESP32Time rtc(0); 

struct DateTime {
  int year;
  int month;
  int day;
  int hour;
  int minute;
  int second;
  long microsecond;
};
#define WS_BLACK TFT_BLACK
#define WS_WHITE TFT_WHITE
#define WS_YELLOW TFT_YELLOW
#define WS_BLUE 0x7E3C

// forward declarations
bool drawTime(void *);
void drawSensor(float t, float h, float b, short tempColor);
void drawBatLevel(TFT_eSprite &spr,int sprX,int SprY,int level);
void drawForecast(int wmo, float minT, float maxT, short rainProba);
const uint16_t* getIconFromWMO(int wmo);
String getDescriptionFromWMO(int wmo);
bool getTime(void *);
DateTime parseISO8601(const String& iso8601);
bool getSensor(void *);
bool getForecast(void *);
void setupApi();
void handlePost();

#endif 
 
