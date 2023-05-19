#include "ESPEasySerial_USBCDC.h"

#include "ESPEasySerial_USB.h"

#if USES_USBCDC

volatile bool usbActive = false;

volatile int32_t eventidTriggered = ESP_EVENT_ANY_ID;

// See:
// - https://espressif-docs.readthedocs-hosted.com/projects/arduino-esp32/en/latest/api/usb_cdc.html
// - https://docs.espressif.com/projects/esp-idf/en/v4.3/esp32s2/api-guides/usb-console.html
static void usbcdcEventCallback(void *arg, esp_event_base_t event_base, int32_t event_id, void *event_data) {
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

ESPEasySerial_USBCDC_t::ESPEasySerial_USBCDC_t(ESPEasySerialPort port)
  : _serial(nullptr),
  _mustDelete(false)
{
  int uart_nr = -1;

  if (port == ESPEasySerialPort::usb_cdc_0) {
    uart_nr = 0;
  }
  else if (port == ESPEasySerialPort::usb_cdc_1) {
    uart_nr = 1;
  }

  if (uart_nr > 0) {
    _port = port;

    if ((uart_nr == 0) && (_usbcdc_serial != nullptr)) {
      _serial = _usbcdc_serial;
    } else {
      _serial     = new USBCDC(uart_nr);
      _mustDelete = true;
    }
  }
}

ESPEasySerial_USBCDC_t::~ESPEasySerial_USBCDC_t()
{
  if (_serial != nullptr) {
    _serial->end();

    if (_mustDelete) {
      delete  _serial;
      _serial = nullptr;
    }
  }
}

void ESPEasySerial_USBCDC_t::begin(unsigned long baud)
{
  if (_serial != nullptr) {
    _serial->begin();
    delay(10);
    _serial->onEvent(usbcdcEventCallback);
    delay(1);
  }
}

void ESPEasySerial_USBCDC_t::end() {
  if (_serial != nullptr) {
    _serial->end();
  }
}

int ESPEasySerial_USBCDC_t::available(void)
{
  if (_serial != nullptr) {
    return _serial->available();
  }
  return 0;
}

int ESPEasySerial_USBCDC_t::availableForWrite(void)
{
  if (_serial != nullptr) {
    return _serial->availableForWrite();
  }
  return 0;
}

int ESPEasySerial_USBCDC_t::peek(void)
{
  if (_serial != nullptr) {
    return _serial->peek();
  }
  return 0;
}

int ESPEasySerial_USBCDC_t::read(void)
{
  if (_serial != nullptr) {
    return _serial->read();
  }
  return 0;
}

size_t ESPEasySerial_USBCDC_t::read(uint8_t *buffer,
                                    size_t   size)
{
  if (_serial != nullptr) {
    return _serial->read(buffer, size);
  }
  return 0;
}

void ESPEasySerial_USBCDC_t::flush(void)
{
  if (_serial != nullptr) {
    return _serial->flush();
  }
}

void ESPEasySerial_USBCDC_t::flush(bool txOnly)
{
  flush();
}

size_t ESPEasySerial_USBCDC_t::write(uint8_t value)
{
  if (_serial != nullptr) {
    return _serial->write(value);
  }
  return 0;
}

size_t ESPEasySerial_USBCDC_t::write(const uint8_t *buffer,
                                     size_t         size)
{
  if (_serial != nullptr) {
    return _serial->write(buffer, size);
  }
  return 0;
}

ESPEasySerial_USBCDC_t::operator bool() const
{
  return _serial != nullptr;
}

void ESPEasySerial_USBCDC_t::setDebugOutput(bool enabled) {
  if (_serial != nullptr) {
    return _serial->setDebugOutput(enabled);
  }
}

size_t ESPEasySerial_USBCDC_t::setRxBufferSize(size_t new_size)
{
  if (_serial != nullptr) {
    return _serial->setRxBufferSize(new_size);
  }
  return 0;
}

size_t ESPEasySerial_USBCDC_t::setTxBufferSize(size_t new_size)
{
  if (_serial != nullptr) {
    return new_size;
  }
  return 0;
}

#endif // if USES_USBCDC
