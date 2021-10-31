#include "../PluginStructs/P120_data_struct.h"

#ifdef USES_P120

P120_data_struct::P120_data_struct(
  uint8_t                i2c_addr,
  int                    threshold)
  : _threshold(threshold), initialized(false)
{
  // Call begin() immediately, or else _i2cPort in lipo object is still a nullptr
  // lipo.begin();
  // _threshold = constrain(_threshold, 1, 32);
}

// bool P120_data_struct::begin()
// {
  // quickStart() - Restarts the MAX1704x to allow it to re-"guess" the
  // parameters that go into its SoC algorithms. Calling this in your setup()
  // usually results in more accurate SoC readings.
  // Output: 0 on success, positive integer on fail.

  // FIXME TD-er: Looks like begin() and/or quickStart() may not return expected values.
  // const bool success = lipo.begin() && (lipo.quickStart() == 0);
  // lipo.begin();
  // lipo.quickStart();
//   const bool success = true;

//   if (success) {
//     switch (_device) {
//       case MAX1704X_MAX17043:
//       case MAX1704X_MAX17044:
//         break;
//       case MAX1704X_MAX17048:
//       case MAX1704X_MAX17049:
//         lipo.enableSOCAlert();
//         break;
//     }
//     lipo.setThreshold(_threshold);
//     initialized = true;
//     return true;
//   }
//   initialized = false;
//   return false;
// }

// bool P120_data_struct::read(bool clearAlert)
// {
//   voltage    = lipo.getVoltage();
//   soc        = lipo.getSOC();
//   changeRate = lipo.getChangeRate();
//   return lipo.getAlert(clearAlert);
// }

// void P120_data_struct::clearAlert() {
//   alert = false;
//   lipo.clearAlert();
// }

#endif // ifdef USES_P120
