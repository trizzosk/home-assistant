# Home Assistant setup

This guideline **include** my setup of the `Home Assistant`, especially:
- environment measurements (temp, CO2, etc.) inside rooms
- home alarm (sensors, management, etc.)
- backups and offloading to NAS or any other device (via ssh/scp)

This guideline **does not cover** standard `Home Assistant` setup procedure, including booting from PCIe SSD, etc..

# Table of Contents:

- [Environment measurements using custom ESP32 board and IKEA VINDRIKTNING sensor](#environment-measurements-using-custom-esp32-board-and-ikea-vindriktning-sensor)
  - [Removing IKEA board and adjusting the case](#removing-ikea-board-and-adjusting-the-case)
  - [Wiring and internal setup](#wiring-and-internal-setup)
  - [The coding stuff](#the-coding-stuff)
- [To-Do:](#to-do)
 
# Environment measurements using custom ESP32 board and IKEA VINDRIKTNING sensor

**Hardware components:**

| Component | Link |
| --- | --- |
| Ikea VINDRIKTNING sensor | [Ikea store link](https://www.ikea.com/sk/sk/p/vindriktning-snimac-kvality-vzduchu-80515910/) |
| Any USB-C to USB-A cable | ... |
| 3V charger | ... | 
| LaskaKit ESP-VINDRIKTNING ESP-32 I2C | [Laskakit.cz](https://www.laskakit.cz/laskakit-esp-vindriktning-esp-32-i2c/) |
| LaskaKit SCD41 Sensor for CO2, temperature and humidity | [Laskakit.cz](https://www.laskakit.cz/laskakit-scd41-senzor-co2--teploty-a-vlhkosti-vzduchu/) |
| `μŠup`, STEMMA QT, Qwiic JST-SH 4-pin cable - 5cm | [Laskakit.cz](https://www.laskakit.cz/--sup--stemma-qt--qwiic-jst-sh-4-pin-kabel-5cm/) |

**Software components:**

| Software | Link |
| --- | --- |
| Laskakit example for ESP32 board | [Github.com](https://github.com/LaskaKit/ESP-Vindriktning) |
| PubSubClient by [Nick O'Leary](https://twitter.com/knolleary) | [knolleary.net](https://pubsubclient.knolleary.net)<br>[Github.com](https://github.com/knolleary/pubsubclient) |
| Arduino IDE (latest version) | [Arduino.cc](https://www.arduino.cc/en/software) |

## Removing IKEA board and adjusting the case

Replacing IKEA board from the `VINDRIKTNING` sensor is really easy stuff. THe only drawback is you need to adjust the hole for the USB-C input of the Laskakit board. This is pretty easy using little rasp or sandpaper and it only takes like couple of seconds.

Don't forget to keep the particle senzor inside, we will use it and connect to new board. IKEA particle sensor contains 2 wires - FAN and sensor itself. Keep it inside, don't remove them.

## Wiring and internal setup

Using [μŠup, STEMMA QT, Qwiic JST-SH 4-pin cable - 5cm](https://www.laskakit.cz/--sup--stemma-qt--qwiic-jst-sh-4-pin-kabel-5cm/) connect the board with [LaskaKit SCD41 Sensor](https://www.laskakit.cz/laskakit-scd41-senzor-co2--teploty-a-vlhkosti-vzduchu/). The board contains 2 `μŠup` sockets so you can choose any of them. Then connect FAN and particle sensor to sockets on the board - no worries, both sockets are clearly described on the board.

Now you need to plase `SCD41` sensor properly inside the case. Once done, you can close the case and put all the screws.

## The coding stuff

See the [inside-environment](inside-environment/) for the code. The folder contain template code which you can use and adjust as you want.

**Adjust variables in the template**

```c++
  // LEDs brightness
  - BRIGHTNESS

  // Wifi
  - ssid
  - password

  // Check your hassio MQTT parameters and adjust:
  - MQTT_CLIENT_ID
  - MQTT_SERVER_IP
  - MQTT_SERVER_PORT
  - MQTT_USER
  - MQTT_PASSWORD
  - MQTT_SENSOR_TOPIC

  //In case of detailed logging to Serial, change this to 1, otherwise 0:
  - DEBUG
```

If you think that measured temperature is not accurate, use anothe device to measure temperature and adjust following contant with difference:
```
  - TEMP_CONST
```

It really depends if your sensor measurement is affected by relatively near position of the ESP32 processor or not. You have to play with it.

The rest of the code was tested by me and using it since early December 2022. I had several issues with deepsleep mode and Wifi, but I made several adjustiments (wifi.begin... disconnect... etc.).

If you need to change the deepsleep period, adjust the following code:

```c++
// sleep for 5 minutes = 600 seconds to micro-seconds
// 5 mins * 60 secs = 300 * 1.000.000
esp_sleep_enable_timer_wakeup(300 * 1000000);
```

# To-Do:

- Home Assistant sensors setup
- MQTT setup
- Sensors definition
- Dashboard
