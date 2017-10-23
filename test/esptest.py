#basic stuff needed in each test

from espeasy import *
from node import *
import config
import time
import paho.mqtt.client as mqtt
import json
from espcore import *



### mqtt stuff
logging.getLogger("MQTT").debug("Connecting to {mqtt_broker}".format(mqtt_broker=config.mqtt_broker))

mqtt_client = mqtt.Client()
mqtt_client.connect(config.mqtt_broker, 1883, 60)
mqtt_client.loop_start()
mqtt_client.subscribe('#')

mqtt_messages=[]
def mqtt_on_message(client, userdata, message):
    logging.getLogger("MQTT").debug("Received message '" + str(message.payload) + "' on topic '"
        + message.topic + "' with QoS " + str(message.qos))
    mqtt_messages.append(message)

mqtt_client.on_message=mqtt_on_message


### create node objects and espeasy objects
node=[]
espeasy=[]

for n in config.nodes:
    node.append(Node(n, "node"+str(len(node))))
    espeasy.append(EspEasy(node[-1]))
