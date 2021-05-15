#include "../PluginStructs/P039_data_struct.h"

#ifdef USES_P039

P039_data_struct::P039_data_struct(
      uint8_t        l_mainState,
      uint8_t        l_command,
      uint16_t       l_conversionResult,
      uint8_t        l_devicefaults,
      unsigned long  l_timer, 
      bool           l_convReady)
  : mainState(l_mainState), command(l_command), conversionResult(l_conversionResult), deviceFaults(l_devicefaults), timer(l_timer), convReady(l_convReady) {}

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