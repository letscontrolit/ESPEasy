#include "../Helpers/ESPEasyMutex.h"

#ifdef ESP8266

  // ESP8266 doesn't have a proper mutex system.
  // This is by no means a perfect implementation, but the assembly implementation was causing lots of issues with esp8266/Arduino 3.0.0
  // See: https://github.com/letscontrolit/ESPEasy/issues/3693#issuecomment-868847669
  ESPEasy_Mutex::ESPEasy_Mutex() {
    _mutex = 1;
  }

#endif