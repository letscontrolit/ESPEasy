#ifndef ESPEASYCORE_ESPEASY_CONSOLE_H
#define ESPEASYCORE_ESPEASY_CONSOLE_H

#include "../../ESPEasy_common.h"

#include "../ESPEasyCore/ESPEasy_Console_Port.h"

#ifdef ESP8266
# define ESPEASY_CONSOLE_TX_BUFFSIZE  256
#endif // ifdef ESP8266
#ifdef ESP32

// Ideal buffer size is a trade-off between bootspeed
// and not missing data when the ESP is busy processing stuff.
// Since we do have a separate buffer in the console,
// it may just take less time in the background tasks to dump
// any logs as larger chunks can be transferred at once.
# define ESPEASY_CONSOLE_TX_BUFFSIZE  1024
#endif   // ifdef ESP32


class EspEasy_Console_t
{
public:

  EspEasy_Console_t();


  // Typically called after settings have been loaded.
  void   reInit();

  void   begin(uint32_t baudrate);

  void   init();

  // Process data from serial port
  void   loop();

  // Return true when something got written, or when the buffer was already empty
  bool   process_serialWriteBuffer();

  void   setDebugOutput(bool enable);

  String getPortDescription() const;

#if USES_ESPEASY_CONSOLE_FALLBACK_PORT
  String getFallbackPortDescription() const;
#endif

private:

#if FEATURE_DEFINE_SERIAL_CONSOLE_PORT
  ESPeasySerial*  getPort();
#else // if FEATURE_DEFINE_SERIAL_CONSOLE_PORT
  HardwareSerial* getPort();
#endif // if FEATURE_DEFINE_SERIAL_CONSOLE_PORT

  void            endPort();

  EspEasy_Console_Port _mainSerial;

#if FEATURE_DEFINE_SERIAL_CONSOLE_PORT
# if USES_ESPEASY_CONSOLE_FALLBACK_PORT

  // Serial port to be always used as HW Serial0
  EspEasy_Console_Port _fallbackSerial;
# endif // if USES_ESPEASY_CONSOLE_FALLBACK_PORT
#endif // if FEATURE_DEFINE_SERIAL_CONSOLE_PORT
}; // class EspEasy_Console_t


#endif // ifndef ESPEASYCORE_ESPEASY_CONSOLE_H
