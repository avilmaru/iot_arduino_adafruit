/*
 * 
  MKR ENV Shield - Read Sensors

  This example reads the sensors on-board the MKR ENV shield
  and send them to the Adafruit IOT Cloud

  The circuit:
  - Arduino MKR board
  - Arduino MKR ENV Shield attached

*/

#include <ArduinoJson.h>  
#include <avr/dtostrf.h>
#include <SPI.h>

////////////////////////////////
#include <Arduino_MKRENV.h>

////////////////////////////////
#include <Adafruit_GFX.h>     // Core graphics library
#include <Adafruit_ST7735.h>  // Hardware-specific library for ST7735

#define TFT_CS        3
#define TFT_RST       2 
#define TFT_DC        1

Adafruit_ST7735 tft = Adafruit_ST7735(TFT_CS, TFT_DC, TFT_RST);  

/////////////////////////
#include <WiFiNINA.h>
#include "arduino_secrets.h" 

char ssid[] = SECRET_SSID;         // your network SSID (name)
char pass[] = SECRET_PASS;         // your network password (use for WPA, or use as key for WEP)
int status = WL_IDLE_STATUS;
char server[] = "io.adafruit.com"; // name address for Adafruit IOT Cloud

///////////////////////////
float _temperature = 0;
float _humidity = 0;
float _pressure = 0;
float _lux = 0;

unsigned long lastConnectionTime = 0;              // last time you connected to the server, in milliseconds
const unsigned long postingInterval = 7000;       // delay between updates, in milliseconds

// Initialize the client library
WiFiClient client;

void setup() {
 
  Serial.begin(9600);
  //while (!Serial); // wait for serial port to connect. Needed for native USB port only

  // Init ST7735R chip, green tab
  tft.initR(INITR_144GREENTAB); 

  tft.fillScreen(ST77XX_BLACK);

  // Init MKR ENV shield
  if (!ENV.begin()) {
    Serial.println("Failed to initialize MKR ENV shield!");
    while (1);
  }
  
  connectToWIFI();
 
}

void loop() {

 // if 7 seconds have passed since your last connection,
  // then connect again and send data:
  if (millis() - lastConnectionTime > postingInterval) 
  {
    readSensors();
    displayValuesOnTFT();
    displayValuesOnSerial();
    httpRequest(); // send data to Cloud
  }
  
}


/* 
 * This method makes a HTTP connection to the server and post deread sensor values 
 * to the Adafruit IOT Cloud
 */

void httpRequest() 
{

  
/*
 * https://io.adafruit.com/api/docs/#operation/createGroupData
 * 
 * POST /{username}/groups/{group_key}/data
 * 
 * JSON:
 * 
{
  "location": {
    "lat": 0,
    "lon": 0,
    "ele": 0
  },
  "feeds": [
    {
      "key": "string",
      "value": "string"
    }
  ],
  "created_at": "string"
}
 */

  const size_t capacity = JSON_ARRAY_SIZE(3) + 3*JSON_OBJECT_SIZE(2) + 2*JSON_OBJECT_SIZE(3) + 130;
  StaticJsonDocument<capacity> doc;

   // Add the "location" object
  JsonObject location = doc.createNestedObject("location");
  location["lat"] = 0;
  location["lon"] = 0;
  location["ele"] = 0;
  
  // Add the "feeds" array
  JsonArray feeds = doc.createNestedArray("feeds");
  JsonObject feed1 = feeds.createNestedObject();
  feed1["key"] = "temperature";
  feed1["value"] = _temperature;
  JsonObject feed2 = feeds.createNestedObject();
  feed2["key"] = "humidity";
  feed2["value"] = _humidity;
  JsonObject feed3 = feeds.createNestedObject();
  feed3["key"] = "pressure";
  feed3["value"] = _pressure;
  JsonObject feed4 = feeds.createNestedObject();
  feed4["key"] = "light_intensity";
  feed4["value"] = _lux;

  
  // close any connection before send a new request.
  // This will free the socket on the Nina module
  client.stop();

  Serial.println("\nStarting connection to server...");
  if (client.connect(server, 80)) 
  {
    Serial.println("connected to server");
    // Make a HTTP request:
    client.println("POST /api/v2/" IO_USERNAME "/groups/" IO_GROUP "/data HTTP/1.1"); 
    client.println("Host: io.adafruit.com");  
    client.println("Connection: close");  
    client.print("Content-Length: ");  
    client.println(measureJson(doc));  
    client.println("Content-Type: application/json");  
    client.println("X-AIO-Key: " IO_KEY); 

    // Terminate headers with a blank line
    client.println();
    // Send JSON document in body
    serializeJson(doc, client);

    // note the time that the connection was made:
    lastConnectionTime = millis();

    testdrawtext(5,120,"data sent to Cloud!", ST77XX_WHITE,1);
    Serial.println("data sent!");
    
  } else {
    // if you couldn't make a connection:
    testdrawtext(10,120,"connection failed!", ST77XX_WHITE,1);
    Serial.println("connection failed!");
  }

  
}

void connectToWIFI()
{
   // check for the WiFi module:
  if (WiFi.status() == WL_NO_MODULE) {
    Serial.println("Communication with WiFi module failed!");
    // don't continue
    while (true);
  }

  String fv = WiFi.firmwareVersion();
  if (fv < "1.0.0") {
    Serial.println("Please upgrade the firmware");
  }

  // attempt to connect to Wifi network:
  while (status != WL_CONNECTED) {
    testdrawtext(0,0,"Attempting to connect to SSID:", ST77XX_WHITE,1);
    testdrawtext(0,20,ssid, ST77XX_WHITE,1);
    Serial.print("Attempting to connect to SSID: ");
    Serial.println(ssid);
    // Connect to WPA/WPA2 network. Change this line if using open or WEP network:
    status = WiFi.begin(ssid, pass);

    // wait 10 seconds for connection:
    delay(10000);
  }

  testdrawtext(0,40,"Connected to wifi", ST77XX_WHITE,1);
  Serial.println("Connected to wifi");
  delay(2000);
  printWifiStatus();
}


void printWifiStatus() {
  // print the SSID of the network you're attached to:
  Serial.print("SSID: ");
  Serial.println(WiFi.SSID());

  // print your board's IP address:
  IPAddress ip = WiFi.localIP();
  Serial.print("IP Address: ");
  Serial.println(ip);

  // print the received signal strength:
  long rssi = WiFi.RSSI();
  Serial.print("signal strength (RSSI):");
  Serial.print(rssi);
  Serial.println(" dBm");
}


void testdrawtext(int x, int y, char *text, uint16_t color, int size) {
  tft.setCursor(x, y);
  tft.setTextColor(color);
  tft.setTextWrap(true);
  tft.setTextSize(size);
  tft.print(text);
}

//Read sensors value: Temperature, Humidity, Pressure, Lux
void readSensors()
{
  _temperature = ENV.readTemperature();
  _humidity = ENV.readHumidity();
  _pressure = ENV.readPressure();
  _lux = ENV.readLux();
}

// Display values on TFT
void displayValuesOnTFT()
{

  // float to char conversion
  char temperature[6];
  char humidity[6];
  char pressure[7];
  char lux[8];
  
  dtostrf(_temperature, 5, 2, temperature);
  dtostrf(_humidity, 5, 2, humidity);
  dtostrf(_pressure, 5, 2, pressure);
  dtostrf(_lux, 6, 2, lux);
  
  // Display on TFT
  tft.fillScreen(ST77XX_BLACK);
  
  testdrawtext(0,5,"T:", ST77XX_RED,2);
  testdrawtext(25,5,temperature, ST77XX_RED,2);
  testdrawtext(110,10,"C", ST77XX_RED,1);
  
  testdrawtext(0,35,"H:", ST77XX_GREEN,2);
  testdrawtext(25,35,humidity, ST77XX_GREEN,2);
  testdrawtext(110,40,"%", ST77XX_GREEN,1);
  
  testdrawtext(0,65,"P:", ST77XX_YELLOW,2);
  testdrawtext(25,65,pressure, ST77XX_YELLOW,2);
  testdrawtext(110,70,"kPa", ST77XX_YELLOW,1);
  
  testdrawtext(0,95,"L:", ST77XX_WHITE,2);
  testdrawtext(25,95,lux, ST77XX_WHITE,2);
  testdrawtext(110,100,"Lux", ST77XX_WHITE,1);
 
}

// Display values on Serial Port
void displayValuesOnSerial()
{
  Serial.println();
    
  Serial.print("Temperature = ");
  Serial.print(_temperature);
  Serial.println(" °C");

  Serial.print("Humidity    = ");
  Serial.print(_humidity);
  Serial.println(" %");

  Serial.print("Pressure    = ");
  Serial.print(_pressure);
  Serial.println(" kPa");

  Serial.print("Lux     = ");
  Serial.println(_lux);

  Serial.println();

}
