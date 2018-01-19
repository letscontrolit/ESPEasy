from espcore import *

import json
import threading
from queue import Queue
from logging import getLogger
import time


SENSOR_TYPE_SINGLE               =   1
SENSOR_TYPE_TEMP_HUM             =   2
SENSOR_TYPE_TEMP_BARO            =   3
SENSOR_TYPE_TEMP_HUM_BARO        =   4
SENSOR_TYPE_DUAL                 =   5
SENSOR_TYPE_TRIPLE               =   6
SENSOR_TYPE_QUAD                 =   7
SENSOR_TYPE_SWITCH               =  10
SENSOR_TYPE_DIMMER               =  11
SENSOR_TYPE_LONG                 =  20
SENSOR_TYPE_WIND                 =  21

class ControllerEmu:
    """class that emulates and decodes various types of controllers. run various threads in the background to recveive and queue stuff"""



    def __init__(self):
        self.log=logging.getLogger("controller")
        self.start_mqtt()
        self.start_http()


    def start_mqtt(self):
        """generic mqtt receiver. just queues all reqeived mqtt messages on all topics"""
        getLogger("mqtt").debug("Connecting to {mqtt_broker}".format(mqtt_broker=config.mqtt_broker))

        import paho.mqtt.client as mqtt

        mqtt_client = mqtt.Client()
        mqtt_client.connect(config.mqtt_broker, 1883, 60)
        mqtt_client.loop_start()
        mqtt_client.subscribe('#')

        self.mqtt_messages=Queue()
        def mqtt_on_message(client, userdata, message):
            logging.getLogger("mqtt").debug("Received message '" + str(message.payload) + "' on topic '"
                + message.topic + "' with QoS " + str(message.qos))
            self.mqtt_messages.put(message)

        mqtt_client.on_message=mqtt_on_message


    def clear_mqtt(self):
        """clear queue"""
        while not self.mqtt_messages.empty():
            self.mqtt_messages.get()


    def start_http(self):
        """generic http receiver. just queues all shallow copy of all http requests objects"""
        import bottle

        self.http_requests = Queue()
        @bottle.post('<filename:path>')
        @bottle.get('<filename:path>')
        def urlhandler(filename):
            logging.getLogger("http").debug(bottle.request.method+" "+str(dict(bottle.request.params)))
            self.http_requests.put(bottle.request.copy())

        http_thread=threading.Thread(target=bottle.run,  kwargs=dict(host='0.0.0.0', port=config.http_port, reloader=False))
        http_thread.daemon=True
        http_thread.start()


    def clear_http(self):
        """clear queue"""
        while not self.http_requests.empty():
            self.http_requests.get()




    def recv_domoticz_http(self, sensor_type, idx, timeout=60):
        """recv a domoticz http request from espeasy, and convert back to espeasy values"""

        start_time=time.time()
        self.log.info("Waiting for domoticz http request idx {idx} with sensortype {sensor_type}".format(sensor_type=sensor_type,idx=idx))

        self.clear_http()

        # read and parse http requests
        while time.time()-start_time<timeout:
            request=self.http_requests.get(block=True, timeout=timeout)
            if request.path == "/json.htm":
                if int(request.params.get('idx'))==idx:
                    if request.params.get('param')=='udevice':
                        svalues_str=request.params.get('svalue').split(";")
                        svalues=[]
                        for svalue in svalues_str:
                            svalues.append(float(svalue))

                        if ( sensor_type==SENSOR_TYPE_SINGLE or sensor_type==SENSOR_TYPE_LONG ) and len(svalues)==1:
                            return svalues
                        elif sensor_type==SENSOR_TYPE_DUAL and len(svalues)==2:
                            return svalues
                        elif sensor_type==SENSOR_TYPE_TEMP_HUM and len(svalues)==3:
                            return svalues
                        elif sensor_type==SENSOR_TYPE_BARO and len(svalues)==5:
                            return [svalues[0], svalues[3]]
                        elif sensor_type==SENSOR_TYPE_TRIPLE and len(svalues)==3:
                            return svalues
                        elif sensor_type==SENSOR_TYPE_TEMHUM_BARO and len(svalues)==5:
                            return [svalues[0],svalues[2], svalues[3]]
                        elif sensor_type==SENSOR_TYPE_QUAD and len(svalues)==4:
                            return svalues
                        elif sensor_type==SENSOR_TYPE_WIND and len(svalues)==5:
                            return [svalues[0], svalues[2], svalues[3]]

                    elif request.params.get('param')=='switchlight':
                        if sensor_type==SENSOR_TYPE_DIMMER or sensor_type == SENSOR_TYPE_SWITCH:
                            if request.params.get('switchcmd') == 'Off':
                                return [0]
                            elif request.params.get('switchcmd') == 'On':
                                return [1]
                            elif request.params.get('switchcmd') == 'Set Level':
                                return [int(request.params.get('level'))]

        raise(Exception("Timeout"))


    def recv_domoticz_mqtt(self, sensor_type, idx, timeout=60):
        """recv a domoticz mqtt request from espeasy, and convert back to espeasy values"""


        start_time=time.time()
        self.log.info("Waiting for domoticz mqtt request idx {idx} with sensortype {sensor_type}".format(sensor_type=sensor_type,idx=idx))

        self.clear_mqtt()

        # read and parse mqtt requests
        while time.time()-start_time<timeout:
            message=self.mqtt_messages.get(block=True, timeout=timeout)
            if message.topic == "domoticz/in":
                #decode domoticz json (should be valid json! otherwise there is a bug)
                params=json.loads(message.payload.decode())
                if int(params.get('idx'))==idx:
                    if 'command' not in params:
                        svalues_str=params.get('svalue').split(";")
                        svalues=[]
                        for svalue in svalues_str:
                            svalues.append(float(svalue))

                        if ( sensor_type==SENSOR_TYPE_SINGLE or sensor_type==SENSOR_TYPE_LONG ) and len(svalues)==1:
                            return svalues
                        elif sensor_type==SENSOR_TYPE_DUAL and len(svalues)==2:
                            return svalues
                        elif sensor_type==SENSOR_TYPE_TEMP_HUM and len(svalues)==3:
                            return svalues
                        elif sensor_type==SENSOR_TYPE_TEMP_BARO and len(svalues)==5:
                            return [svalues[0], svalues[3]]
                        elif sensor_type==SENSOR_TYPE_TRIPLE and len(svalues)==3:
                            return svalues
                        elif sensor_type==SENSOR_TYPE_TEMP_HUM_BARO and len(svalues)==5:
                            return [svalues[0],svalues[2], svalues[3]]
                        elif sensor_type==SENSOR_TYPE_QUAD and len(svalues)==4:
                            return svalues
                        elif sensor_type==SENSOR_TYPE_WIND and len(svalues)==5:
                            return [svalues[0], svalues[2], svalues[3]]

                    elif params.get('command')=='switchlight':
                        if sensor_type==SENSOR_TYPE_DIMMER or sensor_type == SENSOR_TYPE_SWITCH:
                            if params.get('switchcmd') == 'Off':
                                return [0]
                            elif params.get('switchcmd') == 'On':
                                return [1]
                            elif params.get('switchcmd') == 'Set Level':
                                return [int(params.get('level'))]

        raise(Exception("Timeout while expecting mqtt json message"))
