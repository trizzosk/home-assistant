# Nameday sensor in Hone Assistant - my way

There are 2 parts of the guideline:
- get data from web and post them to Home Assistant (MQTT) using python script
- configure a custom sensor in Home Assistant

# Get data and post them to HA using python

I made a simple python script which runs on my "server" (it's simple Raspbery Pi 4B running Ubuntu Server LTS). 

Install requirements using attached `requirements.txt`, as usual:

```shell
python3 pip install -r requirements.txt
```

## Script for data download and parsing

I used forked json file which originally is provided by [@zoltancsontos](https://github.com/zoltancsontos). Since the json key was starting with "0" I made a change and added dummy key.

Example:

```json
{
   "0":{
    "1":"dummy"
   },
   "1":{
      "1":"Nový rok",
      "2":"Alexandra, Karina, Ábel, Makar, Karin, Kara, Kasandra, Sanda, Sandra, Saša, Senda",
      "3":"Daniela, Danila, Danuta, Genovéva, Radmila",
      "4":"Drahoslav, Drahoľub, Drahomil, Drahoň, Drahoš, Duchoslav, León, Títus, Drahoľuba, Drahomila, Duchoslava, Leóna, Leónia",
      "5":"Andrea, Artúr",
      "6":"Antónia, Melchior, Melichar, Menhard",
      "7":"Bohuslava, Atila, Lucián, Bohuna, Boleslava, Božislava, Luciána",
      "8":"Severín, Čestmír, Pravomil, Čestmíra, Pravomila, Severína",
      "9":"Alexej, Alex, Domoľub, Julián, Pravoľub, Vladan, Vladen, Alexia, Pravoľuba, Vladana, Vladena",
      "10":"Dáša, Agatón, Dalimil, Dalimír, Dalimila, Dalimíra",
      "11":"Malvína, Honorát, Tasilo, Honoráta",
      "12":"Ernest, Arkád, Arnošt, Arkádia, Arnoštka, Erna, Ernestína",
      "13":"Rastislav, Čistomil, Čistoslav, Rastic, Rastimír, Ratislav, Vidor, Čistomila, Čistoslava, Rastislava, Ratislava",
      ...
      ...
      ...
   },
   "2":{
      "1":"Tatiana, Hynek, Trifon, Táňa",
      "2":"Erik, Erika, Aida",
      "3":"Blažej, Celerín, Celerína",
      "4":"Veronika, Nika, Verena, Verona",
      "5":"Agáta, Moderát, Modest, Leda, Moderáta, Modesta",
      "6":"Dorota, Dorisa, Titanila",
      ...
      ...
      ...
   },
   "3":{
    "1":"Albín",
      "2":"Anežka",
      "3":"Bohumil, Bohumila, Ticián, Ginda, Kunigunda, Ticiána",
      "4":"Kazimír, Gerazim, Romeo, Jadrana, Kazimíra",
      "5":"Fridrich, Lucius, Teofil, Friderika, Teofila",
      "6":"Radoslav, Radoslava, Fridolín, Koriolán, Radislav, Radovan, Felícia, Fridolína, Radislava",
      "7":"Tomáš, Tomislav, Tomáška, Tomislava",
      ...
      ...
      ...
   }
```

Originally I used this hack for the key id "0", but after I change the source json add added dummy key, this is not needed. 

```python
today_names = namedays[str(today.month-1)][str(today.day)]
tomorrow_names = namedays[str(tomorrow.month-1)][str(tomorrow.day)]
```

### Configuration variables in the script

Adjust following variables in the `nameday.py`:

```python
mqtt_host = "xx.xx.xx.xx"
mqtt_port = 65536
mqtt_user = '__USER__'

mqtt_auth = {'username':"__USERS__", 'password':"__PASSWORD__"}
```
### MQTT

In case you want a different topic in your `MQTT`` broker, adjust following lines:

```python
        mqtt_publish.single("nameday/today", json.dumps(today_payload), hostname=mqtt_host, port=mqtt_port, auth=mqtt_auth)
        mqtt_publish.single("nameday/tomorrow", json.dumps(tomorrow_payload), hostname=mqtt_host, port=mqtt_port, auth=mqtt_auth)
```

Change the `nameday/today` and `nameday/tomorrow` as you wish. But - please use than adjusted topic name for proper sensor configuration (see down below).

## Running the script as cron

```shell
* */1 * * * /usr/bin/python3 /home/trizzo/scripts/nameday.py >/dev/null 2>&1
```

Easy, huh?

# Custom sensor in Home Assistant

Since we know MQTT topic names, we can easily configure MQTT sensor:

```yaml
    - name: "Meniny dnes"
      unique_id: "nameday_today"
      state_topic: "nameday/today"
      value_template: "{{ value_json.names }}"
    - name: "Meniny zajtra"
      unique_id: "nameday_tomorrow"
      state_topic: "nameday/tomorrow"
      value_template: "{{ value_json.names }}"
```

If you don't have any MQTT sensor yet configured, don't forget to add this in your `configuration.yaml`:

```yaml
mqtt:
  sensor:
```

That's it. Just run the script, wait a bit and Home Assistant will process your data and show results on lovelace dashboard.
