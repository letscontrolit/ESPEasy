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

  void addToSerialBuffer(const __FlashStringHelper *line);
  void addToSerialBuffer(const String& line);
  void addToSerialBuffer(char c);

  void addNewlineToSerialBuffer();

  // Return true when something got written, or when the buffer was already empty
  bool process_serialWriteBuffer();

  void setDebugOutput(bool enable);

  String getPortDescription() const;

#if USES_ESPEASY_CONSOLE_FALLBACK_PORT
  String getFallbackPortDescription() const;
#endif


private:

  bool handledByPluginSerialIn();

  void readInput(EspEasy_Console_Port& port);

#if FEATURE_DEFINE_SERIAL_CONSOLE_PORT
  ESPeasySerial * getPort();
#else // if FEATURE_DEFINE_SERIAL_CONSOLE_PORT
  HardwareSerial* getPort();
#endif // if FEATURE_DEFINE_SERIAL_CONSOLE_PORT

  void            endPort();

  int             availableForWrite();

  uint32_t _baudrate = 115200u;
  EspEasy_Console_Port _mainSerial;

#if FEATURE_DEFINE_SERIAL_CONSOLE_PORT
# if USES_ESPEASY_CONSOLE_FALLBACK_PORT
  EspEasy_Console_Port _fallbackSerial;
# endif

  // Cache the used settings, so we can check whether to change the console serial
  uint8_t _console_serial_port = DEFAULT_CONSOLE_PORT;
  int8_t _console_serial_rxpin = DEFAULT_CONSOLE_PORT_RXPIN;
  int8_t _console_serial_txpin = DEFAULT_CONSOLE_PORT_TXPIN;
#endif
};


#endif // ifndef ESPEASYCORE_ESPEASY_CONSOLE_H
