#include <Arduino.h>
#include "WeatherStation.h"
#include <arduino-timer.h>

//--- Sensor data var
sensorData  fromSensor;

//--- Time object
auto timer = timer_create_default();

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

  setupApi();
  getTime(NULL);

  //--- Tempo pour les tests hors maison
  // rtc.setTime(17, 0, 12, 26, 11, 2024);

  //--- Initialize display
  tft.init();
  tft.setRotation(1);
  tft.fillScreen(TFT_BLACK);
  sprite.createSprite(320,240);
  
  //--- Create Timers for main Weather Station functions
  timer.every(500,drawTime);               // Every 500ms, display time
  timer.every(15*60*1000,getTime);              // Every 15mn
  timer.every(30000,getSensor);            // Every 30s
  timer.every(60000,getForecast);          // Every 60s
}

void loop() {
  server.handleClient();
  timer.tick();
}

//--- getInternet Time From API server and set RTC time.
DateTime parseISO8601(const String& iso8601) {
  DateTime dt;
  sscanf(iso8601.c_str(), "%4d-%2d-%2dT%2d:%2d:%2d.%7ld",
         &dt.year, &dt.month, &dt.day,
         &dt.hour, &dt.minute, &dt.second, &dt.microsecond);
  return dt;
}

bool getTime(void *) {
  HTTPClient http;
  int httpResponseCode;
  JsonDocument jsonDoc;
  String payload;
  DeserializationError error;
  const char * datetime;
  DateTime dt;

  http.begin(timeServer);
  httpResponseCode = http.GET();
  if (httpResponseCode > 0){
    payload = http.getString();
    // Serial.println(httpResponseCode);
    // Serial.println(payload);
  } else {
    Serial.print("ERROR: bad HTTP1 request: ");
    Serial.println(httpResponseCode);
  }

  error = deserializeJson(jsonDoc,payload);
  if (!error) {
    datetime = jsonDoc["dateTime"];
    dt = parseISO8601(String(datetime));
    rtc.setTime(dt.second, dt.minute, dt.hour, dt.day, dt.month, dt.year);
  }
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

//--- setup and launch ReST API web server
void setupApi() {
  Serial.println("INFO: Web server started");
  server.on("/post-data", HTTP_POST, handlePost);

  // start Web server
  server.begin();
}

//--- handle POST request : store new values of sensor data to fromSensor structure.
//    Error handling is minimal : nothing is done if temp/humi/bat.level are non sense values!!
void handlePost() {
  JsonDocument jsonDoc;
  char buffer[128];
  String payload;

  if (server.hasArg("plain") == false) {
    jsonDoc.clear();
    jsonDoc["error"] = "true";
    jsonDoc["reason"] = "no body found.";
    serializeJson(jsonDoc,buffer);
    
    server.send(200,"application/json",buffer);
  } else {
    payload = server.arg("plain");
    deserializeJson(jsonDoc,payload);

    // store received data
    fromSensor.t = jsonDoc["temperature"];
    fromSensor.h = jsonDoc["humidity"];
    fromSensor.b = jsonDoc["batt_per"];
    fromSensor.is_update = true;

    server.send(200, "application/json", "{}");

  } 
}

//--- Drawing functions
bool drawTime(void *) {
  struct tm now;
  int dw;
  char tempo[20];
  TFT_eSprite spr = TFT_eSprite(&tft);

  spr.createSprite(320,50);
  spr.fillSprite(TFT_BLACK);
  now = rtc.getTimeStruct();
  dw = rtc.getDayofWeek();

  spr.loadFont(arialround14);
  spr.setTextDatum(MC_DATUM);
  
  sprintf(tempo,"%s %02d %s %4d",days[dw],now.tm_mday,months[now.tm_mon],(now.tm_year+1900));
  spr.drawString(tempo,160,10);
  sprintf(tempo,"%02d:%02d:%02d",now.tm_hour,now.tm_min,now.tm_sec);
  spr.loadFont(arialround36);
  spr.drawString(tempo,160,40);
  spr.pushSprite(0,0);

  return true;
}
