#ifndef ESPEASYCORE_ESPEASY_CONSOLE_H
#define ESPEASYCORE_ESPEASY_CONSOLE_H

#include "../../ESPEasy_common.h"

#include "../Helpers/SerialWriteBuffer.h"

#if FEATURE_DEFINE_SERIAL_CONSOLE_PORT
# include <ESPeasySerial.h>
#else // if FEATURE_DEFINE_SERIAL_CONSOLE_PORT
# include <HardwareSerial.h>
#endif // if FEATURE_DEFINE_SERIAL_CONSOLE_PORT

#include <deque>

#if FEATURE_IMPROV
#include "../Helpers/Improv_Helper.h"
#endif



class EspEasy_Console_t {
public:

  EspEasy_Console_t();

  ~EspEasy_Console_t();

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

  void readInput(Stream& stream);

#if FEATURE_DEFINE_SERIAL_CONSOLE_PORT
  ESPeasySerial * getPort();
#else // if FEATURE_DEFINE_SERIAL_CONSOLE_PORT
  HardwareSerial* getPort();
#endif // if FEATURE_DEFINE_SERIAL_CONSOLE_PORT

  void            endPort();

  int             availableForWrite();


#define CONSOLE_INPUT_BUFFER_SIZE          128

  int SerialInByteCounter{};
  char InputBuffer_Serial[CONSOLE_INPUT_BUFFER_SIZE + 2]{};


  SerialWriteBuffer_t _serialWriteBuffer;

  uint32_t _baudrate = 115200u;

#if FEATURE_DEFINE_SERIAL_CONSOLE_PORT

  // Cache the used settings, so we can check whether to change the console serial
  uint8_t _console_serial_port = DEFAULT_CONSOLE_PORT;
  int8_t _console_serial_rxpin = DEFAULT_CONSOLE_PORT_RXPIN;
  int8_t _console_serial_txpin = DEFAULT_CONSOLE_PORT_TXPIN;
  ESPeasySerial *_serial       = nullptr;
# if USES_ESPEASY_CONSOLE_FALLBACK_PORT
  ESPeasySerial *_serial_fallback = nullptr;
  SerialWriteBuffer_t _serial_fallback_WriteBuffer;
# endif // if USES_ESPEASY_CONSOLE_FALLBACK_PORT

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

  /*
   #if ARDUINO_USB_MODE
   #if ARDUINO_USB_CDC_ON_BOOT//Serial used for USB CDC
     HWCDC Serial;
   #else
     HWCDC USBSerial;
   #endif
   #endif
   */
};


#endif // ifndef ESPEASYCORE_ESPEASY_CONSOLE_H
