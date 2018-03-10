### High level ESPEasy config.
### Things like configging a controller or device via http
### I've created it in way that you can just copy/past form parameters from Chromium's header view

from espcore import *




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


    def post_controller(self, index, data):
        """post controller form to espeasy"""
        self._node.http_post(
            twice=True, # needed for controllers and devices because of the way its implemented
            page="controllers",

            params="""
                index:{index}
            """.format(index=index),

            data=data
        )

    def controller_domoticz_mqtt(self, index=1, controllerip=config.mqtt_broker, controllerport=config.mqtt_port, **kwargs):
        """config controller to use domoticz via mqtt"""

        self._node.log.info("Configuring controller domoticz mqtt "+str(kwargs))
        self.post_controller(index,"""
                protocol:2
                usedns:0
                controllerip:{controllerip}
                controllerport:{controllerport}
                controlleruser:
                controllerpassword:
                controllersubscribe:domoticz/out
                controllerpublish:domoticz/in
                controllerenabled:on
            """.format(controllerip=controllerip, controllerport=controllerport, **kwargs)
        )


    def controller_domoticz_http(self, index=1, controllerip=config.test_server, controllerport=config.http_port, **kwargs):
        """config controller to use domoticz via http"""

        self._node.log.info("Configuring controller domoticz http "+str(kwargs))
        self.post_controller(index,"""
                protocol:1
                usedns:0
                controllerip:{controllerip}
                controllerport:{controllerport}
                controlleruser:
                controllerpassword:
                controllerenabled:on
            """.format(controllerip=controllerip, controllerport=controllerport, **kwargs)
        )


    def controller_nodo(self, index=1,controllerip=config.test_server, controllerport=config.linebased_port, **kwargs):
        """config controller to use nodo"""

        self._node.log.info("Configuring controller nodo "+str(kwargs))
        self.post_controller(index,"""
                protocol:3
                usedns:0
                controllerip:{controllerip}
                controllerport:{controllerport}
                controllerpassword:
                controllerenabled:on
            """.format(controllerip=controllerip, controllerport=controllerport,**kwargs)
        )


    def controller_thingspeak(self, index=1, **kwargs):

        self._node.log.info("Configuring controller thingspeak "+str(kwargs))
        self.post_controller(index,"""
                protocol:4
                usedns:0
                controllerip:{controllerip}
                controllerport:{controllerport}
                controlleruser:
                controllerpassword:thingspeakkey1234
                controllerenabled:on
            """.format(**kwargs)
        )



    def post_device(self, index, data):
        """post a device form to espeasy"""
        self._node.log.info("Configuring tasknumber {index}".format(index=index))
        self._node.http_post(
            twice=True, # needed for controllers and devices because of the way its implemented
            page="devices",

            params="""
                index:{index}
            """.format(index=index),
            data=data
        )

    # def device_p001(self, index, **kwargs):
    #     self._node.log.info("Config device plugin p001 "+str(kwargs))
    #
    #     self.post_device(index, """
    #             TDNUM:1
    #             TDN:
    #             TDE:on
    #             taskdevicepin1:{taskdevicepin1}
    #             plugin_001_type:{plugin_001_type}
    #             plugin_001_button:{plugin_001_button}
    #             TDSD1:on
    #             TDID1:{TDID1}
    #             TDT:0
    #             TDVN1:Switch
    #             edit:1
    #         """.format(**kwargs)
    #     )


    # def device_p004(self, **kwargs):
    #     self._node.log.info("Config ds18b20 with "+str(kwargs))
    #
    #     self._node.http_post(
    #         twice=True, # needed for controllers and devices because of the way its implemented
    #         page="devices",
    #
    #         params="""
    #             index:{index}
    #         """.format(**kwargs),
    #
    #         data="""
    #             TDNUM:4
    #             TDN:temp
    #             TDE:on
    #             taskdevicepin1:{taskdevicepin1}
    #             plugin_004_dev:{plugin_004_dev}
    #             plugin_004_res:{plugin_004_res}
    #             TDT:5
    #             TDVN1:Temperature
    #             TDF1:
    #             TDVD1:2
    #             TDSD1:on
    #             TDID1:{TDID1}
    #             edit:1
    #             page:1
    #         """.format(**kwargs)
    #     )

    # def device_p005(self, **kwargs):
    #     self._node.log.info("Config DHT22 on D3 with "+str(kwargs))
    #
    #     self._node.http_post(
    #         twice=True, # needed for controllers and devices because of the way its implemented
    #         page="devices",
    #
    #         params="""
    #             index:{index}
    #         """.format(**kwargs),
    #
    #         data="""
    #             TDNUM:5
    #             TDN:
    #             TDE:on
    #             taskdevicepin1:0
    #             plugin_005_dhttype:22
    #             TDSD1:on
    #             TDID1:{TDID1}
    #             TDT:5
    #             TDVN1:Temperature
    #             TDF1:
    #             TDVD1:2
    #             TDVN2:Humidity
    #             TDF2:
    #             TDVD2:2
    #             edit:1
    #             page:1
    #         """.format(**kwargs)
    #     )


    def device_p033(self, index, **kwargs):
        self._node.log.info("Config dummy device "+str(kwargs))

        self.post_device(index=index,
            data="""
                TDNUM:33
                TDN:
                TDE:on
                plugin_033_sensortype:{plugin_033_sensortype}
                TDSD1:on
                TDID1:{TDID1}
                TDT:1
                TDVN1:first
                TDVD1:2
                TDVN2:second
                TDVD2:2
                TDVN3:third
                TDVD3:2
                TDVN4:fourth
                TDVD4:2
                edit:1
                page:1
            """.format(**kwargs)
        )


    # def device_p036(self, **kwargs):
    #     self._node.log.info("Config framed oled p036 with "+str(kwargs))
    #
    #     self._node.http_post(
    #         twice=True, # needed for controllers and devices because of the way its implemented
    #         page="devices",
    #
    #         params="""
    #             index:{index}
    #         """.format(**kwargs),
    #
    #         data="""
    #             TDNUM:36
    #             TDN:
    #             TDE:on
    #             plugin_036_adr:60
    #             plugin_036_rotate:1
    #             plugin_036_nlines:1
    #             plugin_036_scroll:1
    #             Plugin_036_template1:espeasy
    #             Plugin_036_template2:test
    #             Plugin_036_template3:suite
    #             Plugin_036_template4:test1
    #             Plugin_036_template5:test2
    #             Plugin_036_template6:test3
    #             Plugin_036_template7:test4
    #             Plugin_036_template8:test5
    #             Plugin_036_template9:test6
    #             Plugin_036_template10:test7
    #             Plugin_036_template11:test8
    #             Plugin_036_template12:test9
    #             taskdevicepin3:-1
    #             plugin_036_timer:0
    #             TDT:1
    #             edit:1
    #             page:1
    #         """.format(**kwargs)
    #     )
