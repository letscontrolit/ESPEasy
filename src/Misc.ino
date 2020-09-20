#include "ESPEasy_common.h"

#include "src/DataStructs/Caches.h"
#include "src/DataStructs/NodeStruct.h"
#include "src/DataStructs/PinMode.h"
#include "src/DataStructs/RTCStruct.h"
#include "src/DataStructs/StorageLayout.h"

#include "src/Globals/CRCValues.h"
#include "src/Globals/Cache.h"
#include "src/Globals/Device.h"
#include "src/Globals/ESPEasy_Scheduler.h"
#include "src/Globals/CPlugins.h"
#include "src/Globals/Plugins.h"
#include "src/Globals/Plugins_other.h"
#include "src/Globals/RTC.h"
#include "src/Globals/ResetFactoryDefaultPref.h"
#include "src/Globals/Services.h"
#include "src/Globals/Settings.h"

#include "src/Helpers/Hardware.h"
#include "src/Helpers/Convert.h"

#include "ESPEasyWifi.h"


#ifdef ESP32
 
  //MFD: adding tone support here while waiting for the Arduino Espressif implementation to catch up
  //As recomandation is not to use external libraries the following code was taken from: https://github.com/lbernstone/Tone Thanks
  #define TONE_CHANNEL 15

  void noToneESP32(uint8_t pin, uint8_t channel=TONE_CHANNEL)
  {
      ledcDetachPin(pin);
      ledcWrite(channel, 0);
  }

  void toneESP32(uint8_t pin, unsigned int frequency, unsigned long duration, uint8_t channel=TONE_CHANNEL)
  {
      if (ledcRead(channel)) {
          log_e("Tone channel %d is already in use", ledcRead(channel));
          return;
      }
      ledcAttachPin(pin, channel);
      ledcWriteTone(channel, frequency);
      if (duration) {
          delay(duration);
          noToneESP32(pin, channel);
      }    
  }


#endif
/*********************************************************************************************\
   ESPEasy specific strings
\*********************************************************************************************/


String getNodeTypeDisplayString(byte nodeType) {
  switch (nodeType)
  {
    case NODE_TYPE_ID_ESP_EASY_STD:     return F("ESP Easy");
    case NODE_TYPE_ID_ESP_EASYM_STD:    return F("ESP Easy Mega");
    case NODE_TYPE_ID_ESP_EASY32_STD:   return F("ESP Easy 32");
    case NODE_TYPE_ID_RPI_EASY_STD:     return F("RPI Easy");
    case NODE_TYPE_ID_ARDUINO_EASY_STD: return F("Arduino Easy");
    case NODE_TYPE_ID_NANO_EASY_STD:    return F("Nano Easy");
  }
  return "";
}


#ifdef USES_MQTT
String getMQTT_state() {
  switch (MQTTclient.state()) {
    case MQTT_CONNECTION_TIMEOUT     : return F("Connection timeout");
    case MQTT_CONNECTION_LOST        : return F("Connection lost");
    case MQTT_CONNECT_FAILED         : return F("Connect failed");
    case MQTT_DISCONNECTED           : return F("Disconnected");
    case MQTT_CONNECTED              : return F("Connected");
    case MQTT_CONNECT_BAD_PROTOCOL   : return F("Connect bad protocol");
    case MQTT_CONNECT_BAD_CLIENT_ID  : return F("Connect bad client_id");
    case MQTT_CONNECT_UNAVAILABLE    : return F("Connect unavailable");
    case MQTT_CONNECT_BAD_CREDENTIALS: return F("Connect bad credentials");
    case MQTT_CONNECT_UNAUTHORIZED   : return F("Connect unauthorized");
    default: break;
  }
  return "";
}
#endif //USES_MQTT

/********************************************************************************************\
  Get system information
  \*********************************************************************************************/
String getLastBootCauseString() {
  switch (lastBootCause)
  {
    case BOOT_CAUSE_MANUAL_REBOOT: return F("Manual reboot");
    case BOOT_CAUSE_DEEP_SLEEP: //nobody should ever see this, since it should sleep again right away.
       return F("Deep sleep");
    case BOOT_CAUSE_COLD_BOOT:
       return F("Cold boot");
    case BOOT_CAUSE_EXT_WD:
       return F("External Watchdog");
  }
  return getUnknownString();
}

#ifdef ESP32
// See https://github.com/espressif/esp-idf/blob/master/components/esp32/include/rom/rtc.h
String getResetReasonString(byte icore) {
  bool isDEEPSLEEP_RESET(false);
  switch (rtc_get_reset_reason( (RESET_REASON) icore)) {
    case NO_MEAN                : return F("NO_MEAN");
    case POWERON_RESET          : return F("Vbat power on reset");
    case SW_RESET               : return F("Software reset digital core");
    case OWDT_RESET             : return F("Legacy watch dog reset digital core");
    case DEEPSLEEP_RESET        : isDEEPSLEEP_RESET = true; break;
    case SDIO_RESET             : return F("Reset by SLC module, reset digital core");
    case TG0WDT_SYS_RESET       : return F("Timer Group0 Watch dog reset digital core");
    case TG1WDT_SYS_RESET       : return F("Timer Group1 Watch dog reset digital core");
    case RTCWDT_SYS_RESET       : return F("RTC Watch dog Reset digital core");
    case INTRUSION_RESET        : return F("Instrusion tested to reset CPU");
    case TGWDT_CPU_RESET        : return F("Time Group reset CPU");
    case SW_CPU_RESET           : return F("Software reset CPU");
    case RTCWDT_CPU_RESET       : return F("RTC Watch dog Reset CPU");
    case EXT_CPU_RESET          : return F("for APP CPU, reseted by PRO CPU");
    case RTCWDT_BROWN_OUT_RESET : return F("Reset when the vdd voltage is not stable");
    case RTCWDT_RTC_RESET       : return F("RTC Watch dog reset digital core and rtc module");
    default: break;
  }
  if (isDEEPSLEEP_RESET) {
    String reason = F("Deep Sleep, Wakeup reason (");
    reason += rtc_get_wakeup_cause();
    reason += ')';
    return reason;
  }
  return getUnknownString();
}
#endif

String getResetReasonString() {
  #ifdef ESP32
  String reason = F("CPU0: ");
  reason += getResetReasonString(0);
  reason += F(" CPU1: ");
  reason += getResetReasonString(1);
  return reason;
  #else
  return ESP.getResetReason();
  #endif
}

String getSystemBuildString() {
  String result;
  result += BUILD;
  result += ' ';
  result += F(BUILD_NOTES);
  return result;
}

String getPluginDescriptionString() {
  String result;
  #ifdef PLUGIN_BUILD_NORMAL
    result += F(" [Normal]");
  #endif
  #ifdef PLUGIN_BUILD_TESTING
    result += F(" [Testing]");
  #endif
  #ifdef PLUGIN_BUILD_DEV
    result += F(" [Development]");
  #endif
  #ifdef PLUGIN_DESCR
  result += " [";
  result += F(PLUGIN_DESCR);
  result += ']';
  #endif
  #ifdef USE_NON_STANDARD_24_TASKS
  result += F(" 24tasks");
  #endif
  result.trim();
  return result;
}

String getSystemLibraryString() {
  String result;
  #if defined(ESP32)
    result += F("ESP32 SDK ");
    result += ESP.getSdkVersion();
  #else
    result += F("ESP82xx Core ");
    result += ESP.getCoreVersion();
    result += F(", NONOS SDK ");
    result += system_get_sdk_version();
    result += F(", LWIP: ");
    result += getLWIPversion();
  #endif
  if (puyaSupport()) {
    result += F(" PUYA support");
  }
  return result;
}

#ifndef ESP32
String getLWIPversion() {
  String result;
  result += LWIP_VERSION_MAJOR;
  result += '.';
  result += LWIP_VERSION_MINOR;
  result += '.';
  result += LWIP_VERSION_REVISION;
  if (LWIP_VERSION_IS_RC) {
    result += F("-RC");
    result += LWIP_VERSION_RC;
  } else if (LWIP_VERSION_IS_DEVELOPMENT) {
    result += F("-dev");
  }
  return result;
}
#endif

bool puyaSupport() {
  bool supported = false;
#ifdef PUYA_SUPPORT
  // New support starting core 2.5.0
  if (PUYA_SUPPORT) supported = true;
#endif
#ifdef PUYASUPPORT
  // Old patch
  supported = true;
#endif
  return supported;
}

uint8_t getFlashChipVendorId() {
#ifdef PUYA_SUPPORT
  return ESP.getFlashChipVendorId();
#else
  #if defined(ESP8266)
    uint32_t flashChipId = ESP.getFlashChipId();
    return (flashChipId & 0x000000ff);
  #else
    return 0xFF; // Not an existing function for ESP32
  #endif
#endif
}

bool flashChipVendorPuya() {
  uint8_t vendorId = getFlashChipVendorId();
  return vendorId == 0x85;  // 0x146085 PUYA
}


/*********************************************************************************************\
 Memory management
\*********************************************************************************************/


// For keeping track of 'cont' stack
// See: https://github.com/esp8266/Arduino/issues/2557
//      https://github.com/esp8266/Arduino/issues/5148#issuecomment-424329183
//      https://github.com/letscontrolit/ESPEasy/issues/1824
#ifdef ESP32
// FIXME TD-er: For ESP32 you need to provide the task number, or NULL to get from the calling task.
uint32_t getCurrentFreeStack() {
  register uint8_t *sp asm("a1");
  return (sp - pxTaskGetStackStart(NULL));
}

uint32_t getFreeStackWatermark() {
  return uxTaskGetStackHighWaterMark(NULL);
}

// FIXME TD-er: Must check if these functions are also needed for ESP32.
bool canYield() { return true; }

#else
#ifdef CORE_PRE_2_4_2
// All version before core 2.4.2
extern "C" {
#include <cont.h>
  extern cont_t g_cont;
}

uint32_t getCurrentFreeStack() {
  register uint32_t *sp asm("a1");
  return 4 * (sp - g_cont.stack);
}

uint32_t getFreeStackWatermark() {
  return cont_get_free_stack(&g_cont);
}

bool canYield() {
  return cont_can_yield(&g_cont);
}

bool allocatedOnStack(const void* address) {
  register uint32_t *sp asm("a1");
  if (sp < address) return false;
  return g_cont.stack < address;
}


#else
// All version from core 2.4.2
// See: https://github.com/esp8266/Arduino/pull/5018
//      https://github.com/esp8266/Arduino/pull/4553
extern "C" {
#include <cont.h>
  extern cont_t* g_pcont;
}

uint32_t getCurrentFreeStack() {
  // https://github.com/esp8266/Arduino/issues/2557
  register uint32_t *sp asm("a1");
  return 4 * (sp - g_pcont->stack);
}

uint32_t getFreeStackWatermark() {
  return cont_get_free_stack(g_pcont);
}

bool canYield() {
  return cont_can_yield(g_pcont);
}

bool allocatedOnStack(const void* address) {
  register uint32_t *sp asm("a1");
  if (sp < address) return false;
  return g_pcont->stack < address;
}

#endif // ARDUINO_ESP8266_RELEASE_2_x_x
#endif // ESP32






bool remoteConfig(struct EventStruct *event, const String& string)
{
  checkRAM(F("remoteConfig"));
  bool success = false;
  String  command = parseString(string, 1);

  if (command == F("config"))
  {
    success = true;

    if (parseString(string, 2) == F("task"))
    {
      String configTaskName = parseStringKeepCase(string, 3);
      // FIXME TD-er: This command is not using the tolerance setting
      // tolerantParseStringKeepCase(Line, 4);
      String configCommand  = parseStringToEndKeepCase(string, 4);

      if ((configTaskName.length() == 0) || (configCommand.length() == 0)) {
        return success; // TD-er: Should this be return false?
      }
      taskIndex_t index = findTaskIndexByName(configTaskName);

      if (validTaskIndex(index))
      {
        event->TaskIndex = index;
        success          = PluginCall(PLUGIN_SET_CONFIG, event, configCommand);
      }
    }
  }
  return success;
}

/*********************************************************************************************\
   Collect the stored preference for factory default 
\*********************************************************************************************/
void applyFactoryDefaultPref() {
  // TODO TD-er: Store it in more places to make it more persistent
  Settings.ResetFactoryDefaultPreference = ResetFactoryDefaultPreference.getPreference();
}


/*********************************************************************************************\
   Device GPIO name functions to share flash strings
\*********************************************************************************************/
String formatGpioDirection(gpio_direction direction) {
  switch (direction) {
    case gpio_input:         return F("&larr; ");
    case gpio_output:        return F("&rarr; ");
    case gpio_bidirectional: return F("&#8644; ");
  }
  return "";
}

String formatGpioLabel(int gpio, bool includeWarning) {
  int  pinnr = -1;
  bool input, output, warning;

  if (getGpioInfo(gpio, pinnr, input, output, warning)) {
    if (!includeWarning) {
      return createGPIO_label(gpio, pinnr, true, true, false);
    }
    return createGPIO_label(gpio, pinnr, input, output, warning);
  }
  return "-";
}

String formatGpioName(const String& label, gpio_direction direction, bool optional) {
  int reserveLength = 5 /* "GPIO " */ + 8 /* "&#8644; " */ + label.length();

  if (optional) {
    reserveLength += 11;
  }
  String result;
  result.reserve(reserveLength);
  result += F("GPIO ");
  result += formatGpioDirection(direction);
  result += label;

  if (optional) {
    result += F("(optional)");
  }
  return result;
}

String formatGpioName(const String& label, gpio_direction direction) {
  return formatGpioName(label, direction, false);
}

String formatGpioName_input(const String& label) {
  return formatGpioName(label, gpio_input, false);
}

String formatGpioName_output(const String& label) {
  return formatGpioName(label, gpio_output, false);
}

String formatGpioName_bidirectional(const String& label) {
  return formatGpioName(label, gpio_bidirectional, false);
}

String formatGpioName_input_optional(const String& label) {
  return formatGpioName(label, gpio_input, true);
}

String formatGpioName_output_optional(const String& label) {
  return formatGpioName(label, gpio_output, true);
}

// RX/TX are the only signals which are crossed, so they must be labelled like this:
// "GPIO <-- TX" and "GPIO --> RX"
String formatGpioName_TX(bool optional) {
  return formatGpioName("RX", gpio_output, optional);
}

String formatGpioName_RX(bool optional) {
  return formatGpioName("TX", gpio_input, optional);
}

String formatGpioName_TX_HW(bool optional) {
  return formatGpioName("RX (HW)", gpio_output, optional);
}

String formatGpioName_RX_HW(bool optional) {
  return formatGpioName("TX (HW)", gpio_input, optional);
}

#ifdef ESP32

String formatGpioName_ADC(int gpio_pin) {
  int adc,ch, t;
  if (getADC_gpio_info(gpio_pin, adc, ch, t)) {
    if (adc == 0) {
      return F("Hall Effect");
    }
    String res = F("ADC# ch?");
    res.replace("#", String(adc));
    res.replace("?", String(ch));
    if (t >= 0) {
      res += F(" (T");
      res += t;
      res += ')';
    }
    return res;
  }
  return "";
}

#endif

/*********************************************************************************************\
   set pin mode & state (info table)
  \*********************************************************************************************/
/*
void setPinState(byte plugin, byte index, byte mode, uint16_t value)
{
  // plugin number and index form a unique key
  // first check if this pin is already known
  bool reUse = false;
  for (byte x = 0; x < PINSTATE_TABLE_MAX; x++)
    if ((pinStates[x].plugin == plugin) && (pinStates[x].index == index))
    {
      pinStates[x].mode = mode;
      pinStates[x].value = value;
      reUse = true;
      break;
    }

  if (!reUse)
  {
    for (byte x = 0; x < PINSTATE_TABLE_MAX; x++)
      if (pinStates[x].plugin == 0)
      {
        pinStates[x].plugin = plugin;
        pinStates[x].index = index;
        pinStates[x].mode = mode;
        pinStates[x].value = value;
        break;
      }
  }
}
*/

/*********************************************************************************************\
   get pin mode & state (info table)
  \*********************************************************************************************/

/*
bool getPinState(byte plugin, byte index, byte *mode, uint16_t *value)
{
  for (byte x = 0; x < PINSTATE_TABLE_MAX; x++)
    if ((pinStates[x].plugin == plugin) && (pinStates[x].index == index))
    {
      *mode = pinStates[x].mode;
      *value = pinStates[x].value;
      return true;
    }
  return false;
}

*/
/*********************************************************************************************\
   check if pin mode & state is known (info table)
  \*********************************************************************************************/
/*
bool hasPinState(byte plugin, byte index)
{
  for (byte x = 0; x < PINSTATE_TABLE_MAX; x++)
    if ((pinStates[x].plugin == plugin) && (pinStates[x].index == index))
    {
      return true;
    }
  return false;
}

*/



/*********************************************************************************************\
   report pin mode & state (info table) using json
  \*********************************************************************************************/
String getPinStateJSON(bool search, uint32_t key, const String& log, int16_t noSearchValue)
{
  checkRAM(F("getPinStateJSON"));
  printToWebJSON = true;
  byte mode = PIN_MODE_INPUT;
  int16_t value = noSearchValue;
  bool found = false;

  if (search && existPortStatus(key))
  {
    mode = globalMapPortStatus[key].mode;
    value = globalMapPortStatus[key].state;
    found = true;
  }

  if (!search || (search && found))
  {
    String reply;
    reply.reserve(128);
    reply += F("{\n\"log\": \"");
    reply += log.substring(7, 32); // truncate to 25 chars, max MQTT message size = 128 including header...
    reply += F("\",\n\"plugin\": ");
    reply += getPluginFromKey(key);
    reply += F(",\n\"pin\": ");
    reply += getPortFromKey(key);
    reply += F(",\n\"mode\": \"");
    reply += getPinModeString(mode);
    reply += F("\",\n\"state\": ");
    reply += value;
    reply += F("\n}\n");
    return reply;
  }
  return "?";
}

String getPinModeString(byte mode) {
  switch (mode)
  {
    case PIN_MODE_UNDEFINED:    return F("undefined");
    case PIN_MODE_INPUT:        return F("input");
    case PIN_MODE_INPUT_PULLUP: return F("input pullup");
    case PIN_MODE_INPUT_PULLDOWN: return F("input pulldown");
    case PIN_MODE_OFFLINE:      return F("offline");
    case PIN_MODE_OUTPUT:       return F("output");
    case PIN_MODE_PWM:          return F("PWM");
    case PIN_MODE_SERVO:        return F("servo");
    default:
      break;
  }
  return F("ERROR: Not Defined");
}

#if defined(ESP32)
void analogWriteESP32(int pin, int value)
{
  // find existing channel if this pin has been used before
  int8_t ledChannel = -1;

  for (byte x = 0; x < 16; x++) {
    if (ledChannelPin[x] == pin) {
      ledChannel = x;
    }
  }

  if (ledChannel == -1)             // no channel set for this pin
  {
    for (byte x = 0; x < 16; x++) { // find free channel
      if (ledChannelPin[x] == -1)
      {
        int freq = 5000;
        ledChannelPin[x] = pin; // store pin nr
        ledcSetup(x, freq, 10); // setup channel
        ledcAttachPin(pin, x);  // attach to this pin
        ledChannel = x;
        break;
      }
    }
  }
  ledcWrite(ledChannel, value);
}
#endif // if defined(ESP32)


/********************************************************************************************\
  Status LED
\*********************************************************************************************/
#define PWMRANGE_FULL 1023
#define STATUS_PWM_NORMALVALUE (PWMRANGE_FULL>>2)
#define STATUS_PWM_NORMALFADE (PWMRANGE_FULL>>8)
#define STATUS_PWM_TRAFFICRISE (PWMRANGE_FULL>>1)

void statusLED(bool traffic)
{
  static int gnStatusValueCurrent = -1;
  static long int gnLastUpdate = millis();

  if (Settings.Pin_status_led == -1)
    return;

  if (gnStatusValueCurrent<0)
    pinMode(Settings.Pin_status_led, OUTPUT);

  int nStatusValue = gnStatusValueCurrent;

  if (traffic)
  {
    nStatusValue += STATUS_PWM_TRAFFICRISE; //ramp up fast
  }
  else
  {

    if (NetworkConnected())
    {
      long int delta = timePassedSince(gnLastUpdate);
      if (delta>0 || delta<0 )
      {
        nStatusValue -= STATUS_PWM_NORMALFADE; //ramp down slowly
        nStatusValue = std::max(nStatusValue, STATUS_PWM_NORMALVALUE);
        gnLastUpdate=millis();
      }
    }
    //AP mode is active
    else if (WifiIsAP(WiFi.getMode()))
    {
      nStatusValue = ((millis()>>1) & PWMRANGE_FULL) - (PWMRANGE_FULL>>2); //ramp up for 2 sec, 3/4 luminosity
    }
    //Disconnected
    else
    {
      nStatusValue = (millis()>>1) & (PWMRANGE_FULL>>2); //ramp up for 1/2 sec, 1/4 luminosity
    }
  }

  nStatusValue = constrain(nStatusValue, 0, PWMRANGE_FULL);

  if (gnStatusValueCurrent != nStatusValue)
  {
    gnStatusValueCurrent = nStatusValue;

    long pwm = nStatusValue * nStatusValue; //simple gamma correction
    pwm >>= 10;
    if (Settings.Pin_status_led_Inversed)
      pwm = PWMRANGE_FULL-pwm;

    #if defined(ESP8266)
      analogWrite(Settings.Pin_status_led, pwm);
    #endif // if defined(ESP8266)
    #if defined(ESP32)
       analogWriteESP32(Settings.Pin_status_led, pwm);
    #endif // if defined(ESP32)
  }
}


/********************************************************************************************\
  delay in milliseconds with background processing
  \*********************************************************************************************/
void delayBackground(unsigned long dsdelay)
{
  unsigned long timer = millis() + dsdelay;
  while (!timeOutReached(timer))
    backgroundtasks();
}


/********************************************************************************************\
  Check to see if a given argument is a valid taskIndex (argc = 0 => command)
  \*********************************************************************************************/
taskIndex_t parseCommandArgumentTaskIndex(const String& string, unsigned int argc)
{
  taskIndex_t taskIndex = INVALID_TASK_INDEX;
  const int ti = parseCommandArgumentInt(string, argc);
  if (ti > 0) {
    // Task Index used as argument in commands start at 1.
    taskIndex = static_cast<taskIndex_t>(ti - 1);
  }
  return taskIndex;
}


/********************************************************************************************\
  Get int from command argument (argc = 0 => command)
  \*********************************************************************************************/
int parseCommandArgumentInt(const String& string, unsigned int argc)
{
  int value = 0;
  if (argc > 0) {
    // No need to check for the command (argc == 0)
    String TmpStr;
    if (GetArgv(string.c_str(), TmpStr, argc + 1)) { 
      value = CalculateParam(TmpStr.c_str()); 
    }
  }
  return value;
}

/********************************************************************************************\
  Parse a command string to event struct
  \*********************************************************************************************/
void parseCommandString(struct EventStruct *event, const String& string)
{
  checkRAM(F("parseCommandString"));
  event->Par1 = parseCommandArgumentInt(string, 1);
  event->Par2 = parseCommandArgumentInt(string, 2);
  event->Par3 = parseCommandArgumentInt(string, 3);
  event->Par4 = parseCommandArgumentInt(string, 4);
  event->Par5 = parseCommandArgumentInt(string, 5);
}



/********************************************************************************************\
  Toggle controller enabled state
  \*********************************************************************************************/
bool setControllerEnableStatus(controllerIndex_t controllerIndex, bool enabled)
{
  if (!validControllerIndex(controllerIndex)) return false;
  checkRAM(F("setControllerEnableStatus"));
  // Only enable controller if it has a protocol configured
  if (Settings.Protocol[controllerIndex] != 0 || !enabled) {
    Settings.ControllerEnabled[controllerIndex] = enabled;
    return true;
  }
  return false;
}

/********************************************************************************************\
  Toggle task enabled state
  \*********************************************************************************************/
bool setTaskEnableStatus(taskIndex_t taskIndex, bool enabled)
{
  if (!validTaskIndex(taskIndex)) return false;
  checkRAM(F("setTaskEnableStatus"));
  // Only enable task if it has a Plugin configured
  if (validPluginID(Settings.TaskDeviceNumber[taskIndex]) || !enabled) {
    Settings.TaskDeviceEnabled[taskIndex] = enabled;
    if (enabled) {
      Scheduler.schedule_task_device_timer(taskIndex, millis() + 10);
    }
    return true;
  }
  return false;
}


/********************************************************************************************\
  Clear task settings for given task
  \*********************************************************************************************/
void taskClear(taskIndex_t taskIndex, bool save)
{
  if (!validTaskIndex(taskIndex)) return;
  checkRAM(F("taskClear"));
  Settings.clearTask(taskIndex);
  ExtraTaskSettings.clear(); // Invalidate any cached values.
  ExtraTaskSettings.TaskIndex = taskIndex;
  if (save) {
    SaveTaskSettings(taskIndex);
    SaveSettings();
  }
}






/********************************************************************************************\
  check the program memory hash
  The const MD5_MD5_MD5_MD5_BoundariesOfTheSegmentsGoHere... needs to remain unchanged as it will be replaced by
  - 16 bytes md5 hash, followed by
  - 4 * uint32_t start of memory segment 1-4
  - 4 * uint32_t end of memory segment 1-4
  currently there are only two segemts included in the hash. Unused segments have start adress 0.
  Execution time 520kb @80Mhz: 236ms
  Returns: 0 if hash compare fails, number of checked bytes otherwise.
  The reference hash is calculated by a .py file and injected into the binary.
  Caution: currently the hash sits in an unchecked segment. If it ever moves to a checked segment, make sure
  it is excluded from the calculation !
  \*********************************************************************************************/
#if defined(ARDUINO_ESP8266_RELEASE_2_3_0)
void dump (uint32_t addr) { //Seems already included in core 2.4 ...
  serialPrint (String(addr, HEX));
  serialPrint(": ");
  for (uint32_t a = addr; a < addr + 16; a++)
  {
    serialPrint ( String(pgm_read_byte(a), HEX));
    serialPrint (" ");
  }
  serialPrintln("");
}
#endif

/*
uint32_t progMemMD5check(){
    checkRAM(F("progMemMD5check"));
    #define BufSize 10
    uint32_t calcBuffer[BufSize];
    CRCValues.numberOfCRCBytes = 0;
    memcpy (calcBuffer,CRCValues.compileTimeMD5,16);                                                  // is there still the dummy in memory ? - the dummy needs to be replaced by the real md5 after linking.
    if( memcmp (calcBuffer, "MD5_MD5_MD5_",12)==0){                                                   // do not memcmp with CRCdummy directly or it will get optimized away.
        addLog(LOG_LEVEL_INFO, F("CRC  : No program memory checksum found. Check output of crc2.py"));
        return 0;
    }
    MD5Builder md5;
    md5.begin();
    for (int l = 0; l<4; l++){                                                                            // check max segments,  if the pointer is not 0
        uint32_t *ptrStart = (uint32_t *)&CRCValues.compileTimeMD5[16+l*4];
        uint32_t *ptrEnd =   (uint32_t *)&CRCValues.compileTimeMD5[16+4*4+l*4];
        if ((*ptrStart) == 0) break;                                                                      // segment not used.
        for (uint32_t i = *ptrStart; i< (*ptrEnd) ; i=i+sizeof(calcBuffer)){                              // "<" includes last byte
             for (int buf = 0; buf < BufSize; buf ++){
                calcBuffer[buf] = pgm_read_dword((uint32_t*)i+buf);                                       // read 4 bytes
                CRCValues.numberOfCRCBytes+=sizeof(calcBuffer[0]);
             }
             md5.add((uint8_t *)&calcBuffer[0],(*ptrEnd-i)<sizeof(calcBuffer) ? (*ptrEnd-i):sizeof(calcBuffer) );     // add buffer to md5. At the end not the whole buffer. md5 ptr to data in ram.
        }
   }
   md5.calculate();
   md5.getBytes(CRCValues.runTimeMD5);
   if ( CRCValues.checkPassed())  {
      addLog(LOG_LEVEL_INFO, F("CRC  : program checksum       ...OK"));
      return CRCValues.numberOfCRCBytes;
   }
   addLog(LOG_LEVEL_INFO, F("CRC  : program checksum       ...FAIL"));
   return 0;
}
*/

/********************************************************************************************\
  Handler for keeping ExtraTaskSettings up to date using cache
  \*********************************************************************************************/
String getTaskDeviceName(taskIndex_t TaskIndex) {
  LoadTaskSettings(TaskIndex);
  return ExtraTaskSettings.TaskDeviceName;
}


/********************************************************************************************\
  Reset all settings to factory defaults
  \*********************************************************************************************/
void ResetFactory()
{
  const GpioFactorySettingsStruct gpio_settings(ResetFactoryDefaultPreference.getDeviceModel());

  checkRAM(F("ResetFactory"));
  // Direct Serial is allowed here, since this is only an emergency task.
  serialPrint(F("RESET: Resetting factory defaults... using "));
  serialPrint(getDeviceModelString(ResetFactoryDefaultPreference.getDeviceModel()));
  serialPrintln(F(" settings"));
  delay(1000);
  if (readFromRTC())
  {
    serialPrint(F("RESET: Warm boot, reset count: "));
    serialPrintln(String(RTC.factoryResetCounter));
    if (RTC.factoryResetCounter >= 3)
    {
      serialPrintln(F("RESET: Too many resets, protecting your flash memory (powercycle to solve this)"));
      return;
    }
  }
  else
  {
    serialPrintln(F("RESET: Cold boot"));
    initRTC();
    // TODO TD-er: Store set device model in RTC.
  }

  RTC.flashCounter=0; //reset flashcounter, since we're already counting the number of factory-resets. we dont want to hit a flash-count limit during reset.
  RTC.factoryResetCounter++;
  saveToRTC();

  //always format on factory reset, in case of corrupt FS
  ESPEASY_FS.end();
  serialPrintln(F("RESET: formatting..."));
  ESPEASY_FS.format();
  serialPrintln(F("RESET: formatting done..."));
  if (!ESPEASY_FS.begin())
  {
    serialPrintln(F("RESET: FORMAT FS FAILED!"));
    return;
  }


  //pad files with extra zeros for future extensions
  String fname;

  fname=FILE_CONFIG;
  InitFile(fname.c_str(), CONFIG_FILE_SIZE);

  fname=FILE_SECURITY;
  InitFile(fname.c_str(), 4096);

  #ifdef USES_NOTIFIER
  fname=FILE_NOTIFICATION;
  InitFile(fname.c_str(), 4096);
  #endif
  fname=FILE_RULES;
  InitFile(fname.c_str(), 0);

  Settings.clearMisc();
  if (!ResetFactoryDefaultPreference.keepNTP()) {
    Settings.clearTimeSettings();
    Settings.UseNTP			= DEFAULT_USE_NTP;
    strcpy_P(Settings.NTPHost, PSTR(DEFAULT_NTP_HOST));
    Settings.TimeZone		= DEFAULT_TIME_ZONE;
    Settings.DST   			= DEFAULT_USE_DST;
  }

  if (!ResetFactoryDefaultPreference.keepNetwork()) {
    Settings.clearNetworkSettings();
    // TD-er Reset access control
    str2ip(F(DEFAULT_IPRANGE_LOW), SecuritySettings.AllowedIPrangeLow);
    str2ip(F(DEFAULT_IPRANGE_HIGH), SecuritySettings.AllowedIPrangeHigh);
    SecuritySettings.IPblockLevel = DEFAULT_IP_BLOCK_LEVEL;

    #if DEFAULT_USE_STATIC_IP
      str2ip((char*)DEFAULT_IP, Settings.IP);
      str2ip((char*)DEFAULT_DNS, Settings.DNS);
      str2ip((char*)DEFAULT_GW, Settings.Gateway);
      str2ip((char*)DEFAULT_SUBNET, Settings.Subnet);
    #endif
  }

  Settings.clearNotifications();
  Settings.clearControllers();
  Settings.clearTasks();
  if (!ResetFactoryDefaultPreference.keepLogSettings()) {
    Settings.clearLogSettings();
    str2ip((char*)DEFAULT_SYSLOG_IP, Settings.Syslog_IP);

    setLogLevelFor(LOG_TO_SYSLOG, DEFAULT_SYSLOG_LEVEL);
    setLogLevelFor(LOG_TO_SERIAL, DEFAULT_SERIAL_LOG_LEVEL);
    setLogLevelFor(LOG_TO_WEBLOG, DEFAULT_WEB_LOG_LEVEL);
    setLogLevelFor(LOG_TO_SDCARD, DEFAULT_SD_LOG_LEVEL);
    Settings.SyslogFacility	= DEFAULT_SYSLOG_FACILITY;
    Settings.UseValueLogger = DEFAULT_USE_SD_LOG;
  }
  if (!ResetFactoryDefaultPreference.keepUnitName()) {
    Settings.clearUnitNameSettings();
    Settings.Unit           = UNIT;
    strcpy_P(Settings.Name, PSTR(DEFAULT_NAME));
    Settings.UDPPort				= DEFAULT_SYNC_UDP_PORT;
  }
  if (!ResetFactoryDefaultPreference.keepWiFi()) {
    strcpy_P(SecuritySettings.WifiSSID, PSTR(DEFAULT_SSID));
    strcpy_P(SecuritySettings.WifiKey, PSTR(DEFAULT_KEY));
    strcpy_P(SecuritySettings.WifiAPKey, PSTR(DEFAULT_AP_KEY));
    SecuritySettings.WifiSSID2[0] = 0;
    SecuritySettings.WifiKey2[0] = 0;
  }
  strcpy_P(SecuritySettings.Password, PSTR(DEFAULT_ADMIN_PASS));

  Settings.ResetFactoryDefaultPreference = ResetFactoryDefaultPreference.getPreference();

  // now we set all parameters that need to be non-zero as default value


  Settings.PID             = ESP_PROJECT_PID;
  Settings.Version         = VERSION;
  Settings.Build           = BUILD;
//  Settings.IP_Octet				 = DEFAULT_IP_OCTET;
  Settings.Delay           = DEFAULT_DELAY;
  Settings.Pin_i2c_sda     = gpio_settings.i2c_sda;
  Settings.Pin_i2c_scl     = gpio_settings.i2c_scl;
  Settings.Pin_status_led  = gpio_settings.status_led;
  Settings.Pin_status_led_Inversed  = DEFAULT_PIN_STATUS_LED_INVERSED;
  Settings.Pin_sd_cs       = -1;
  Settings.Pin_Reset       = DEFAULT_PIN_RESET_BUTTON;
  Settings.Protocol[0]     = DEFAULT_PROTOCOL;
  Settings.deepSleep_wakeTime       = false;
  Settings.CustomCSS       = false;
  Settings.InitSPI         = DEFAULT_SPI;
  for (taskIndex_t x = 0; x < TASKS_MAX; x++)
  {
    Settings.TaskDevicePin1[x] = -1;
    Settings.TaskDevicePin2[x] = -1;
    Settings.TaskDevicePin3[x] = -1;
    Settings.TaskDevicePin1PullUp[x] = true;
    Settings.TaskDevicePin1Inversed[x] = false;
    for (controllerIndex_t y = 0; y < CONTROLLER_MAX; y++)
      Settings.TaskDeviceSendData[y][x] = true;
    Settings.TaskDeviceTimer[x] = Settings.Delay;
  }

  // advanced Settings
  Settings.UseRules 		= DEFAULT_USE_RULES;
  Settings.ControllerEnabled[0] = DEFAULT_CONTROLLER_ENABLED;
  Settings.MQTTRetainFlag_unused	= DEFAULT_MQTT_RETAIN;
  Settings.MessageDelay_unused	= DEFAULT_MQTT_DELAY;
  Settings.MQTTUseUnitNameAsClientId_unused = DEFAULT_MQTT_USE_UNITNAME_AS_CLIENTID;

  // allow to set default latitude and longitude
  #ifdef DEFAULT_LATITUDE
    Settings.Latitude   = DEFAULT_LATITUDE;
  #endif
  #ifdef DEFAULT_LONGITUDE
    Settings.Longitude  = DEFAULT_LONGITUDE;
  #endif

  Settings.UseSerial		= DEFAULT_USE_SERIAL;
  Settings.BaudRate		= DEFAULT_SERIAL_BAUD;

  Settings.ETH_Phy_Addr            = gpio_settings.eth_phyaddr;
  Settings.ETH_Pin_mdc             = gpio_settings.eth_mdc;
  Settings.ETH_Pin_mdio            = gpio_settings.eth_mdio;
  Settings.ETH_Pin_power           = gpio_settings.eth_power;
  Settings.ETH_Phy_Type            = gpio_settings.eth_phytype;
  Settings.ETH_Clock_Mode          = gpio_settings.eth_clock_mode;
  Settings.NetworkMedium           = gpio_settings.active_network_medium;

/*
	Settings.GlobalSync						= DEFAULT_USE_GLOBAL_SYNC;

	Settings.IP_Octet						= DEFAULT_IP_OCTET;
	Settings.WDI2CAddress					= DEFAULT_WD_IC2_ADDRESS;
	Settings.UseSSDP						= DEFAULT_USE_SSDP;
	Settings.ConnectionFailuresThreshold	= DEFAULT_CON_FAIL_THRES;
	Settings.WireClockStretchLimit			= DEFAULT_I2C_CLOCK_LIMIT;
*/
  Settings.I2C_clockSpeed     = DEFAULT_I2C_CLOCK_SPEED;

#ifdef PLUGIN_DESCR
  strcpy_P(Settings.Name, PSTR(PLUGIN_DESCR));
#endif

  addPredefinedPlugins(gpio_settings);
  addPredefinedRules(gpio_settings);

#if DEFAULT_CONTROLLER
  {
    // Place in a scope to have its memory freed ASAP
    MakeControllerSettings(ControllerSettings);
    if (AllocatedControllerSettings()) {
      safe_strncpy(ControllerSettings.Subscribe, F(DEFAULT_SUB), sizeof(ControllerSettings.Subscribe));
      safe_strncpy(ControllerSettings.Publish, F(DEFAULT_PUB), sizeof(ControllerSettings.Publish));
      safe_strncpy(ControllerSettings.MQTTLwtTopic, F(DEFAULT_MQTT_LWT_TOPIC), sizeof(ControllerSettings.MQTTLwtTopic));
      safe_strncpy(ControllerSettings.LWTMessageConnect, F(DEFAULT_MQTT_LWT_CONNECT_MESSAGE), sizeof(ControllerSettings.LWTMessageConnect));
      safe_strncpy(ControllerSettings.LWTMessageDisconnect, F(DEFAULT_MQTT_LWT_DISCONNECT_MESSAGE), sizeof(ControllerSettings.LWTMessageDisconnect));
      str2ip((char*)DEFAULT_SERVER, ControllerSettings.IP);
      ControllerSettings.setHostname(F(DEFAULT_SERVER_HOST));
      ControllerSettings.UseDNS = DEFAULT_SERVER_USEDNS;
      ControllerSettings.useExtendedCredentials(DEFAULT_USE_EXTD_CONTROLLER_CREDENTIALS);
      ControllerSettings.Port = DEFAULT_PORT;
      setControllerUser(0, ControllerSettings, F(DEFAULT_CONTROLLER_USER));
      setControllerPass(0, ControllerSettings, F(DEFAULT_CONTROLLER_PASS));

      SaveControllerSettings(0, ControllerSettings);
    }
  }
#endif

  SaveSettings();

  checkRAM(F("ResetFactory2"));
  serialPrintln(F("RESET: Succesful, rebooting. (you might need to press the reset button if you've justed flashed the firmware)"));
  //NOTE: this is a known ESP8266 bug, not our fault. :)
  delay(1000);
  WiFi.persistent(true); // use SDK storage of SSID/WPA parameters
  intent_to_reboot = true;
  WifiDisconnect(); // this will store empty ssid/wpa into sdk storage
  WiFi.persistent(false); // Do not use SDK storage of SSID/WPA parameters
  reboot();
}


/********************************************************************************************\
  If RX and TX tied together, perform emergency reset to get the system out of boot loops
  \*********************************************************************************************/

void emergencyReset()
{
  // Direct Serial is allowed here, since this is only an emergency task.
  Serial.begin(115200);
  Serial.write(0xAA);
  Serial.write(0x55);
  delay(1);
  if (Serial.available() == 2)
    if (Serial.read() == 0xAA && Serial.read() == 0x55)
    {
      serialPrintln(F("\n\n\rSystem will reset to factory defaults in 10 seconds..."));
      delay(10000);
      ResetFactory();
    }
}


/********************************************************************************************\
  Get free system mem
  \*********************************************************************************************/
unsigned long FreeMem(void)
{
  #if defined(ESP8266)
    return system_get_free_heap_size();
  #endif
  #if defined(ESP32)
    return ESP.getFreeHeap();
  #endif
}


unsigned long getMaxFreeBlock()
{
  unsigned long freemem = FreeMem();
  #ifdef CORE_POST_2_5_0
    // computing max free block is a rather extensive operation, so only perform when free memory is already low.
    if (freemem < 6144) {
      return ESP.getMaxFreeBlockSize();
    }
  #endif
  return freemem;
}




void logtimeStringToSeconds(const String& tBuf, int hours, int minutes, int seconds)
{
  #ifndef BUILD_NO_DEBUG
  if (loglevelActiveFor(LOG_LEVEL_DEBUG)) {
    String log;
    log = F("timeStringToSeconds: ");
    log += tBuf;
    log += F(" -> ");
    log += hours;
    log += ':';
    log += minutes;
    log += ':';
    log += seconds;
    addLog(LOG_LEVEL_DEBUG, log);
  }

  #endif // ifndef BUILD_NO_DEBUG
}

// convert old and new time string to nr of seconds
// return whether it should be considered a time string.
bool timeStringToSeconds(const String& tBuf, int& time_seconds) {
  time_seconds = -1;
  int hours              = 0;
  int minutes            = 0;
  int seconds            = 0;
  const int hour_sep_pos = tBuf.indexOf(':');

  if (hour_sep_pos < 0) {
    // Only hours, separator not found.
    if (validIntFromString(tBuf, hours)) {
      time_seconds = hours * 60 * 60;
    }

    // It is a valid time string, but could also be just a numerical.
    logtimeStringToSeconds(tBuf, hours, minutes, seconds);
    return false;
  }

  if (!validIntFromString(tBuf.substring(0, hour_sep_pos), hours)) {
    logtimeStringToSeconds(tBuf, hours, minutes, seconds);
    return false;
  }
  const int min_sep_pos = tBuf.indexOf(':', hour_sep_pos + 1);

  if (min_sep_pos < 0) {
    // Old format, only HH:MM
    if (!validIntFromString(tBuf.substring(hour_sep_pos + 1), minutes)) {
      logtimeStringToSeconds(tBuf, hours, minutes, seconds);
      return false;    
    }
  } else {
    // New format, only HH:MM:SS
    if (!validIntFromString(tBuf.substring(hour_sep_pos + 1, min_sep_pos), minutes)) {
      logtimeStringToSeconds(tBuf, hours, minutes, seconds);
      return false;
    }

    if (!validIntFromString(tBuf.substring(min_sep_pos + 1), seconds)) {
      logtimeStringToSeconds(tBuf, hours, minutes, seconds);
      return false;
    }
  }

  if ((minutes < 0) || (minutes > 59)) { return false; }

  if ((seconds < 0) || (seconds > 59)) { return false; }
  time_seconds = hours * 60 * 60 + minutes * 60 + seconds;
  logtimeStringToSeconds(tBuf, hours, minutes, seconds);
  return true;
}


/********************************************************************************************\
   Delayed reboot, in case of issues, do not reboot with high frequency as it might not help...
 \*********************************************************************************************/
void delayedReboot(int rebootDelay)
{
  // Direct Serial is allowed here, since this is only an emergency task.
  while (rebootDelay != 0)
  {
    serialPrint(F("Delayed Reset "));
    serialPrintln(String(rebootDelay));
    rebootDelay--;
    delay(1000);
  }
  reboot();
}

void reboot() {
  prepareShutdown();
  #if defined(ESP32)
  ESP.restart();
  #else // if defined(ESP32)
  ESP.reset();
  #endif // if defined(ESP32)
}

/********************************************************************************************\
   Parse string template
 \*********************************************************************************************/
String parseTemplate(String& tmpString)
{
  return parseTemplate(tmpString, false);
}

String parseTemplate(String& tmpString, bool useURLencode)
{
  return parseTemplate_padded(tmpString, 0, useURLencode);
}

String parseTemplate_padded(String& tmpString, byte minimal_lineSize)
{
  return parseTemplate_padded(tmpString, minimal_lineSize, false);
}

String parseTemplate_padded(String& tmpString, byte minimal_lineSize, bool useURLencode)
{
  checkRAM(F("parseTemplate_padded"));
  START_TIMER

  // Keep current loaded taskSettings to restore at the end.
  byte   currentTaskIndex = ExtraTaskSettings.TaskIndex;
  String newString;
  newString.reserve(minimal_lineSize); // Our best guess of the new size.


  if (parseTemplate_CallBack_ptr != nullptr)
     parseTemplate_CallBack_ptr(tmpString, useURLencode);
  parseSystemVariables(tmpString, useURLencode);
  

  int startpos = 0;
  int lastStartpos = 0;
  int endpos = 0;
  String deviceName, valueName, format;

  while (findNextDevValNameInString(tmpString, startpos, endpos, deviceName, valueName, format)) {
    // First copy all upto the start of the [...#...] part to be replaced.
    newString += tmpString.substring(lastStartpos, startpos);
    
    // deviceName is lower case, so we can compare literal string (no need for equalsIgnoreCase)
    if (deviceName.equals(F("plugin")))
    {
      // Handle a plugin request.
      // For example: "[Plugin#GPIO#Pinstate#N]"
      // The command is stored in valueName & format
      String command;
      command.reserve(valueName.length() + format.length() + 1);
      command  = valueName;
      command += '#';
      command += format;
      command.replace('#', ',');

      if (PluginCall(PLUGIN_REQUEST, 0, command))
      {
        // Do not call transformValue here.
        // The "format" is not empty so must not call the formatter function.
        newString += command;
      }
    }
    else if (deviceName.equals(F("var")) || deviceName.equals(F("int"))) 
    {
      // Address an internal variable either as float or as int
      // For example: Let,10,[VAR#9]
      int varNum;

      if (validIntFromString(valueName, varNum)) {
        if ((varNum > 0) && (varNum <= CUSTOM_VARS_MAX)) {
          unsigned char nr_decimals = 2;
          if (deviceName.equals(F("int"))) {
            nr_decimals = 0;
          } else if (format.length() != 0)
          {
            // There is some formatting here, so do not throw away decimals
            nr_decimals = 6;
          }
          String value = String(customFloatVar[varNum - 1], nr_decimals);
          value.trim();
          transformValue(newString, minimal_lineSize, value, format, tmpString);
        }
      }
    }
    else 
    {
      // Address a value from a plugin.
      // For example: "[bme#temp]"
      // If value name is unknown, run a PLUGIN_GET_CONFIG command.
      // For example: "[<taskname>#getLevel]"
      taskIndex_t taskIndex = findTaskIndexByName(deviceName);

      if (validTaskIndex(taskIndex) && Settings.TaskDeviceEnabled[taskIndex]) {
        byte valueNr = findDeviceValueIndexByName(valueName, taskIndex);

        if (valueNr != VARS_PER_TASK) {
          // here we know the task and value, so find the uservar
          // Try to format and transform the values
          bool   isvalid;
          String value = formatUserVar(taskIndex, valueNr, isvalid);

          if (isvalid) {
            transformValue(newString, minimal_lineSize, value, format, tmpString);
          }
        } else {
          // try if this is a get config request
          struct EventStruct TempEvent;
          TempEvent.TaskIndex = taskIndex;
          String tmpName = valueName;

          if (PluginCall(PLUGIN_GET_CONFIG, &TempEvent, tmpName))
          {
            transformValue(newString, minimal_lineSize, tmpName, format, tmpString);
          }                  
        }
      }
    }
    

    // Conversion is done (or impossible) for the found "[...#...]"
    // Continue with the next one.
    lastStartpos = endpos + 1;
    startpos     = endpos + 1;

    // This may have taken some time, so call delay()
    delay(0);
  }
  
  // Copy the rest of the string (or all if no replacements were done)
  newString += tmpString.substring(lastStartpos);

  checkRAM(F("parseTemplate2"));

  // Restore previous loaded taskSettings
  if (currentTaskIndex != 255)
  {
    LoadTaskSettings(currentTaskIndex);
  }

  parseStandardConversions(newString, useURLencode);

  // process other markups as well
  parse_string_commands(newString); 

  // padding spaces
  while (newString.length() < minimal_lineSize) {
    newString += ' ';
  }

  STOP_TIMER(PARSE_TEMPLATE_PADDED);
  checkRAM(F("parseTemplate3"));
  return newString;
}

// Find the first (enabled) task with given name
// Return INVALID_TASK_INDEX when not found, else return taskIndex
taskIndex_t findTaskIndexByName(const String& deviceName)
{
  // cache this, since LoadTaskSettings does take some time.
  auto result = Cache.taskIndexName.find(deviceName);
  if (result != Cache.taskIndexName.end()) {
    return result->second;
  }
  for (taskIndex_t taskIndex = 0; taskIndex < TASKS_MAX; taskIndex++)
  {
    if (Settings.TaskDeviceEnabled[taskIndex]) {
      String taskDeviceName = getTaskDeviceName(taskIndex);
      if (taskDeviceName.length() != 0)
      {
        // Use entered taskDeviceName can have any case, so compare case insensitive.
        if (deviceName.equalsIgnoreCase(taskDeviceName))
        {
          Cache.taskIndexName[deviceName] = taskIndex;
          return taskIndex;
        }
      }
    }
  }
  return INVALID_TASK_INDEX;
}

// Find the first device value index of a taskIndex.
// Return VARS_PER_TASK if none found.
byte findDeviceValueIndexByName(const String& valueName, taskIndex_t taskIndex) 
{
  const deviceIndex_t deviceIndex = getDeviceIndex_from_TaskIndex(taskIndex);
  if (!validDeviceIndex(deviceIndex)) return VARS_PER_TASK;

  // cache this, since LoadTaskSettings does take some time.
  // We need to use a cache search key including the taskIndex,
  // to allow several tasks to have the same value names.
  String cache_valueName;
  cache_valueName.reserve(valueName.length() + 4);
  cache_valueName = valueName;
  cache_valueName += '#'; // The '#' cannot exist in a value name, use it in the cache key.
  cache_valueName += taskIndex;
  cache_valueName.toLowerCase(); // No need to store multiple versions of the same entry with only different case.

  auto result = Cache.taskIndexValueName.find(cache_valueName);
  if (result != Cache.taskIndexValueName.end()) {
    return result->second;
  }
  LoadTaskSettings(taskIndex); // Probably already loaded, but just to be sure

  const byte valCount = Device[deviceIndex].ValueCount;
  for (byte valueNr = 0; valueNr < valCount; valueNr++)
  {
    // Check case insensitive, since the user entered value name can have any case.
    if (valueName.equalsIgnoreCase(ExtraTaskSettings.TaskDeviceValueNames[valueNr]))
    {
      Cache.taskIndexValueName[cache_valueName] = valueNr;
      return valueNr;
    }
  }
  return VARS_PER_TASK;
}

// Find positions of [...#...] in the given string.
// Only update pos values on success.
// Return true when found.
bool findNextValMarkInString(const String& input, int& startpos, int& hashpos, int& endpos) {
  int tmpStartpos = input.indexOf('[', startpos);

  if (tmpStartpos == -1) { return false; }
  int tmpHashpos = input.indexOf('#', tmpStartpos);

  if (tmpHashpos == -1) { return false; }
  // We found a hash position, check if there is another '[' inbetween.
  for (int i = tmpStartpos; i < tmpHashpos; ++i) {
    if (input[i] == '[') {
      tmpStartpos = i;
    }
  }

  int tmpEndpos = input.indexOf(']', tmpStartpos);

  if (tmpEndpos == -1) { return false; }

  if (tmpHashpos < tmpEndpos) {
    hashpos  = tmpHashpos;
    startpos = tmpStartpos;
    endpos   = tmpEndpos;
    return true;
  }
  return false;
}

// Find [deviceName#valueName] or [deviceName#valueName#format]
// DeviceName and valueName will be returned in lower case.
// Format may contain case sensitive formatting syntax.
bool findNextDevValNameInString(const String& input, int& startpos, int& endpos, String& deviceName, String& valueName, String& format) {
  int hashpos;

  if (!findNextValMarkInString(input, startpos, hashpos, endpos)) { return false; }
  deviceName = input.substring(startpos + 1, hashpos);
  valueName  = input.substring(hashpos + 1, endpos);
  hashpos    = valueName.indexOf('#');

  if (hashpos != -1) {
    // Found an extra '#' in the valueName, will split valueName and format.
    format    = valueName.substring(hashpos + 1);
    valueName = valueName.substring(0, hashpos);
  } else {
    format = "";
  }
  deviceName.toLowerCase();
  valueName.toLowerCase();
  return true;
}

/********************************************************************************************\
  Transform values
\*********************************************************************************************/
// Syntax: [task#value#transformation#justification]
// valueFormat="transformation#justification"
void transformValue(
	String& newString,
  byte lineSize,
	String value,
	String& valueFormat,
  const String &tmpString)
{
  // FIXME TD-er: This function does append to newString and uses its length to perform right aling.
  // Is this the way it is intended to use?
  
  checkRAM(F("transformValue"));

  // start changes by giig1967g - 2018-04-20
  // Syntax: [task#value#transformation#justification]
  // valueFormat="transformation#justification"
  if (valueFormat.length() > 0) //do the checks only if a Format is defined to optimize loop
  {
    String valueJust = "";

    int hashtagIndex = valueFormat.indexOf('#');
    if (hashtagIndex >= 0)
    {
      valueJust = valueFormat.substring(hashtagIndex + 1); //Justification part
      valueFormat = valueFormat.substring(0, hashtagIndex); //Transformation part
    }

    // valueFormat="transformation"
    // valueJust="justification"
    if (valueFormat.length() > 0) //do the checks only if a Format is defined to optimize loop
    {
      int logicVal = 0;
      float valFloat = 0.0f;
      if (validFloatFromString(value, valFloat))
      {
        //to be used for binary values (0 or 1)
        logicVal = static_cast<int>(roundf(valFloat)) == 0 ? 0 : 1; 
      } else {
        if (value.length() > 0) {
          logicVal = 1;
        }        
      }
      String tempValueFormat = valueFormat;
      {
        const int invertedIndex = tempValueFormat.indexOf('!');
        if (invertedIndex != -1) {
          // We must invert the value.
          logicVal = (logicVal == 0) ? 1 : 0;
          // Remove the '!' from the string.
          tempValueFormat.remove(invertedIndex,1);
        }
      }

      const int rightJustifyIndex = tempValueFormat.indexOf('R');
      const bool rightJustify = rightJustifyIndex >= 0 ? 1 : 0;
      if (rightJustify)
        tempValueFormat.remove(rightJustifyIndex,1);

      const int tempValueFormatLength = tempValueFormat.length();

      //Check Transformation syntax
      if (tempValueFormatLength > 0)
      {
        switch (tempValueFormat[0])
          {
          case 'V': //value = value without transformations
            break;
          case 'p': // Password hide using asterisks or custom character: pc
            {
              char maskChar = '*';
              if (tempValueFormatLength > 1)
              {
                maskChar = tempValueFormat[1];
              }
              if (value == F("0")) {
                value = "";
              } else {
                const int valueLength = value.length();
                for (int i = 0; i < valueLength; i++) {
                  value[i] = maskChar;
                }
              }
            }
            break;
          case 'O':
            value = logicVal == 0 ? F("OFF") : F(" ON"); //(equivalent to XOR operator)
            break;
          case 'C':
            value = logicVal == 0 ? F("CLOSE") : F(" OPEN");
            break;
          case 'c':
            value = logicVal == 0 ? F("CLOSED") : F("  OPEN");
            break;
          case 'M':
            value = logicVal == 0 ? F("AUTO") : F(" MAN");
            break;
          case 'm':
            value = logicVal == 0 ? F("A") : F("M");
            break;
          case 'H':
            value = logicVal == 0 ? F("COLD") : F(" HOT");
            break;
          case 'U':
            value = logicVal == 0 ? F("DOWN") : F("  UP");
            break;
          case 'u':
            value = logicVal == 0 ? F("D") : F("U");
            break;
          case 'Y':
            value = logicVal == 0 ? F(" NO") : F("YES");
            break;
          case 'y':
            value = logicVal == 0 ? F("N") : F("Y");
            break;
          case 'X':
            value = logicVal == 0 ? F("O") : F("X");
            break;
          case 'I':
            value = logicVal == 0 ? F("OUT") : F(" IN");
            break;
          case 'L':
            value = logicVal == 0 ? F(" LEFT") : F("RIGHT");
            break;
          case 'l':
            value = logicVal == 0 ? F("L") : F("R");
            break;
          case 'Z' :// return "0" or "1"
            value = logicVal == 0 ? "0" : "1";
            break;
          case 'D' ://Dx.y min 'x' digits zero filled & 'y' decimal fixed digits
          case 'd' ://like above but with spaces padding
            {
              int x;
              int y;
              x = 0;
              y = 0;

              switch (tempValueFormatLength)
              {
                case 2: //Dx
                  if (isDigit(tempValueFormat[1]))
                  {
                    x = (int)tempValueFormat[1]-'0';
                  }
                  break;
                case 3: //D.y
                  if (tempValueFormat[1]=='.' && isDigit(tempValueFormat[2]))
                  {
                    y = (int)tempValueFormat[2]-'0';
                  }
                  break;
                case 4: //Dx.y
                  if (isDigit(tempValueFormat[1]) && tempValueFormat[2]=='.' && isDigit(tempValueFormat[3]))
                  {
                    x = (int)tempValueFormat[1]-'0';
                    y = (int)tempValueFormat[3]-'0';
                  }
                  break;
                case 1: //D
                default: //any other combination x=0; y=0;
                  break;
              }
              value = toString(valFloat,y);
              int indexDot = value.indexOf('.');
              if (indexDot == -1) {
                indexDot = value.length();
              }              
              for (byte f = 0; f < (x - indexDot); f++) {
                value = (tempValueFormat[0]=='d'? ' ' : '0') + value;
              }
              break;
            }
          case 'F' :// FLOOR (round down)
            value = (int)floorf(valFloat);
            break;
          case 'E' :// CEILING (round up)
            value = (int)ceilf(valFloat);
            break;
          default:
            value = F("ERR");
            break;
          }

          // Check Justification syntax
          const int valueJustLength = valueJust.length();
          if (valueJustLength > 0) //do the checks only if a Justification is defined to optimize loop
          {
            value.trim(); //remove right justification spaces for backward compatibility
            switch (valueJust[0])
            {
            case 'P' :// Prefix Fill with n spaces: Pn
              if (valueJustLength > 1)
              {
                if (isDigit(valueJust[1])) //Check Pn where n is between 0 and 9
                {
                  int filler = valueJust[1] - value.length() - '0' ; //char '0' = 48; char '9' = 58
                  for (byte f = 0; f < filler; f++)
                    newString += ' ';
                }
              }
              break;
            case 'S' :// Suffix Fill with n spaces: Sn
              if (valueJustLength > 1)
              {
                if (isDigit(valueJust[1])) //Check Sn where n is between 0 and 9
                {
                  int filler = valueJust[1] - value.length() - '0' ; //48
                  for (byte f = 0; f < filler; f++)
                    value += ' ';
                }
              }
              break;
            case 'L': //left part of the string
              if (valueJustLength > 1)
              {
                if (isDigit(valueJust[1])) //Check n where n is between 0 and 9
                {
                  value = value.substring(0,(int)valueJust[1]-'0');
                }
              }
              break;
            case 'R': //Right part of the string
              if (valueJustLength > 1)
              {
                if (isDigit(valueJust[1])) //Check n where n is between 0 and 9
                {
                  value = value.substring(std::max(0,(int)value.length()-((int)valueJust[1]-'0')));
                 }
              }
              break;
            case 'U': //Substring Ux.y where x=firstChar and y=number of characters
              if (valueJustLength > 1)
              {
                if (isDigit(valueJust[1]) && valueJust[2]=='.' && isDigit(valueJust[3]) && valueJust[1] > '0' && valueJust[3] > '0')
                {
                  value = value.substring(std::min((int)value.length(),(int)valueJust[1]-'0'-1),(int)valueJust[1]-'0'-1+(int)valueJust[3]-'0');
                }
                else
                {
                  newString += F("ERR");
                }
              }
              break;
            case 'C': // Capitalize First Word-Character value (space/period are checked)
              if (value.length() > 0) {
                value.toLowerCase();
                bool nextCapital = true;
                for (uint8_t i = 0; i < value.length();i++) {
                  if (nextCapital)
                    value[i] = toupper(value[i]);
                  nextCapital = (value[i] == ' ' || value[i] == '.'); // Very simple, capitalize-first-after-space/period
                }
              }
              break;
            case 'u': // Uppercase
              value.toUpperCase();
              break;
            case 'l': // Lowercase
              value.toLowerCase();
              break;
            default:
              newString += F("ERR");
              break;
          }
        }
      }
      if (rightJustify)
      {
        int filler = lineSize - newString.length() - value.length() - tmpString.length() ;
        for (byte f = 0; f < filler; f++)
          newString += ' ';
      }
      {
#ifndef BUILD_NO_DEBUG
        if (loglevelActiveFor(LOG_LEVEL_DEBUG)) {
          String logFormatted = F("DEBUG: Formatted String='");
          logFormatted += newString;
          logFormatted += value;
          logFormatted += '\'';
          addLog(LOG_LEVEL_DEBUG, logFormatted);
        }
#endif
      }
    }
  }
  //end of changes by giig1967g - 2018-04-18

  newString += value;
  {
#ifndef BUILD_NO_DEBUG
    if (loglevelActiveFor(LOG_LEVEL_DEBUG_DEV)) {
      String logParsed = F("DEBUG DEV: Parsed String='");
      logParsed += newString;
      logParsed += '\'';
      addLog(LOG_LEVEL_DEBUG_DEV, logParsed);
    }
#endif
  }
  checkRAM(F("transformValue2"));
}

/********************************************************************************************\
  Calculate function for simple expressions
  \*********************************************************************************************/
#define CALCULATE_OK                            0
#define CALCULATE_ERROR_STACK_OVERFLOW          1
#define CALCULATE_ERROR_BAD_OPERATOR            2
#define CALCULATE_ERROR_PARENTHESES_MISMATCHED  3
#define CALCULATE_ERROR_UNKNOWN_TOKEN           4
#define STACK_SIZE 10 // was 50
#define TOKEN_MAX 20

float globalstack[STACK_SIZE];
float *sp = globalstack - 1;
float *sp_max = &globalstack[STACK_SIZE - 1];

#define is_operator(c)  (c == '+' || c == '-' || c == '*' || c == '/' || c == '^' || c == '%')
#define is_unary_operator(c)  (c == '!')

int push(float value)
{
  if (sp != sp_max) // Full
  {
    *(++sp) = value;
    return 0;
  }
  else
    return CALCULATE_ERROR_STACK_OVERFLOW;
}

float pop()
{
  if (sp != (globalstack - 1)) // empty
    return *(sp--);
  else
    return 0.0f;
}

float apply_operator(char op, float first, float second)
{
  switch (op)
  {
    case '+':
      return first + second;
    case '-':
      return first - second;
    case '*':
      return first * second;
    case '/':
      return first / second;
    case '%':
      return static_cast<int>(round(first)) % static_cast<int>(round(second));
    case '^':
      return pow(first, second);
    default:
      return 0;
  }
}

float apply_unary_operator(char op, float first)
{
  switch (op)
  {
    case '!':
      return (round(first) == 0) ? 1 : 0;
    default:
      return 0;
  }
}

char *next_token(char *linep)
{
  while (isspace(*(linep++)));
  while (*linep && !isspace(*(linep++)));
  return linep;
}

int RPNCalculate(char* token)
{
  if (token[0] == 0)
    return 0; // geen moeite doen voor een lege string

  if (is_operator(token[0]) && token[1] == 0)
  {
    float second = pop();
    float first = pop();

    if (push(apply_operator(token[0], first, second)))
      return CALCULATE_ERROR_STACK_OVERFLOW;
  } else if (is_unary_operator(token[0]) && token[1] == 0)
  {
    float first = pop();

    if (push(apply_unary_operator(token[0], first)))
      return CALCULATE_ERROR_STACK_OVERFLOW;

  } else // Als er nog een is, dan deze ophalen
    if (push(atof(token))) // is het een waarde, dan op de stack plaatsen
      return CALCULATE_ERROR_STACK_OVERFLOW;

  return 0;
}

// operators
// precedence   operators         associativity
// 3            !                 right to left
// 2            * / %             left to right
// 1            + - ^             left to right
int op_preced(const char c)
{
  switch (c)
  {
    case '!':
      return 4;
    case '^':
      return 3;
    case '*':
    case '/':
    case '%':
      return 2;
    case '+':
    case '-':
      return 1;
  }
  return 0;
}

bool op_left_assoc(const char c)
{
  switch (c)
  {
    case '^':
    case '*':
    case '/':
    case '+':
    case '-':
    case '%':
      return true;     // left to right
    case '!':
      return false;    // right to left
  }
  return false;
}

unsigned int op_arg_count(const char c)
{
  switch (c)
  {
    case '^':
    case '*':
    case '/':
    case '+':
    case '-':
    case '%':
      return 2;
    case '!':
      return 1;
  }
  return 0;
}


int Calculate(const char *input, float* result)
{
  #define TOKEN_LENGTH 25
  checkRAM(F("Calculate"));
  const char *strpos = input, *strend = input + strlen(input);
  char token[TOKEN_LENGTH];
  char c, oc, *TokenPos = token;
  char stack[32];       // operator stack
  unsigned int sl = 0;  // stack length
  char     sc;          // used for record stack element
  int error = 0;

  //*sp=0; // bug, it stops calculating after 50 times
  sp = globalstack - 1;
  oc=c=0;

  if (input[0] == '=') {
    ++strpos;
    c = *strpos;
  }

  while (strpos < strend)
  {
	  if ((TokenPos - &token[0]) >= (TOKEN_LENGTH - 1)) return CALCULATE_ERROR_STACK_OVERFLOW;
    // read one token from the input stream
    oc = c;
    c = *strpos;
    if (c != ' ')
    {
      // If the token is a number (identifier), then add it to the token queue.
      if ((c >= '0' && c <= '9') || c == '.' || (c == '-' && is_operator(oc)))
      {
        *TokenPos = c;
        ++TokenPos;
      }

      // If the token is an operator, op1, then:
      else if (is_operator(c) || is_unary_operator(c))
      {
        *(TokenPos) = 0;
        error = RPNCalculate(token);
        TokenPos = token;
        if (error)return error;
        while (sl > 0 && sl < 31)
        {
          sc = stack[sl - 1];
          // While there is an operator token, op2, at the top of the stack
          // op1 is left-associative and its precedence is less than or equal to that of op2,
          // or op1 has precedence less than that of op2,
          // The differing operator priority decides pop / push
          // If 2 operators have equal priority then associativity decides.
          if (is_operator(sc) && ((op_left_assoc(c) && (op_preced(c) <= op_preced(sc))) || (op_preced(c) < op_preced(sc))))
          {
            // Pop op2 off the stack, onto the token queue;
            *TokenPos = sc;
            ++TokenPos;
            *(TokenPos) = 0;
            error = RPNCalculate(token);
            TokenPos = token;
            if (error)return error;
            sl--;
          }
          else
            break;
        }
        // push op1 onto the stack.
        stack[sl] = c;
        ++sl;
      }
      // If the token is a left parenthesis, then push it onto the stack.
      else if (c == '(')
      {
		if (sl >= 32) return CALCULATE_ERROR_STACK_OVERFLOW;
        stack[sl] = c;
        ++sl;
      }
      // If the token is a right parenthesis:
      else if (c == ')')
      {
        bool pe = false;
        // Until the token at the top of the stack is a left parenthesis,
        // pop operators off the stack onto the token queue
        while (sl > 0)
        {
          *(TokenPos) = 0;
          error = RPNCalculate(token);
          TokenPos = token;
          if (error)return error;
          if (sl > 32) return CALCULATE_ERROR_STACK_OVERFLOW;
          sc = stack[sl - 1];
          if (sc == '(')
          {
            pe = true;
            break;
          }
          else
          {
            *TokenPos = sc;
            ++TokenPos;
            sl--;
          }
        }
        // If the stack runs out without finding a left parenthesis, then there are mismatched parentheses.
        if (!pe)
          return CALCULATE_ERROR_PARENTHESES_MISMATCHED;

        // Pop the left parenthesis from the stack, but not onto the token queue.
        sl--;

        // If the token at the top of the stack is a function token, pop it onto the token queue.
		// FIXME TD-er: This sc value is never used, it is re-assigned a new value before it is being checked.
        if (sl > 0 && sl < 32)
          sc = stack[sl - 1];

      }
      else
        return CALCULATE_ERROR_UNKNOWN_TOKEN;
    }
    ++strpos;
  }
  // When there are no more tokens to read:
  // While there are still operator tokens in the stack:
  while (sl > 0)
  {
    sc = stack[sl - 1];
    if (sc == '(' || sc == ')')
      return CALCULATE_ERROR_PARENTHESES_MISMATCHED;

    *(TokenPos) = 0;
    error = RPNCalculate(token);
    TokenPos = token;
    if (error)return error;
    *TokenPos = sc;
    ++TokenPos;
    --sl;
  }

  *(TokenPos) = 0;
  error = RPNCalculate(token);
  TokenPos = token;
  if (error)
  {
    *result = 0;
    return error;
  }
  *result = *sp;
  checkRAM(F("Calculate2"));
  return CALCULATE_OK;
}

int CalculateParam(const char *TmpStr) {
  int returnValue;

  // Minimize calls to the Calulate function.
  // Only if TmpStr starts with '=' then call Calculate(). Otherwise do not call it
  if (TmpStr[0] != '=') {
    returnValue=str2int(TmpStr);
  } else {
    float param=0;
    // Starts with an '=', so Calculate starting at next position
    int returnCode=Calculate(&TmpStr[1], &param);
    if (returnCode!=CALCULATE_OK) {
      String errorDesc;
      switch (returnCode) {
        case CALCULATE_ERROR_STACK_OVERFLOW:
          errorDesc = F("Stack Overflow");
          break;
        case CALCULATE_ERROR_BAD_OPERATOR:
          errorDesc = F("Bad Operator");
          break;
        case CALCULATE_ERROR_PARENTHESES_MISMATCHED:
          errorDesc = F("Parenthesis mismatch");
          break;
        case CALCULATE_ERROR_UNKNOWN_TOKEN:
          errorDesc = F("Unknown token");
          break;
        default:
          errorDesc = F("Unknown error");
          break;
        }
        if (loglevelActiveFor(LOG_LEVEL_ERROR)) {
          String log = String(F("CALCULATE PARAM ERROR: ")) + errorDesc;
          addLog(LOG_LEVEL_ERROR, log);
          log = F("CALCULATE PARAM ERROR details: ");
          log += TmpStr;
          log += F(" = ");
          log += round(param);
          addLog(LOG_LEVEL_ERROR, log);
        }
      }
#ifndef BUILD_NO_DEBUG
        else {
      if (loglevelActiveFor(LOG_LEVEL_DEBUG)) {
        String log = F("CALCULATE PARAM: ");
        log += TmpStr;
        log += F(" = ");
        log += round(param);
        addLog(LOG_LEVEL_DEBUG, log);
      }
    }
#endif
    returnValue=round(param); //return integer only as it's valid only for device and task id
  }
  return returnValue;
}

void SendValueLogger(taskIndex_t TaskIndex)
{
#if !defined(BUILD_NO_DEBUG) || defined(FEATURE_SD)
  bool featureSD = false;
  String logger;
  #ifdef FEATURE_SD
    featureSD = true;
  #endif
  
  if (featureSD || loglevelActiveFor(LOG_LEVEL_DEBUG)) {
    const deviceIndex_t DeviceIndex = getDeviceIndex_from_TaskIndex(TaskIndex);
    if (validDeviceIndex(DeviceIndex)) {
      LoadTaskSettings(TaskIndex);
      for (byte varNr = 0; varNr < Device[DeviceIndex].ValueCount; varNr++)
      {
        logger += node_time.getDateString('-');
        logger += ' ';
        logger += node_time.getTimeString(':');
        logger += ',';
        logger += Settings.Unit;
        logger += ',';
        logger += getTaskDeviceName(TaskIndex);
        logger += ',';
        logger += ExtraTaskSettings.TaskDeviceValueNames[varNr];
        logger += ',';
        logger += formatUserVarNoCheck(TaskIndex, varNr);
        logger += "\r\n";
      }
      addLog(LOG_LEVEL_DEBUG, logger);
    }

  }
#endif

#ifdef FEATURE_SD
  String filename = F("VALUES.CSV");
  File logFile = SD.open(filename, FILE_WRITE);
  if (logFile)
    logFile.print(logger);
  logFile.close();
#endif
}




//#ifdef PLUGIN_BUILD_TESTING

//#define isdigit(n) (n >= '0' && n <= '9') //Conflicts with ArduJson 6+, when this lib is used there is no need for this macro

/********************************************************************************************\
  Generate a tone of specified frequency on pin
  \*********************************************************************************************/
void tone_espEasy(uint8_t _pin, unsigned int frequency, unsigned long duration) {
  #ifdef ESP32
    toneESP32(_pin,frequency,duration);
  #else
    analogWriteFreq(frequency);
    //NOTE: analogwrite reserves IRAM and uninitalized ram.
    analogWrite(_pin,100);
    delay(duration);
    analogWrite(_pin,0);
  #endif
}

/********************************************************************************************\
  Play RTTTL string on specified pin
  \*********************************************************************************************/
void play_rtttl(uint8_t _pin, const char *p )
{
  checkRAM(F("play_rtttl"));
  #define OCTAVE_OFFSET 0
  // FIXME: Absolutely no error checking in here

  const int notes[] = { 0,
    262, 277, 294, 311, 330, 349, 370, 392, 415, 440, 466, 494,
    523, 554, 587, 622, 659, 698, 740, 784, 831, 880, 932, 988,
    1047, 1109, 1175, 1245, 1319, 1397, 1480, 1568, 1661, 1760, 1865, 1976,
    2093, 2217, 2349, 2489, 2637, 2794, 2960, 3136, 3322, 3520, 3729, 3951
  };



  byte default_dur = 4;
  byte default_oct = 6;
  int bpm = 63;
  int num;
  long wholenote;
  long duration;
  byte note;
  byte scale;

  // format: d=N,o=N,b=NNN:
  // find the start (skip name, etc)

  while(*p != ':') p++;    // ignore name
  p++;                     // skip ':'

  // get default duration
  if(*p == 'd')
  {
    p++; p++;              // skip "d="
    num = 0;
    while(isdigit(*p))
    {
      num = (num * 10) + (*p++ - '0');
    }
    if(num > 0) default_dur = num;
    p++;                   // skip comma
  }

  // get default octave
  if(*p == 'o')
  {
    p++; p++;              // skip "o="
    num = *p++ - '0';
    if(num >= 3 && num <=7) default_oct = num;
    p++;                   // skip comma
  }

  // get BPM
  if(*p == 'b')
  {
    p++; p++;              // skip "b="
    num = 0;
    while(isdigit(*p))
    {
      num = (num * 10) + (*p++ - '0');
    }
    bpm = num;
    p++;                   // skip colon
  }

  // BPM usually expresses the number of quarter notes per minute
  wholenote = (60 * 1000L / bpm) * 4;  // this is the time for whole note (in milliseconds)

  // now begin note loop
  while(*p)
  {
    // first, get note duration, if available
    num = 0;
    while(isdigit(*p))
    {
      num = (num * 10) + (*p++ - '0');
    }

    if (num) duration = wholenote / num;
    else duration = wholenote / default_dur;  // we will need to check if we are a dotted note after

    // now get the note
    switch(*p)
    {
      case 'c':
        note = 1;
        break;
      case 'd':
        note = 3;
        break;
      case 'e':
        note = 5;
        break;
      case 'f':
        note = 6;
        break;
      case 'g':
        note = 8;
        break;
      case 'a':
        note = 10;
        break;
      case 'b':
        note = 12;
        break;
      case 'p':
      default:
        note = 0;
    }
    p++;

    // now, get optional '#' sharp
    if(*p == '#')
    {
      note++;
      p++;
    }

    // now, get optional '.' dotted note
    if(*p == '.')
    {
      duration += duration/2;
      p++;
    }

    // now, get scale
    if(isdigit(*p))
    {
      scale = *p - '0';
      p++;
    }
    else
    {
      scale = default_oct;
    }

    scale += OCTAVE_OFFSET;

    if(*p == ',')
      p++;       // skip comma for next note (or we may be at the end)

    // now play the note
    if(note)
    {
      tone_espEasy(_pin, notes[(scale - 4) * 12 + note], duration);
    }
    else
    {
      delay(duration/10);
    }
  }
 checkRAM(F("play_rtttl2"));
}

//#endif

bool OTA_possible(uint32_t& maxSketchSize, bool& use2step) {
#if defined(ESP8266)
  // Compute the current free space and sketch size, rounded to 4k blocks.
  // These block bounaries are needed for erasing a full block on flash.
  const uint32_t freeSketchSpace = (ESP.getFreeSketchSpace() - 0x1000) & 0xFFFFF000;
  const uint32_t currentSketchSize = (ESP.getSketchSize() + 0x1000) & 0xFFFFF000;
  const uint32_t smallestOtaImageSizeNeeded = (((SMALLEST_OTA_IMAGE + 16) + 0x1000) & 0xFFFFF000);
  const bool otaPossible = freeSketchSpace >= smallestOtaImageSizeNeeded;
  use2step = freeSketchSpace < currentSketchSize; // Assume the new image has the same size.
  if (use2step) {
    const uint32_t totalSketchSpace = freeSketchSpace + currentSketchSize;
    maxSketchSize = totalSketchSpace - smallestOtaImageSizeNeeded;
  } else {
    maxSketchSize = freeSketchSpace;
  }
  maxSketchSize -= 16; // Must leave 16 bytes at the end.
  if (maxSketchSize > MAX_SKETCH_SIZE) maxSketchSize = MAX_SKETCH_SIZE;
  return otaPossible;
#elif defined(ESP32)
  maxSketchSize = MAX_SKETCH_SIZE;
  use2step = false;
  return true;
#else
  return false;
#endif
}

#ifdef FEATURE_ARDUINO_OTA
/********************************************************************************************\
  Allow updating via the Arduino OTA-protocol. (this allows you to upload directly from platformio)
  \*********************************************************************************************/

void ArduinoOTAInit()
{
  checkRAM(F("ArduinoOTAInit"));

  ArduinoOTA.setPort(ARDUINO_OTA_PORT);
  ArduinoOTA.setHostname(Settings.getHostname().c_str());
  if (SecuritySettings.Password[0]!=0)
    ArduinoOTA.setPassword(SecuritySettings.Password);

  ArduinoOTA.onStart([]() {
      serialPrintln(F("OTA  : Start upload"));
      ArduinoOTAtriggered = true;
      ESPEASY_FS.end(); //important, otherwise it fails
  });

  ArduinoOTA.onEnd([]() {
      serialPrintln(F("\nOTA  : End"));
      //"dangerous": if you reset during flash you have to reflash via serial
      //so dont touch device until restart is complete
      serialPrintln(F("\nOTA  : DO NOT RESET OR POWER OFF UNTIL BOOT+FLASH IS COMPLETE."));
      //delay(100);
      //reboot(); //Not needed, node reboots automaticall after calling onEnd and succesfully flashing
  });
  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
    if (Settings.UseSerial)
      Serial.printf("OTA  : Progress %u%%\r", (progress / (total / 100)));
  });

  ArduinoOTA.onError([](ota_error_t error) {
      serialPrint(F("\nOTA  : Error (will reboot): "));
      if (error == OTA_AUTH_ERROR) serialPrintln(F("Auth Failed"));
      else if (error == OTA_BEGIN_ERROR) serialPrintln(F("Begin Failed"));
      else if (error == OTA_CONNECT_ERROR) serialPrintln(F("Connect Failed"));
      else if (error == OTA_RECEIVE_ERROR) serialPrintln(F("Receive Failed"));
      else if (error == OTA_END_ERROR) serialPrintln(F("End Failed"));

      delay(100);
      reboot();
  });
  ArduinoOTA.begin();

  if (loglevelActiveFor(LOG_LEVEL_INFO)) {
    String log = F("OTA  : Arduino OTA enabled on port ");
    log += ARDUINO_OTA_PORT;
    addLog(LOG_LEVEL_INFO, log);
  }
}

#endif




/**********************************************************
*                                                         *
* Helper Functions for managing the status data structure *
*                                                         *
**********************************************************/

void savePortStatus(uint32_t key, struct portStatusStruct &tempStatus) {
  // FIXME TD-er: task and monitor are unsigned, should we only check for == ????
  if (tempStatus.task<=0 && tempStatus.monitor<=0 && tempStatus.command<=0)
    globalMapPortStatus.erase(key);
  else
    globalMapPortStatus[key] = tempStatus;
}

bool existPortStatus(uint32_t key) {
  bool retValue = false;
  //check if KEY exists:
  std::map<uint32_t,portStatusStruct>::iterator it;
  it = globalMapPortStatus.find(key);
  if (it != globalMapPortStatus.end()) {  //if KEY exists...
    retValue = true;
  }
  return retValue;
}

void removeTaskFromPort(uint32_t key) {
  if (existPortStatus(key)) {
    (globalMapPortStatus[key].task > 0) ? globalMapPortStatus[key].task-- : globalMapPortStatus[key].task = 0;
    if (globalMapPortStatus[key].task<=0 && globalMapPortStatus[key].monitor<=0 && globalMapPortStatus[key].command<=0&& globalMapPortStatus[key].init<=0)
      globalMapPortStatus.erase(key);
  }
}

void removeMonitorFromPort(uint32_t key) {
  if (existPortStatus(key)) {
    globalMapPortStatus[key].monitor=0;
    if (globalMapPortStatus[key].task<=0 && globalMapPortStatus[key].monitor<=0 && globalMapPortStatus[key].command<=0&& globalMapPortStatus[key].init<=0)
      globalMapPortStatus.erase(key);
  }
}

void addMonitorToPort(uint32_t key) {
  globalMapPortStatus[key].monitor=1;
}

uint32_t createKey(uint16_t pluginNumber, uint16_t portNumber) {
  return (uint32_t) pluginNumber << 16 | portNumber;
}

pluginID_t getPluginFromKey(uint32_t key) {
  return static_cast<pluginID_t>((key >> 16) & 0xFFFF);
}

uint16_t getPortFromKey(uint32_t key) {
  return static_cast<uint16_t>(key & 0xFFFF);
}

//#######################################################################################################
//############################ quite acurate but slow color converter####################################
//#######################################################################################################
// uses H 0..360 S 1..100 I/V 1..100 (according to homie convention)
// Source https://blog.saikoled.com/post/44677718712/how-to-convert-from-hsi-to-rgb-white

void HSV2RGB(float H, float S, float I, int rgb[3]) {
  int r, g, b;
  H = fmod(H,360); // cycle H around to 0-360 degrees
  H = 3.14159f*H/(float)180; // Convert to radians.
  S = S / 100;
  S = S>0?(S<1?S:1):0; // clamp S and I to interval [0,1]
  I = I / 100;
  I = I>0?(I<1?I:1):0;

  // Math! Thanks in part to Kyle Miller.
  if(H < 2.09439f) {
    r = 255*I/3*(1+S*cos(H)/cos(1.047196667f-H));
    g = 255*I/3*(1+S*(1-cos(H)/cos(1.047196667f-H)));
    b = 255*I/3*(1-S);
  } else if(H < 4.188787f) {
    H = H - 2.09439f;
    g = 255*I/3*(1+S*cos(H)/cos(1.047196667f-H));
    b = 255*I/3*(1+S*(1-cos(H)/cos(1.047196667f-H)));
    r = 255*I/3*(1-S);
  } else {
    H = H - 4.188787f;
    b = 255*I/3*(1+S*cos(H)/cos(1.047196667f-H));
    r = 255*I/3*(1+S*(1-cos(H)/cos(1.047196667f-H)));
    g = 255*I/3*(1-S);
  }
  rgb[0]=r;
  rgb[1]=g;
  rgb[2]=b;
}

// uses H 0..360 S 1..100 I/V 1..100 (according to homie convention)
// Source https://blog.saikoled.com/post/44677718712/how-to-convert-from-hsi-to-rgb-white

void HSV2RGBW(float H, float S, float I, int rgbw[4]) {
  int r, g, b, w;
  float cos_h, cos_1047_h;
  H = fmod(H,360); // cycle H around to 0-360 degrees
  H = 3.14159f*H/(float)180; // Convert to radians.
  S = S / 100;
  S = S>0?(S<1?S:1):0; // clamp S and I to interval [0,1]
  I = I / 100;
  I = I>0?(I<1?I:1):0;

  if(H < 2.09439f) {
    cos_h = cos(H);
    cos_1047_h = cos(1.047196667f-H);
    r = S*255*I/3*(1+cos_h/cos_1047_h);
    g = S*255*I/3*(1+(1-cos_h/cos_1047_h));
    b = 0;
    w = 255*(1-S)*I;
  } else if(H < 4.188787f) {
    H = H - 2.09439f;
    cos_h = cos(H);
    cos_1047_h = cos(1.047196667f-H);
    g = S*255*I/3*(1+cos_h/cos_1047_h);
    b = S*255*I/3*(1+(1-cos_h/cos_1047_h));
    r = 0;
    w = 255*(1-S)*I;
  } else {
    H = H - 4.188787f;
    cos_h = cos(H);
    cos_1047_h = cos(1.047196667f-H);
    b = S*255*I/3*(1+cos_h/cos_1047_h);
    r = S*255*I/3*(1+(1-cos_h/cos_1047_h));
    g = 0;
    w = 255*(1-S)*I;
  }

  rgbw[0]=r;
  rgbw[1]=g;
  rgbw[2]=b;
  rgbw[3]=w;
}


// Simple bitwise get/set functions

uint8_t get8BitFromUL(uint32_t number, byte bitnr) {
  return (number >> bitnr) & 0xFF;
}

void set8BitToUL(uint32_t& number, byte bitnr, uint8_t value) {
  uint32_t mask     = (0xFFUL << bitnr);
  uint32_t newvalue = ((value << bitnr) & mask);

  number = (number & ~mask) | newvalue;
}

uint8_t get4BitFromUL(uint32_t number, byte bitnr) {
  return (number >> bitnr) &  0x0F;
}

void set4BitToUL(uint32_t& number, byte bitnr, uint8_t value) {
  uint32_t mask     = (0x0FUL << bitnr);
  uint32_t newvalue = ((value << bitnr) & mask);

  number = (number & ~mask) | newvalue;
}
