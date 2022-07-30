#ifndef PLUGINSTRUCTS_P005_DATA_STRUCT_H
#define PLUGINSTRUCTS_P005_DATA_STRUCT_H

#include "../../_Plugin_Helper.h"
#ifdef USES_P005

# define P005_DHT11    11
# define P005_DHT12    12
# define P005_DHT22    22
# define P005_AM2301   23
# define P005_SI7021   70

struct P005_data_struct : public PluginTaskData_base {
  P005_data_struct(struct EventStruct *event);

  /*********************************************************************************************\
  * DHT sub to wait until a pin is in a certain state
  \*********************************************************************************************/
  bool waitState(int state);

  /*********************************************************************************************\
  * Perform the actual reading + interpreting of data.
  \*********************************************************************************************/
  bool readDHT(struct EventStruct *event);


  int8_t DHT_pin;
  uint8_t SensorModel;
};

#endif // ifdef USES_P005
#endif // ifndef PLUGINSTRUCTS_P005_DATA_STRUCT_H
