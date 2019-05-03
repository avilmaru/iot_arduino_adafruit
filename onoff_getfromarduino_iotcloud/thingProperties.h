#include <ArduinoIoTCloud.h>
#include <WiFiConnectionManager.h>

const char THING_ID[] = "";

const char SSID[]     = SECRET_SSID;    // Network SSID (name)
const char PASS[]     = SECRET_PASS;    // Network password (use for WPA, or use as key for WEP)

void onONOFFChange();

bool ON_OFF;


void initProperties(){
  
  ArduinoCloud.setThingId(THING_ID);
  ArduinoCloud.addProperty(ON_OFF, READWRITE, ON_CHANGE, onONOFFChange);
  
}

ConnectionManager *ArduinoIoTPreferredConnection = new WiFiConnectionManager(SSID, PASS);
