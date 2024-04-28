import paho.mqtt.publish as publish
from datetime import datetime
import json

#Function to calculate the percentage of year progress
def year_progress():
    now = datetime.now()
    year_start = datetime(now.year, 1, 1)
    year_end = datetime(now.year + 1, 1, 1)
    total_days = (year_end - year_start).days
    days_passed = (now - year_start).days
    return round((days_passed / total_days) * 100, 2)

# Function to post the result to the MQTT broker
def post_to_mqtt(username, password, topic, percentage):
    auth = {'username': username, 'password': password}
    payload = {'percentage': percentage}
    publish.single(topic, payload=json.dumps(payload), auth=auth, hostname="__IP_HOST_OF_MQTT_BROKER__")


mqtt_username = 'YOUR_MQTT_USER' 
mqtt_password = 'YOUR_MQTT_PASSWORD' 
mqtt_topic = 'your/mqtt/topic'
percentage = year_progress()
post_to_mqtt(mqtt_username, mqtt_password, mqtt_topic, percentage)