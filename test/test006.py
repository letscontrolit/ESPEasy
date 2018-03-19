#!/usr/bin/env python3

from esptest import *

# hardware requirements:
# - node 0 (will be output pin)
# - node 1 (will be input/sender)
# - D6 connected to eachother

# tests:
# - GPIO boot state

pin_input=12;

@step()
def prepare():
    node[0].reboot()
    node[0].pingserial()
    node[0].serialcmd("resetFlashWriteCounter")
    node[0].serialcmd("TaskClearAll")
    node[0].serialcmd("Save")
    node[1].reboot()
    node[1].pingserial()
    node[1].serialcmd("resetFlashWriteCounter")
    node[1].serialcmd("TaskClearAll")
    espeasy[1].controller_domoticz_mqtt()
    espeasy[1].post_device(1, """
                TDNUM:1
                TDN:
                TDE:on
                taskdevicepin1:{pin_input}
                plugin_001_type:0
                plugin_001_button:0
                TDSD1:on
                TDID1:6000
                TDT:1
                TDVN1:Switch
                edit:1
                page:1


            """.format(pin_input=pin_input))


@step()
def test():

    # log.info("Setting bootsate to default and rebooting")
    # node[0].http_post(
    #         page="hardware",
    #         data="""
    #             pled:-1
    #             pledi:on
    #             psda:4
    #             pscl:5
    #             sd:-1
    #             p0:0
    #             p2:0
    #             p4:0
    #             p5:0
    #             p9:0
    #             p10:0
    #             p12:0
    #             p13:0
    #             p14:0
    #             p15:0
    #             p16:0
    #         """)
    # node[0].reboot()
    # node[0].pingserial()


    log.info("Setting bootsate to LOW rebooting")
    espeasy[0]._node.http_post(
            page="hardware",
            data="""
                pled:-1
                pledi:on
                psda:4
                pscl:5
                sd:-1
                p0:0
                p2:0
                p4:0
                p5:0
                p9:0
                p10:0
                p12:1
                p13:0
                p14:0
                p15:0
                p16:0
            """)

    node[0].reboot()
    node[0].pingserial()

    controller.clear()
    values=controller.recv_domoticz_mqtt(SENSOR_TYPE_SWITCH,6000)
    test_is(values[0],0)


    log.info("Setting bootsate to HIGH rebooting")
    espeasy[0]._node.http_post(
            page="hardware",
            data="""
                pled:-1
                pledi:on
                psda:4
                pscl:5
                sd:-1
                p0:0
                p2:0
                p4:0
                p5:0
                p9:0
                p10:0
                p12:2
                p13:0
                p14:0
                p15:0
                p16:0
            """)

    node[0].reboot()
    node[0].pingserial()

    controller.clear()
    values=controller.recv_domoticz_mqtt(SENSOR_TYPE_SWITCH,6000)
    test_is(values[0],1)






if __name__=='__main__':
    completed()
