# Measuring environment data with IKEA Vindriktning sensor.... sort of :lol

It was couple of months ago, back in 2022, I was browsing Twitter feed and one of guys I follow posted a message about the `LaskaKit` development board and `IKEA Vindriktning` sensor. Since the sensor looks pretty cool (yes, its' not the most pretty, but it fits in almost every possible interior design). Although he did not provide any step-by-step procedure, I accepted a challenge to try it myself and learn something about ESP32 platform (in general). So, my journey started.

# Table of contents

- [Basic idea and objectives](#basic-idea-and-objectives)
- [Key functionalities, highlights of the setup](#key-functionalities-highlights-of-the-setup)
- [Components](#components)
- [Procedure](#procedure)
  - [Removing IKEA board and adjusting the case](#removing-ikea-board-and-adjusting-the-case)
  - [Wiring and internal setup](#wiring-and-internal-setup)
  - [Arduino IDE setup](#arduino-ide-setup)
  - [The coding stuff](#the-coding-stuff)
  - [MQTT setup in Home Assistant](#mqtt-setup-in-home-assistant)
  - [Home Assistant setup](#home-assistant-setup)
- [Homework](#homework)
- [TO-DO](#to-do)

# Basic idea and objectives

My idea was, ehm still is, to measure temperature, humidity and CO2 in every sector of our family apartment. Next step will be to reverse, maybe replace, Danfoss unit for heating management (current device is pretty dumb...). I want to dynamically change heating schemas according current environment conditions and planned activities (vacation mode, empty room,...).

# Key functionalities, highlights of the setup

- LaskaKit board is equipped with addressable LEDs', but I decided not to use them. So the sensor sends its values and status only using MQTT. I find this pretty useful, especially for those who don't want to have any light in the room during nights
- If you want to use LEDs', check the original code of LaskaKit (see the link down below).
- I still use the original particle meter. Once you don't want to measure particles in the air, you only need to unwire fan and sensor, remove them from the case and adjust code (removing references and functions + adjusting the MQTT + `Home Assistant` sensor config). Try it yourself, it's very easy.
- I use the deepsleep mode at the end of the loop to reduce electricity consumption (I know, ESP32 do have very little consumption but still - it runs every 10 minutes, so...). If you dont' want it, just comment part of the code at the end of the `loop()` and uncomment `delay(...)` right before the deepsleep section. Looping will function anyway but there won't be any deepsleep between cycles.

# Components

Down below I put the most important parts of the setup. Of course, you need to have working `Home Assistant` box/instance in order to utilize and extent the functionalities as you want.

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
| Laskakit example code for ESP32 board | [Github.com](https://github.com/LaskaKit/ESP-Vindriktning) |
| PubSubClient by [Nick O'Leary](https://twitter.com/knolleary) for MQTT | [knolleary.net](https://pubsubclient.knolleary.net)<br>[Github.com](https://github.com/knolleary/pubsubclient) |
| Arduino IDE (latest version) | [Arduino.cc](https://www.arduino.cc/en/software) |

# Procedure

## Removing IKEA board and adjusting the case

Replacing IKEA board from the `VINDRIKTNING` sensor is really easy stuff. The only drawback is you need to adjust the hole for the USB-C connector of the Laskakit board. This is pretty easy using little rasp or sandpaper and it only takes like couple of seconds.

Don't forget to keep the particle senzor and fan inside, we will use it and connect to new board. IKEA particle sensor contains 2 wires - FAN and sensor itself. Keep it inside, don't remove them.

## Wiring and internal setup

Using [μŠup, STEMMA QT, Qwiic JST-SH 4-pin cable - 5cm](https://www.laskakit.cz/--sup--stemma-qt--qwiic-jst-sh-4-pin-kabel-5cm/) connect the board with [LaskaKit SCD41 Sensor](https://www.laskakit.cz/laskakit-scd41-senzor-co2--teploty-a-vlhkosti-vzduchu/). The board contains 2 `μŠup` sockets so you can choose any of them. Then connect FAN and particle sensor to sockets on the board - no worries, both sockets are clearly described on the board.

Now you need to place `SCD41` sensor properly inside the case. Once done, you can close the case and put all the screws. You can experiment with some ESP32 processor cover, some kind of mounting frame for the SCD41 sensor.

## Arduino IDE setup

There are plenty of guidelines how to setup Arduino IDE to work with ESP boards. In order to fully support all required functionalities in this guideline, you just need to do following:
- add the ESP32 kit to your Arduino IDE
- install `PubSubClient`
- choose correct serial port after you connect board to your laptop
- change the speed rate of the port to `115200` in order to flash and connect properly to the ESP32 board.

>**Note:**<br>
>To make it easier for you - to connect the board to your laptop use the **USB-C to USB-A cable** instead of pure USB-C. With only USB-C cable, the development board did not connect to my laptop (Macbook Pro M1). The board is build with automated detection of serial connection to a computer, so you don't need to reboot the board to flashmode manually.

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

Now you are ready to compile and flash the board with your custom code.

## MQTT setup in Home Assistant

I will not write how to install the `Mosquito broker` (`MQTT`) addon. For this check the official documentation. The only requirement is to create a user which we will use in our case to connect our MQTT broker. Here is the extract from the official `MQTT` documentation:

```
Create a new user for MQTT via your Home Assistant's frontend Configuration -> Users (manage users) , (i.e. not on Mosquitto's Configuration tab). Notes:

- This name cannot be homeassistant or addons, those are reserved usernames.
- If you do not see the option to create a new user, ensure that Advanced Mode is enabled in your Home Assistant profile.
```

That's it, we can move on to setup of our sensors.

## Home Assistant setup

For editing Home Assistant configuration files I recommend to use either [Home Assistant Configuration Editor Helper](https://github.com/htmltiger/config-editor-card) or, which is my preferred component, utilise the [Studio Code Server](https://github.com/hassio-addons/addon-vscode) addon. In order to setup sensors you have to adjust `configuration.yaml` and add following lines:

```yaml
mqtt:
  sensor:
    - name: "sensor-temperature"
      unique_id: your_sensor_unique_id
      state_topic: "topic/name"
      unit_of_measurement: "°C"
      value_template: "{{ value_json.temperature }}"
    - name: "sensor-humidity"
      unique_id: your_sensor_unique_id
      state_topic: "topic/name"
      unit_of_measurement: "%"
      value_template: "{{ value_json.humidity }}"
    - name: "sensor-CO2"
      unique_id: your_sensor_unique_id
      state_topic: "topic/name"
      unit_of_measurement: "ppm"
      value_template: "{{ value_json.co2 }}"
    - name: "sensor-particles"
      unique_id: your_sensor_unique_id
      state_topic: "topic/name"
      unit_of_measurement: "ppm"
      value_template: "{{ value_json.pm2_5 }}"
```

**Explanation:**

| Parameter |  Description |
| --- | --- |
| `name` | The name of your sensor as it is used in Home Assistant for any other device name. |
| `unique_id` | Basically its the `entity_id` as used for identify every device connected. This needs to be unique across your Home Assistant instance. I use prefix like room name and then I add measurement name - e.g. Living-Room-Temperature, \*-Humidity, etc.. |
| `state_topic` | The name of the MQTT endpoint where we publish data from our sensor - this has to be the same like we defined earlier in the code - see the variable `MQTT_SENSOR_TOPIC` above. |
| `unit_of_measurement` | This is pretty clear, I guess. |
| `value_template` | This commands the Home Assistant which value to look up in the JSON message in the broker. |

# Homework

Yes - based on this, you can easily create custom flows (e.g. using `Node-Red` addon), which will notify you in case of unhealth values measured (high CO2 concentration, low/high humidity etc.). These can be pretty easy, but can be pretty complex as well (during summer, when average temperature of last 10 values higher than something -> turn on the AC, set average -5 degrees for 30 minutes, etc.). It's up to you, I see the `Node-Red` as very very powerful component.

# TO-DO

- e-INK display with enclosure for displaying environment values in fancy way (currently waiting for e-INK display delivery)
