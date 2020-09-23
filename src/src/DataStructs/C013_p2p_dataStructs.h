#ifndef DATASTRUCTS_C013_P2P_DATASTRUCTS_H
#define DATASTRUCTS_C013_P2P_DATASTRUCTS_H

#include <Arduino.h>

#include "ESPEasyLimits.h"

// These structs are sent to other nodes, so make sure not to change order or offset in struct.


struct C013_SensorInfoStruct
{
  C013_SensorInfoStruct();

  bool isValid() const;

  byte header = 255;
  byte ID     = 3;
  byte sourceUnit;
  byte destUnit;
  byte sourceTaskIndex;
  byte destTaskIndex;
  byte deviceNumber;
  char taskName[26];
  char ValueNames[VARS_PER_TASK][26];
};

struct C013_SensorDataStruct
{
  C013_SensorDataStruct();

  bool isValid() const;
  
  byte  header = 255;
  byte  ID     = 5;
  byte  sourceUnit;
  byte  destUnit;
  byte  sourceTaskIndex;
  byte  destTaskIndex;
  float Values[VARS_PER_TASK];
};


#endif // DATASTRUCTS_C013_P2P_DATASTRUCTS_H
