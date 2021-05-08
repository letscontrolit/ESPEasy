#include "../PluginStructs/P115_data_struct.h"

#ifdef USES_P115

P115_data_struct::P115_data_struct(
  uint8_t                i2c_addr,
  sfe_max1704x_devices_e device,
  int                    threshold)
  : lipo(device) {
  lipo.setThreshold(threshold);
}

bool P115_data_struct::begin()
{
  // quickStart() - Restarts the MAX17043 to allow it to re-"guess" the
  // parameters that go into its SoC algorithms. Calling this in your setup()
  // usually results in more accurate SoC readings.
  // Output: 0 on success, positive integer on fail.
  return lipo.quickStart() == 0;
}

bool P115_data_struct::read(bool clearAlert)
{
  voltage = lipo.getVoltage();
  soc     = lipo.getSOC();
  return lipo.getAlert(clearAlert);
}

#endif