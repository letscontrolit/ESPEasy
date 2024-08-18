#ifndef HELPERS__PLUGIN_HELPER_GPIO_H
#define HELPERS__PLUGIN_HELPER_GPIO_H


#include "../../_Plugin_Helper.h"

struct GPIO_plugin_helper_data_t {
  GPIO_plugin_helper_data_t(
    pluginID_t pluginNumber,
    uint16_t   pin,
    uint32_t   debounceInterval_ms,
    uint32_t   doubleClickMaxInterval_ms,
    uint32_t   longpressMinInterval_ms,
    uint8_t    dcMode,
    bool       safeButton,
    bool       sendBootState);

  ~GPIO_plugin_helper_data_t();

  bool init(
    struct EventStruct *event,
    int8_t              pinState,
    uint8_t             pinModeValue);

  uint32_t       _portStatus_key;
  uint32_t       _debounceTimer;
  const uint32_t _debounceInterval_ms;
  uint32_t       _doubleClickTimer;
  const uint32_t _doubleClickMaxInterval_ms;
  uint32_t       _longpressTimer;
  const uint32_t _longpressMinInterval_ms;

  int16_t _doubleClickCounter;
  int16_t _safeButtonCounter;

  const uint16_t _pin;

  const uint8_t _dcMode; // use doubleclick (0,1,2,3)
  const bool    _safeButton;
  bool          _longpressFired;
  const  bool   _sendBootState;
};


/*
   void GPIO_plugin_helper_tenPerSecond(
   struct EventStruct        *event,
   const __FlashStringHelper *prefix,
   GPIO_plugin_helper_data_t& data,
   int8_t                     pinState);
 */
#endif // ifndef HELPERS__PLUGIN_HELPER_GPIO_H
