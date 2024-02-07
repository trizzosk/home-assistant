#include <ArduinoJson.h>
#include <ArduinoJson.hpp>

#ifndef BOARD_HAS_PSRAM
#error "Please enable PSRAM !!!"
#endif

#if CONFIG_IDF_TARGET_ESP32S3
#include "pcf8563.h"
#include <Wire.h>
#endif

#if CONFIG_IDF_TARGET_ESP32
#define BATT_PIN            36
#elif CONFIG_IDF_TARGET_ESP32S3
#define BATT_PIN            14
#else
#error "Platform not supported"
#endif

#if defined(CONFIG_IDF_TARGET_ESP32)
#define SD_MISO             12
#define SD_MOSI             13
#define SD_SCLK             14
#define SD_CS               15
#elif defined(CONFIG_IDF_TARGET_ESP32S3)
#define SD_MISO             16
#define SD_MOSI             15
#define SD_SCLK             11
#define SD_CS               42
#else
#error "Platform not supported"
#endif

#include <NTP.h>

#include <PubSubClient.h>
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
#include <WiFiClientSecure.h>

#include "font/monterey.h"
#include "font/monterey10.h"

#include <Arduino_JSON.h>

#include <HTTPClient.h>

#include <driver/adc.h>
#include "esp_adc_cal.h"

// Include picture files
#include "sofa.h"
#include "cat.h"
#include "bed-king.h"
#include "chair-rolling.h"
#include "timer.h"
#include "fuse.h"
#include "battery-unknown.h"
#include "weather-cloudy.h"
#include "molecule-co2.h"
#include "cake-variant.h"

#define SCREEN_WIDTH EPD_WIDTH
#define SCREEN_HEIGHT EPD_HEIGHT

#define White 0xFF
#define LightGrey 0xBB
#define Grey 0x88
#define DarkGrey 0x44
#define Black 0x00

#define BATT_PIN   (14)
#define MIN_USB_VOL 4.0

#define MQTT_VERSION MQTT_VERSION_3_1_1

WiFiClient wifiClient;
WiFiClientSecure wifiClientSecure;
PubSubClient client(wifiClient);

WiFiUDP wifiUdp;
NTP ntp(wifiUdp);

enum alignment { LEFT,
                 RIGHT,
                 CENTER };

int vref = 1100;

uint8_t *framebuffer;

GFXfont currentFont;

// Wifi setup
const char *ssid = "WIFI_SSID";
const char *password = "WIFI_PASSWORD";

/* 
  API endpoints
*/

//ZOE
const char *req_temp_zoe = "http://__IP__:8123/api/states/sensor.hb_zoeroom_temperature";
const char *req_hum_zoe = "http://__IP__:8123/api/states/sensor.hb_zoeroom_humidity";

//OFFICE
const char *req_temp_office = "http://__IP__:8123/api/states/sensor.hb_office_space_temperature";
const char *req_hum_office = "http://__IP__:8123/api/states/sensor.hb_office_space_humidity";

//BEDROOM
const char *req_temp_bedroom = "http://__IP__:8123/api/states/sensor.sonoff_a4800539db_temperature";
const char *req_hum_bedroom = "http://__IP__:8123/api/states/sensor.sonoff_a4800539db_humidity";

//LIVING ROOM
const char *req_temp_living = "http://__IP__:8123/api/states/sensor.living_room_temperature";
const char *req_hum_living = "http://__IP__:8123/api/states/sensor.living_room_humidity";
const char *req_co2_living = "http://__IP__:8123/api/states/sensor.living_room_co2";

//WEATHER
const char *req_temp_weather = "http://__IP__:8123/api/states/weather.home_2";

const char *req_nameday = "http://__IP__:8123/api/states/sensor.meniny_dnes";
const char *req_nameday_tomorrow = "http://__IP__:8123/api/states/sensor.meniny_zajtra";

// MQTT definition for battery status reporting

const PROGMEM char* MQTT_CLIENT_ID = "obyvacka_eink_display";
const PROGMEM char* MQTT_SERVER_IP = "__IP__";
const PROGMEM uint16_t MQTT_SERVER_PORT = 1883;
const PROGMEM char* MQTT_USER = "mqtt_user";
const PROGMEM char* MQTT_PASSWORD = "mqtt_password";
const PROGMEM char* MQTT_SENSOR_TOPIC = "obyvacka_eink_display/battery";
const PROGMEM char* MQTT_SENSOR_TOPIC1 = "obyvacka_eink_display/last_update";

/*
  DEBUG switch
    0 = debug off
    1 = debug on
*/
int DEBUG = 0;

void setup() 
{
  char buf[128];

  Serial.begin(115200);

  delay(1000);

  // put your setup code here, to run once:
  framebuffer = (uint8_t *)ps_calloc(sizeof(uint8_t), EPD_WIDTH * EPD_HEIGHT / 2);

  if (!framebuffer && DEBUG == 1) 
  {
    Serial.println("Memory alloc failed!");
  }

  memset(framebuffer, 0xFF, EPD_WIDTH * EPD_HEIGHT / 2);

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
  }

  //initialize display
  epd_init();
  epd_poweron();
  epd_clear();

  esp_adc_cal_characteristics_t adc_chars;
  
  esp_adc_cal_value_t val_type = esp_adc_cal_characterize((adc_unit_t)ADC_UNIT_2, (adc_atten_t)ADC_ATTEN_DB_11, (adc_bits_width_t)ADC_WIDTH_BIT_12, 1100, &adc_chars); 
  
  pinMode(14, OUTPUT);
  
  delay(1000);  // necessary to leave display to full redraw

  epd_poweroff_all();

  ntp.ruleDST("CEST", Last, Sun, Mar, 2, 120);  // last sunday in march 2:00, timetone +120min (+1 GMT + 1h summertime offset)

  ntp.ruleSTD("CET", Last, Sun, Oct, 3, 60);  // last sunday in october 3:00, timezone +60min (+1 GMT)

  ntp.begin("NTP_SERVER_IP");
}

void loop() 
{
  // put your main code here, to run repeatedly:
  epd_poweron();

  delay(100);

  epd_clear();

  delay(500);

  GetAndDisplayStatuses();

  DisplayBatteryStatus();

  DisplayWeather ();

  DisplayNameDay();

  DisplayNameDayTomorrow ();

  // poweroff display!!!!
  delay(1500);

  epd_poweroff_all();

  int deepSleepMinutes = 900;

  // finally, deepsleep
  WiFi.disconnect();

  Serial.println("Reaching deepsleep...");

  // If in DEBUG mode, just wait for some time, stay awake and restart
  if (DEBUG == 1)
  {
    delay(6000);
  }

  else
  {
    // Not in DEBUG mode so enter deepsleep for certain period based on deepSLeppMinutes value
    esp_sleep_enable_timer_wakeup(deepSleepMinutes * 1000000);

    esp_deep_sleep_start();
  }
}

void GetAndDisplayStatuses()
{
  String temp_zoe, temp_living, temp_bedroom, temp_office, hum_zoe, hum_living, hum_bedroom, hum_office, co2_living, sZoeTemp, sBedroomTemp, sLivingTemp, sOfficeTemp, sZoeHum, sBedroomHum, sLivingHum, sOfficeHum, sLivingCO2;
  String mTempZoe, mTempBedroom, mTempLiving, mTempOffice, mHumZoe, mHumBedroom, mHumLiving, mHumOffice, mCO2Living;

  temp_zoe = httpGETRequest(req_temp_zoe);
  temp_bedroom = httpGETRequest(req_temp_bedroom);
  temp_living = httpGETRequest(req_temp_living);
  temp_office = httpGETRequest(req_temp_office);

  hum_zoe = httpGETRequest(req_hum_zoe);
  hum_bedroom = httpGETRequest(req_hum_bedroom);
  hum_living = httpGETRequest(req_hum_living);
  hum_office = httpGETRequest(req_hum_office);
  co2_living = httpGETRequest(req_co2_living);

  // parse json

  JSONVar js_temp_zoe = JSON.parse(temp_zoe);
  JSONVar js_temp_bedroom = JSON.parse(temp_bedroom);
  JSONVar js_temp_living = JSON.parse(temp_living);
  JSONVar js_temp_office = JSON.parse(temp_office);
  JSONVar js_hum_zoe = JSON.parse(hum_zoe);
  JSONVar js_hum_bedroom = JSON.parse(hum_bedroom);
  JSONVar js_hum_living = JSON.parse(hum_living);
  JSONVar js_hum_office = JSON.parse(hum_office);
  JSONVar js_co2_living = JSON.parse(co2_living);

  if (DEBUG == 1 && JSON.typeof(js_temp_zoe) == "undefined")
  {
    Serial.println("temp_zoe: Parsing JSON input failed!");
  }

  else
  {
    if (js_temp_zoe.hasOwnProperty("state"))
    {
      if (JSON.stringify(js_temp_zoe["state"]) == "\"unavailable\"")
      {
        sZoeTemp = "-- °C";
      }

      else
      {
        sZoeTemp = JSON.stringify(js_temp_zoe["state"]) + "°C";

        sZoeTemp.replace("\"", "");
      }
    }
  }

  if (DEBUG == 1 && JSON.typeof_(js_temp_bedroom) == "undefined")
  {
    Serial.println("temp_bedroom: Parsing JSON input failed!");
  }

  else
  {
    if (js_temp_bedroom.hasOwnProperty("state"))
    {
      if (JSON.stringify(js_temp_bedroom["state"]) == "\"unavailable\"")
      {
        sBedroomTemp = "-- °C";
      }

      else
      {
        sBedroomTemp = JSON.stringify(js_temp_bedroom["state"]) + "°C";

        sBedroomTemp.replace("\"", "");
      }
    }
  }

  if (DEBUG == 1 && JSON.typeof_(js_temp_living) == "undefined")
  {
    Serial.println("temp_living: Parsing JSON input failed!");
  }

  else
  {
    if (js_temp_living.hasOwnProperty("state"))
    {
      if (JSON.stringify(js_temp_living["state"]) == "\"unavailable\"")
      {
        sLivingTemp = "-- °C";
      }

      else
      {
        sLivingTemp = JSON.stringify(js_temp_living["state"]);

        sLivingTemp.replace("\"", "");

        sLivingTemp = sLivingTemp.substring(0,5) + "°C";
      }
    }
  }

  if (DEBUG == 1 && JSON.typeof_(js_temp_office) == "undefined")
  {
    Serial.println("temp_office: Parsing JSON input failed!");
  }

  else
  {
    if (js_temp_office.hasOwnProperty("state"))
    {
      if (JSON.stringify(js_temp_office["state"]) == "\"unavailable\"")
      {
        sOfficeTemp = "-- °C";
      }

      else
      {
        sOfficeTemp = JSON.stringify(js_temp_office["state"]) + "°C";

        sOfficeTemp.replace("\"", "");
      }
    }
  }

  if (DEBUG == 1 && JSON.typeof_(js_hum_zoe) == "undefined")
  {
    Serial.println("hum_zoe: Parsing JSON input failed!");
  }

  else
  {
    if (js_hum_zoe.hasOwnProperty("state"))
    {
      if (JSON.stringify(js_hum_zoe["state"]) == "\"unavailable\"")
      {
        sZoeHum = "-- %";
      }

      else
      {
        sZoeHum = JSON.stringify(js_hum_zoe["state"]) + "%";

        sZoeHum.replace("\"", "");
      }
    }
  }

  if (DEBUG == 1 && JSON.typeof_(js_hum_bedroom) == "undefined")
  {
    Serial.println("hum_bedroom: Parsing JSON inout failed!");
  }

  else
  {
    if (js_hum_bedroom.hasOwnProperty("state"))
    {
      if (JSON.stringify(js_hum_bedroom["state"]) == "\"unavailable\"")
      {
        sBedroomHum = "-- %";
      }

      else
      {
        sBedroomHum = JSON.stringify(js_hum_bedroom["state"]) + "%";

        sBedroomHum.replace("\"", "");
      }
    }
  }

  if (DEBUG == 1 && JSON.typeof_(js_hum_living) == "undefined")
  {
    Serial.println("hum_living: Parsing JSON input failed!");
  }

  else
  {
    if (js_hum_living.hasOwnProperty("state"))
    {
      if (JSON.stringify(js_hum_living["state"]) == "\"unavailable\"")
      {
        sLivingHum = "-- %";
      }

      else
      {
        sLivingHum = JSON.stringify(js_hum_living["state"]);

        sLivingHum.replace("\"", "");

        sLivingHum = sLivingHum.substring(0,5) + "%";
      }
    }
  }

  if (DEBUG == 1 && JSON.typeof_(js_hum_office) == "undefined")
  {
    Serial.println("hum_office: Parsing JSON input failed!");
  }

  else
  {
    if (js_hum_office.hasOwnProperty("state"))
    {
      if (JSON.stringify(js_hum_office["state"]) == "\"unavailable\"")
      {
        sOfficeHum = "-- %";
      }

      else
      {
        sOfficeHum = JSON.stringify(js_hum_office["state"]) + "%";

        sOfficeHum.replace("\"", "");
      }
    }
  }

  if (DEBUG == 1 && JSON.typeof_(js_co2_living) == "undefined")
  {
    Serial.println("co2_living: Pasing JSON input failed!");
  }

  else
  {
    if (js_co2_living.hasOwnProperty("state"))
    {
      if (JSON.stringify(js_co2_living["state"]) == "\"unavailable\"")
      {
        sLivingCO2 = "-- ppm";
      }

      else
      {
        sLivingCO2 = JSON.stringify(js_co2_living["state"]) + "ppm";

        sLivingCO2.replace("\"", "");
      }
    }
  }

  Rect_t sofa_area = {
    .x = 30,
    .y = 14,
    .width = sofa_width,
    .height = sofa_height
  };

  epd_draw_grayscale_image(sofa_area, (uint8_t *) sofa_data);

  int cursor_x = 85;
  int cursor_y = 50;

  const char *c_line1 = sLivingTemp.c_str();

  writeln((GFXfont *)&FiraSans, c_line1, &cursor_x, &cursor_y, NULL);

  delay(500);

  sofa_area = {
    .x = 30,
    .y = 64,
    .width = sofa_width,
    .height = sofa_height
  };

  epd_draw_grayscale_image(sofa_area, (uint8_t *) sofa_data);

  const char *c_line2 = sLivingHum.c_str();

  cursor_x = 85;
  cursor_y += 50;

  writeln((GFXfont *)&FiraSans, c_line2, &cursor_x, &cursor_y, NULL);

  delay(500);

  const char *c_line3 = sLivingCO2.c_str();

  cursor_x = 85;
  cursor_y += 50;

  Rect_t co2_area = {
    .x = 30,
    .y = 114,
    .width = co2_width,
    .height = co2_height
  };

  epd_draw_grayscale_image(co2_area, (uint8_t *) co2_data);

  writeln((GFXfont *)&FiraSans, c_line3, &cursor_x, &cursor_y, NULL);

  delay(500);

  const char *c_line4 = sZoeTemp.c_str();

  cursor_x = 85;
  cursor_y += 50;

  Rect_t cat_area = {
    .x = 30,
    .y = 164,
    .width = cat_width,
    .height = cat_height
  };

  epd_draw_grayscale_image(cat_area, (uint8_t *) cat_data);

  writeln((GFXfont *)&FiraSans, c_line4, &cursor_x, &cursor_y, NULL);

  delay(500);

  const char *c_line5 = sZoeHum.c_str();

  cursor_x = 85;
  cursor_y += 50;

  cat_area = {
    .x = 30,
    .y = 214,
    .width = cat_width,
    .height = cat_height
  };

  epd_draw_grayscale_image(cat_area, (uint8_t *) cat_data);

  writeln((GFXfont *)&FiraSans, c_line5, &cursor_x, &cursor_y, NULL);

  delay(500);

  const char *c_line6 = sBedroomTemp.c_str();

  cursor_x = 85;
  cursor_y += 50;

  Rect_t bed_area = {
    .x = 30,
    .y = 264,
    .width = bed_width,
    .height = bed_height
  };

  epd_draw_grayscale_image(bed_area, (uint8_t *) bed_data);

  writeln((GFXfont *)&FiraSans, c_line6, &cursor_x, &cursor_y, NULL);

  delay(500);

  const char *c_line7 = sBedroomHum.c_str();

  cursor_x = 85;
  cursor_y += 50;

  bed_area = {
    .x = 30,
    .y = 314,
    .width = bed_width,
    .height = bed_height
  };

  epd_draw_grayscale_image(bed_area, (uint8_t *) bed_data);

  writeln((GFXfont *)&FiraSans, c_line7, &cursor_x, &cursor_y, NULL);

  delay(500);

  const char *c_line8 = sOfficeTemp.c_str();

  cursor_x = 85;
  cursor_y += 50;

  Rect_t office_area = {
    .x = 30,
    .y = 364,
    .width = office_width,
    .height = office_height
  };

  epd_draw_grayscale_image(office_area, (uint8_t *) office_data);

  writeln((GFXfont *)&FiraSans, c_line8, &cursor_x, &cursor_y, NULL);

  delay(500);

  const char *c_line9 = sOfficeHum.c_str();

  cursor_x = 85;
  cursor_y += 50;

  office_area = {
    .x = 30,
    .y = 414,
    .width = office_width,
    .height = office_height
  };

  epd_draw_grayscale_image(office_area, (uint8_t *) office_data);

  writeln((GFXfont *)&FiraSans, c_line9, &cursor_x, &cursor_y, NULL);

  delay(500);

  String line111 = GetCurrentTime();

  const char *c_line10 = line111.c_str();

  cursor_x = 85;
  cursor_y += 50;

  Rect_t timer_area = {
    .x = 30,
    .y = 464,
    .width = timer_width,
    .height = timer_height
  };

  epd_draw_grayscale_image(timer_area, (uint8_t *) timer_data);

  writeln((GFXfont *)&FiraSans, c_line10, &cursor_x, &cursor_y, NULL);

}

void DisplayBatteryStatus()
{
  digitalWrite(14, HIGH);

  esp_adc_cal_characteristics_t adc_chars;

  delay(10);
  
  esp_adc_cal_value_t val_type = esp_adc_cal_characterize(
        ADC_UNIT_2,
        ADC_ATTEN_DB_11,
        ADC_WIDTH_BIT_12,
        1100,
        &adc_chars
    );
  
  if (val_type == ESP_ADC_CAL_VAL_EFUSE_VREF) {
        Serial.printf("eFuse Vref: %umV\r\n", adc_chars.vref);
        vref = adc_chars.vref;
    }

  uint16_t v = analogRead(BATT_PIN);

  float battery_voltage = (((float)v / 4095.0) * 2.0 * 3.3 * (vref / 1000.0));

  if (battery_voltage > MIN_USB_VOL)
  {
    battery_voltage = MIN_USB_VOL;
  }
  
  float batt_max = MIN_USB_VOL;

  float batt_min = 3.3;

  float batt_range = batt_max - batt_min;

  float batt_diff = battery_voltage - batt_min;

  float batt_percentage = (batt_diff / batt_range) * 100;

  digitalWrite(14, LOW);

  int cursor_x = 500;
  int cursor_y = 50;

  String line111 = String(battery_voltage) + "V";

  const char *c_line1 = line111.c_str();

  Rect_t fuse_area = {
    .x = 440,
    .y = 14,
    .width = fuse_width,
    .height = fuse_height
  };

  epd_draw_grayscale_image(fuse_area, (uint8_t *) fuse_data);

  writeln((GFXfont *)&FiraSans, c_line1, &cursor_x, &cursor_y, NULL);

  cursor_x = 500;
  cursor_y += 50;

  line111 = String(batt_percentage) + "%";

  const char *c_line2 = line111.c_str();

  Rect_t battery_area = {
    .x = 440,
    .y = 64,
    .width = battery_width,
    .height = battery_height
  };

  epd_draw_grayscale_image(battery_area, (uint8_t *) battery_data);

  writeln((GFXfont *)&FiraSans, c_line2, &cursor_x, &cursor_y, NULL);

  PublishData(String(battery_voltage), String(batt_percentage));
}

void DisplayWeather ()
{
  String weather;

  weather = httpGETRequest(req_temp_weather);

  JSONVar js_weather = JSON.parse(weather);

  if (DEBUG == 1 && JSON.typeof(js_weather) == "undefined")
  {
    Serial.println("weather: Parsing JSON input failed!");
  }

  else
  {
    String actual_temp = JSON.stringify(js_weather["attributes"]["temperature"]) + "°C";

    String actual_state = JSON.stringify(js_weather["state"]);

    actual_state.replace("\"", "");

    if (DEBUG == 1)
    {
      Serial.println(actual_temp);

      Serial.println(actual_state);
    }

    const char *c_line99 = actual_temp.c_str();

    const char *c_line999 = actual_state.c_str();

    int cursor_x = 500;
    int cursor_y = 150;

    Rect_t weather_area = {
      .x = 440,
      .y = 114,
      .width = weather_width,
      .height = weather_height
    };

    epd_draw_grayscale_image(weather_area, (uint8_t *) weather_data);

    writeln((GFXfont *)&FiraSans, c_line99, &cursor_x, &cursor_y, NULL);

    weather_area = {
      .x = 440,
      .y = 164,
      .width = weather_width,
      .height = weather_height
    };

    cursor_x = 500;
    cursor_y = 200;

    epd_draw_grayscale_image(weather_area, (uint8_t *) weather_data);

    writeln((GFXfont *)&FiraSans, c_line999, &cursor_x, &cursor_y, NULL);
  }
}

String httpGETRequest(const char *serverName) 
{
  HTTPClient http;

  http.begin(serverName);

  http.addHeader("Authorization", "Bearer __BEARER_TOKEN__");

  // Send HTTP POST request
  int httpResponseCode = http.GET();

  String payload = "{}";

  if (httpResponseCode > 0) 
  {
    if (DEBUG == 1) 
    {
      Serial.print("HTTP Response code: ");
      Serial.println(httpResponseCode);
    }
    payload = http.getString();
  }

  else 
  {
    if (DEBUG == 1)
    {
      Serial.print("Error code: ");
      Serial.println(httpResponseCode);
    }
  }

  // Free resources
  http.end();

  return payload;
}

String GetCurrentTime()
{
  delay(100);

  ntp.update();

  JSONVar myJson;

  myJson["last_updated"] = ntp.formattedTime("%d.%m.%Y %H:%M");

  String jsonString = JSON.stringify(myJson);

  if (!client.connected())
  {
    reconnect();

    client.publish(MQTT_SENSOR_TOPIC1, jsonString.c_str(), true);

    delay (200);

    yield();

    client.loop();

    client.disconnect();
  }
  
  return ntp.formattedTime("%d.%m.%Y %H:%M");
}

void PublishData(String battVolt, String battPerc)
{
  if (battVolt == "" || battPerc == "")
  {
    if (DEBUG == 1)
    {
      Serial.print("MQTT PublishData: must contain values.\n\n");
    }
  }

  else
  {
    JSONVar myJson;

    myJson["voltage"] = battVolt;

    myJson["percentage"] = battPerc;

    String jsonString = JSON.stringify(myJson);

    if (DEBUG == 1)
    {
      Serial.print("****************************\nPrepared JSON for MQTT submit:" + jsonString + "\n\n");
    }

    if (!client.connected())
    {
      reconnect();

      client.publish(MQTT_SENSOR_TOPIC, jsonString.c_str(), true);

      delay (200);

      yield();

      client.loop();

      client.disconnect();
    }
  }
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

void DisplayNameDay ()
{
  String nameDay, sNameDay;

  nameDay = "";
  sNameDay = "";

  nameDay = httpGETRequest(req_nameday);

  JSONVar js_nameDay = JSON.parse(nameDay);

  if (DEBUG == 1 && JSON.typeof(js_nameDay) == "undefined")
  {
    Serial.println("nameday: Parsing JSON input failed!");
  }

  else
  {
    if (js_nameDay.hasOwnProperty("state"))
    {
      if (JSON.stringify(js_nameDay["state"]) == "\"unavailable\"")
      {
        sNameDay = "nezname...";
      }

      else
      {
        sNameDay = JSON.stringify(js_nameDay["state"]);

        sNameDay.replace("\"", "");
      }
    }
  }

  Rect_t cake_area = {
      .x = 440,
      .y = 314,
      .width = cake_width,
      .height = cake_height
    };

  epd_draw_grayscale_image(cake_area, (uint8_t *) cake_data);

  String line_name = "Dnes má meniny: ";

  const char *c_line9 = line_name.c_str();

  int cursor_x = 500;
  int cursor_y = 350;

  writeln((GFXfont *)&FiraSans, c_line9, &cursor_x, &cursor_y, NULL);

  const char *c_line99 = sNameDay.c_str();

  cursor_x = 500;
  cursor_y = 400;

  writeln((GFXfont *)&FiraSans, c_line99, &cursor_x, &cursor_y, NULL);
}

void DisplayNameDayTomorrow ()
{
  String nameDay, sNameDay;

  nameDay = "";
  sNameDay = "";

  nameDay = httpGETRequest(req_nameday_tomorrow);

  JSONVar js_nameDay = JSON.parse(nameDay);

  if (DEBUG == 1 && JSON.typeof(js_nameDay) == "undefined")
  {
    Serial.println("nameday: Parsing JSON input failed!");
  }

  else
  {
    if (js_nameDay.hasOwnProperty("state"))
    {
      if (JSON.stringify(js_nameDay["state"]) == "\"unavailable\"")
      {
        sNameDay = "nezname...";
      }

      else
      {
        sNameDay = JSON.stringify(js_nameDay["state"]);

        sNameDay.replace("\"", "");
      }
    }
  }

  Rect_t cake_area = {
      .x = 440,
      .y = 414,
      .width = cake_width,
      .height = cake_height
    };

  epd_draw_grayscale_image(cake_area, (uint8_t *) cake_data);

  String line_name = "Zajtra má meniny: ";

  const char *c_line9 = line_name.c_str();

  int cursor_x = 500;
  int cursor_y = 450;

  writeln((GFXfont *)&FiraSans, c_line9, &cursor_x, &cursor_y, NULL);

  const char *c_line99 = sNameDay.c_str();

  cursor_x = 500;
  cursor_y = 500;

  writeln((GFXfont *)&FiraSans, c_line99, &cursor_x, &cursor_y, NULL);
}
