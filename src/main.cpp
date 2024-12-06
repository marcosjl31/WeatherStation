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

  //--- Initialize display
  tft.init();
  tft.setRotation(1);
  tft.fillScreen(WS_BLACK);

  //--- Tempo pour les tests hors maison
  // rtc.setTime(17, 0, 12, 26, 11, 2024);

  fromSensor.t = 11.1; // set dummy values for first time display
  fromSensor.h = 22.2;
  fromSensor.b = 74.4;
  fromSensor.is_update = true;
  //--------------------------------------

  drawTime(NULL);
  getSensor(NULL);
  getForecast(NULL);
  
  //--- Create Timers for main Weather Station functions
  timer.every(500,drawTime);               // Every 500ms, display time
  timer.every(15*60*1000,getTime);         // Every 15mn
  timer.every(5*60*1000,getSensor);        // Every 5mn
  timer.every(10*60*1000,getForecast);     // Every 10mn
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
    // Serial.println(httpResponseCode);      // for debug purpose
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
  if (fromSensor.is_update) {
    fromSensor.is_update = false;

    drawSensor(fromSensor.t,fromSensor.h,fromSensor.b);
  }
  return true;
} 

bool getForecast(void *) {
  HTTPClient http;
  int httpResponseCode;
  JsonDocument jsonDoc;
  String payload;
  DeserializationError error;
  float minT, maxT;
  int rainProba;
  int wmoCode;

  // get forecast
  http.begin(weatherServer);
  httpResponseCode = http.GET();
  if (httpResponseCode > 0){
    payload = http.getString();
    // Serial.println(httpResponseCode);      // for debug purpose
    // Serial.println(payload);
  } else{
    Serial.print("ERROR: bad HTTP1 request: ");
    Serial.println(httpResponseCode);
  }

  error = deserializeJson(jsonDoc,payload);
  if (!error) {
    wmoCode = jsonDoc["current"]["weather_code"];
    minT = jsonDoc["daily"]["temperature_2m_min"][0];
    maxT = jsonDoc["daily"]["temperature_2m_max"][0];
    rainProba = jsonDoc["daily"]["precipitation_probability_max"][0];

    // display forecast 
    drawForecast(wmoCode,minT,maxT,rainProba);
  }

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

//--- return PROGMEM iconname from wmo code
const uint16_t* getIconFromWMO(int wmo) {
  if (wmo == 0) return sunny;
  if (wmo == 1) return mainlysunny;
  if (wmo == 2) return partlycloudy;
  if (wmo == 3) return cloudy;
  if (wmo == 45 || wmo == 48) return fog;
  if (wmo == 51 || wmo == 61 || wmo == 80 || wmo == 83) return lightrain;
  if (wmo == 53 || wmo == 55 || wmo == 63 || wmo == 65) return rain;
  if (wmo == 56 || wmo == 57 || wmo == 66 || wmo == 67) return freezingrain;
  if (wmo == 71) return lightsnow;
  if (wmo == 73 || wmo == 75 || wmo == 77) return snow;
  if (wmo == 85 || wmo == 86) return snow;
  if (wmo >= 95 && wmo <= 99) return storms;
  return unknown;
}

// return Short Weather Description from WMO code
String getDescriptionFromWMO(int wmo) {
  if (wmo == 0) return "Ensoleillé";
  if (wmo == 1) return "Plutôt ensoleillé";
  if (wmo == 2) return "Plutôt couvert";
  if (wmo == 3) return "Nuageux";
  if (wmo == 45 || wmo == 48) return "Brouillard";
  if (wmo == 51 || wmo == 61 || wmo == 80 || wmo == 83) return "Légères averses";
  if (wmo == 53 || wmo == 55 || wmo == 63 || wmo == 65) return "Pluie";
  if (wmo == 56 || wmo == 57 || wmo == 66 || wmo == 67) return "Pluie verglaçante";
  if (wmo == 71) return "Averses de neige";
  if (wmo == 73 || wmo == 75 || wmo == 77) return "Neige";
  if (wmo == 85 || wmo == 86) return "Neige";
  if (wmo >= 95 && wmo <= 99) return "orages";
  return "";
}

//--- Drawing functions
bool drawTime(void *) {
  struct tm now;
  int dw;
  char tempo[20];

  sprite.createSprite(320,50);
  sprite.fillSprite(WS_BLACK);
  sprite.setTextColor(WS_WHITE);
  now = rtc.getTimeStruct();
  dw = rtc.getDayofWeek();

  sprite.loadFont(arialround14);
  sprite.setTextDatum(MC_DATUM);
  
  sprintf(tempo,"%s %02d %s %4d",days[dw],now.tm_mday,months[now.tm_mon],(now.tm_year+1900));
  sprite.drawString(tempo,160,10);
  sprintf(tempo,"%02d:%02d:%02d",now.tm_hour,now.tm_min,now.tm_sec);
  sprite.loadFont(arialround36);
  sprite.drawString(tempo,160,40);
  sprite.pushSprite(0,0);

  sprite.deleteSprite();

  return true;
}

void drawSensor(float t, float h, float b) {
  char tempo[10];

  sprite.createSprite(170,140);
  sprite.fillSprite(WS_BLACK);
  sprite.setTextColor(WS_BLUE);

  sprite.loadFont(arialround14);
  sprite.setTextDatum(CR_DATUM);
  sprite.drawString(townName,160,20);


  // display Temp & Humi
  sprite.loadFont(arialround36);
  sprite.setTextColor(WS_WHITE);
  sprintf(tempo,"%2d °C",int(fromSensor.t+0.5));
  sprite.setTextDatum(MC_DATUM);
  sprite.drawString(tempo,55,75);
  sprite.loadFont(arialround14);
  sprintf(tempo,"%2d %%",int(fromSensor.h+0.5));
  sprite.drawString(tempo,55,110);

  // display batt level
  drawBatLevel(sprite,120,60,int(fromSensor.b+0.5));

  sprite.pushSprite(150,50);
  sprite.deleteSprite();
}

void drawBatLevel(TFT_eSprite &spr,int sprX,int sprY,int level) {
  spr.drawFastVLine(sprX,sprY,58,WS_WHITE);
  spr.drawFastVLine(sprX+30,sprY,58,WS_WHITE);
  spr.drawFastVLine(sprX+10,sprY-4,4,WS_WHITE);
  spr.drawFastVLine(sprX+20,sprY-4,4,WS_WHITE);
  spr.drawFastHLine(sprX,sprY,10,WS_WHITE);
  spr.drawFastHLine(sprX+10,sprY-4,10,WS_WHITE);
  spr.drawFastHLine(sprX+20,sprY,10,WS_WHITE);
  spr.drawFastHLine(sprX,sprY+58,30,WS_WHITE);

  if (level > 85) 
    spr.fillRect(sprX+2,sprY+2,27,10,TFT_GREEN);
  if (level > 65)
    spr.fillRect(sprX+2,sprY+13,27,10,TFT_GREEN);
  if (level > 45) 
    spr.fillRect(sprX+2,sprY+24,27,10,TFT_GREEN);
  if (level > 25)
    spr.fillRect(sprX+2,sprY+35,27,10,TFT_ORANGE);
  spr.fillRect(sprX+2,sprY+46,27,10,TFT_RED);
}

void drawForecast(int wmo, float minT, float maxT, short rainProba) {
  char tempo[10];

  sprite.createSprite(150,150);   // icon sprite
  sprite.setSwapBytes(true);
  sprite.fillSprite(WS_BLACK);
  // Cf. exemple : Sprite_image_4bit de la lib TFT
  
  sprite.pushImage(15,15,128,128,getIconFromWMO(wmo));
  
  sprite.pushSprite(0,45);
  sprite.deleteSprite();

  sprite.createSprite(320,55);    // text sprite (bottom display for weather condition, min/max temp and rain %)
  sprite.fillSprite(WS_BLACK);
  sprite.setTextColor(WS_YELLOW);
  sprite.loadFont(arialround36);
  sprite.setTextDatum(CR_DATUM);
  sprite.drawString(getDescriptionFromWMO(wmo),315,18);
  sprite.setTextColor(WS_BLUE);
  sprite.loadFont(arialround14);
  sprintf(tempo,"Pluie %2d %%",rainProba);
  sprite.drawString(tempo,315,45);
  sprintf(tempo,"Min %4.1f °C",minT);
  sprite.setTextDatum(CL_DATUM);
  sprite.drawString(tempo,5,45);
  sprintf(tempo,"Max %4.1f °C",maxT);
  sprite.setTextDatum(MC_DATUM);
  sprite.drawString(tempo,160,45);

  sprite.pushSprite(0,185);
  sprite.deleteSprite();
}
