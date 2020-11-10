#ifndef PLUGINSTRUCTS_P004_DATA_STRUCT_H
#define PLUGINSTRUCTS_P004_DATA_STRUCT_H

#include "../../_Plugin_Helper.h"
#ifdef USES_P004

# include "../../ESPEasy_common.h"

struct P004_data_struct : public PluginTaskData_base {
  P004_data_struct(int8_t        pin,
                   const uint8_t addr[],
                   uint8_t       res);


  bool          initiate_read();

  bool          read_temp(float& value) const;

  String        get_formatted_address() const;

  unsigned long get_timer() const {
    return _timer;
  }

  bool measurement_active() const {
    return _measurementActive;
  }

  void set_measurement_inactive() {
    _measurementActive = false;
  }

private:

  unsigned long _timer             = 0;
  uint8_t       _addr[8]           = { 0 };
  int8_t        _gpio              = -1;
  uint8_t       _res               = 0;
  bool          _measurementActive = false;
};

#endif // ifdef USES_P004
#endif // ifndef PLUGINSTRUCTS_P004_DATA_STRUCT_H
