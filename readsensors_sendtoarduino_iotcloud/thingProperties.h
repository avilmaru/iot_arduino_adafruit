#include <ArduinoIoTCloud.h>
#include <WiFiConnectionManager.h>

const char THING_ID[] = ""; 

const char SSID[]     = SECRET_SSID;    // Network SSID (name)
const char PASS[]     = SECRET_PASS;    // Network password (use for WPA, or use as key for WEP)

float HUMIDITY;
float TEMPERATURE;
float PRESSURE;
float LIGHT_INTENSITY;

void initProperties(){
  ArduinoCloud.setThingId(THING_ID);
  ArduinoCloud.addProperty(HUMIDITY, READ, 2 * SECONDS, NULL);
  ArduinoCloud.addProperty(TEMPERATURE, READ, 2 * SECONDS, NULL);
  ArduinoCloud.addProperty(PRESSURE, READ, 2 * SECONDS, NULL);
  ArduinoCloud.addProperty(LIGHT_INTENSITY, READ, 2 * SECONDS, NULL);
  
}

ConnectionManager *ArduinoIoTPreferredConnection = new WiFiConnectionManager(SSID, PASS);
