#ifndef PLUGINSTRUCTS__STATSONLY_DATA_STRUCT_H
#define PLUGINSTRUCTS__STATSONLY_DATA_STRUCT_H

#include "../../_Plugin_Helper.h"

#ifdef USES_PLUGIN_STATS

// Dummy class to be able to use the PluginStats for plugins that don't (yet) have their own PluginTaskData.
struct _StatsOnly_data_struct  : public PluginTaskData_base {
    _StatsOnly_data_struct();

};

#endif
#endif