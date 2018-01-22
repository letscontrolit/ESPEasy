#!/usr/bin/env python3

from esptest import *

# hardware requirements:
# - node 0
# - DHT22 on D3

# tests:
# - correct values are send to domoticz via mqtt


@step()
def prepare():
    node[0].reboot()
    node[0].pingserial()
    node[0].serialcmd("resetFlashWriteCounter")
    espeasy[0].controller_domoticz_mqtt()
    espeasy[0].post_device(index=1,data="""
                TDNUM:5
                TDN:
                TDE:on
                taskdevicepin1:0
                plugin_005_dhttype:22
                TDSD1:on
                TDID1:4001
                TDT:5
                TDVN1:Temperature
                TDF1:
                TDVD1:2
                TDVN2:Humidity
                TDF2:
                TDVD2:2
                edit:1
                page:1

        """)


@step()
def test():
    results=controller.recv_domoticz_mqtt(SENSOR_TYPE_TEMP_HUM,4001)
    test_in_range(results[0], -5,40)
    test_in_range(results[1], 10,80)


if __name__=='__main__':
    completed()
