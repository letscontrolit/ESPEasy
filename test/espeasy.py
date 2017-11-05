### High level ESPEasy config.
### Things like configging a controller or device via http
### I've created it in way that you can just copy/past form parameters from Chromium's header view


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

        self._node.log.info("Config controller domoticz mqtt "+str(kwargs))
        self._node.http_post(
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


    def device_p001(self, **kwargs):
        self._node.log.info("Config device plugin p001 "+str(kwargs))

        self._node.http_post(
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
