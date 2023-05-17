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

  size_t  availableForWrite() const;

  bool _defaultPortActive = true;

#define INPUT_BUFFER_SIZE          128

  int SerialInByteCounter{};
  char InputBuffer_Serial[INPUT_BUFFER_SIZE + 2]{};


  std::deque<char>_serialWriteBuffer;

  uint32_t _baudrate = 115200u;

#if FEATURE_DEFINE_SERIAL_CONSOLE_PORT

  // Cache the used settings, so we can check whether to change the console serial
  uint8_t _console_serial_port = 2; // ESPEasySerialPort::serial0
  int8_t _console_serial_rxpin = 3;
  int8_t _console_serial_txpin = 1;
  ESPeasySerial *_serial      = nullptr;

#else // if FEATURE_DEFINE_SERIAL_CONSOLE_PORT

  HardwareSerial *_serial = &Serial;
#endif  // if FEATURE_DEFINE_SERIAL_CONSOLE_PORT


#ifdef ESP32

  /*
   #if CONFIG_IDF_TARGET_ESP32C3 ||  // support USB via HWCDC using JTAG interface
       CONFIG_IDF_TARGET_ESP32S2 ||  // support USB via USBCDC
       CONFIG_IDF_TARGET_ESP32S3     // support USB via HWCDC using JTAG interface or USBCDC
   */
# if CONFIG_IDF_TARGET_ESP32C3 || CONFIG_IDF_TARGET_ESP32S2 || CONFIG_IDF_TARGET_ESP32S3

  // #if CONFIG_TINYUSB_CDC_ENABLED              // This define is not recognized here so use USE_USB_CDC_CONSOLE
#  ifdef USE_USB_CDC_CONSOLE
#   if ARDUINO_USB_MODE

  // ESP32C3/S3 embedded USB using JTAG interface
#    include "HWCDC.h"
#    define CONSOLE_USES_HWCDC 1
  HWCDC _hwcdc_serial;
#   else // No ARDUINO_USB_MODE
#    include "USB.h"
#    include "USBCDC.h"
#    define CONSOLE_USES_USBCDC 1
  USBCDC _usbcdc_serial;
#   endif // ARDUINO_USB_MODE
#  endif  // ifdef USE_USB_CDC_CONSOLE
# endif   // if CONFIG_IDF_TARGET_ESP32C3 || CONFIG_IDF_TARGET_ESP32S2 || CONFIG_IDF_TARGET_ESP32S3
#endif    // ifdef ESP32

#ifndef CONSOLE_USES_HWCDC
# define CONSOLE_USES_HWCDC 0
#endif // ifndef CONSOLE_USES_HWCDC

#ifndef CONSOLE_USES_USBCDC
# define CONSOLE_USES_USBCDC 0
#endif // ifndef CONSOLE_USES_USBCDC
};


#endif // ifndef ESPEASYCORE_ESPEASY_CONSOLE_H
