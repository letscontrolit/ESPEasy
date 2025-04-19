#include "Port_ESPEasySerial_USB_HWCDC.h"

#if USES_HWCDC

// #include <USB.h>

volatile bool usbActive = false;

volatile int32_t eventidTriggered = ESP_EVENT_ANY_ID;

static void hwcdcEventCallback(void *arg, esp_event_base_t event_base, int32_t event_id, void *event_data) {
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

Port_ESPEasySerial_USB_HWCDC_t::Port_ESPEasySerial_USB_HWCDC_t(const ESPEasySerialConfig& config)
  :
# if ARDUINO_USB_CDC_ON_BOOT // Serial used for USB CDC
  _hwcdc_serial(&Serial)
# else // if ARDUINO_USB_CDC_ON_BOOT
  _hwcdc_serial(&USBSerial)
# endif // if ARDUINO_USB_CDC_ON_BOOT
{
  _config.port = ESPEasySerialPort::usb_hw_cdc;

  //  USB.begin();
  if (_hwcdc_serial != nullptr) {
    // _hwcdc_serial->end();

    //    _config.rxBuffSize = _hwcdc_serial->setRxBufferSize(_config.rxBuffSize);
    //    _config.txBuffSize = _hwcdc_serial->setTxBufferSize(_config.txBuffSize);

    // See: https://github.com/espressif/arduino-esp32/issues/9043
    //    _hwcdc_serial->setTxTimeoutMs(0);       // sets no timeout when trying to write to USB HW CDC

    //    _hwcdc_serial->begin();

    //    _hwcdc_serial->onEvent(hwcdcEventCallback);
  }
}

Port_ESPEasySerial_USB_HWCDC_t::~Port_ESPEasySerial_USB_HWCDC_t() {
  if (_hwcdc_serial != nullptr) {
    // _hwcdc_serial->end();
  }
}

void Port_ESPEasySerial_USB_HWCDC_t::begin(unsigned long baud)
{
  _config.baud = baud;

  if (_hwcdc_serial != nullptr) {
    _config.rxBuffSize = _hwcdc_serial->setRxBufferSize(_config.rxBuffSize);
    _config.txBuffSize = _hwcdc_serial->setTxBufferSize(_config.txBuffSize);
    _hwcdc_serial->begin();
    delay(10);

    //   _hwcdc_serial->onEvent(hwcdcEventCallback);
    delay(1);
  }
}

void Port_ESPEasySerial_USB_HWCDC_t::end() {
  // Disabled for now
  // See: https://github.com/espressif/arduino-esp32/issues/8224
  if (_hwcdc_serial != nullptr) {
    // _hwcdc_serial->end();
  }
}

int Port_ESPEasySerial_USB_HWCDC_t::available(void)
{
  if (_hwcdc_serial != nullptr) {
    return _hwcdc_serial->available();
  }
  return 0;
}

int Port_ESPEasySerial_USB_HWCDC_t::availableForWrite(void)
{
  if (_hwcdc_serial != nullptr) {
    const int res = _hwcdc_serial->availableForWrite();
    return res > 64 ? 64 : res;
  }
  return 0;
}

int Port_ESPEasySerial_USB_HWCDC_t::peek(void)
{
  if (_hwcdc_serial != nullptr) {
    return _hwcdc_serial->peek();
  }
  return 0;
}

int Port_ESPEasySerial_USB_HWCDC_t::read(void)
{
  if (_hwcdc_serial != nullptr) {
    return _hwcdc_serial->read();
  }
  return 0;
}

size_t Port_ESPEasySerial_USB_HWCDC_t::read(uint8_t *buffer,
                                            size_t   size)
{
  if (_hwcdc_serial != nullptr) {
    return _hwcdc_serial->read(buffer, size);
  }
  return 0;
}

void Port_ESPEasySerial_USB_HWCDC_t::flush(void)
{
  if (_hwcdc_serial != nullptr) {
    return _hwcdc_serial->flush();
  }
}

void Port_ESPEasySerial_USB_HWCDC_t::flush(bool txOnly)
{
  flush();
}

size_t Port_ESPEasySerial_USB_HWCDC_t::write(uint8_t value)
{
  if (operator bool()) {
    return _hwcdc_serial->write(value);
  }
  return 0;
}

size_t Port_ESPEasySerial_USB_HWCDC_t::write(const uint8_t *buffer,
                                             size_t         size)
{
  if (operator bool()) {
    return _hwcdc_serial->write(buffer, size);
  }
  return 0;
}

Port_ESPEasySerial_USB_HWCDC_t::operator bool() const
{
  if (_hwcdc_serial != nullptr) {
    // return usbActive;
    const bool connected = (*_hwcdc_serial);
    return connected;
  }
  return false;
}

void Port_ESPEasySerial_USB_HWCDC_t::setDebugOutput(bool enabled) {
  if (_hwcdc_serial != nullptr) {
    return _hwcdc_serial->setDebugOutput(enabled);
  }
}

size_t Port_ESPEasySerial_USB_HWCDC_t::setRxBufferSize(size_t new_size)
{
  if (_hwcdc_serial != nullptr) {
    _config.rxBuffSize = _hwcdc_serial->setRxBufferSize(new_size);
    return _config.rxBuffSize;
  }
  return 0;
}

size_t Port_ESPEasySerial_USB_HWCDC_t::setTxBufferSize(size_t new_size)
{
  if (_hwcdc_serial != nullptr) {
    _config.txBuffSize = _hwcdc_serial->setTxBufferSize(new_size);
    return _config.txBuffSize;
  }
  return 0;
}

bool Port_ESPEasySerial_USB_HWCDC_t::setRS485Mode(int8_t rtsPin, bool enableCollisionDetection)
{
  return false;
}

#endif // if USES_HWCDC
