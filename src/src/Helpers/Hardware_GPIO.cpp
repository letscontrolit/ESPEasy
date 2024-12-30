#include "../Helpers/Hardware_GPIO.h"


#include "../Globals/Settings.h"
#include "../Helpers/Hardware_defines.h"
#include "../Helpers/Hardware_device_info.h"

// ********************************************************************************
// Get info of a specific GPIO pin
// ********************************************************************************


bool isSerialConsolePin(int gpio) {
  if (!Settings.UseSerial) { return false; }

#if defined(SOC_RX0) && defined(SOC_TX0)
  return (gpio == SOC_TX0 || gpio == SOC_RX0)
         #if USES_ESPEASY_CONSOLE_FALLBACK_PORT
         && Settings.console_serial0_fallback
         #endif // if USES_ESPEASY_CONSOLE_FALLBACK_PORT
  ;
#else
  static_assert(false, "Implement processor architecture");
  return false;
#endif
}
