#include <Arduino.h>
#include "WeatherStation.h"


// convert this format 2024-11-06T10:46:59.2012025 to varables 
DateTime parseISO8601(const String& iso8601) {
  DateTime dt;
  sscanf(iso8601.c_str(), "%4d-%2d-%2dT%2d:%2d:%2d.%7ld",
         &dt.year, &dt.month, &dt.day,
         &dt.hour, &dt.minute, &dt.second, &dt.microsecond);
  return dt;
}

void draw()
  {
    sprite.fillSprite(TFT_BLACK); 
    sprite.drawString(rtc.getTime(),60,60,7);
    sprite.pushSprite(0,0);
  }


void getTime()
{
    HTTPClient http;
    String payload; 
   
    http.begin(timeServer);
    int httpResponseCode = http.GET();

    if (httpResponseCode > 0) {
      payload = http.getString();
      Serial.println(httpResponseCode);
      Serial.println(payload);
    } else {
      Serial.print("Error on HTTP1 request: ");
      Serial.println(httpResponseCode);
    }
    http.end();

      JsonDocument doc;
      DeserializationError error = deserializeJson(doc, payload);
      if (!error) {
      const char* datetime = doc["dateTime"];
    
      DateTime dt = parseISO8601(String(datetime));  

  Serial.print("Year: "); Serial.println(dt.year);
  Serial.print("Month: "); Serial.println(dt.month);
  Serial.print("Day: "); Serial.println(dt.day);
  Serial.print("Hour: "); Serial.println(dt.hour);
  Serial.print("Minute: "); Serial.println(dt.minute);
  Serial.print("Second: "); Serial.println(dt.second);

  rtc.setTime(dt.second, dt.minute, dt.hour, dt.day, dt.month, dt.year); 
  }
}

void setup() {
  Serial.begin(115200);

  tft.init();
  tft.setRotation(1);
  tft.fillScreen(TFT_BLACK);
  sprite.createSprite(320,170);
  analogWrite(38,80);  //set brightness

   //connect board to wifi , if cant, esp32 will make wifi network, connect to that network with password "password"
    WiFiManager wifiManager;
    wifiManager.setConfigPortalTimeout(5000);

    IPAddress _ip,_gw,_mask;
    _ip.fromString(static_ip);
    _gw.fromString(static_gw);
    _mask.fromString(static_mask);

    wifiManager.setHostname(staHostname);

    wifiManager.setSTAStaticIPConfig(_ip,_gw,_mask);

    if(!wifiManager.autoConnect("DefineWifiConf","password")) {
        Serial.println("Failed to connect and hit timeout");
        delay(3000);
        ESP.restart();
    }
    Serial.println("Connected.");
  
  getTime();
}

void loop() {
  draw();
    if(millis()>timePassed+1000000)
  {
    getTime();
    timePassed=millis();
  }
}
