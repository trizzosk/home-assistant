/*
  Template, needs to adjust following constans or variables:
  - BRIGHTNESS
  - ssid
  - password

  Check your hassio MQTT parameters and adjust:
  - MQTT_CLIENT_ID
  - MQTT_SERVER_IP
  - MQTT_SERVER_PORT
  - MQTT_USER
  - MQTT_PASSWORD
  - MQTT_SENSOR_TOPIC

  In case of detailed logging to Serial, change this to 1, otherwise 0:
  - DEBUG

  When temperature from the box is not accurate, try to measure it with proper device and change this constant:
  - TEMP_CONST
*/
#include <PubSubClient.h>

#include <ArduinoJson.h>
#include <ArduinoJson.hpp>

#include <WiFi.h>
#include <WiFiAP.h>
#include <WiFiClient.h>
#include <WiFiGeneric.h>
#include <WiFiMulti.h>
#include <WiFiSTA.h>
#include <WiFiScan.h>
#include <WiFiServer.h>
#include <WiFiType.h>
#include <WiFiUdp.h>

#include "pm1006.h"
#include <Adafruit_NeoPixel.h>
#include <SensirionI2CScd4x.h>
#include <Wire.h>

#define PIN_FAN 12
#define PIN_LED 25
#define RXD2 16
#define TXD2 17

#define BRIGHTNESS 1

#define PM_LED 1
#define TEMP_LED 2
#define CO2_LED 3

// Wifi setup
const char* ssid = "__WIFI_NAME_HERE__";
const char* password = "__YOUR_SSID_PASSWORD__";

static PM1006 pm1006(&Serial2);
Adafruit_NeoPixel rgbWS = Adafruit_NeoPixel(3, PIN_LED, NEO_GRB + NEO_KHZ800);
SensirionI2CScd4x scd4x;

// MQTT definition
#define MQTT_VERSION MQTT_VERSION_3_1_1

// MQTT: ID, server IP, port, username and password
// TO-DO: adjust to my hassio instance
const PROGMEM char* MQTT_CLIENT_ID = "__CLIENT_NAME__";
const PROGMEM char* MQTT_SERVER_IP = "256.256.256.256";
const PROGMEM uint16_t MQTT_SERVER_PORT = 65537;
const PROGMEM char* MQTT_USER = "__MY_USER___";
const PROGMEM char* MQTT_PASSWORD = "__MY_SUPER_PASSWORD__";

// MQTT: topic
const PROGMEM char* MQTT_SENSOR_TOPIC = "__topic__/__name__";

WiFiClient wifiClient;

PubSubClient client(wifiClient);

// DEBUG switch
//   0 = debug off
//   1 = debug on
int DEBUG = 0;

// For adjusting measured temperature (sometimes when senzor is placed nearer to ESP32 processor)
float TEMP_CONST = 6.0;

void setup() 
{
  pinMode(PIN_FAN, OUTPUT); // Fan

  rgbWS.begin(); // WS2718
  
  rgbWS.setBrightness(BRIGHTNESS);
  
  Serial.begin(115200);
  
  Serial2.begin(9600, SERIAL_8N1, RXD2, TXD2);

  Wire.begin();
  
  uint16_t error;

  char errorMessage[256];
  
  scd4x.begin(Wire);
  
  Serial.println("Start...");
  
  delay(500);
  
  // Practically turn off LEDs
  setColorWS(0, 0, 0, 1);

  setColorWS(0, 0, 0, 2);
  
  setColorWS(0, 0, 0, 3);

  // stop potentially previously started measurement
  error = scd4x.stopPeriodicMeasurement();
  if (error)
  {
    if (DEBUG == 1)
    {
      Serial.print("SCD41 Error trying to execute stopPeriodicMeasurement(): ");

      errorToString(error, errorMessage, 256);

      Serial.println(errorMessage);
    }

    alert(CO2_LED);
  }

  uint16_t serial0;

  uint16_t serial1;
  
  uint16_t serial2;

  error = scd4x.getSerialNumber(serial0, serial1, serial2);
  
  if (error)
  {
    if (DEBUG == 1)
    {
      Serial.print("SCD41 Error trying to execute getSerialNumber(): ");

      errorToString(error, errorMessage, 256);
      
      Serial.println(errorMessage);
    }
      
    alert(CO2_LED);
  }
  
  else 
  {
    if (DEBUG == 1)
    {
      printSerialNumber(serial0, serial1, serial2);
    }
  }

  // Start Measurement
  error = scd4x.startPeriodicMeasurement();
  if (error) 
  {
    if (DEBUG == 1)
    {
      Serial.print("SCD41 Error trying to execute startPeriodicMeasurement(): ");

      errorToString(error, errorMessage, 256);
      
      Serial.println(errorMessage);
    }
      
    alert(CO2_LED);
  }

  /* Setup a Wifi connection */
  // this needs to be done because of troubles after 100 rebbots from deep sleep mode
  WiFi.disconnect();

  WiFi.mode(WIFI_STA);

  WiFi.begin();
  
  delay(500);
  
  Serial.println("Connecting to WiFi...");

  WiFi.setSleep(false);
  
  WiFi.setAutoReconnect(true);

  WiFi.persistent(true);
  
  WiFi.begin(ssid, password);
  
  delay(500);
  
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
  
    Serial.print(".");
  }

  if (DEBUG == 1)
  {
    Serial.println();

    Serial.print("Connected to WiFi network, IP adress of the device: ");

    Serial.println(WiFi.localIP());

    Serial.println();

    Serial.println("Waiting for first measurement... (5 sec)");
  }
}

void loop() 
{
  uint16_t error;

  char errorMessage[256];

  /*
    LOW - based on my measurements, HIGH don't needs to be set here for measurement - difference is irrelevant
  */
  digitalWrite(PIN_FAN, LOW);

  if (DEBUG == 1)
  {
    Serial.println("Fan ON");
  }
  
  delay(10000);

  // Define measurement variables
  uint16_t pm2_5;

  uint16_t co2;
  
  float temperature;
  
  float humidity;
  
  if (pm1006.read_pm25(&pm2_5))
  {
    if (DEBUG == 1)
    {
      printf("PM2.5 = %u\n", pm2_5);
    }
  }

  else
  {
    if (DEBUG == 1)
    {
      Serial.println("Measurement failed!");
    }
    
    alert(PM_LED);
  }

  delay(1000);

  digitalWrite(PIN_FAN, LOW);

  if (DEBUG == 1)
  {
    Serial.println("Fan OFF");
  }

  error = scd4x.readMeasurement(co2, temperature, humidity);

  if (error)
  {
    if (DEBUG == 1)
    {
      Serial.print("SCD41 Error trying to execute readMeasurement(): ");

      errorToString(error, errorMessage, 256);

      Serial.println(errorMessage);
    }
    
    alert(CO2_LED);
  } 

  else if (co2 == 0) 
  {
    if (DEBUG == 1)
    {
      Serial.println("Invalid sample detected, skipping.");
    }
  } 
  
  else 
  {
    if (DEBUG == 1)
    {
      Serial.print("Co2:");

      Serial.print(co2);
      
      Serial.print("\t");
      
      Serial.print(" Temperature:");
      
      Serial.print(temperature);
      
      Serial.print("\t");
      
      Serial.print(" Humidity:");
      
      Serial.println(humidity);
    }

    // Uncomment following session to use LEDs as status indicators     
    /*
    if(co2 < 1000)
    {
      setColorWS(0, 255, 0, CO2_LED);
    }
    
    if((co2 >= 1000) && (co2 < 1200))
    {
      setColorWS(128, 255, 0, CO2_LED);
    }
    
    if((co2 >= 1200) && (co2 < 1500))
    {
    setColorWS(255, 255, 0, CO2_LED);
    }
    
    if((co2 >= 1500) && (co2 < 2000))
    {
      setColorWS(255, 128, 0, CO2_LED);
    }
    
    if(co2 >= 2000)
    {
      setColorWS(255, 0, 0, CO2_LED);
    }

    if(temperature < 23.0)
    {
      setColorWS(0, 0, 255, TEMP_LED);
    }

    if((temperature >= 23.0) && (temperature < 28.0))
    {
      setColorWS(0, 255, 0, TEMP_LED);
    }

    if(temperature >= 28.0)
    {
      setColorWS(255, 0, 0, TEMP_LED);
    }
    */
  }

  // PM LED
  // Uncomment following section to use LEDs as status indicators
  /*
  if(pm2_5 < 30)
  {
    setColorWS(0, 255, 0, PM_LED);
  }
  
  if((pm2_5 >= 30) && (pm2_5 < 40))
  {
    setColorWS(128, 255, 0, PM_LED);
  }
  
  if((pm2_5 >= 40) && (pm2_5 < 80))
  {
  setColorWS(255, 255, 0, PM_LED);
  }
  
  if((pm2_5 >= 80) && (pm2_5 < 90))
  {
    setColorWS(255, 128, 0, PM_LED);
  }
  
  if(pm2_5 >= 90)
  {
    setColorWS(255, 0, 0, PM_LED);
  }
  */

  if(WiFi.status() == WL_CONNECTED)
  {
    if (!client.connected())
    {
      reconnect();

      temperature = temperature - TEMP_CONST;

      publishData(temperature, humidity, co2, pm2_5);

      delay (1000);

      client.loop();

      client.disconnect();
    }

    else
    {
      temperature = temperature - TEMP_CONST;

      publishData(temperature, humidity, co2, pm2_5);

      delay(1000);

      client.loop();

      client.disconnect();
    }
  }
  
  else 
  {
    if (DEBUG == 1)
    {
      Serial.println("Wi-Fi disconnected, reconnecting...");

      alert(PM_LED);
    }

    WiFi.reconnect();

    delay(500);

    if(WiFi.status() == WL_CONNECTED)
    {
      if (!client.connected())
      {
        reconnect();

        temperature = temperature - TEMP_CONST;

        publishData(temperature, humidity, co2, pm2_5);

        delay (1000);

        client.loop();

        client.disconnect();
      }
    }

    else
    {
      WiFi.begin();

      delay(500);

      WiFi.begin(ssid, password);

      delay(500);

      if(WiFi.status() == WL_CONNECTED)
      {
        if (!client.connected())
        {
          reconnect();

          temperature = temperature - TEMP_CONST;

          publishData(temperature, humidity, co2, pm2_5);

          delay (1000);

          client.loop();

          client.disconnect();
        }
      }
    }
  }

  WiFi.disconnect();
  
  // sleep for 5 minutes = 600 seconds to micro-seconds
  // 5 mins * 60 secs = 300 * 1.000.000
  esp_sleep_enable_timer_wakeup(300 * 1000000); 

  Serial2.flush();
  
  Serial.flush(); 
  
  delay(500);
  
  esp_deep_sleep_start();
}

void alert(int id)
{
  int i = 0;

  while (1)
  {
    if (i > 10)
    {
      Serial.println("Maybe need Reboot...");

      break;
    }

    rgbWS.setBrightness(255);

    setColorWS(255, 0, 0, id); 

    delay(200);

    rgbWS.setBrightness(BRIGHTNESS);

    setColorWS(0, 0, 0, id);

    delay(200);

    i++;
  }
}

void setColorWS(byte r, byte g, byte b, int id)
{
  uint32_t rgb;  
 
  rgb = rgbWS.Color(r, g, b); 
 
  rgbWS.setPixelColor(id - 1, rgb);
 
  rgbWS.show();
}

void printUint16Hex(uint16_t value)
{
  Serial.print(value < 4096 ? "0" : "");

  Serial.print(value < 256 ? "0" : "");

  Serial.print(value < 16 ? "0" : "");

  Serial.print(value, HEX);
}

void printSerialNumber(uint16_t serial0, uint16_t serial1, uint16_t serial2)
{
  Serial.print("SCD41 Serial: 0x");

  printUint16Hex(serial0);

  printUint16Hex(serial1);

  printUint16Hex(serial2);

  Serial.println();
}

void publishData(float p_temperature, float p_humidity, uint16_t co2, uint16_t pm2_5)
{
  StaticJsonDocument<256> root;
  
  root["temperature"] = p_temperature;
  
  root["humidity"] = p_humidity;

  root["co2"] = co2;

  root["pm2_5"] = pm2_5;

  char data[256];
  
  serializeJson(root, data);

  if (DEBUG == 1)
  {
    Serial.println();

    Serial.print(data);

    Serial.println();
  }

  client.publish(MQTT_SENSOR_TOPIC, data, false);

  delay (200);

  yield();
}

void reconnect()
{
  // Loop until we're reconnected
  while (!client.connected())
  {
    if (DEBUG == 1)
    {
      Serial.println("INFO: Attempting MQTT connection...");
    }
    
    client.setServer(MQTT_SERVER_IP, MQTT_SERVER_PORT);

    // Attempt to connect
    if (client.connect(MQTT_CLIENT_ID, MQTT_USER, MQTT_PASSWORD))
    {
      if (DEBUG == 1)
      {
        Serial.println("INFO: connected");
      }
    } 

    else
    {
      if (DEBUG == 1)
      {
        Serial.print("ERROR: failed, rc=");

        Serial.print(client.state());

        Serial.println("DEBUG: try again in 5 seconds");
      }

      // Wait 10 seconds before retrying
      delay(10000);
    }
  }
}