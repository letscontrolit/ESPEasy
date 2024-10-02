#include "../DataStructs/PluginStats_Config.h"


#if FEATURE_PLUGIN_STATS

PluginStats_Config_t::PluginStats_Config_t(const PluginStats_Config_t& other)
{
  setStored(other.getStored());
}

PluginStats_Config_t & PluginStats_Config_t::operator=(const PluginStats_Config_t& other)
{
  setStored(other.getStored());
  return *this;
}

#endif // if FEATURE_PLUGIN_STATS
