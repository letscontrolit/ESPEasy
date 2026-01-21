#include "../DataStructs/NWPluginData_static_runtime.h"

#ifdef ESP8266

# include "../../../src/Helpers/StringConverter.h"

namespace ESPEasy {
namespace net {

NWPluginData_static_runtime::NWPluginData_static_runtime(bool isAP, const char *eventInterfaceName)
  : _isAP(isAP), _eventInterfaceName(eventInterfaceName) {}

bool NWPluginData_static_runtime::started() const
{
  // FIXME TD-er: Does this work reliable on ESP8266?
  return _startStopStats.isOn();
}

bool NWPluginData_static_runtime::connected() const
{
  if (!_isAP) { return WiFi.status() == WL_CONNECTED; }
  return false;
}

bool NWPluginData_static_runtime::isDefaultRoute() const
{
  // FIXME TD-er: Should I just return connected() here?
  return !_isAP;
}

bool NWPluginData_static_runtime::hasIP() const
{
  return _gotIPStats.isOn();
}

void NWPluginData_static_runtime::mark_start()
{
  _startStopStats.setOn();
  # ifndef BUILD_NO_DEBUG
  addLog(LOG_LEVEL_INFO, _isAP ? F("AP: Started") : F("STA: Started"));
  # endif
}

void NWPluginData_static_runtime::mark_stop()
{
  _startStopStats.setOff();
  # ifndef BUILD_NO_DEBUG
  addLog(LOG_LEVEL_INFO, _isAP ? F("AP: Stopped") : F("STA: Stopped"));
  # endif
}

void NWPluginData_static_runtime::mark_got_IP()
{
  // Set OnOffTimer to off so we can also count how often we het new IP
  _gotIPStats.forceSet(true);
  # ifndef BUILD_NO_DEBUG
  addLog(LOG_LEVEL_INFO, _isAP ? F("AP: Got IP") : F("STA: Got IP"));
  # endif
}

void NWPluginData_static_runtime::mark_lost_IP()
{
  _gotIPStats.setOff();
  # ifndef BUILD_NO_DEBUG
  addLog(LOG_LEVEL_INFO, _isAP ? F("AP: Lost IP") : F("STA: Lost IP"));
  # endif
}

void NWPluginData_static_runtime::mark_begin_establish_connection()
{
  _establishConnectStats.forceSet(true);
  _connectedStats.setOff();
  _operationalStats.setOff();
}

void NWPluginData_static_runtime::mark_connected()
{
  _establishConnectStats.setOff();
  _connectedStats.setOn();
}

void NWPluginData_static_runtime::log_connected()
{
# ifndef BUILD_NO_DEBUG

  if (loglevelActiveFor(LOG_LEVEL_INFO)) {
    if (_establishConnectStats.getCycleCount()) {
      addLog(LOG_LEVEL_INFO, concat(
               F("STA: Connected, took: "),
               format_msec_duration_HMS(
                 _establishConnectStats.getLastOnDuration_ms())));
    } else {
      addLog(LOG_LEVEL_INFO, F("STA: Connected"));
    }
  }
# endif // ifndef BUILD_NO_DEBUG
}

void NWPluginData_static_runtime::mark_disconnected()
{
  _establishConnectStats.setOff();
  _connectedStats.setOff();
  _operationalStats.setOff();
}

void NWPluginData_static_runtime::log_disconnected()
{
# ifndef BUILD_NO_DEBUG

  if (!_isAP && loglevelActiveFor(LOG_LEVEL_INFO)) {
    addLog(LOG_LEVEL_INFO, concat(
             F("STA: Disconnected. Connected for: "),
             format_msec_duration_HMS(_connectedStats.getLastOnDuration_ms())));
  }
# endif // ifndef BUILD_NO_DEBUG
}

} // namespace net
} // namespace ESPEasy

#endif // ifdef ESP8266
