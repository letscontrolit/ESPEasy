#ifndef HELPER_CPLUGIN_SENSORTYPEHELPER_H
#define HELPER_CPLUGIN_SENSORTYPEHELPER_H

#include <Arduino.h>

#include "../DataStructs/DeviceStruct.h"

/*********************************************************************************************\
   Get value count from sensor type

   Only use this function to determine nr of output values when changing output type of a task
   To get the actual output values for a task, use getValueCountForTask
\*********************************************************************************************/
uint8_t getValueCountFromSensorType(Sensor_VType sensorType);

bool isSimpleOutputDataType(Sensor_VType sensorType);

const __FlashStringHelper * getSensorTypeLabel(Sensor_VType sensorType);

void sensorTypeHelper_webformLoad_allTypes(struct EventStruct *event, uint8_t pconfigIndex);

void sensorTypeHelper_webformLoad_simple(struct EventStruct *event, uint8_t pconfigIndex);

void sensorTypeHelper_webformLoad(struct EventStruct *event, uint8_t pconfigIndex, int optionCount, const uint8_t options[]);

void sensorTypeHelper_saveOutputSelector(struct EventStruct *event, uint8_t pconfigIndex, uint8_t valueIndex, const String& defaultValueName);

void pconfig_webformSave(struct EventStruct *event, uint8_t pconfigIndex);

void sensorTypeHelper_loadOutputSelector(
  struct EventStruct *event, uint8_t pconfigIndex, uint8_t valuenr,
  int optionCount, const __FlashStringHelper * options[], const int indices[] = nullptr);

void sensorTypeHelper_loadOutputSelector(
  struct EventStruct *event, uint8_t pconfigIndex, uint8_t valuenr,
  int optionCount, const String options[], const int indices[] = nullptr);


#endif // HELPER_CPLUGIN_SENSORTYPEHELPER_H