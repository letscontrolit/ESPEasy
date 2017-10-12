### High level ESPEasy config.
### Things like configging a controller or device via http
### I've created it in way that you can just copy/past form parameters from Chromium's header view


class EspEasy:

    def __init__(self, node):
        self._node=node


    def controller_domoticz_mqtt(self, **kwargs):
        """config controller to use domoticz via mqtt"""

        self._node.http_post(
            page="controllers",

            params="""
                index: {index}
            """.format(**kwargs),

            data="""
                protocol:2
                usedns:0
                controllerip:{mqtt_broker}
                controllerport:1883
                controlleruser:
                controllerpassword: http://urltje
                controllersubscribe:domoticz/out
                controllerpublish:domoticz/in
                controllerenabled:on
            """.format(**kwargs)
        )
