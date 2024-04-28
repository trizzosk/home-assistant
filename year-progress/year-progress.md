# Fancy year progress sensor (% of actual year of 365 days) with beautiful progress bar on your dashboard

There is a simple profile on X network [@year_progress](https://twitter.com/year_progress?) which shows fancy progress bar every day as a percentage of actual year progress. I know that this is nothing crucial or important for your home automation or smart home installation. Anyway - it can bring a new fancy element to your dashboard.

![Year progress](./images/Screenshot%202024-04-28%20at%2020.34.06.png)

# Components

Here are the main 3 components:

- Python script to calculate a value
- HomeAssistant MQTT sensor
- HACS addon

# Python script

Probably someone will find more elegant solution but this was very quick to write and fast to deploy. I am not a Python developer, so every script for me is kind of challenge. The [script](./code/year-progress.py) calculates the year progress, construct a json payload and publish it to your MQTT broker. Please adjusts variables values for your MQTT broker (IP or hostname, user, password and topic).

I run the script on my "home server" (basically Raspberry PI 4B with Ubuntu Server installed), which acts as media server, zabbix monitoring, samba share etc.. Here is `crontab` example:

```shell
*/10 * * * * /usr/bin/python3 /home/trizzo/scripts/year-progress.py >/dev/null 2>&1
```

I run it every 10 minutes. That's because of possible errors or restarts of HomeAssistant and MQTT broker where missing values can mess up your dashboard (progress bar).

# HomeAssistant MQTT sensor

I rely on MQTT sensor config. Here is the example:

```yaml
mqtt:
  sensor:
    - name: "Year progress"
      unique_id: "year_progress"
      state_topic: "your/mqtt/topic"
      unit_of_measurement: "%"
      value_template: "{{ value_json.percentage }}"
      payload_available: "online"
      payload_not_available: "offline"
```

Don't forget to adjust `state_topic` based on your MQTT. Config shall be in your `configuration.yaml` file. After this adjustment, don't forget:

- run the above python script manually or wait for cron job to do its job,
- reload the config (`Developer tools` -> hit the `Restart` button and choose `Quick reload`)

Then go to `Settings` -> `Devices & services` -> `Entities` and search for  `year` as shown on the picture.

![Year progress sensor](./images/Screenshot%202024-04-28%20at%2020.33.42.png)

# `HACS` addon and dashboard config

Progress bar as a dashboard object/card is not present as a default option. You need to add custom HACS components:

- `card-mod` [link](https://github.com/thomasloven/lovelace-card-mod) -> adds option to customise your card elements with `CSS styles`
- `bar-card` [link](https://github.com/custom-cards/bar-card) -> adds the progress bar card

Both components can be easily add to your HomeAssistant instance using `HACS` addon store. Just follow instructions for every component. At the end add a new card with `year_progress` (or any other sensor name which you chose in the config) sensor and adjust the progress bar as you wish.

<br><br>