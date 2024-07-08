#include "../DataStructs/C016_binary_element.h"

#ifdef USES_C016

pluginID_t C016_binary_element::getPluginID() const
{
  const uint16_t value =
    static_cast<uint16_t>(valueCount_and_pluginID_msb & 0xF0) << 4 +
      pluginID_lsb;

  return pluginID_t::toPluginID(value);
}

void C016_binary_element::setPluginID(pluginID_t pluginID)
{
  pluginID_lsb                = (pluginID.value & 0xFF);
  valueCount_and_pluginID_msb =
    (valueCount_and_pluginID_msb & 0x0F) +
    ((pluginID.value >> 4) & 0xF0);
}

uint8_t C016_binary_element::getValueCount() const
{
  return valueCount_and_pluginID_msb & 0x0F;
}

void C016_binary_element::setValueCount(uint8_t valueCount)
{
  valueCount_and_pluginID_msb =
    (valueCount_and_pluginID_msb & 0xF0) +
    valueCount & 0x0F;
}

#endif // ifdef USES_C016
