import json
import datetime
import requests
import paho.mqtt.publish as mqtt_publish

# URL of the JSON file on GitHub
json_url = "https://raw.githubusercontent.com/trizzosk/slovak-name-days-json/master/slovak-nameday-list.json"

# Local MQTT broker settings
mqtt_host = "xx.xx.xx.xx"
mqtt_port = 65536
mqtt_user = '__USER__'

mqtt_auth = {'username':"__USER__", 'password':"__PASSWORD__"}

def get_namedays():
    # Download the JSON file
    response = requests.get(json_url)
    if response.status_code == 200:
        namedays = json.loads(response.text)
        # Get today's and tomorrow's date
        today = datetime.date.today()
        tomorrow = today + datetime.timedelta(days=1)
        # Get names for today and tomorrow
        today_names = namedays[str(today.month)][str(today.day)]
        tomorrow_names = namedays[str(tomorrow.month)][str(tomorrow.day)]
        # Create payload for MQTT
        today_payload = {
            "date": today.isoformat(),
            "names": today_names
        }
        tomorrow_payload = {
            "date": tomorrow.isoformat(),
            "names": tomorrow_names
        }
        # Publish payload to MQTT topics
        mqtt_publish.single("nameday/today", json.dumps(today_payload), hostname=mqtt_host, port=mqtt_port, auth=mqtt_auth)
        mqtt_publish.single("nameday/tomorrow", json.dumps(tomorrow_payload), hostname=mqtt_host, port=mqtt_port, auth=mqtt_auth)
    else:
        print("Failed to download JSON file")

if __name__ == "__main__":
    get_namedays()