#include <Arduino.h>
#include "WeatherStation.h"
#include <arduino-timer.h>

auto timer = timer_create_default();

bool getTime(void *) {
  Serial.println("---> In getTime");
  return true;
} 

bool getSensor(void *) {
  Serial.println("---> In getSensor");
  return true;
} 

bool getForecast(void *) {
  Serial.println("---> In getForecast");
  return true;
}

void setup() {
  Serial.begin(115200);
  while (!Serial) {}; 
  Serial.print("WeatherStation");
  Serial.println(VW_Version);

  timer.every(15000,getTime);              // Every 15s
  timer.every(30000,getSensor);            // Every 30s
  timer.every(60000,getForecast);          // Every 60s
}

void loop() {
  timer.tick();
}
