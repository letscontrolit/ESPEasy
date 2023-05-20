#ifndef ESPEASYCORE_ESPEASY_CONSOLE_H
#define ESPEASYCORE_ESPEASY_CONSOLE_H

#include "../../ESPEasy_common.h"


#if FEATURE_DEFINE_SERIAL_CONSOLE_PORT
# include <ESPeasySerial.h>
#else // if FEATURE_DEFINE_SERIAL_CONSOLE_PORT
# include <HardwareSerial.h>
#endif // if FEATURE_DEFINE_SERIAL_CONSOLE_PORT


#include <deque>




class EspEasy_Console_t {
public:

  EspEasy_Console_t();

  ~EspEasy_Console_t();

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

private:

  int           getRoomLeft() const;

  Stream      * getPort();
  const Stream* getPort() const;

  void endPort();

  int  availableForWrite();

  bool _defaultPortActive = true;

#define CONSOLE_INPUT_BUFFER_SIZE          128

  int SerialInByteCounter{};
  char InputBuffer_Serial[CONSOLE_INPUT_BUFFER_SIZE + 2]{};


  std::deque<char>_serialWriteBuffer;

  uint32_t _baudrate = 115200u;

#if FEATURE_DEFINE_SERIAL_CONSOLE_PORT

  // Cache the used settings, so we can check whether to change the console serial
  uint8_t _console_serial_port = 2; // ESPEasySerialPort::serial0
  int8_t _console_serial_rxpin = 3;
  int8_t _console_serial_txpin = 1;
  ESPeasySerial *_serial      = nullptr;

#else // if FEATURE_DEFINE_SERIAL_CONSOLE_PORT

#if !defined(NO_GLOBAL_INSTANCES) && !defined(NO_GLOBAL_SERIAL) && ARDUINO_USB_CDC_ON_BOOT //Serial used for USB CDC
  HardwareSerial *_serial = &Serial0;
#else
  HardwareSerial *_serial = &Serial;
#endif
#endif  // if FEATURE_DEFINE_SERIAL_CONSOLE_PORT


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
