#pragma once

#ifdef ARDUINO
# include <Arduino.h>
#endif

#ifndef MAX_ATTEMPTS_WIFI_CONNECTION
  # define MAX_ATTEMPTS_WIFI_CONNECTION 20
#endif

#ifndef DELAY_MS_WAIT_WIFI_CONNECTION
  # define DELAY_MS_WAIT_WIFI_CONNECTION 500
#endif


#include <cstdint>
#include <functional>
#include <string>
#include <vector>

namespace ImprovTypes {

enum ParseState : uint8_t {
  VALID_INCOMPLETE = 0,
  VALID_COMPLETE = 1,
  INVALID = 2
};

enum Error : uint8_t {
  ERROR_NONE              = 0x00,
  ERROR_INVALID_RPC       = 0x01,
  ERROR_UNKNOWN_RPC       = 0x02,
  ERROR_UNABLE_TO_CONNECT = 0x03,
  ERROR_NOT_AUTHORIZED    = 0x04,
  ERROR_INVALID_CHECKSUM  = 0x05,
  ERROR_EMPTY_SSID        = 0x06,
  ERROR_UNKNOWN           = 0xFF,
};

enum State : uint8_t {
  STATE_STOPPED                = 0x00,
  STATE_AWAITING_AUTHORIZATION = 0x01,
  STATE_AUTHORIZED             = 0x02,
  STATE_PROVISIONING           = 0x03,
  STATE_PROVISIONED            = 0x04,
};

enum Command : uint8_t {
  UNKNOWN           = 0x00,
  WIFI_SETTINGS     = 0x01,
  IDENTIFY          = 0x02,
  GET_CURRENT_STATE = 0x02,
  GET_DEVICE_INFO   = 0x03,
  GET_WIFI_NETWORKS = 0x04,
  BAD_CHECKSUM      = 0xFF,
};

static const uint8_t CAPABILITY_IDENTIFY   = 0x01;
static const uint8_t IMPROV_SERIAL_VERSION = 1;

enum ImprovSerialType : uint8_t {
  TYPE_CURRENT_STATE = 0x01,
  TYPE_ERROR_STATE   = 0x02,
  TYPE_RPC           = 0x03,
  TYPE_RPC_RESPONSE  = 0x04
};

struct ImprovCommand {
  Command     command{};
  std::string ssid;
  std::string password;
};

struct ImprovWiFiParamsStruct {
  std::string firmwareName;
  std::string firmwareVersion;
  std::string deviceName;
  std::string deviceUrl;
  std::string chipVariant =
#if CONFIG_IDF_TARGET_ESP32
    "ESP32"
#elif defined(CONFIG_IDF_TARGET_ESP32S2)
    "ESP32-S2"
#elif defined(CONFIG_IDF_TARGET_ESP32S3)
    "ESP32-S3"
#elif defined(CONFIG_IDF_TARGET_ESP32C2)
    "ESP32-C2"
#elif defined(CONFIG_IDF_TARGET_ESP32C3)
    "ESP32-C3"
#elif defined(CONFIG_IDF_TARGET_ESP32C6)
    "ESP32-C6"
#elif defined(CONFIG_IDF_TARGET_ESP32H2)
    "ESP32-H2"
#elif defined(ESP8266)
    "ESP8266"
#else
    "Unknown"
#endif
  ;
};
}
