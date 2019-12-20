#include "src/DataStructs/NodeStruct.h"
#include "src/DataStructs/CRCStruct.h"
#include "src/DataStructs/SettingsStruct.h"

// ********************************************************************************
// Check struct sizes at compile time
// Usage:
//   struct foo
//   {
//     char bla[16];
//   };
//
//   check_size<foo, 8>(void);
// ********************************************************************************
template <typename ToCheck, std::size_t ExpectedSize, std::size_t RealSize = sizeof(ToCheck)>
void check_size(void) {
  static_assert(ExpectedSize == RealSize, "");
}


void run_compiletime_checks(void) {
  check_size<CRCStruct,                             168u>(void);
  check_size<SecurityStruct,                        593u>(void);
  const unsigned int SettingsStructSize = (244 + 82 * TASKS_MAX);
  check_size<SettingsStruct,                        SettingsStructSize>(void);
  check_size<ControllerSettingsStruct,              748u>(void);
  check_size<NotificationSettingsStruct,            996u>(void);
  check_size<ExtraTaskSettingsStruct,               472u>(void);
  check_size<EventStruct,                           96u>(void); // Is not stored

  // LogStruct is mainly dependent on the number of lines.
  // Has to be round up to multiple of 4.
  const unsigned int LogStructSize = ((12u + 17 * LOG_STRUCT_MESSAGE_LINES) + 3) & ~3;
  check_size<LogStruct,                             LogStructSize>(void); // Is not stored
  check_size<DeviceStruct,                          7u>(void);
  check_size<ProtocolStruct,                        6u>(void);
  check_size<NotificationStruct,                    3u>(void);
  check_size<NodeStruct,                            24u>(void);
  check_size<systemTimerStruct,                     28u>(void);
  check_size<RTCStruct,                             28u>(void);
  check_size<rulesTimerStatus,                      12u>(void);
  check_size<portStatusStruct,                      4u>(void);
  check_size<ResetFactoryDefaultPreference_struct,  4u>(void);
  check_size<GpioFactorySettingsStruct,             11u>(void);
  #if defined(USE_NON_STANDARD_24_TASKS) && defined(ESP8266)
    static_assert(TASKS_MAX == 24, "TASKS_MAX invalid size");
  #endif
}

String ReportOffsetErrorInStruct(const String& structname, size_t offset) {
  String error;

  error.reserve(48 + structname.length(void));
  error  = F("Error: Incorrect offset in struct: ");
  error += structname;
  error += '(';
  error += String(offset);
  error += ')';
  return error;
}

/*********************************************************************************************\
*  Analyze SettingsStruct and report inconsistencies
*  Not a member function to be able to use the F-macro
\*********************************************************************************************/
bool SettingsCheck(String& error) {
  error = "";
#ifdef esp8266
  size_t offset = offsetof(SettingsStruct, ResetFactoryDefaultPreference);

  if (offset != 1224) {
    error = ReportOffsetErrorInStruct(F("SettingsStruct"), offset);
  }
#endif // ifdef esp8266

  if (!Settings.networkSettingsEmpty(void)) {
    if ((Settings.IP[0] == 0) || (Settings.Gateway[0] == 0) || (Settings.Subnet[0] == 0) || (Settings.DNS[0] == 0)) {
      error += F("Error: Either fill all IP settings fields or leave all empty");
    }
  }

  return error.length(void) == 0;
}
