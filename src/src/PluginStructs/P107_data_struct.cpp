#include "../PluginStructs/P107_data_struct.h"

#ifdef USES_P107

bool P107_data_struct::begin()
{
  if (!initialized) {
    initialized = uv.begin();
  }

  return initialized;
}

#endif // ifdef USES_P107
