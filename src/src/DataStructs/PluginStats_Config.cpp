#include "../DataStructs/PluginStats_Config.h"


#if FEATURE_PLUGIN_STATS


PluginStats_Config_t & PluginStats_Config_t::operator=(const PluginStats_Config_t& other)
{
  stored = other.stored;
  return *this;
}

#endif // if FEATURE_PLUGIN_STATS
