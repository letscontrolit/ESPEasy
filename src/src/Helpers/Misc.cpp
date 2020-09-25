#include "Misc.h"

#include "../DataStructs/Caches.h"
#include "../DataStructs/ControllerSettingsStruct.h"
#include "../DataStructs/ExtendedControllerCredentialsStruct.h"
#include "../DataStructs/NodeStruct.h"
#include "../DataStructs/PinMode.h"
#include "../DataStructs/RTCStruct.h"
#include "../DataStructs/StorageLayout.h"


#include "../Globals/CPlugins.h"
#include "../Globals/CRCValues.h"
#include "../Globals/Cache.h"
#include "../Globals/Device.h"
#include "../Globals/ESPEasy_Scheduler.h"
#include "../Globals/ExtraTaskSettings.h"
#include "../Globals/GlobalMapPortStatus.h"
#include "../Globals/MQTT.h"
#include "../Globals/Plugins.h"
#include "../Globals/Plugins_other.h"
#include "../Globals/RTC.h"
#include "../Globals/ResetFactoryDefaultPref.h"
#include "../Globals/SecuritySettings.h"
#include "../Globals/Services.h"
#include "../Globals/Settings.h"
#include "../Globals/Statistics.h"

#include "../Helpers/Convert.h"
#include "../Helpers/ESPEasy_FactoryDefault.h"
#include "../Helpers/ESPEasy_Storage.h"
#include "../Helpers/ESPEasy_time_calc.h"
#include "../Helpers/ESPEasyRTC.h"
#include "../Helpers/Hardware.h"
#include "../Helpers/PeriodicalActions.h"
#include "../Helpers/PortStatus.h"
#include "../Helpers/Rules_calculate.h"
#include "../Helpers/StringConverter.h"
#include "../Helpers/StringParser.h"

#include "../../ESPEasy_common.h"
#include "../../ESPEasyWifi.h"
#include "../../_CPlugin_Helper.h"
#include "../../_Plugin_Helper.h"


#ifdef ESP32

void noToneESP32(uint8_t pin, uint8_t channel)
{
  ledcDetachPin(pin);
  ledcWrite(channel, 0);
}

void toneESP32(uint8_t pin, unsigned int frequency, unsigned long duration, uint8_t channel)
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

#endif // ifdef ESP32

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
    case MQTT_CONNECTION_TIMEOUT: return F("Connection timeout");
    case MQTT_CONNECTION_LOST: return F("Connection lost");
    case MQTT_CONNECT_FAILED: return F("Connect failed");
    case MQTT_DISCONNECTED: return F("Disconnected");
    case MQTT_CONNECTED: return F("Connected");
    case MQTT_CONNECT_BAD_PROTOCOL: return F("Connect bad protocol");
    case MQTT_CONNECT_BAD_CLIENT_ID: return F("Connect bad client_id");
    case MQTT_CONNECT_UNAVAILABLE: return F("Connect unavailable");
    case MQTT_CONNECT_BAD_CREDENTIALS: return F("Connect bad credentials");
    case MQTT_CONNECT_UNAUTHORIZED: return F("Connect unauthorized");
    default: break;
  }
  return "";
}

#endif // USES_MQTT

/********************************************************************************************\
   Get system information
 \*********************************************************************************************/
String getLastBootCauseString() {
  switch (lastBootCause)
  {
    case BOOT_CAUSE_MANUAL_REBOOT: return F("Manual reboot");
    case BOOT_CAUSE_DEEP_SLEEP: // nobody should ever see this, since it should sleep again right away.
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

  switch (rtc_get_reset_reason((RESET_REASON)icore)) {
    case NO_MEAN: return F("NO_MEAN");
    case POWERON_RESET: return F("Vbat power on reset");
    case SW_RESET: return F("Software reset digital core");
    case OWDT_RESET: return F("Legacy watch dog reset digital core");
    case DEEPSLEEP_RESET: isDEEPSLEEP_RESET = true; break;
    case SDIO_RESET: return F("Reset by SLC module, reset digital core");
    case TG0WDT_SYS_RESET: return F("Timer Group0 Watch dog reset digital core");
    case TG1WDT_SYS_RESET: return F("Timer Group1 Watch dog reset digital core");
    case RTCWDT_SYS_RESET: return F("RTC Watch dog Reset digital core");
    case INTRUSION_RESET: return F("Instrusion tested to reset CPU");
    case TGWDT_CPU_RESET: return F("Time Group reset CPU");
    case SW_CPU_RESET: return F("Software reset CPU");
    case RTCWDT_CPU_RESET: return F("RTC Watch dog Reset CPU");
    case EXT_CPU_RESET: return F("for APP CPU, reseted by PRO CPU");
    case RTCWDT_BROWN_OUT_RESET: return F("Reset when the vdd voltage is not stable");
    case RTCWDT_RTC_RESET: return F("RTC Watch dog reset digital core and rtc module");
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

#endif // ifdef ESP32

String getResetReasonString() {
  #ifdef ESP32
  String reason = F("CPU0: ");
  reason += getResetReasonString(0);
  reason += F(" CPU1: ");
  reason += getResetReasonString(1);
  return reason;
  #else // ifdef ESP32
  return ESP.getResetReason();
  #endif // ifdef ESP32
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
  #endif // ifdef PLUGIN_BUILD_NORMAL
  #ifdef PLUGIN_BUILD_TESTING
  result += F(" [Testing]");
  #endif // ifdef PLUGIN_BUILD_TESTING
  #ifdef PLUGIN_BUILD_DEV
  result += F(" [Development]");
  #endif // ifdef PLUGIN_BUILD_DEV
  #ifdef PLUGIN_DESCR
  result += " [";
  result += F(PLUGIN_DESCR);
  result += ']';
  #endif // ifdef PLUGIN_DESCR
  #ifdef USE_NON_STANDARD_24_TASKS
  result += F(" 24tasks");
  #endif // ifdef USE_NON_STANDARD_24_TASKS
  result.trim();
  return result;
}

String getSystemLibraryString() {
  String result;

  #if defined(ESP32)
  result += F("ESP32 SDK ");
  result += ESP.getSdkVersion();
  #else // if defined(ESP32)
  result += F("ESP82xx Core ");
  result += ESP.getCoreVersion();
  result += F(", NONOS SDK ");
  result += system_get_sdk_version();
  result += F(", LWIP: ");
  result += getLWIPversion();
  #endif // if defined(ESP32)

  if (puyaSupport()) {
    result += F(" PUYA support");
  }
  return result;
}

#ifdef ESP8266
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

#endif // ifdef ESP8266

bool puyaSupport() {
  bool supported = false;

#ifdef PUYA_SUPPORT

  // New support starting core 2.5.0
  if (PUYA_SUPPORT) { supported = true; }
#endif // ifdef PUYA_SUPPORT
#ifdef PUYASUPPORT

  // Old patch
  supported = true;
#endif // ifdef PUYASUPPORT
  return supported;
}

uint8_t getFlashChipVendorId() {
#ifdef PUYA_SUPPORT
  return ESP.getFlashChipVendorId();
#else // ifdef PUYA_SUPPORT
  # if defined(ESP8266)
  uint32_t flashChipId = ESP.getFlashChipId();
  return flashChipId & 0x000000ff;
  # else // if defined(ESP8266)
  return 0xFF; // Not an existing function for ESP32
  # endif // if defined(ESP8266)
#endif // ifdef PUYA_SUPPORT
}

bool flashChipVendorPuya() {
  uint8_t vendorId = getFlashChipVendorId();

  return vendorId == 0x85; // 0x146085 PUYA
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
  register uint8_t *sp asm ("a1");

  return sp - pxTaskGetStackStart(NULL);
}

uint32_t getFreeStackWatermark() {
  return uxTaskGetStackHighWaterMark(NULL);
}

// FIXME TD-er: Must check if these functions are also needed for ESP32.
bool canYield() {
  return true;
}

#else // ifdef ESP32

uint32_t getCurrentFreeStack() {
  // https://github.com/esp8266/Arduino/issues/2557
  register uint32_t *sp asm ("a1");

  return 4 * (sp - g_pcont->stack);
}

uint32_t getFreeStackWatermark() {
  return cont_get_free_stack(g_pcont);
}

bool canYield() {
  return cont_can_yield(g_pcont);
}

bool allocatedOnStack(const void *address) {
  register uint32_t *sp asm ("a1");

  if (sp < address) { return false; }
  return g_pcont->stack < address;
}

#endif // ESP32


bool remoteConfig(struct EventStruct *event, const String& string)
{
  checkRAM(F("remoteConfig"));
  bool   success = false;
  String command = parseString(string, 1);

  if (command == F("config"))
  {
    success = true;

    if (parseString(string, 2) == F("task"))
    {
      String configTaskName = parseStringKeepCase(string, 3);

      // FIXME TD-er: This command is not using the tolerance setting
      // tolerantParseStringKeepCase(Line, 4);
      String configCommand = parseStringToEndKeepCase(string, 4);

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
  return formatGpioName(F("RX"), gpio_output, optional);
}

String formatGpioName_RX(bool optional) {
  return formatGpioName(F("TX"), gpio_input, optional);
}

String formatGpioName_TX_HW(bool optional) {
  return formatGpioName(F("RX (HW)"), gpio_output, optional);
}

String formatGpioName_RX_HW(bool optional) {
  return formatGpioName(F("TX (HW)"), gpio_input, optional);
}

#ifdef ESP32

String formatGpioName_ADC(int gpio_pin) {
  int adc, ch, t;

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

#endif // ifdef ESP32

// ********************************************************************************
// Add a GPIO pin select dropdown list for 8266, 8285 or ESP32
// ********************************************************************************
String createGPIO_label(int gpio, int pinnr, bool input, bool output, bool warning) {
  if (gpio < 0) { return F("- None -"); }
  String result;

  result.reserve(24);
  result  = F("GPIO-");
  result += gpio;

  if (pinnr >= 0) {
    result += F(" (D");
    result += pinnr;
    result += ')';
  }

  if (input != output) {
    result += ' ';
    result += input ? F(HTML_SYMBOL_INPUT) : F(HTML_SYMBOL_OUTPUT);
  }

  if (warning) {
    result += ' ';
    result += F(HTML_SYMBOL_WARNING);
  }
  bool serialPinConflict = (Settings.UseSerial && (gpio == 1 || gpio == 3));

  if (serialPinConflict) {
    if (gpio == 1) { result += F(" TX0"); }

    if (gpio == 3) { result += F(" RX0"); }
  }
  return result;
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
void statusLED(bool traffic)
{
  static int gnStatusValueCurrent = -1;
  static long int gnLastUpdate    = millis();

  if (Settings.Pin_status_led == -1) {
    return;
  }

  if (gnStatusValueCurrent < 0) {
    pinMode(Settings.Pin_status_led, OUTPUT);
  }

  int nStatusValue = gnStatusValueCurrent;

  if (traffic)
  {
    nStatusValue += STATUS_PWM_TRAFFICRISE; // ramp up fast
  }
  else
  {
    if (NetworkConnected())
    {
      long int delta = timePassedSince(gnLastUpdate);

      if ((delta > 0) || (delta < 0))
      {
        nStatusValue -= STATUS_PWM_NORMALFADE; // ramp down slowly
        nStatusValue  = std::max(nStatusValue, STATUS_PWM_NORMALVALUE);
        gnLastUpdate  = millis();
      }
    }

    // AP mode is active
    else if (WifiIsAP(WiFi.getMode()))
    {
      nStatusValue = ((millis() >> 1) & PWMRANGE_FULL) - (PWMRANGE_FULL >> 2); // ramp up for 2 sec, 3/4 luminosity
    }

    // Disconnected
    else
    {
      nStatusValue = (millis() >> 1) & (PWMRANGE_FULL >> 2); // ramp up for 1/2 sec, 1/4 luminosity
    }
  }

  nStatusValue = constrain(nStatusValue, 0, PWMRANGE_FULL);

  if (gnStatusValueCurrent != nStatusValue)
  {
    gnStatusValueCurrent = nStatusValue;

    long pwm = nStatusValue * nStatusValue; // simple gamma correction
    pwm >>= 10;

    if (Settings.Pin_status_led_Inversed) {
      pwm = PWMRANGE_FULL - pwm;
    }

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

  while (!timeOutReached(timer)) {
    backgroundtasks();
  }
}

/********************************************************************************************\
   Check to see if a given argument is a valid taskIndex (argc = 0 => command)
 \*********************************************************************************************/
taskIndex_t parseCommandArgumentTaskIndex(const String& string, unsigned int argc)
{
  taskIndex_t taskIndex = INVALID_TASK_INDEX;
  const int   ti        = parseCommandArgumentInt(string, argc);

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
  if (!validControllerIndex(controllerIndex)) { return false; }
  checkRAM(F("setControllerEnableStatus"));

  // Only enable controller if it has a protocol configured
  if ((Settings.Protocol[controllerIndex] != 0) || !enabled) {
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
  if (!validTaskIndex(taskIndex)) { return false; }
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
  if (!validTaskIndex(taskIndex)) { return; }
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
void dump(uint32_t addr) { // Seems already included in core 2.4 ...
  serialPrint(String(addr, HEX));
  serialPrint(": ");

  for (uint32_t a = addr; a < addr + 16; a++)
  {
    serialPrint(String(pgm_read_byte(a), HEX));
    serialPrint(" ");
  }
  serialPrintln("");
}

#endif // if defined(ARDUINO_ESP8266_RELEASE_2_3_0)

/*
   uint32_t progMemMD5check(){
    checkRAM(F("progMemMD5check"));
 #define BufSize 10
    uint32_t calcBuffer[BufSize];
    CRCValues.numberOfCRCBytes = 0;
    memcpy (calcBuffer,CRCValues.compileTimeMD5,16);                                                  // is there still the dummy in memory
       ? - the dummy needs to be replaced by the real md5 after linking.
    if( memcmp (calcBuffer, "MD5_MD5_MD5_",12)==0){                                                   // do not memcmp with CRCdummy
       directly or it will get optimized away.
        addLog(LOG_LEVEL_INFO, F("CRC  : No program memory checksum found. Check output of crc2.py"));
        return 0;
    }
    MD5Builder md5;
    md5.begin();
    for (int l = 0; l<4; l++){                                                                            // check max segments,  if the
       pointer is not 0
        uint32_t *ptrStart = (uint32_t *)&CRCValues.compileTimeMD5[16+l*4];
        uint32_t *ptrEnd =   (uint32_t *)&CRCValues.compileTimeMD5[16+4*4+l*4];
        if ((*ptrStart) == 0) break;                                                                      // segment not used.
        for (uint32_t i = *ptrStart; i< (*ptrEnd) ; i=i+sizeof(calcBuffer)){                              // "<" includes last byte
             for (int buf = 0; buf < BufSize; buf ++){
                calcBuffer[buf] = pgm_read_dword((uint32_t*)i+buf);                                       // read 4 bytes
                CRCValues.numberOfCRCBytes+=sizeof(calcBuffer[0]);
             }
             md5.add((uint8_t *)&calcBuffer[0],(*ptrEnd-i)<sizeof(calcBuffer) ? (*ptrEnd-i):sizeof(calcBuffer) );     // add buffer to md5.
                At the end not the whole buffer. md5 ptr to data in ram.
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
   If RX and TX tied together, perform emergency reset to get the system out of boot loops
 \*********************************************************************************************/
void emergencyReset()
{
  // Direct Serial is allowed here, since this is only an emergency task.
  Serial.begin(115200);
  Serial.write(0xAA);
  Serial.write(0x55);
  delay(1);

  if (Serial.available() == 2) {
    if ((Serial.read() == 0xAA) && (Serial.read() == 0x55))
    {
      serialPrintln(F("\n\n\rSystem will reset to factory defaults in 10 seconds..."));
      delay(10000);
      ResetFactory();
    }
  }
}

/********************************************************************************************\
   Get free system mem
 \*********************************************************************************************/
unsigned long FreeMem(void)
{
  #if defined(ESP8266)
  return system_get_free_heap_size();
  #endif // if defined(ESP8266)
  #if defined(ESP32)
  return ESP.getFreeHeap();
  #endif // if defined(ESP32)
}

unsigned long getMaxFreeBlock()
{
  unsigned long freemem = FreeMem();

  #ifdef CORE_POST_2_5_0

  // computing max free block is a rather extensive operation, so only perform when free memory is already low.
  if (freemem < 6144) {
    return ESP.getMaxFreeBlockSize();
  }
  #endif // ifdef CORE_POST_2_5_0
  return freemem;
}

void logtimeStringToSeconds(const String& tBuf, int hours, int minutes, int seconds)
{
  #ifndef BUILD_NO_DEBUG

  if (loglevelActiveFor(LOG_LEVEL_DEBUG)) {
    String log;
    log  = F("timeStringToSeconds: ");
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





void SendValueLogger(taskIndex_t TaskIndex)
{
#if !defined(BUILD_NO_DEBUG) || defined(FEATURE_SD)
  bool   featureSD = false;
  String logger;
  # ifdef FEATURE_SD
  featureSD = true;
  # endif // ifdef FEATURE_SD

  if (featureSD || loglevelActiveFor(LOG_LEVEL_DEBUG)) {
    const deviceIndex_t DeviceIndex = getDeviceIndex_from_TaskIndex(TaskIndex);

    if (validDeviceIndex(DeviceIndex)) {
      LoadTaskSettings(TaskIndex);
      const byte valueCount = getValueCountForTask(TaskIndex);

      for (byte varNr = 0; varNr < valueCount; varNr++)
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
#endif // if !defined(BUILD_NO_DEBUG) || defined(FEATURE_SD)

#ifdef FEATURE_SD
  String filename = F("VALUES.CSV");
  File   logFile  = SD.open(filename, FILE_WRITE);

  if (logFile) {
    logFile.print(logger);
  }
  logFile.close();
#endif // ifdef FEATURE_SD
}

// #ifdef PLUGIN_BUILD_TESTING

// #define isdigit(n) (n >= '0' && n <= '9') //Conflicts with ArduJson 6+, when this lib is used there is no need for this macro

/********************************************************************************************\
   Generate a tone of specified frequency on pin
 \*********************************************************************************************/
void tone_espEasy(uint8_t _pin, unsigned int frequency, unsigned long duration) {
  #ifdef ESP32
  toneESP32(_pin, frequency, duration);
  #else // ifdef ESP32
  analogWriteFreq(frequency);

  // NOTE: analogwrite reserves IRAM and uninitalized ram.
  analogWrite(_pin, 100);
  delay(duration);
  analogWrite(_pin, 0);
  #endif // ifdef ESP32
}

/********************************************************************************************\
   Play RTTTL string on specified pin
 \*********************************************************************************************/
void play_rtttl(uint8_t _pin, const char *p)
{
  checkRAM(F("play_rtttl"));
  #define OCTAVE_OFFSET 0

  // FIXME: Absolutely no error checking in here

  const int notes[] = { 0,
                        262, 277,  294,   311,  330,  349,  370,  392,  415,  440,  466,  494,
                        523, 554,  587,   622,  659,  698,  740,  784,  831,  880,  932,  988,
                        1047,1109, 1175,  1245, 1319, 1397, 1480, 1568, 1661, 1760, 1865, 1976,
                        2093,2217, 2349,  2489, 2637, 2794, 2960, 3136, 3322, 3520, 3729, 3951
  };


  byte default_dur = 4;
  byte default_oct = 6;
  int  bpm         = 63;
  int  num;
  long wholenote;
  long duration;
  byte note;
  byte scale;

  // format: d=N,o=N,b=NNN:
  // find the start (skip name, etc)

  while (*p != ':') { p++; // ignore name
  }
  p++;                     // skip ':'

  // get default duration
  if (*p == 'd')
  {
    p++; p++; // skip "d="
    num = 0;

    while (isdigit(*p))
    {
      num = (num * 10) + (*p++ - '0');
    }

    if (num > 0) { default_dur = num; }
    p++; // skip comma
  }

  // get default octave
  if (*p == 'o')
  {
    p++; p++; // skip "o="
    num = *p++ - '0';

    if ((num >= 3) && (num <= 7)) { default_oct = num; }
    p++; // skip comma
  }

  // get BPM
  if (*p == 'b')
  {
    p++; p++; // skip "b="
    num = 0;

    while (isdigit(*p))
    {
      num = (num * 10) + (*p++ - '0');
    }
    bpm = num;
    p++; // skip colon
  }

  // BPM usually expresses the number of quarter notes per minute
  wholenote = (60 * 1000L / bpm) * 4; // this is the time for whole note (in milliseconds)

  // now begin note loop
  while (*p)
  {
    // first, get note duration, if available
    num = 0;

    while (isdigit(*p))
    {
      num = (num * 10) + (*p++ - '0');
    }

    if (num) { duration = wholenote / num; }
    else { duration = wholenote / default_dur; // we will need to check if we are a dotted note after
    }

    // now get the note
    switch (*p)
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
    if (*p == '#')
    {
      note++;
      p++;
    }

    // now, get optional '.' dotted note
    if (*p == '.')
    {
      duration += duration / 2;
      p++;
    }

    // now, get scale
    if (isdigit(*p))
    {
      scale = *p - '0';
      p++;
    }
    else
    {
      scale = default_oct;
    }

    scale += OCTAVE_OFFSET;

    if (*p == ',') {
      p++; // skip comma for next note (or we may be at the end)
    }

    // now play the note
    if (note)
    {
      tone_espEasy(_pin, notes[(scale - 4) * 12 + note], duration);
    }
    else
    {
      delay(duration / 10);
    }
  }
  checkRAM(F("play_rtttl2"));
}

// #endif

bool OTA_possible(uint32_t& maxSketchSize, bool& use2step) {
#if defined(ESP8266)

  // Compute the current free space and sketch size, rounded to 4k blocks.
  // These block bounaries are needed for erasing a full block on flash.
  const uint32_t freeSketchSpace            = (ESP.getFreeSketchSpace() - 0x1000) & 0xFFFFF000;
  const uint32_t currentSketchSize          = (ESP.getSketchSize() + 0x1000) & 0xFFFFF000;
  const uint32_t smallestOtaImageSizeNeeded = (((SMALLEST_OTA_IMAGE + 16) + 0x1000) & 0xFFFFF000);
  const bool     otaPossible                = freeSketchSpace >= smallestOtaImageSizeNeeded;
  use2step = freeSketchSpace < currentSketchSize; // Assume the new image has the same size.

  if (use2step) {
    const uint32_t totalSketchSpace = freeSketchSpace + currentSketchSize;
    maxSketchSize = totalSketchSpace - smallestOtaImageSizeNeeded;
  } else {
    maxSketchSize = freeSketchSpace;
  }
  maxSketchSize -= 16; // Must leave 16 bytes at the end.

  if (maxSketchSize > MAX_SKETCH_SIZE) { maxSketchSize = MAX_SKETCH_SIZE; }
  return otaPossible;
#elif defined(ESP32)
  maxSketchSize = MAX_SKETCH_SIZE;
  use2step      = false;
  return true;
#else // if defined(ESP8266)
  return false;
#endif // if defined(ESP8266)
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

  if (SecuritySettings.Password[0] != 0) {
    ArduinoOTA.setPassword(SecuritySettings.Password);
  }

  ArduinoOTA.onStart([]() {
    serialPrintln(F("OTA  : Start upload"));
    ArduinoOTAtriggered = true;
    ESPEASY_FS.end(); // important, otherwise it fails
  });

  ArduinoOTA.onEnd([]() {
    serialPrintln(F("\nOTA  : End"));

    // "dangerous": if you reset during flash you have to reflash via serial
    // so dont touch device until restart is complete
    serialPrintln(F("\nOTA  : DO NOT RESET OR POWER OFF UNTIL BOOT+FLASH IS COMPLETE."));

    // delay(100);
    // reboot(); //Not needed, node reboots automaticall after calling onEnd and succesfully flashing
  });
  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
    if (Settings.UseSerial) {
      Serial.printf("OTA  : Progress %u%%\r", (progress / (total / 100)));
    }
  });

  ArduinoOTA.onError([](ota_error_t error) {
    serialPrint(F("\nOTA  : Error (will reboot): "));

    if (error == OTA_AUTH_ERROR) { serialPrintln(F("Auth Failed")); }
    else if (error == OTA_BEGIN_ERROR) { serialPrintln(F("Begin Failed")); }
    else if (error == OTA_CONNECT_ERROR) { serialPrintln(F("Connect Failed")); }
    else if (error == OTA_RECEIVE_ERROR) { serialPrintln(F("Receive Failed")); }
    else if (error == OTA_END_ERROR) { serialPrintln(F("End Failed")); }

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

#endif // ifdef FEATURE_ARDUINO_OTA




// #######################################################################################################
// ############################ quite acurate but slow color converter####################################
// #######################################################################################################
// uses H 0..360 S 1..100 I/V 1..100 (according to homie convention)
// Source https://blog.saikoled.com/post/44677718712/how-to-convert-from-hsi-to-rgb-white

void HSV2RGB(float H, float S, float I, int rgb[3]) {
  int r, g, b;

  H = fmod(H, 360);                // cycle H around to 0-360 degrees
  H = 3.14159f * H / (float)180;   // Convert to radians.
  S = S / 100;
  S = S > 0 ? (S < 1 ? S : 1) : 0; // clamp S and I to interval [0,1]
  I = I / 100;
  I = I > 0 ? (I < 1 ? I : 1) : 0;

  // Math! Thanks in part to Kyle Miller.
  if (H < 2.09439f) {
    r = 255 * I / 3 * (1 + S * cos(H) / cos(1.047196667f - H));
    g = 255 * I / 3 * (1 + S * (1 - cos(H) / cos(1.047196667f - H)));
    b = 255 * I / 3 * (1 - S);
  } else if (H < 4.188787f) {
    H = H - 2.09439f;
    g = 255 * I / 3 * (1 + S * cos(H) / cos(1.047196667f - H));
    b = 255 * I / 3 * (1 + S * (1 - cos(H) / cos(1.047196667f - H)));
    r = 255 * I / 3 * (1 - S);
  } else {
    H = H - 4.188787f;
    b = 255 * I / 3 * (1 + S * cos(H) / cos(1.047196667f - H));
    r = 255 * I / 3 * (1 + S * (1 - cos(H) / cos(1.047196667f - H)));
    g = 255 * I / 3 * (1 - S);
  }
  rgb[0] = r;
  rgb[1] = g;
  rgb[2] = b;
}

// uses H 0..360 S 1..100 I/V 1..100 (according to homie convention)
// Source https://blog.saikoled.com/post/44677718712/how-to-convert-from-hsi-to-rgb-white

void HSV2RGBW(float H, float S, float I, int rgbw[4]) {
  int   r, g, b, w;
  float cos_h, cos_1047_h;

  H = fmod(H, 360);                // cycle H around to 0-360 degrees
  H = 3.14159f * H / (float)180;   // Convert to radians.
  S = S / 100;
  S = S > 0 ? (S < 1 ? S : 1) : 0; // clamp S and I to interval [0,1]
  I = I / 100;
  I = I > 0 ? (I < 1 ? I : 1) : 0;

  if (H < 2.09439f) {
    cos_h      = cos(H);
    cos_1047_h = cos(1.047196667f - H);
    r          = S * 255 * I / 3 * (1 + cos_h / cos_1047_h);
    g          = S * 255 * I / 3 * (1 + (1 - cos_h / cos_1047_h));
    b          = 0;
    w          = 255 * (1 - S) * I;
  } else if (H < 4.188787f) {
    H          = H - 2.09439f;
    cos_h      = cos(H);
    cos_1047_h = cos(1.047196667f - H);
    g          = S * 255 * I / 3 * (1 + cos_h / cos_1047_h);
    b          = S * 255 * I / 3 * (1 + (1 - cos_h / cos_1047_h));
    r          = 0;
    w          = 255 * (1 - S) * I;
  } else {
    H          = H - 4.188787f;
    cos_h      = cos(H);
    cos_1047_h = cos(1.047196667f - H);
    b          = S * 255 * I / 3 * (1 + cos_h / cos_1047_h);
    r          = S * 255 * I / 3 * (1 + (1 - cos_h / cos_1047_h));
    g          = 0;
    w          = 255 * (1 - S) * I;
  }

  rgbw[0] = r;
  rgbw[1] = g;
  rgbw[2] = b;
  rgbw[3] = w;
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
