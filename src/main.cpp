#include <Arduino.h>
#include "WeatherStation.h"
#include <arduino-timer.h>

//--- Sensor data var
sensorData  fromSensor;

//--- Time object
auto timer = timer_create_default();

bool getTime(void *) {
  Serial.println("---> In getTime");
  return true;
} 

bool getSensor(void *) {
  Serial.println("---> In getSensor");
  if (fromSensor.is_update) {
    // New values have been posted, display them
    Serial.print("Temp:"); Serial.println(fromSensor.t);
    Serial.print("Humi:"); Serial.println(fromSensor.h);
    Serial.print("Batt:"); Serial.println(fromSensor.b);
    fromSensor.is_update = false;
  }
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

  //--- WiFi connection using Captive_AP.
  wifiManager.setConfigPortalTimeout(5000);
  IPAddress _ip,_gw,_mask,_dns;
  _ip.fromString(static_ip);
  _gw.fromString(static_gw);
  _mask.fromString(static_mask);
  _dns.fromString(static_dns);

  wifiManager.setHostname(staHostname);
  wifiManager.setSTAStaticIPConfig(_ip,_gw,_mask,_dns);

  if(!wifiManager.autoConnect("AP_WifiConfig","password")) {
    Serial.println("Failed to connect and hit timeout");
    delay(3000);
    ESP.restart();
  }

  Serial.println("INFO: connected to WiFi");


  //--- Create Timers for main Weather Station functions
  timer.every(15000,getTime);              // Every 15s
  timer.every(30000,getSensor);            // Every 30s
  timer.every(60000,getForecast);          // Every 60s
}

void loop() {
  timer.tick();
}

//--- setup and launch ReST API web server
void setupApi() {
  server.on("/post-data", HTTP_POST, handlePost);

  // start Web server
  server.begin();
}

//--- handle POST request : store new values of sensor data to fromSensor structure.
//    Error handling is minimal : nothing is done if temp/humi/bat.level are non sense values!!
void handlePost() {
  JsonDocument jsonDoc;
  char buffer[128];
  String body;

  if (server.hasArg("plain") == false) {
    jsonDoc.clear();
    jsonDoc["error"] = "true";
    jsonDoc["reason"] = "no body found.";
    serializeJson(jsonDoc,buffer);
    
    server.send(200,"application/json",buffer);
  } else {
    body = server.arg("plain");
    deserializeJson(jsonDoc,body);

    // store received data
    fromSensor.t = jsonDoc["temperature"];
    fromSensor.h = jsonDoc["humidity"];
    fromSensor.b = jsonDoc["battery"];
    fromSensor.is_update = true;

    server.send(200, "application/json", "{}");

  } 
}
