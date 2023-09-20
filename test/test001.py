#!/usr/bin/env python3

from esptest import *

# hardware requirements:
# - node 0
# - node 1
# - D6 connected to eachother

# tests:
# - pulsing the node0 pin and picking it up on node1 which sends it via domoticz mqtt

@step()
def prepare():
    node[0].reboot()
    node[0].pingserial()
    node[0].serialcmd("resetFlashWriteCounter")
    node[1].reboot()
    node[1].pingserial()
    node[1].serialcmd("resetFlashWriteCounter")
    espeasy[1].controller_domoticz_mqtt()
    espeasy[1].post_device(1, """
                TDNUM:1
                TDN:
                TDE:on
                taskdevicepin1:12
                plugin_001_type:1
                plugin_001_button:0
                TDSD1:on
                TDID1:1000
                TDT:0
                TDVN1:Switch
                edit:1
            """)



@step()
def test():
    espeasy[0].control(cmd="gpio,12,1")
    pause(5)
    controller.clear_mqtt()
    espeasy[0].control(cmd="gpio,12,0")
    values=controller.recv_domoticz_mqtt(SENSOR_TYPE_SWITCH,1000)
    test_is(values[0],0)



if __name__=='__main__':
    completed()
