#include "Port_ESPEasySerial_USBCDC.h"


#if USES_USBCDC

# if !ARDUINO_USB_CDC_ON_BOOT
USBCDC ESPEasySerial_USBCDC_port0(0);

// USBCDC ESPEasySerial_USBCDC_port1(1);
# endif // if !ARDUINO_USB_CDC_ON_BOOT

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

Port_ESPEasySerial_USBCDC_t::Port_ESPEasySerial_USBCDC_t(const ESPEasySerialConfig& config)
{
  # if ARDUINO_USB_CDC_ON_BOOT

  # else // if ARDUINO_USB_CDC_ON_BOOT
  _serial = nullptr;

  if (config.port == ESPEasySerialPort::usb_cdc_0) {
    _serial = &ESPEasySerial_USBCDC_port0;
  }

  /*
     else if (config.port == ESPEasySerialPort::usb_cdc_1) {
     _serial = &ESPEasySerial_USBCDC_port1;
     }
   */
  # endif // if ARDUINO_USB_CDC_ON_BOOT

  if (_serial != nullptr) {
    _config.port = config.port;

    //    USB.onEvent(usbcdcEventCallback);
    //    _serial->onEvent(usbcdcEventCallback);
    delay(10);
    _config.rxBuffSize = _serial->setRxBufferSize(_config.rxBuffSize);

    // TD-er: No setTxBufferSize()
    //    _config.txBuffSize = _serial->setTxBufferSize(_config.txBuffSize);
    _serial->begin();

    //    _serial->onEvent(usbcdcEventCallback);
    //    USB.begin();
    delay(1);
  }
}

Port_ESPEasySerial_USBCDC_t::~Port_ESPEasySerial_USBCDC_t()
{
  if (_serial != nullptr) {
    _serial->end();
  }
}

void Port_ESPEasySerial_USBCDC_t::begin(unsigned long baud)
{  
  if (_serial != nullptr) {
    _config.baud = baud;
    //    USB.onEvent(usbcdcEventCallback);
    //    _serial->onEvent(usbcdcEventCallback);
    delay(10);
    _config.rxBuffSize = _serial->setRxBufferSize(_config.rxBuffSize);

    // TD-er: No setTxBufferSize()
    //    _config.txBuffSize = _serial->setTxBufferSize(_config.txBuffSize);
    _serial->begin();

    //    _serial->onEvent(usbcdcEventCallback);
    //    USB.begin();
    delay(1);
  }
}

void Port_ESPEasySerial_USBCDC_t::end() {
  if (_serial != nullptr) {
    _serial->end();
  }
}

int Port_ESPEasySerial_USBCDC_t::available(void)
{
  if (_serial != nullptr) {
    return _serial->available();
  }
  return 0;
}

int Port_ESPEasySerial_USBCDC_t::availableForWrite(void)
{
  if (_serial != nullptr) {
    const int res = _serial->availableForWrite();
    return res > 64 ? 64 : res;
  }
  return 0;
}

int Port_ESPEasySerial_USBCDC_t::peek(void)
{
  if (_serial != nullptr) {
    return _serial->peek();
  }
  return 0;
}

int Port_ESPEasySerial_USBCDC_t::read(void)
{
  if (_serial != nullptr) {
    return _serial->read();
  }
  return 0;
}

size_t Port_ESPEasySerial_USBCDC_t::read(uint8_t *buffer,
                                         size_t   size)
{
  if (_serial != nullptr) {
    return _serial->read(buffer, size);
  }
  return 0;
}

void Port_ESPEasySerial_USBCDC_t::flush(void)
{
  if (_serial != nullptr) {
    return _serial->flush();
  }
}

void Port_ESPEasySerial_USBCDC_t::flush(bool txOnly)
{
  flush();
}

size_t Port_ESPEasySerial_USBCDC_t::write(uint8_t value)
{
  if (_serial != nullptr) {
    return _serial->write(value);
  }
  return 0;
}

size_t Port_ESPEasySerial_USBCDC_t::write(const uint8_t *buffer,
                                          size_t         size)
{
  if (_serial != nullptr) {
    return _serial->write(buffer, size);
  }
  return 0;
}

Port_ESPEasySerial_USBCDC_t::operator bool() const
{
  if (_serial != nullptr) {
    const bool res = (*_serial);
    return res;

    //    return  usbActive;
  }
  return false;
}

void Port_ESPEasySerial_USBCDC_t::setDebugOutput(bool enabled) {
  if (_serial != nullptr) {
    return _serial->setDebugOutput(enabled);
  }
}

size_t Port_ESPEasySerial_USBCDC_t::setRxBufferSize(size_t new_size)
{
  if (_serial != nullptr) {
    _config.rxBuffSize = _serial->setRxBufferSize(new_size);
    return _config.rxBuffSize;
  }
  return 0;
}

size_t Port_ESPEasySerial_USBCDC_t::setTxBufferSize(size_t new_size)
{
  // TD-er: No setTxBufferSize()

  /*
     if (_serial != nullptr) {
      _config.txBuffSize = _serial->setTxBufferSize(new_size);
      return _config.txBuffSize;
     }
   */
  return 0;
}

bool Port_ESPEasySerial_USBCDC_t::setRS485Mode(int8_t rtsPin, bool enableCollisionDetection)
{
  return false;
}


int Port_ESPEasySerial_USBCDC_t::getBaudRate() const
{
  if (_serial != nullptr) {
    return _serial->baudRate();
  }
  return 0;
}

#endif // if USES_USBCDC
