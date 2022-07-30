#ifndef PLUGINSTRUCTS_P005_DATA_STRUCT_H
#define PLUGINSTRUCTS_P005_DATA_STRUCT_H

#include "../../_Plugin_Helper.h"
#ifdef USES_P005

# define P005_DHT11    11
# define P005_DHT12    12
# define P005_DHT22    22
# define P005_AM2301   23
# define P005_SI7021   70

# define P005_error_no_reading          1
# define P005_error_protocol_timeout    2
# define P005_error_checksum_error      3
# define P005_error_invalid_NAN_reading 4
# define P005_info_temperature          5
# define P005_info_humidity             6


struct P005_data_struct : public PluginTaskData_base {
  P005_data_struct(struct EventStruct *event);

  /*********************************************************************************************\
  * DHT sub to log an error
  \*********************************************************************************************/
  void P005_log(struct EventStruct *event,
                int                 logNr);

  /*********************************************************************************************\
  * DHT sub to wait until a pin is in a certain state
  \*********************************************************************************************/
  bool P005_waitState(int state);

  /*********************************************************************************************\
  * Perform the actual reading + interpreting of data.
  \*********************************************************************************************/
  bool P005_do_plugin_read(struct EventStruct *event);


  /*********************************************************************************************\
  * DHT sub to get an 8 bit value from the receiving bitstream
  \*********************************************************************************************/
  int Plugin_005_read_dht_dat(void);


  uint8_t Plugin_005_DHT_Pin;
  uint8_t Par3;
};

#endif // ifdef USES_P005
#endif // ifndef PLUGINSTRUCTS_P005_DATA_STRUCT_H
