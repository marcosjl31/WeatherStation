#ifndef _WEATHER_STATION_H

#define _WEATHER_STATION_H

#include <Arduino.h>
#include <WiFiManager.h>
#include <WebServer.h>
#include <TFT_eSPI.h>
#include <ArduinoJson.h>
#include <HTTPClient.h>
#include <ESP32Time.h>

#define VW_Version "v. 0.01"

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

WebServer server(80);

// Json var to handle JSON deserialization either of API request (POST from Temp Sensor)
// or getting time from Internet.


// API Web server for accurate time getTime.
String timeServer = "https://timeapi.io/api/time/current/zone?timeZone=Europe%2FParis";
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

// forward declarations
bool getTime(void *);
DateTime parseISO8601(const String& iso8601);
bool getSensor(void *);
bool getForecast(void *);
void setupApi();
void handlePost();

#endif 
 
