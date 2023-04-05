#include "../Helpers/ESPEasy_checks.h"


#include "../../ESPEasy_common.h"
#ifndef BUILD_MINIMAL_OTA

#include "../DataStructs/CRCStruct.h"
#include "../DataStructs/ControllerSettingsStruct.h"
#include "../DataStructs/DeviceStruct.h"
#include "../DataStructs/ESPEasy_EventStruct.h"
#include "../DataStructs/ExtraTaskSettingsStruct.h"
#include "../DataStructs/FactoryDefaultPref.h"
#include "../DataStructs/GpioFactorySettingsStruct.h"
#include "../DataStructs/LogStruct.h"
#if FEATURE_ESPEASY_P2P
#include "../DataStructs/NodeStruct.h"
#endif
#include "../DataStructs/PortStatusStruct.h"
#include "../DataStructs/ProtocolStruct.h"
#if FEATURE_CUSTOM_PROVISIONING
#include "../DataStructs/ProvisioningStruct.h"
#endif
#include "../DataStructs/RTCStruct.h"
#include "../DataStructs/SecurityStruct.h"
#include "../DataStructs/SettingsStruct.h"
#include "../DataStructs/SystemTimerStruct.h"

#include "../Globals/ExtraTaskSettings.h"
#include "../Globals/Settings.h"

#include "../Helpers/ESPEasy_Storage.h"
#include "../Helpers/StringConverter.h"

#include <cstddef>

#ifdef USES_C013
#include "../DataStructs/C013_p2p_dataStructs.h"
#endif

#ifdef USES_C016
#include "../ControllerQueue/C016_queue_element.h"
#endif

#if FEATURE_NOTIFIER
#include "../DataStructs/NotificationStruct.h"
#include "../DataStructs/NotificationSettingsStruct.h"
#endif // if FEATURE_NOTIFIER


// ********************************************************************************
// Check struct sizes at compile time
// Usage:
//   struct foo
//   {
//     char bla[16];
//   };
//
//   check_size<foo, 8>();
// ********************************************************************************
template <typename ToCheck, std::size_t ExpectedSize, std::size_t RealSize = sizeof(ToCheck)>
void check_size() {
  static_assert(ExpectedSize == RealSize, "");
}



// ********************************************************************************
// Check struct sizes at compile time
// Usage:
//   struct X { int a, b, c, d; }
//   static_assert(ExpectedSize == offsetof(X, c), "");
// ********************************************************************************

void run_compiletime_checks() {
  #ifndef LIMIT_BUILD_SIZE
  check_size<CRCStruct,                             204u>();
  check_size<SecurityStruct,                        593u>();
  #ifdef ESP32
  const unsigned int SettingsStructSize = (336 + 84 * TASKS_MAX);
  #endif
  #ifdef ESP8266
  const unsigned int SettingsStructSize = (312 + 84 * TASKS_MAX);
  #endif
  #if FEATURE_CUSTOM_PROVISIONING
  check_size<ProvisioningStruct,                    256u>();  
  #endif
  check_size<SettingsStruct,                        SettingsStructSize>();
  check_size<ControllerSettingsStruct,              820u>();
  #if FEATURE_NOTIFIER
  check_size<NotificationSettingsStruct,            996u>();
  #endif // if FEATURE_NOTIFIER
  check_size<ExtraTaskSettingsStruct,               536u>();
  #if ESP_IDF_VERSION_MAJOR > 3
  // String class has increased with 4 bytes
  check_size<EventStruct,                           120u>(); // Is not stored
  #else
  check_size<EventStruct,                           100u>(); // Is not stored
  #endif


  // LogStruct is mainly dependent on the number of lines.
  // Has to be round up to multiple of 4.
  #if ESP_IDF_VERSION_MAJOR > 3
  // String class has increased with 4 bytes
  const unsigned int LogStructSize = ((13u + 21 * LOG_STRUCT_MESSAGE_LINES) + 3) & ~3;
  #else
  const unsigned int LogStructSize = ((13u + 17 * LOG_STRUCT_MESSAGE_LINES) + 3) & ~3;
  #endif
  check_size<LogStruct,                             LogStructSize>(); // Is not stored
  check_size<DeviceStruct,                          9u>(); // Is not stored
  check_size<ProtocolStruct,                        6u>();
  #if FEATURE_NOTIFIER
  check_size<NotificationStruct,                    3u>();
  #endif // if FEATURE_NOTIFIER
  #if FEATURE_ESPEASY_P2P
  check_size<NodeStruct,                            66u>();
  #endif
  #if FEATURE_CUSTOM_PROVISIONING
  check_size<ProvisioningStruct,                    256u>();
  #endif
  check_size<systemTimerStruct,                     28u>();
  check_size<RTCStruct,                             32u>();
  check_size<portStatusStruct,                      6u>();
  check_size<ResetFactoryDefaultPreference_struct,  4u>();
  check_size<GpioFactorySettingsStruct,             18u>();
  #ifdef USES_C013
  check_size<C013_SensorInfoStruct,                 137u>();
  check_size<C013_SensorDataStruct,                 24u>();
  #endif
  #ifdef USES_C016
  check_size<C016_binary_element,                   24u>();
  #endif


  #if FEATURE_NON_STANDARD_24_TASKS && defined(ESP8266)
    static_assert(TASKS_MAX == 24, "TASKS_MAX invalid size");
  #endif

  // Check for alignment issues at compile time
  {
    const unsigned int ControllerUser_offset = 256u;
    static_assert(ControllerUser_offset == offsetof(SecurityStruct, ControllerUser), "");

    const unsigned int ControllerPassword_offset = 256u + (CONTROLLER_MAX * 26);
    static_assert(ControllerPassword_offset == offsetof(SecurityStruct, ControllerPassword), "");

    const unsigned int Password_offset = ControllerPassword_offset + (CONTROLLER_MAX * 64);
    static_assert(Password_offset == offsetof(SecurityStruct, Password), "");

    const unsigned int AllowedIPrangeLow_offset = Password_offset + 26;
    static_assert(AllowedIPrangeLow_offset == offsetof(SecurityStruct, AllowedIPrangeLow), "");

    const unsigned int IPblockLevel_offset = AllowedIPrangeLow_offset + 8;
    static_assert(IPblockLevel_offset == offsetof(SecurityStruct, IPblockLevel), "");

    const unsigned int ProgmemMd5_offset = IPblockLevel_offset + 1;
    static_assert(ProgmemMd5_offset == offsetof(SecurityStruct, ProgmemMd5), "");

    const unsigned int md5_offset = ProgmemMd5_offset + 16;
    static_assert(md5_offset == offsetof(SecurityStruct, md5), "");

    #if FEATURE_CUSTOM_PROVISIONING
    const unsigned int prov_pass_offset = 62u;
    static_assert(prov_pass_offset == offsetof(ProvisioningStruct, pass), "");


    #endif
  }


  static_assert(192u == offsetof(SettingsStruct, Protocol), "");
  static_assert(195u == offsetof(SettingsStruct, Notification), "CONTROLLER_MAX has changed?");
  static_assert(198u == offsetof(SettingsStruct, TaskDeviceNumber), "NOTIFICATION_MAX has changed?");

  // All settings related to N_TASKS
  static_assert((200 + TASKS_MAX) == offsetof(SettingsStruct, OLD_TaskDeviceID), ""); // 32-bit alignment, so offset of 2 bytes.
  static_assert((200 + (67 * TASKS_MAX)) == offsetof(SettingsStruct, ControllerEnabled), ""); 

  // Used to compute true offset.
  //const size_t offset = offsetof(SettingsStruct, ControllerEnabled);
  //check_size<SettingsStruct, offset>();

  #endif
}

#ifndef LIMIT_BUILD_SIZE
String ReportOffsetErrorInStruct(const String& structname, size_t offset) {
  String error;
  if (error.reserve(48 + structname.length())) {
    error  = F("Error: Incorrect offset in struct: ");
    error += structname;
    error += wrap_braces(String(offset));
  }
  return error;
}
#endif

/*********************************************************************************************\
*  Analyze SettingsStruct and report inconsistencies
*  Not a member function to be able to use the F-macro
\*********************************************************************************************/
bool SettingsCheck(String& error) {
  error = String();
  #ifndef LIMIT_BUILD_SIZE
#ifdef esp8266
  size_t offset = offsetof(SettingsStruct, ResetFactoryDefaultPreference);

  if (offset != 1224) {
    error = ReportOffsetErrorInStruct(F("SettingsStruct"), offset);
  }
#endif // ifdef esp8266

  if (!Settings.networkSettingsEmpty()) {
    if ((Settings.IP[0] == 0) || (Settings.Gateway[0] == 0) || (Settings.Subnet[0] == 0) || (Settings.DNS[0] == 0)) {
      error += F("Error: Either fill all IP settings fields or leave all empty");
    }
  }

  #endif

  return error.isEmpty();
}

#include "../Helpers/Numerical.h"

String checkTaskSettings(taskIndex_t taskIndex) {
  String err = LoadTaskSettings(taskIndex);
  #if !defined(PLUGIN_BUILD_MINIMAL_OTA) && !defined(ESP8266_1M)
  if (err.length() > 0) return err;
  if (!ExtraTaskSettings.checkUniqueValueNames()) {
    return F("Use unique value names");
  }
  if (!ExtraTaskSettings.checkInvalidCharInNames()) {
    return concat(F("Invalid character in name. Do not use space or '"), ExtraTaskSettingsStruct::getInvalidCharsForNames()) + '\'';
  }
  String deviceName = ExtraTaskSettings.TaskDeviceName;
  NumericalType detectedType;
  if (isNumerical(deviceName, detectedType)) {
    return F("Invalid name. Should not be numeric.");
  }
  if (deviceName.isEmpty()) {
    if (Settings.TaskDeviceEnabled[taskIndex]) {
      // Decide what to do here, for now give a warning when task is enabled.
      return F("Warning: Task Device Name is empty. It is adviced to give tasks an unique name");
    }
  }
  // Do not use the cached function findTaskIndexByName since that one does rely on the fact names should be unique.
  for (taskIndex_t i = 0; i < TASKS_MAX; ++i) {
    if (i != taskIndex && Settings.TaskDeviceEnabled[i]) {
      LoadTaskSettings(i);
      if (ExtraTaskSettings.TaskDeviceName[0] != 0) {
        if (strcasecmp(ExtraTaskSettings.TaskDeviceName, deviceName.c_str()) == 0) {
          err = F("Task Device Name is not unique, conflicts with task ID #");
          err += (i+1);
//          return err;
        }
      }
    }
  }

  err += LoadTaskSettings(taskIndex);
  #endif 
  return err;
}
#endif