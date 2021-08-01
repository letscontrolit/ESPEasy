#include "../PluginStructs/P039_data_struct.h"

#ifdef USES_P039

P039_data_struct::P039_data_struct(
      uint16_t       l_conversionResult,
      uint8_t        l_devicefaults,
      unsigned long  l_timer,
      bool           l_sensorFault, 
      bool           l_convReady)
  :  conversionResult(l_conversionResult), deviceFaults(l_devicefaults), timer(l_timer), sensorFault(l_sensorFault), convReady(l_convReady) {}

bool P039_data_struct::begin()
{
  return false;
}

bool P039_data_struct::read()
{
 return false;
}

bool P039_data_struct::write()
{
 return false;
}

#endif // ifdef USES_P039