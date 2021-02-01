#ifndef HELPER_CPLUGIN_SENSORTYPEHELPER_H
#define HELPER_CPLUGIN_SENSORTYPEHELPER_H

#include <Arduino.h>

#include "../DataStructs/DeviceStruct.h"

/*********************************************************************************************\
   Get value count from sensor type

   Only use this function to determine nr of output values when changing output type of a task
   To get the actual output values for a task, use getValueCountForTask
\*********************************************************************************************/
byte getValueCountFromSensorType(Sensor_VType sensorType);

String getSensorTypeLabel(Sensor_VType sensorType);

void sensorTypeHelper_webformLoad_allTypes(struct EventStruct *event, byte pconfigIndex);

void sensorTypeHelper_webformLoad_header();

void sensorTypeHelper_webformLoad_simple(struct EventStruct *event, byte pconfigIndex);

void sensorTypeHelper_webformLoad(struct EventStruct *event, byte pconfigIndex, int optionCount, const byte options[]);

void sensorTypeHelper_saveOutputSelector(struct EventStruct *event, byte pconfigIndex, byte valueIndex, const String& defaultValueName);

void pconfig_webformSave(struct EventStruct *event, byte pconfigIndex);

void sensorTypeHelper_loadOutputSelector(
  struct EventStruct *event, byte pconfigIndex, byte valuenr,
  int optionCount, const String options[], const int indices[] = NULL);


#endif // HELPER_CPLUGIN_SENSORTYPEHELPER_H