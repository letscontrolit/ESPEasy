#ifndef ESPEASYCORE_ESPEASY_CONSOLE_PORT_H
#define ESPEASYCORE_ESPEASY_CONSOLE_PORT_H


#include "../../ESPEasy_common.h"

#include "../Helpers/SerialWriteBuffer.h"

#if FEATURE_DEFINE_SERIAL_CONSOLE_PORT
# include <ESPeasySerial.h>
#else // if FEATURE_DEFINE_SERIAL_CONSOLE_PORT
# include <HardwareSerial.h>
#endif // if FEATURE_DEFINE_SERIAL_CONSOLE_PORT

#if FEATURE_IMPROV
#include "../Helpers/Improv_Helper.h"
#endif


#define CONSOLE_INPUT_BUFFER_SIZE          128

struct EspEasy_Console_Port {
  ~EspEasy_Console_Port();

  operator bool() const;

  int read();
  size_t available() const;

  void endPort();

  void addToSerialBuffer(char c);
  void addToSerialBuffer(const String& line);
  void addNewlineToSerialBuffer();
  bool process_serialWriteBuffer();

  bool process_consoleInput(uint8_t SerialInByte);

  String getPortDescription() const;

  int SerialInByteCounter{};
  char InputBuffer_Serial[CONSOLE_INPUT_BUFFER_SIZE + 2]{};
  SerialWriteBuffer_t _serialWriteBuffer;


#if FEATURE_DEFINE_SERIAL_CONSOLE_PORT
  ESPeasySerial *_serial       = nullptr;
#else // if FEATURE_DEFINE_SERIAL_CONSOLE_PORT
# if !defined(NO_GLOBAL_INSTANCES) && !defined(NO_GLOBAL_SERIAL) && ARDUINO_USB_CDC_ON_BOOT // Serial used for USB CDC
  HardwareSerial *_serial = &Serial0;
# else // if !defined(NO_GLOBAL_INSTANCES) && !defined(NO_GLOBAL_SERIAL) && ARDUINO_USB_CDC_ON_BOOT
  HardwareSerial *_serial = &Serial;
# endif // if !defined(NO_GLOBAL_INSTANCES) && !defined(NO_GLOBAL_SERIAL) && ARDUINO_USB_CDC_ON_BOOT
#endif // if FEATURE_DEFINE_SERIAL_CONSOLE_PORT
#if FEATURE_IMPROV

  Improv_Helper_t _improv;

#endif

};


#endif