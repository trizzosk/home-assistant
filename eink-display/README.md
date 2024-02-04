# Simple status display using eink Lilygo EPD47 unit

I have couple of sensors placed in my appartment measuring room temperature, humidity, CO2 and particles. I added all those data to `Home Assistant` dashboard but wanted to present the data for anybody at home also who does not use the mobile app.

![Front side](images/20240203_140140_5220491365269653277.jpg)
![Back side](images/20240203_140036_8090161508967312431.jpg)

Check the code inside the [code](./code/) folder.

# Basic idea and objectives

Some years ago freind of mine prepared similar device for his purpose - simple meteo dashboard showing actual weather and precipation from home sensors. I came to idea to use the concept and present measurements for anybody. 

# Key functionalities, highlights of the setup

- Periodical data fetch from `Home Assistant` API and display outcome on eink display
- Wireless operation - no power cable, utilizing 18650 battery and custom case
- Measuring estimated remaining battery capacity
  
# Components

All listed components are easy to obtain and available on many eshops. Of course, you need to have working `Home Assistant` box/instance in order to utilize and extent the functionalities as you want.

| Component | Link |
| --- | --- |
| LilyGO TTGO T5-4.7" E-Paper ESP32-S3 | [Laskakit.cz](https://www.laskakit.cz/lilygo-ttgo-t5-4-7--e-paper-esp32-wifi-modul/) |
| LiPol battery unit | [Laskakit.cz](https://www.laskakit.cz/ehao-lipol-baterie-603048-900mah-3-7v/) |

**Software components:**

Here are main software components which I used for inspiration about the concept and some of them are necessary for the operation (see includes in code).

| Software | Link |
| --- | --- |
| Weather station | [Github.com](https://github.com/Xinyuan-LilyGO/LilyGo-EPD-4-7-OWM-Weather-Display) |
| PubSubClient by [Nick O'Leary](https://twitter.com/knolleary) for MQTT | [knolleary.net](https://pubsubclient.knolleary.net)<br>[Github.com](https://github.com/knolleary/pubsubclient) |
| Arduino_JSON | [Github.com](https://github.com/arduino-libraries/Arduino_JSON) | 
| NTP library for Arduino framework | [Github.com](https://github.com/sstaub/NTP) |

# 3D printed case

Since the ESP32-S3 board is not very often used, it was tricky to find proper 3D model for this board. However, I found one [here](https://www.printables.com/model/522518-lilygo-t5-47-s3-e-paper-case). Be aware that battery will not fit inside as well as soldered GPIO head. 

# Image convertion (Material Design Icons)

For converting images (Material Design Icons) I use `imgconvert.py` script available [here](https://github.com/Xinyuan-LilyGO/LilyGo-EPD47/blob/master/scripts/imgconvert.py). Just be aware that you have to use jpg file format for conversion. The png format with trasnparency is not properly converted.

# Dynamic refresh time based on time of the day

I came to the idea that during night there is no reason to refresh data on display every 15 minutes. This will refresh data every 60 minutes between 22:00 (10:00pm) and 5:00 (05:00am).

```C++
void loop()
{
  ...
  ...
  ...
  // Change the IP address of your prefered NTP server
  configTime(3600, 0, "192.168.0.1");

  struct tm timeinfo;
  if (!getLocalTime(&timeinfo)) {
    Serial.println("Failed to obtain time");
    return;
  }

  // Determine deep sleep duration based on the current time
  int deepSleepMinutes;

  if (timeinfo.tm_hour >= 5 && timeinfo.tm_hour < 22)
  {
    deepSleepMinutes = 15; // Daytime deep sleep period
  }
  
  else
  {
    deepSleepMinutes = 60; // Nighttime deep sleep period
  }

  int deepSleepMiliseconds = (deepSleepMinutes*60)*1000000;
  ...
  ...
  ...
}
```

Then I changed parameter of the `esp_sleep_enable_timer_wakeup()` function to use dynamically calculated wakeup timer:

```c++
esp_sleep_enable_timer_wakeup(deepSleepMiliseconds);
```
