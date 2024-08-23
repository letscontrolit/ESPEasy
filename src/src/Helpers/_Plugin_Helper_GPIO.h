#ifndef HELPERS__PLUGIN_HELPER_GPIO_H
#define HELPERS__PLUGIN_HELPER_GPIO_H


#include "../../_Plugin_Helper.h"

// Do not change these values as they are stored in P001/P009/P019
#define SWITCH_DC_DISABLED                   0
#define SWITCH_DC_LOW                        1
#define SWITCH_DC_HIGH                       2
#define SWITCH_DC_BOTH                       3
#define SWITCH_LONGPRESS_DISABLED            0
#define SWITCH_LONGPRESS_LOW                 1
#define SWITCH_LONGPRESS_HIGH                2
#define SWITCH_LONGPRESS_BOTH                3
#define SWITCH_TYPE_NORMAL_SWITCH            0
#define SWITCH_TYPE_PUSH_ACTIVE_LOW          1
#define SWITCH_TYPE_PUSH_ACTIVE_HIGH         2
#define SWITCH_TYPE_DIMMER                   255


struct GPIO_plugin_helper_data_t {
  GPIO_plugin_helper_data_t() = delete;
  GPIO_plugin_helper_data_t(
    pluginID_t pluginNumber,
    uint16_t   pin,
    uint32_t   debounceInterval_ms,
    uint32_t   doubleClickMaxInterval_ms,
    uint32_t   longpressMinInterval_ms,
    uint8_t    dcMode,
    uint8_t    longpressEvent,
    bool       safeButton,
    bool       sendBootState,
    uint8_t    switchType  = SWITCH_TYPE_NORMAL_SWITCH,
    uint8_t    dimmerValue = 0);

  ~GPIO_plugin_helper_data_t();

  bool init(struct EventStruct *event,
            int8_t              pinState,
            uint8_t             pinModeValue);

  void tenPerSecond(
    struct EventStruct        *event,
    const __FlashStringHelper *monitorEventString,
    int8_t                     pinState);


  const uint32_t _portStatus_key;
  const uint32_t _debounceInterval_ms;
  const uint32_t _doubleClickMaxInterval_ms;
  const uint32_t _longpressMinInterval_ms;
  uint32_t       _debounceTimer;
  uint32_t       _doubleClickTimer;
  uint32_t       _longpressTimer;

  int16_t _doubleClickCounter;
  int16_t _safeButtonCounter;

  const uint16_t _pin;

  const pluginID_t _pluginNumber;
  const uint8_t    _dcMode;     // use doubleclick (0,1,2,3)
  const uint8_t    _longpressEvent;
  const uint8_t    _switchType; // button option (normal, push high, push low)
  const uint8_t    _dimmerValue;
  const bool       _safeButton;
  const bool       _sendBootState;
  bool             _longpressFired;
};

#endif // ifndef HELPERS__PLUGIN_HELPER_GPIO_H
