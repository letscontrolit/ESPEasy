#ifndef PLUGINSTRUCTS_P151_DATA_STRUCT_H
#define PLUGINSTRUCTS_P151_DATA_STRUCT_H

#include "../../_Plugin_Helper.h"

#ifdef USES_P151
# include "../Helpers/I2C_access.h"

# define P151_MIN_PRESSURE_VALUE    -1e9f
# define P151_MAX_PRESSURE_VALUE    1e9f

# define P151_I2C_ADDR                        PCONFIG(0)
#define P151_OUTPUT_MIN  PCONFIG(1)
#define P151_OUTPUT_MAX  PCONFIG(2)
#define P151_PRESSURE_MIN  PCONFIG_FLOAT(0)
#define P151_PRESSURE_MAX  PCONFIG_FLOAT(1)



/*******************************************
 * P151 Plugin taskdata struct
 ******************************************/
struct P151_data_struct : public PluginTaskData_base {
public:

  P151_data_struct() = default;
  ~P151_data_struct();

  bool plugin_read(struct EventStruct *event) const;
  bool plugin_ten_per_second(struct EventStruct *event);
  bool plugin_fifty_per_second(struct EventStruct *event);

};

#endif // ifdef USES_P151
#endif // ifndef PLUGINSTRUCTS_P151_DATA_STRUCT_H
