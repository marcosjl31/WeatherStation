Weather Station
===============

This is a weather station project that can display :

* Date and time
* weather forecast (current weather conditions, forecast min/max temperatures and rain probability).
* outside temperature and humidity percentage read by i
  [Temperature Sensor (2nd part of this project](https://github.com/marcosjl31/TempSensor)
 
The wheather station is based on a ESP32-Cheap-Yellow-Display. The CYD board connects to Internet using
WifiManager library and can connect to open-meteo.com to get weather data, and to timeapi.io to get
current date and time to set internal RTC of ESP32.

This repo contains the code used and the STL files of the case I used to house the project.

# Code

Code of the project is managed as a PlatformIO project. Communication between TempSensor and Weather
Station is done using a Web ReST API Server on the station listening to JSON formatted requests 
from TempSensor (every 20mn; this can be customized as well).

File WeatherStation.h need to be modified to reflect your location (name of the town, lat/lon coordinates
and Time Zone)
```
// [Please CHANGE THIS] - TimeZone/Town/Latitude/longitude
const String timeZone = "Europe%2FParis";
const String townName = "Toulouse";
const String townLat = "43.603951";
const String townLon = "1.444510";
```

I choose to force IP addresses of both Weather Station and TempSensor connected to my guest WiFi at home.
you can modify as well this configuration

```
// [Please CHANGE THIS] - Custom Static adress for the Weather Station
char static_ip[16] = "192.168.3.10";
char static_gw[16] = "192.168.3.1";
char static_mask[16] = "255.255.255.0";
char static_dns[16] = "192.168.3.1";
```
