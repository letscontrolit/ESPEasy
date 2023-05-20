#include "ESPEasySerial_USB_HWCDC.h"

#if USES_HWCDC

# include "../drivers/ESPEasySerial_USB.h"

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

ESPEasySerial_USB_WHCDC_t::ESPEasySerial_USB_WHCDC_t()
{
  _config.port = ESPEasySerialPort::usb_hw_cdc;
}

void ESPEasySerial_USB_WHCDC_t::begin(unsigned long baud)
{
  if (_hwcdc_serial != nullptr) {
    _hwcdc_serial->begin();
    delay(10);
    _hwcdc_serial->onEvent(hwcdcEventCallback);
    delay(1);
  }
}

void ESPEasySerial_USB_WHCDC_t::end() {
  // Disabled for now
  // See: https://github.com/espressif/arduino-esp32/issues/8224
  if (_hwcdc_serial != nullptr) {
    //    _hwcdc_serial->end();
  }
}

int ESPEasySerial_USB_WHCDC_t::available(void)
{
  if (_hwcdc_serial != nullptr) {
    return _hwcdc_serial->available();
  }
  return 0;
}

int ESPEasySerial_USB_WHCDC_t::availableForWrite(void)
{
  if (_hwcdc_serial != nullptr) {
    return _hwcdc_serial->availableForWrite();
  }
  return 0;
}

int ESPEasySerial_USB_WHCDC_t::peek(void)
{
  if (_hwcdc_serial != nullptr) {
    return _hwcdc_serial->peek();
  }
  return 0;
}

int ESPEasySerial_USB_WHCDC_t::read(void)
{
  if (_hwcdc_serial != nullptr) {
    return _hwcdc_serial->read();
  }
  return 0;
}

size_t ESPEasySerial_USB_WHCDC_t::read(uint8_t *buffer,
                                       size_t   size)
{
  if (_hwcdc_serial != nullptr) {
    return _hwcdc_serial->read(buffer, size);
  }
  return 0;
}

void ESPEasySerial_USB_WHCDC_t::flush(void)
{
  if (_hwcdc_serial != nullptr) {
    return _hwcdc_serial->flush();
  }
}

void ESPEasySerial_USB_WHCDC_t::flush(bool txOnly)
{
  flush();
}

size_t ESPEasySerial_USB_WHCDC_t::write(uint8_t value)
{
  if (_hwcdc_serial != nullptr) {
    return _hwcdc_serial->write(value);
  }
  return 0;
}

size_t ESPEasySerial_USB_WHCDC_t::write(const uint8_t *buffer,
                                        size_t         size)
{
  if (_hwcdc_serial != nullptr) {
    return _hwcdc_serial->write(buffer, size);
  }
  return 0;
}

ESPEasySerial_USB_WHCDC_t::operator bool() const
{
  return _hwcdc_serial != nullptr;
}

void ESPEasySerial_USB_WHCDC_t::setDebugOutput(bool enabled) {
  if (_hwcdc_serial != nullptr) {
    return _hwcdc_serial->setDebugOutput(enabled);
  }
}

size_t ESPEasySerial_USB_WHCDC_t::setRxBufferSize(size_t new_size)
{
  if (_hwcdc_serial != nullptr) {
    return _hwcdc_serial->setRxBufferSize(new_size);
  }
  return 0;
}

size_t ESPEasySerial_USB_WHCDC_t::setTxBufferSize(size_t new_size)
{
  if (_hwcdc_serial != nullptr) {
    return _hwcdc_serial->setTxBufferSize(new_size);
  }
  return 0;
}

#endif // if USES_HWCDC
