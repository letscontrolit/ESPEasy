#!/usr/bin/env python3

from esptest import *

# hardware requirements:
# - node 0
# - 2x ds18b12 connected to D4

# tests:
# - correct values are send to domoticz via mqtt
# - test if powercycle value is valid (https://github.com/letscontrolit/ESPEasy/issues/719)



@step()
def prepare():
    node[0].reboot()
    node[0].pingserial()
    node[0].serialcmd("resetFlashWriteCounter")
    espeasy[0].controller_domoticz_mqtt()
    espeasy[0].post_device(index=1, data="""
        TDNUM:4
        TDN:temp
        TDE:on
        taskdevicepin1:2
        plugin_004_dev:0
        plugin_004_res:9
        TDT:5
        TDVN1:Temperature
        TDF1:
        TDVD1:2
        TDSD1:on
        TDID1:2001
        edit:1
        page:1
    """)

    espeasy[0].post_device(index=2, data="""
        TDNUM:4
        TDN:temp
        TDE:on
        taskdevicepin1:2
        plugin_004_dev:1
        plugin_004_res:9
        TDT:5
        TDVN1:Temperature
        TDF1:
        TDVD1:2
        TDSD1:on
        TDID1:2002
        edit:1
        page:1
    """)



@step()
def test():
    results=controller.recv_domoticz_mqtt(SENSOR_TYPE_SINGLE,2001)
    test_in_range(results[0], -5,40)
    results=controller.recv_domoticz_mqtt(SENSOR_TYPE_SINGLE,2002)
    test_in_range(results[0], -5,40)


@step()
def powercycle():
    """test result on poweron"""
    if not node[0].powercycle():
        return
    
    results=controller.recv_domoticz_mqtt(SENSOR_TYPE_SINGLE,2001)
    test_in_range(results[0], -5,40)
    results=controller.recv_domoticz_mqtt(SENSOR_TYPE_SINGLE,2002)
    test_in_range(results[0], -5,40)


if __name__=='__main__':
    completed()
