#include "../ESPEasyCore/ESPEasy_Console.h"


#include "../Commands/InternalCommands.h"

#include "../DataTypes/ESPEasy_plugin_functions.h"

#include "../ESPEasyCore/ESPEasy_USB.h"

#include "../Globals/Cache.h"
#include "../Globals/Logging.h"
#include "../Globals/Plugins.h"
#include "../Globals/Settings.h"

#include "../Helpers/Memory.h"

/*
 #if FEATURE_DEFINE_SERIAL_CONSOLE_PORT
 # include "../Helpers/_Plugin_Helper_serial.h"
 #endif // if FEATURE_DEFINE_SERIAL_CONSOLE_PORT
 */


#if USES_HWCDC
volatile bool usbActive = false;

volatile int32_t eventidTriggered = ESP_EVENT_ANY_ID;

static void usbEventCallback(void *arg, esp_event_base_t event_base, int32_t event_id, void *event_data) {
  arduino_hw_cdc_event_data_t *data = (arduino_hw_cdc_event_data_t *)event_data;

  eventidTriggered = event_id;

  switch (event_id) {
    case ARDUINO_HW_CDC_CONNECTED_EVENT:

      usbActive = true;
      break;
    case ARDUINO_HW_CDC_BUS_RESET_EVENT:
      // Serial.println("CDC BUS RESET");
      break;
    case ARDUINO_HW_CDC_RX_EVENT:
      /*
         Serial.printf("CDC RX EVENT [%u]: ", data->rx.len);
         {
          uint8_t buf[data->rx.len];
          size_t len = Serial.read(buf, data->rx.len);
          Serial.write(buf, len);
         }
         Serial.println();
       */
      usbActive = true;
      break;
    case ARDUINO_HW_CDC_TX_EVENT:
      usbActive = true;

      // No example provided
      break;
    case ARDUINO_HW_CDC_MAX_EVENT:
      // No example provided
      break;
    default:
      break;
  }
}

#endif // if USES_HWCDC


#if USES_USBCDC
volatile bool usbActive = false;

volatile int32_t eventidTriggered = ESP_EVENT_ANY_ID;

// See: 
// - https://espressif-docs.readthedocs-hosted.com/projects/arduino-esp32/en/latest/api/usb_cdc.html
// - https://docs.espressif.com/projects/esp-idf/en/v4.3/esp32s2/api-guides/usb-console.html
static void usbEventCallback(void *arg, esp_event_base_t event_base, int32_t event_id, void *event_data) {
  if (event_base == ARDUINO_USB_EVENTS) {
    arduino_usb_event_data_t *data = (arduino_usb_event_data_t *)event_data;

    switch (event_id) {
      case ARDUINO_USB_STARTED_EVENT:
//        HWSerial.println("USB PLUGGED");
        break;
      case ARDUINO_USB_STOPPED_EVENT:
//        HWSerial.println("USB UNPLUGGED");
        break;
      case ARDUINO_USB_SUSPEND_EVENT:
//        HWSerial.printf("USB SUSPENDED: remote_wakeup_en: %u\n", data->suspend.remote_wakeup_en);
        break;
      case ARDUINO_USB_RESUME_EVENT:
//        HWSerial.println("USB RESUMED");
        break;

      default:
        break;
    }
  } else if (event_base == ARDUINO_USB_CDC_EVENTS) {
    arduino_usb_cdc_event_t *data = (arduino_usb_cdc_event_t *)event_data;

    eventidTriggered = event_id;

    switch (event_id) {
      case ARDUINO_USB_CDC_CONNECTED_EVENT:

        usbActive = true;
        break;
      case ARDUINO_USB_CDC_DISCONNECTED_EVENT:
        // Sent when serial port disconnects
        usbActive = true;
        break;
      case ARDUINO_USB_CDC_LINE_STATE_EVENT:
        usbActive = true;
        break;
      case ARDUINO_USB_CDC_LINE_CODING_EVENT:
        // Sent when USB is initialized by USB host (e.g. PC)
        usbActive = true;
        break;
      case ARDUINO_USB_CDC_RX_EVENT:
        /*
           Serial.printf("CDC RX EVENT [%u]: ", data->rx.len);
           {
            uint8_t buf[data->rx.len];
            size_t len = Serial.read(buf, data->rx.len);
            Serial.write(buf, len);
           }
           Serial.println();
         */
        usbActive = true;
        break;
      case ARDUINO_USB_CDC_TX_EVENT:
        usbActive = true;

        // No example provided
        break;
      case ARDUINO_USB_CDC_RX_OVERFLOW_EVENT:
        // No example provided
        break;
      default:
        break;
    }
  }
}

#endif // if USES_USBCDC


EspEasy_Console_t::EspEasy_Console_t() :
#if USES_HWCDC
  _defaultPortActive(true)
#elif USES_USBCDC
  _defaultPortActive(false)
#else // if USES_HWCDC
  _defaultPortActive(true)
#endif // if USES_HWCDC
{
#if USES_HWCDC
  usbActive = false;
#endif // if USES_HWCDC
#if USES_USBCDC
  usbActive = true;
#endif // if USES_USBCDC
#if FEATURE_DEFINE_SERIAL_CONSOLE_PORT
  _serial = new ESPeasySerial(
    static_cast<ESPEasySerialPort>(_console_serial_port),
    _console_serial_rxpin,
    _console_serial_txpin,
    false,
    64);

#endif // if FEATURE_DEFINE_SERIAL_CONSOLE_PORT


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
   #    warning **** ESPEasy_Console uses HWCDC ****
   #   else // No ARDUINO_USB_MODE
  // ESP32Sx embedded USB interface
   #    warning **** ESPEasy_Console uses USBCDC ****
   #   endif  // ARDUINO_USB_MODE

   #  else // No USE_USB_CDC_CONSOLE
  // Fallback serial interface for ESP32C3, S2 and S3 if no USB_SERIAL defined
   #   warning **** ESPEasy_Console uses Serial ****
   #  endif  // USE_USB_CDC_CONSOLE

   # else // No ESP32C3, S2 or S3
  // Fallback serial interface for non ESP32C3, S2 and S3
   #  warning **** ESPEasy_Console uses Serial ****
   # endif  // ESP32C3, S2 or S3

   #else // No ESP32
  // Using the standard Serial0 HW serial port.
   # warning **** ESPEasy_Console uses Serial ****
#endif // ifdef ESP32

#if USES_HWCDC
  _hwcdc_serial->onEvent(usbEventCallback);
#endif // if USES_HWCDC
#if USES_USBCDC
  _usbcdc_serial->onEvent(usbEventCallback);
#endif // if USES_USBCDC
}

EspEasy_Console_t::~EspEasy_Console_t() {
#if FEATURE_DEFINE_SERIAL_CONSOLE_PORT

  if (_serial != nullptr) {
    delete _serial;
    _serial = nullptr;
  }
#endif // if FEATURE_DEFINE_SERIAL_CONSOLE_PORT
}

void EspEasy_Console_t::begin(uint32_t baudrate)
{
  _baudrate = baudrate;

  if (_defaultPortActive) {
#if FEATURE_DEFINE_SERIAL_CONSOLE_PORT
    _serial->begin(baudrate);
    addLog(LOG_LEVEL_INFO, F("ESPEasy console using ESPEasySerial"));
#else // if FEATURE_DEFINE_SERIAL_CONSOLE_PORT
# ifdef ESP8266
    _serial->begin(baudrate);
    addLog(LOG_LEVEL_INFO, F("ESPEasy console using HW Serial"));
# endif // ifdef ESP8266
# ifdef ESP32

    // Allow to flush data from the serial buffers
    // When not opening the USB serial port, the ESP may hang at boot.
    delay(10);
    _serial->end();
    delay(10);
    _serial->begin(baudrate);
    _serial->flush();
# endif // ifdef ESP32
#endif  // if FEATURE_DEFINE_SERIAL_CONSOLE_PORT
  }

  // else
  {
#if USES_USBCDC
    _usbcdc_serial->setRxBufferSize(64);
    _usbcdc_serial->begin(baudrate);
    _usbcdc_serial->onEvent(usbEventCallback);
    USB.begin();
    addLog(LOG_LEVEL_INFO, F("ESPEasy console using USB CDC"));
#endif // if USES_USBCDC
#if USES_HWCDC
    _hwcdc_serial->begin();
    delay(10);
    _hwcdc_serial->onEvent(usbEventCallback);
    delay(1);

    addLog(LOG_LEVEL_INFO, F("ESPEasy console using HWCDC"));

#endif // if USES_HWCDC
  }
}

void EspEasy_Console_t::init() {
  updateActiveTaskUseSerial0();

  if (!Settings.UseSerial) {
    return;
  }

#if FEATURE_DEFINE_SERIAL_CONSOLE_PORT

  // FIXME TD-er: Must detect whether we should swap software serial on pin 3&1 for HW serial if Serial0 is not being used anymore.
  const ESPEasySerialPort port = static_cast<ESPEasySerialPort>(Settings.console_serial_port);

  if ((port == ESPEasySerialPort::serial0) || (port == ESPEasySerialPort::serial0_swap)) {
    if (activeTaskUseSerial0()) {
      return;
    }
  }
#else // if FEATURE_DEFINE_SERIAL_CONSOLE_PORT

  if (activeTaskUseSerial0()) {
    return;
  }
#endif // if FEATURE_DEFINE_SERIAL_CONSOLE_PORT

  if (log_to_serial_disabled) {
    return;
  }


#if FEATURE_DEFINE_SERIAL_CONSOLE_PORT

  //  if (ESPEASY_SERIAL_CONSOLE_PORT.getSerialPortType() == ESPEasySerialPort::not_set)

  /*
     if (Settings.console_serial_port != console_serial_port ||
        Settings.console_serial_rxpin != console_serial_rxpin ||
        Settings.console_serial_txpin != console_serial_txpin) {

      const uint8_t current_console_serial_port = console_serial_port;
      // Update cached values
      console_serial_port  = Settings.console_serial_port;
      console_serial_rxpin = Settings.console_serial_rxpin;
      console_serial_txpin = Settings.console_serial_txpin;

   #ifdef ESP8266
      bool forceSWSerial = static_cast<ESPEasySerialPort>(Settings.console_serial_port) == ESPEasySerialPort::software;
      if (activeTaskUseSerial0()) {
        if (ESPEasySerialPort::sc16is752 != static_cast<ESPEasySerialPort>(console_serial_port)) {
          forceSWSerial = true;
          console_serial_port = static_cast<uint8_t>(ESPEasySerialPort::software);
        }
      }
   #endif

      if (loglevelActiveFor(LOG_LEVEL_INFO)) {
        String log = F("Serial : Change serial console port from: ");
        log += ESPEasySerialPort_toString(static_cast<ESPEasySerialPort>(current_console_serial_port));
        log += F(" to: ");
        log += ESPEasySerialPort_toString(static_cast<ESPEasySerialPort>(console_serial_port));
   #ifdef ESP8266
        if (forceSWSerial) {
          log += F(" (force SW serial)");
        }
   #endif
        addLogMove(LOG_LEVEL_INFO, log);
      }
      process_serialWriteBuffer();


      ESPEASY_SERIAL_CONSOLE_PORT.resetConfig(
        static_cast<ESPEasySerialPort>(console_serial_port),
        console_serial_rxpin,
        console_serial_txpin,
        false,
        64
   #ifdef ESP8266
        , forceSWSerial
   #endif
        );
     }
   */
#endif // if FEATURE_DEFINE_SERIAL_CONSOLE_PORT

  #if USES_HWCDC || USES_USBCDC

  if (_defaultPortActive == usbActive) {
    endPort();
    _defaultPortActive = false;
  }
  #endif // if USES_HWCDC


  begin(Settings.BaudRate);
}

void EspEasy_Console_t::loop()
{
#if USES_HWCDC || USES_USBCDC

  if (eventidTriggered != ESP_EVENT_ANY_ID) {
    # if USES_USBCDC

    if (eventidTriggered != ARDUINO_USB_CDC_TX_EVENT) {
      String log = F("USBCDC : EventID: ");
    # else // if USES_USBCDC

    if (eventidTriggered != ARDUINO_HW_CDC_TX_EVENT) {
      String log = F("HWCDC : EventID: ");
    # endif // if USES_USBCDC

      log             += eventidTriggered;
      eventidTriggered = ESP_EVENT_ANY_ID;
      addLogMove(LOG_LEVEL_INFO, log);
    }
  }
  check_HWCDC_Port();
#endif // if USES_HWCDC || USES_USBCDC

  Stream *port = getPort();

  if (port == nullptr) {
    return;
  }

  if (_defaultPortActive && port->available())
  {
    String dummy;

    if (PluginCall(PLUGIN_SERIAL_IN, 0, dummy)) {
      return;
    }
  }
#if FEATURE_DEFINE_SERIAL_CONSOLE_PORT

  // FIXME TD-er: Must add check whether SW serial may be using the same pins as Serial0
  if (!Settings.UseSerial) { return; }
#else // if FEATURE_DEFINE_SERIAL_CONSOLE_PORT

  if (!Settings.UseSerial || activeTaskUseSerial0()) { return; }
#endif // if FEATURE_DEFINE_SERIAL_CONSOLE_PORT

  while (port->available())
  {
    delay(0);
    const uint8_t SerialInByte = port->read();

    if (SerialInByte == 255) // binary data...
    {
      port->flush();
      return;
    }

    if (isprint(SerialInByte))
    {
      if (SerialInByteCounter < CONSOLE_INPUT_BUFFER_SIZE) { // add char to string if it still fits
        InputBuffer_Serial[SerialInByteCounter++] = SerialInByte;
      }
    }

    if ((SerialInByte == '\r') || (SerialInByte == '\n'))
    {
      if (SerialInByteCounter == 0) {              // empty command?
        break;
      }
      InputBuffer_Serial[SerialInByteCounter] = 0; // serial data completed
      addToSerialBuffer('>');
      addToSerialBuffer(String(InputBuffer_Serial));
      ExecuteCommand_all(EventValueSource::Enum::VALUE_SOURCE_SERIAL, InputBuffer_Serial);
      SerialInByteCounter   = 0;
      InputBuffer_Serial[0] = 0; // serial data processed, clear buffer
    }
  }
}

void EspEasy_Console_t::check_HWCDC_Port()
{
#if USES_HWCDC || USES_USBCDC

  if (_defaultPortActive == !usbActive) {
    return;
  }

  # if USES_HWCDC
  addLog(LOG_LEVEL_INFO, F("HWCDC  : Enable output to USB HWCDC"));
  # else // if USES_HWCDC
  addLog(LOG_LEVEL_INFO, F("USBCDC : Enable output to USB CDC"));
  # endif // if USES_HWCDC

  endPort();
  _defaultPortActive = false;

  begin(_baudrate);
#endif // if USES_HWCDC || USES_USBCDC
}

int EspEasy_Console_t::getRoomLeft() const {
  #ifdef USE_SECOND_HEAP

  // If stored in 2nd heap, we must check this for space
  HeapSelectIram ephemeral;
  #endif // ifdef USE_SECOND_HEAP

  int roomLeft = getMaxFreeBlock();

  if (roomLeft < 1000) {
    roomLeft = 0;                               // Do not append to buffer.
  } else if (roomLeft < 4000) {
    roomLeft = 128 - _serialWriteBuffer.size(); // 1 buffer.
  } else {
    roomLeft -= 4000;                           // leave some free for normal use.
  }
  return roomLeft;
}

void EspEasy_Console_t::addToSerialBuffer(const __FlashStringHelper *line)
{
  addToSerialBuffer(String(line));
}

void EspEasy_Console_t::addToSerialBuffer(char c)
{
  addToSerialBuffer(String(c));
}

void EspEasy_Console_t::addToSerialBuffer(const String& line) {
  // When the buffer is too full, try to dump at least the size of what we try to add.
  const bool mustPop = !process_serialWriteBuffer() && _serialWriteBuffer.size() > 10000;
  {
    #ifdef USE_SECOND_HEAP

    // Allow to store the logs in 2nd heap if present.
    HeapSelectIram ephemeral;
    #endif // ifdef USE_SECOND_HEAP
    int roomLeft = getRoomLeft();

    auto it = line.begin();

    while (roomLeft > 0 && it != line.end()) {
      if (mustPop) {
        _serialWriteBuffer.pop_front();
      }
      _serialWriteBuffer.push_back(*it);
      --roomLeft;
      ++it;
    }
  }
  process_serialWriteBuffer();
}

void EspEasy_Console_t::addNewlineToSerialBuffer() {
  addToSerialBuffer(F("\r\n"));
}

bool EspEasy_Console_t::process_serialWriteBuffer() {
  const size_t bufferSize = _serialWriteBuffer.size();

  if (bufferSize == 0) {
    return true;
  }
  Stream *port = getPort();

  if (port == nullptr) {
    return false;
  }

  const int snip = availableForWrite();

  if (snip > 0) {
    size_t bytes_to_write = bufferSize;

    if (snip < bytes_to_write) { bytes_to_write = snip; }

    while (bytes_to_write > 0 && !_serialWriteBuffer.empty()) {
      const char c = _serialWriteBuffer.front();

      if (Settings.UseSerial) {
        port->write(c);
      }
      _serialWriteBuffer.pop_front();
      --bytes_to_write;
    }
  }

  return bufferSize != _serialWriteBuffer.size();
}

void EspEasy_Console_t::setDebugOutput(bool enable)
{
  if (_defaultPortActive) {
    _serial->setDebugOutput(enable);
  }
}

Stream * EspEasy_Console_t::getPort()
{
  if (_defaultPortActive) {
    return _serial;
  }
    #if USES_HWCDC
  return _hwcdc_serial;
    #elif USES_USBCDC
  return _usbcdc_serial;
    #else // if USES_HWCDC
  return nullptr;
    #endif // if USES_HWCDC
}

const Stream * EspEasy_Console_t::getPort() const
{
  if (_defaultPortActive) {
    return _serial;
  }
    #if USES_HWCDC
  return _hwcdc_serial;
    #elif USES_USBCDC
  return _usbcdc_serial;
    #else // if USES_HWCDC
  return nullptr;
    #endif // if USES_HWCDC
}

void EspEasy_Console_t::endPort()
{
  if (_defaultPortActive) {
    if (_serial != nullptr) {
      _serial->end();
    }
  }
#if USES_HWCDC

  //  _hwcdc_serial->end();
#elif USES_USBCDC
  _usbcdc_serial->end();
#endif // if USES_HWCDC
  delay(10);
}

int EspEasy_Console_t::availableForWrite()
{
  if (_defaultPortActive) {
    if (_serial != nullptr) {
      return _serial->availableForWrite();
    }
    return 0;
  }
    #if USES_HWCDC
  return _hwcdc_serial->availableForWrite();
    #elif USES_USBCDC
  return _usbcdc_serial->availableForWrite();
    #else // if USES_HWCDC
  return 0;
    #endif // if USES_HWCDC
}
