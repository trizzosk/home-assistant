# Simple status display using eink Lilygo EPD47 unit

I have couple of sensors placed in my appartment measuring room temperature, humidity, CO2 and particles. I added all those data to `Home Assistant` dashboard but wanted to present the data for anybody at home also who does not use the mobile app. 

# Table of contents

tbd...

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
| LilyGO TTGO T5-4.7" E-Paper ESP32 | [Laskakit.cz](https://www.laskakit.cz/lilygo-ttgo-t5-4-7--e-paper-esp32-wifi-modul/) |
| 18650 battery holder | [Laskakit.cz](https://www.laskakit.cz/bateriovy-box-1x18650-do-dps/) |

>**Information**:<br>
>There are 2 `LilyGO TTGO T5-4.7" E-Paper ESP32` available - one with 18650 battery holder soldered on board or with `JST-PH` connector for external battery.

**Software components:**

Here are main software components which I used for inspiration about the concept and some of them are necessary for the operation (see includes in code).

| Software | Link |
| --- | --- |
| Weather station | [Github.com](https://github.com/Xinyuan-LilyGO/LilyGo-EPD-4-7-OWM-Weather-Display) |
| PubSubClient by [Nick O'Leary](https://twitter.com/knolleary) for MQTT | [knolleary.net](https://pubsubclient.knolleary.net)<br>[Github.com](https://github.com/knolleary/pubsubclient) |
| Arduino_JSON | [Github.com](https://github.com/arduino-libraries/Arduino_JSON) | 
| NTP library for Arduino framework | [Github.com](https://github.com/sstaub/NTP) |

