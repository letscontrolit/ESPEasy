#ifndef PLUGINSTRUCTS_P004_DATA_STRUCT_H
#define PLUGINSTRUCTS_P004_DATA_STRUCT_H

#include "../../_Plugin_Helper.h"
#ifdef USES_P004

# include "../../ESPEasy_common.h"

struct P004_data_struct : public PluginTaskData_base {
  P004_data_struct(int8_t        pin,
                   const uint8_t addr[],
                   uint8_t       res);


  /*********************************************************************************************\
  *  Dallas Start Temperature Conversion, expected max duration:
  *    9 bits resolution ->  93.75 ms
  *   10 bits resolution -> 187.5 ms
  *   11 bits resolution -> 375 ms
  *   12 bits resolution -> 750 ms
  \*********************************************************************************************/
  void set_timeout();



  unsigned long _timer    = 0;
  uint8_t       _addr[8]  = { 0 };
  int8_t        _gpio     = -1;
  uint8_t       _res      = 0;
  bool          _newValue = false;
};

#endif // ifdef USES_P004
#endif // ifndef PLUGINSTRUCTS_P004_DATA_STRUCT_H
