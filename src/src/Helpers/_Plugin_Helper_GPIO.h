#ifndef HELPERS__PLUGIN_HELPER_GPIO_H
#define HELPERS__PLUGIN_HELPER_GPIO_H


bool GPIO_plugin_helper_init(
  struct EventStruct *event,
  uint32_t            portStatus_key,
  int                 pin,
  int8_t              pinState,
  uint8_t             pinModeValue,
  bool                sendBootState);


#endif // ifndef HELPERS__PLUGIN_HELPER_GPIO_H
