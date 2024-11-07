#ifndef _WEATHER_STATION_H

#define _WEATHER_STATION_H

#include <Arduino.h>
#include <WiFiManager.h>
#include <TFT_eSPI.h>
#include <ArduinoJson.h>
#include <HTTPClient.h>
#include <ESP32Time.h>

// Objects to manipulate display
TFT_eSPI tft = TFT_eSPI();
TFT_eSprite sprite = TFT_eSprite(&tft);

// [Please CHANGE THIS] - Custom Static adress for the Weather Station
char static_ip[16] = "192.168.3.10";
char static_gw[16] = "192.168.3.1";
char static_mask[16] = "255.255.255.0";

// [Please CHANGE THIS] - API Rest server url of TempSensor (change the IP address!)
String tempSensorServer = "http://192.168.3.11/data";


// API Web server for accurate time.
String timeServer = "https://timeapi.io/api/time/current/zone?timeZone=Europe%2FParis";
ESP32Time rtc(0); 

unsigned long timePassed=0; // here we will store time passed

struct DateTime {
  int year;
  int month;
  int day;
  int hour;
  int minute;
  int second;
  long microsecond;
};


#endif 
 