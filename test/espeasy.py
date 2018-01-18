### High level ESPEasy config.
### Things like configging a controller or device via http
### I've created it in way that you can just copy/past form parameters from Chromium's header view

from espcore import *

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

class EspEasy:

    def __init__(self, node):
        self._node=node


    def control(self, **kwargs):
        self._node.log.info("Control "+str(kwargs))
        self._node.http_post(
            page="control",

            params="""
                cmd:{cmd}
            """.format(**kwargs),

        )


    def controller_domoticz_mqtt(self, **kwargs):
        """config controller to use domoticz via mqtt"""

        self._node.log.info("Configuring controller domoticz mqtt "+str(kwargs))
        self._node.http_post(
            twice=True, # needed for controllers and devices because of the way its implemented
            page="controllers",

            params="""
                index:{index}
            """.format(**kwargs),

            data="""
                protocol:2
                usedns:0
                controllerip:{controllerip}
                controllerport:1883
                controlleruser:
                controllerpassword:
                controllersubscribe:domoticz/out
                controllerpublish:domoticz/in
                controllerenabled:on
            """.format(**kwargs)
        )


    def controller_domoticz_http(self, **kwargs):
        """config controller to use domoticz via http"""

        self._node.log.info("Configuring controller domoticz http "+str(kwargs))
        self._node.http_post(
            twice=True, # needed for controllers and devices because of the way its implemented
            page="controllers",

            params="""
                index:{index}
            """.format(**kwargs),

            data="""
                protocol:1
                usedns:0
                controllerip:{controllerip}
                controllerport:{controllerport}
                controlleruser:
                controllerpassword:
                controllerenabled:on
            """.format(**kwargs)
        )

    def recv_domoticz_http(self, sensor_type, idx, timeout=60):
        """recv a domoticz http request from espeasy, and convert back to espeasy values"""

        start_time=time.time()
        logging.getLogger("domoticz http").info("Waiting for request idx {idx} with sensortype {sensor_type}".format(sensor_type=sensor_type,idx=idx))

        # read and parse http requests
        while not http_requests.empty():
            http_requests.get()

        while time.time()-start_time<timeout:
            request=http_requests.get(block=True, timeout=timeout)
            if request.path == "/json.htm" and int(request.params.get('idx'))==idx:
                if request.params.get('param')=='udevice':
                    svalues_str=request.params.get('svalue').split(";")
                    svalues=[]
                    for svalue in svalues_str:
                        svalues.append(float(svalue))

                    if sensor_type==SENSOR_TYPE_SINGLE and len(svalues)==1:
                        return svalues
                    elif sensor_type==SENSOR_TYPE_DUAL and len(svalues)==2:
                        return svalues
                    elif sensor_type==SENSOR_TYPE_HUM and len(svalues)==3:
                        return svalues
                    elif sensor_type==SENSOR_TYPE_BARO and len(svalues)==5:
                        return [svalues[0], svalues[3]]
                    elif sensor_type==SENSOR_TYPE_TRIPLE and len(svalues)==3:
                        return svalues
                    elif sensor_type==SENSOR_TYPE_HUM_BARO and len(svalues)==5:
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



    def controller_thingspeak(self, **kwargs):

        self._node.log.info("Configuring controller thingspeak "+str(kwargs))
        self._node.http_post(
            twice=True, # needed for controllers and devices because of the way its implemented
            page="controllers",

            params="""
                index:{index}
            """.format(**kwargs),

            data="""
                protocol:4
                usedns:0
                controllerip:{controllerip}
                controllerport:{controllerport}
                controlleruser:
                controllerpassword:thingspeakkey1234
                controllerenabled:on
            """.format(**kwargs)
        )


    def device_p001(self, **kwargs):
        self._node.log.info("Config device plugin p001 "+str(kwargs))

        self._node.http_post(
            twice=True, # needed for controllers and devices because of the way its implemented
            page="devices",

            params="""
                index:{index}
            """.format(**kwargs),

            data="""
                TDNUM:1
                TDN:
                TDE:on
                taskdevicepin1:{taskdevicepin1}
                plugin_001_type:{plugin_001_type}
                plugin_001_button:{plugin_001_button}
                TDSD1:on
                TDID1:{TDID1}
                TDT:0
                TDVN1:Switch
                edit:1
            """.format(**kwargs)
        )


    def device_p004(self, **kwargs):
        self._node.log.info("Config device plugin p004 "+str(kwargs))

        self._node.http_post(
            twice=True, # needed for controllers and devices because of the way its implemented
            page="devices",

            params="""
                index:{index}
            """.format(**kwargs),

            data="""
                TDNUM:4
                TDN:temp
                TDE:on
                taskdevicepin1:{taskdevicepin1}
                plugin_004_dev:{plugin_004_dev}
                plugin_004_res:{plugin_004_res}
                TDT:5
                TDVN1:Temperature
                TDF1:
                TDVD1:2
                TDSD1:on
                TDID1:{TDID1}
                edit:1
                page:1
            """.format(**kwargs)
        )

    def device_p036(self, **kwargs):
        self._node.log.info("Config device plugin p036 "+str(kwargs))

        self._node.http_post(
            twice=True, # needed for controllers and devices because of the way its implemented
            page="devices",

            params="""
                index:{index}
            """.format(**kwargs),

            data="""
                TDNUM:36
                TDN:
                TDE:on
                plugin_036_adr:60
                plugin_036_rotate:1
                plugin_036_nlines:1
                plugin_036_scroll:1
                Plugin_036_template1:espeasy
                Plugin_036_template2:test
                Plugin_036_template3:suite
                Plugin_036_template4:test1
                Plugin_036_template5:test2
                Plugin_036_template6:test3
                Plugin_036_template7:test4
                Plugin_036_template8:test5
                Plugin_036_template9:test6
                Plugin_036_template10:test7
                Plugin_036_template11:test8
                Plugin_036_template12:test9
                taskdevicepin3:-1
                plugin_036_timer:0
                TDT:1
                edit:1
                page:1
            """.format(**kwargs)
        )
