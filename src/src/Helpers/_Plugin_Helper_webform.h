#ifndef HELPERS__PLUGIN_HELPER_WEBFORM_H
#define HELPERS__PLUGIN_HELPER_WEBFORM_H

#include "../Globals/Plugins.h"

#include "../Helpers/_Plugin_Helper_GPIO.h"
#include "../../ESPEasy_common.h"


void SwitchWebformLoad(
  const int16_t& bootState,
  const float  & debounce_ms,
  const int16_t& doubleClickEvent,
  float        & doubleClickMaxInterval,
  const int16_t& longPressEvent,
  float        & longPressMinInterval_ms,
  const float  & useSafeButton);

void SwitchWebformSave(
  taskIndex_t TaskIndex,
  pluginID_t  pluginID,
  int16_t   & bootState,
  float     & debounce_ms,
  int16_t   & doubleClickEvent,
  float     & doubleClickMaxInterval,
  int16_t   & longPressEvent,
  float     & longPressMinInterval_ms,
  float     & useSafeButton);


#endif // ifndef HELPERS__PLUGIN_HELPER_WEBFORM_H
