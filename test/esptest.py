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


def mqtt_expect_json(topic, matches, timeout=60):
    """wait until a specific json message is received, and return it decoded. ignores all other messages"""

    start_time=time.time()

    logging.getLogger("MQTT").info("Waiting for json message on topic {topic}, with values {matches}".format(topic=topic, matches=matches))

    # check mqtt results
    while time.time()-start_time<timeout:
        while mqtt_messages:
            message=mqtt_messages.pop()
            try:
                #ignore decoding exceptions
                payload=json.loads(message.payload.decode())
            except:
                continue

            if message.topic == topic:
                ok=True
                for match in matches.items():
                    if not match[0] in payload or payload[match[0]]!=match[1]:
                        ok=False
                if ok:
                    return(payload)
        time.sleep(1)

    raise(Exception("Timeout while expecting mqtt json message"))

### create node objects and espeasy objects
node=[]
espeasy=[]

for n in config.nodes:
    node.append(Node(n, "node"+str(len(node))))
    espeasy.append(EspEasy(node[-1]))
