#ifndef ESPEASYCORE_ESPEASY_CONSOLE_H
#define ESPEASYCORE_ESPEASY_CONSOLE_H

#include "../../ESPEasy_common.h"

#include "../ESPEasyCore/ESPEasy_Console_Port.h"

class EspEasy_Console_t {
public:

  EspEasy_Console_t();


  // Typically called after settings have been loaded.
  void reInit();

  void begin(uint32_t baudrate);

  void init();

  // Process data from serial port
  void loop();

  // Return true when something got written, or when the buffer was already empty
  bool process_serialWriteBuffer();

  void setDebugOutput(bool enable);

  String getPortDescription() const;

#if USES_ESPEASY_CONSOLE_FALLBACK_PORT
  String getFallbackPortDescription() const;
#endif


private:

  bool handledByPluginSerialIn();

#if FEATURE_DEFINE_SERIAL_CONSOLE_PORT
  ESPeasySerial * getPort();
#else // if FEATURE_DEFINE_SERIAL_CONSOLE_PORT
  HardwareSerial* getPort();
#endif // if FEATURE_DEFINE_SERIAL_CONSOLE_PORT

  void            endPort();

  EspEasy_Console_Port _mainSerial;

#if FEATURE_DEFINE_SERIAL_CONSOLE_PORT
# if USES_ESPEASY_CONSOLE_FALLBACK_PORT
  // Serial port to be always used as HW Serial0
  EspEasy_Console_Port _fallbackSerial;
# endif
#endif
};


#endif // ifndef ESPEASYCORE_ESPEASY_CONSOLE_H
