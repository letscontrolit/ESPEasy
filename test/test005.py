#!/usr/bin/env python3

from esptest import *

# hardware requirements:
# - node 0

# tests:
# - sending all data types for all controllers, using dummy devices

@step()
def prepare():
    node[0].reboot()
    node[0].pingserial()
    node[0].serialcmd("resetFlashWriteCounter")
    controller.clear()

#traverse most controllers
for ( title, controller_config, controller_recv ) in [
        ("Domoticz MQTT", espeasy[0].controller_domoticz_mqtt, controller.recv_domoticz_mqtt ),
        ("Domoticz HTTP", espeasy[0].controller_domoticz_http, controller.recv_domoticz_http ),
    ]:


    @step(title)
    def config_controller():
        controller_config()



    @step(title)
    def single():
        node[0].serialcmd("TaskValueSet 1,1,5001")
        espeasy[0].device_p033(index=1, TDID1=5100, plugin_033_sensortype=SENSOR_TYPE_SINGLE)
        results=controller_recv(SENSOR_TYPE_SINGLE,5100)
        test_is(results, [ 5001 ])


    @step(title)
    def long():
        a=50025003
        #long is actually quite hackish in espeasy: since uservar only supports floats, we use 2 uservars and some bitshifting. the controller will see just one value
        node[0].serialcmd("TaskValueSet 1,1,{0}".format(a & 0xffff))
        node[0].serialcmd("TaskValueSet 1,2,{0}".format(a>>16))
        espeasy[0].device_p033(index=1, TDID1=5101, plugin_033_sensortype=SENSOR_TYPE_LONG)
        results=controller_recv(SENSOR_TYPE_LONG,5101)
        test_is(results, [ a ])


    @step(title)
    def dual():
        node[0].serialcmd("TaskValueSet 1,1,5004")
        node[0].serialcmd("TaskValueSet 1,2,5005")
        espeasy[0].device_p033(index=1, TDID1=5102, plugin_033_sensortype=SENSOR_TYPE_DUAL)
        results=controller_recv(SENSOR_TYPE_DUAL,5102)
        test_is(results, [ 5004, 5005 ])


    @step(title)
    def temp_hum():
        node[0].serialcmd("TaskValueSet 1,1,5006")
        node[0].serialcmd("TaskValueSet 1,2,5007")
        espeasy[0].device_p033(index=1, TDID1=5103, plugin_033_sensortype=SENSOR_TYPE_TEMP_HUM)
        results=controller_recv(SENSOR_TYPE_TEMP_HUM,5103)
        test_is(results, [ 5006, 5007 ])


    @step(title)
    def temp_baro():
        node[0].serialcmd("TaskValueSet 1,1,5008")
        node[0].serialcmd("TaskValueSet 1,2,5009")
        espeasy[0].device_p033(index=1, TDID1=5104, plugin_033_sensortype=SENSOR_TYPE_TEMP_BARO)
        results=controller_recv(SENSOR_TYPE_TEMP_BARO,5104)
        test_is(results, [ 5008, 5009 ])


    @step(title)
    def triple():
        node[0].serialcmd("TaskValueSet 1,1,5010")
        node[0].serialcmd("TaskValueSet 1,2,5011")
        node[0].serialcmd("TaskValueSet 1,3,5012")
        espeasy[0].device_p033(index=1, TDID1=5105, plugin_033_sensortype=SENSOR_TYPE_TRIPLE)
        results=controller_recv(SENSOR_TYPE_TRIPLE,5105)
        test_is(results, [ 5010, 5011, 5012 ])


    @step(title)
    def temp_hum_baro():
        node[0].serialcmd("TaskValueSet 1,1,5013")
        node[0].serialcmd("TaskValueSet 1,2,5014")
        node[0].serialcmd("TaskValueSet 1,3,5015")
        espeasy[0].device_p033(index=1, TDID1=5106, plugin_033_sensortype=SENSOR_TYPE_TEMP_HUM_BARO)
        results=controller_recv(SENSOR_TYPE_TEMP_HUM_BARO,5106)
        test_is(results, [ 5013, 5014, 5015 ])


    @step(title)
    def quad():
        node[0].serialcmd("TaskValueSet 1,1,5016")
        node[0].serialcmd("TaskValueSet 1,2,5017")
        node[0].serialcmd("TaskValueSet 1,3,5018")
        node[0].serialcmd("TaskValueSet 1,4,5019")
        espeasy[0].device_p033(index=1, TDID1=5107, plugin_033_sensortype=SENSOR_TYPE_QUAD)
        results=controller_recv(SENSOR_TYPE_QUAD,5107)
        test_is(results, [ 5016, 5017, 5018, 5019 ])


    @step(title)
    def switch():
        node[0].serialcmd("TaskValueSet 1,1,0")
        espeasy[0].device_p033(index=1, TDID1=5108, plugin_033_sensortype=SENSOR_TYPE_SWITCH)
        results=controller_recv(SENSOR_TYPE_SWITCH,5108)
        test_is(results, [ 0 ])
        node[0].serialcmd("TaskValueSet 1,1,1")
        results=controller_recv(SENSOR_TYPE_SWITCH,5108)
        test_is(results, [ 1 ])


    @step(title)
    def dimmer():
        node[0].serialcmd("TaskValueSet 1,1,0")
        espeasy[0].device_p033(index=1, TDID1=5109, plugin_033_sensortype=SENSOR_TYPE_DIMMER)
        results=controller_recv(SENSOR_TYPE_DIMMER,5109)
        test_is(results, [ 0 ])
        node[0].serialcmd("TaskValueSet 1,1,25")
        results=controller_recv(SENSOR_TYPE_DIMMER,5109)
        test_is(results, [ 25 ])



    @step(title)
    def wind():
        node[0].serialcmd("TaskValueSet 1,1,5022")
        node[0].serialcmd("TaskValueSet 1,2,5023")
        node[0].serialcmd("TaskValueSet 1,3,5024")
        espeasy[0].device_p033(index=1, TDID1=5110, plugin_033_sensortype=SENSOR_TYPE_WIND)
        results=controller_recv(SENSOR_TYPE_WIND,5110)
        test_is(results, [ 5022, 5023, 5024 ])

if __name__=='__main__':
    completed()
