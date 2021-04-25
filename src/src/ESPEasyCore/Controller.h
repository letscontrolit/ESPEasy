#ifndef ESPEASYCORE_CONTROLLER_H
#define ESPEASYCORE_CONTROLLER_H

#include "../../ESPEasy_common.h"

#include "../DataTypes/EventValueSource.h"
#include "../Globals/CPlugins.h"

// ********************************************************************************
// Interface for Sending to Controllers
// ********************************************************************************
void sendData(struct EventStruct *event);

bool validUserVar(struct EventStruct *event);

#ifdef USES_MQTT
/*********************************************************************************************\
* Handle incoming MQTT messages
\*********************************************************************************************/

// handle MQTT messages
void incoming_mqtt_callback(char *c_topic, byte *b_payload, unsigned int length);

/*********************************************************************************************\
* Disconnect from MQTT message broker
\*********************************************************************************************/
void MQTTDisconnect();

/*********************************************************************************************\
* Connect to MQTT message broker
\*********************************************************************************************/
bool MQTTConnect(controllerIndex_t controller_idx);

String getMQTTclientID(const ControllerSettingsStruct& ControllerSettings);

/*********************************************************************************************\
* Check connection MQTT message broker
\*********************************************************************************************/
bool MQTTCheck(controllerIndex_t controller_idx);


String getLWT_topic(const ControllerSettingsStruct& ControllerSettings);

String getLWT_messageConnect(const ControllerSettingsStruct& ControllerSettings);

String getLWT_messageDisconnect(const ControllerSettingsStruct& ControllerSettings);

#endif // USES_MQTT

/*********************************************************************************************\
* Send status info to request source
\*********************************************************************************************/
void SendStatusOnlyIfNeeded(struct EventStruct *event, bool param1, uint32_t key, const String& param2, int16_t param3);

bool SourceNeedsStatusUpdate(EventValueSource::Enum eventSource);

void SendStatus(struct EventStruct *event, const String& status);

#ifdef USES_MQTT
bool MQTT_queueFull(controllerIndex_t controller_idx);

bool MQTTpublish(controllerIndex_t controller_idx, taskIndex_t taskIndex,  const char *topic, const char *payload, bool retained);


/*********************************************************************************************\
* Send status info back to channel where request came from
\*********************************************************************************************/
void MQTTStatus(struct EventStruct *event, const String& status);
#endif //USES_MQTT



/*********************************************************************************************\
 * send all sensordata
\*********************************************************************************************/
// void SensorSendAll();


/*********************************************************************************************\
 * send specific sensor task data, effectively calling PluginCall(PLUGIN_READ...)
\*********************************************************************************************/
void SensorSendTask(taskIndex_t TaskIndex);


#endif