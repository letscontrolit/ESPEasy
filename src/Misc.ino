// clean up tcp connections that are in TIME_WAIT status, to conserve memory
// In future versions of WiFiClient it should be possible to call abort(), but
// this feature is not in all upstream versions yet.
// See https://github.com/esp8266/Arduino/issues/1923
// and https://github.com/letscontrolit/ESPEasy/issues/253
#if defined(ESP8266)
  #include <md5.h>
#endif
#if defined(ESP8266)

struct tcp_pcb;
extern struct tcp_pcb* tcp_tw_pcbs;
extern "C" void tcp_abort (struct tcp_pcb* pcb);

void tcpCleanup()
{

     while(tcp_tw_pcbs!=NULL)
    {
      tcp_abort(tcp_tw_pcbs);
    }

 }
#endif


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
#if defined(ARDUINO_ESP8266_RELEASE_2_3_0) || defined(ARDUINO_ESP8266_RELEASE_2_4_0) || defined(ARDUINO_ESP8266_RELEASE_2_4_1)
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

bool isDeepSleepEnabled()
{
  if (!Settings.deepSleep)
    return false;

  //cancel deep sleep loop by pulling the pin GPIO16(D0) to GND
  //recommended wiring: 3-pin-header with 1=RST, 2=D0, 3=GND
  //                    short 1-2 for normal deep sleep / wakeup loop
  //                    short 2-3 to cancel sleep loop for modifying settings
  pinMode(16,INPUT_PULLUP);
  if (!digitalRead(16))
  {
    return false;
  }
  return true;
}

bool readyForSleep()
{
  if (!isDeepSleepEnabled())
    return false;
  if (!checkConnectionsEstablished()) {
    // Allow 12 seconds to establish connections
    return timeOutReached(timerAwakeFromDeepSleep + 12000);
  }
  return timeOutReached(timerAwakeFromDeepSleep + 1000 * Settings.deepSleep);
}

void deepSleep(int delay)
{

  checkRAM(F("deepSleep"));
  if (!isDeepSleepEnabled())
  {
    //Deep sleep canceled by GPIO16(D0)=LOW
    return;
  }

  //first time deep sleep? offer a way to escape
  if (lastBootCause!=BOOT_CAUSE_DEEP_SLEEP)
  {
    addLog(LOG_LEVEL_INFO, F("SLEEP: Entering deep sleep in 30 seconds."));
    if (Settings.UseRules && isDeepSleepEnabled())
      {
        String event = F("System#NoSleep=30");
        rulesProcessing(event);
      }
    delayBackground(30000);
    //disabled?
    if (!isDeepSleepEnabled())
    {
      addLog(LOG_LEVEL_INFO, F("SLEEP: Deep sleep cancelled (GPIO16 connected to GND)"));
      return;
    }
  }
  saveUserVarToRTC();
  deepSleepStart(delay); // Call deepSleepStart function after these checks
}

void deepSleepStart(int delay)
{
  // separate function that is called from above function or directly from rules, usign deepSleep as a one-shot
  String event = F("System#Sleep");
  rulesProcessing(event);


  RTC.deepSleepState = 1;
  saveToRTC();

  if (delay > 4294 || delay < 0)
    delay = 4294;   //max sleep time ~1.2h

  addLog(LOG_LEVEL_INFO, F("SLEEP: Powering down to deepsleep..."));
  #if defined(ESP8266)
    ESP.deepSleep((uint32_t)delay * 1000000, WAKE_RF_DEFAULT);
  #endif
  #if defined(ESP32)
    esp_sleep_enable_timer_wakeup((uint32_t)delay * 1000000);
    esp_deep_sleep_start();
  #endif
}

boolean remoteConfig(struct EventStruct *event, const String& string)
{
  checkRAM(F("remoteConfig"));
  boolean success = false;
  String command = parseString(string, 1);

  if (command == F("config"))
  {
    success = true;
    if (parseString(string, 2) == F("task"))
    {
      String configTaskName = parseStringKeepCase(string, 3);
      String configCommand = parseStringToEndKeepCase(string, 4);
      if (configTaskName.length() == 0 || configCommand.length() == 0)
        return success; // TD-er: Should this be return false?

      int8_t index = getTaskIndexByName(configTaskName);
      if (index != -1)
      {
        event->TaskIndex = index;
        success = PluginCall(PLUGIN_SET_CONFIG, event, configCommand);
      }
    }
  }
  return success;
}

int8_t getTaskIndexByName(String TaskNameSearch)
{

  for (byte x = 0; x < TASKS_MAX; x++)
  {
    LoadTaskSettings(x);
    String TaskName = getTaskDeviceName(x);
    if ((TaskName.length() != 0 ) && (TaskNameSearch.equalsIgnoreCase(TaskName)))
    {
      return x;
    }
  }
  return -1;
}



/*********************************************************************************************\
   set pin mode & state (info table)
  \*********************************************************************************************/
void setPinState(byte plugin, byte index, byte mode, uint16_t value)
{
  // plugin number and index form a unique key
  // first check if this pin is already known
  boolean reUse = false;
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


/*********************************************************************************************\
   get pin mode & state (info table)
  \*********************************************************************************************/
boolean getPinState(byte plugin, byte index, byte *mode, uint16_t *value)
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


/*********************************************************************************************\
   check if pin mode & state is known (info table)
  \*********************************************************************************************/
boolean hasPinState(byte plugin, byte index)
{
  for (byte x = 0; x < PINSTATE_TABLE_MAX; x++)
    if ((pinStates[x].plugin == plugin) && (pinStates[x].index == index))
    {
      return true;
    }
  return false;
}


/*********************************************************************************************\
   Bitwise operators
  \*********************************************************************************************/
bool getBitFromUL(uint32_t number, byte bitnr) {
  return (number >> bitnr) & 1UL;
}

void setBitToUL(uint32_t& number, byte bitnr, bool value) {
  uint32_t newbit = value ? 1UL : 0UL;
  number ^= (-newbit ^ number) & (1UL << bitnr);
}


/*********************************************************************************************\
   report pin mode & state (info table) using json
  \*********************************************************************************************/
String getPinStateJSON(boolean search, byte plugin, byte index, String& log, uint16_t noSearchValue)
{
  checkRAM(F("getPinStateJSON"));
  printToWebJSON = true;
  byte mode = PIN_MODE_INPUT;
  uint16_t value = noSearchValue;
  String reply = "";
  boolean found = false;

  if (search)
  {
    for (byte x = 0; x < PINSTATE_TABLE_MAX; x++)
      if ((pinStates[x].plugin == plugin) && (pinStates[x].index == index))
      {
        mode = pinStates[x].mode;
        value = pinStates[x].value;
        found = true;
        break;
      }
  }

  if (!search || (search && found))
  {
    reply += F("{\n\"log\": \"");
    reply += log.substring(7, 32); // truncate to 25 chars, max MQTT message size = 128 including header...
    reply += F("\",\n\"plugin\": ");
    reply += plugin;
    reply += F(",\n\"pin\": ");
    reply += index;
    reply += F(",\n\"mode\": \"");
    switch (mode)
    {
      case PIN_MODE_UNDEFINED:
        reply += F("undefined");
        break;
      case PIN_MODE_INPUT:
        reply += F("input");
        break;
      case PIN_MODE_OUTPUT:
        reply += F("output");
        break;
      case PIN_MODE_PWM:
        reply += F("PWM");
        break;
      case PIN_MODE_SERVO:
        reply += F("servo");
        break;
    }
    reply += F("\",\n\"state\": ");
    reply += value;
    reply += F("\n}\n");
    return reply;
  }
  return "?";
}


/********************************************************************************************\
  Status LED
\*********************************************************************************************/
#if defined(ESP32)
  #define PWMRANGE 1024
#endif
#define STATUS_PWM_NORMALVALUE (PWMRANGE>>2)
#define STATUS_PWM_NORMALFADE (PWMRANGE>>8)
#define STATUS_PWM_TRAFFICRISE (PWMRANGE>>1)

void statusLED(boolean traffic)
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

    if (wifiStatus == ESPEASY_WIFI_SERVICES_INITIALIZED)
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
      nStatusValue = ((millis()>>1) & PWMRANGE) - (PWMRANGE>>2); //ramp up for 2 sec, 3/4 luminosity
    }
    //Disconnected
    else
    {
      nStatusValue = (millis()>>1) & (PWMRANGE>>2); //ramp up for 1/2 sec, 1/4 luminosity
    }
  }

  nStatusValue = constrain(nStatusValue, 0, PWMRANGE);

  if (gnStatusValueCurrent != nStatusValue)
  {
    gnStatusValueCurrent = nStatusValue;

    long pwm = nStatusValue * nStatusValue; //simple gamma correction
    pwm >>= 10;
    if (Settings.Pin_status_led_Inversed)
      pwm = PWMRANGE-pwm;

    #if defined(ESP8266)
      analogWrite(Settings.Pin_status_led, pwm);
    #endif
  }
}


/********************************************************************************************\
  delay in milliseconds with background processing
  \*********************************************************************************************/
void delayBackground(unsigned long delay)
{
  unsigned long timer = millis() + delay;
  while (!timeOutReached(timer))
    backgroundtasks();
}


/********************************************************************************************\
  Parse a command string to event struct
  \*********************************************************************************************/
void parseCommandString(struct EventStruct *event, const String& string)
{
  checkRAM(F("parseCommandString"));
  char *TmpStr1 = new char[INPUT_COMMAND_SIZE]();
  event->Par1 = 0;
  event->Par2 = 0;
  event->Par3 = 0;
  event->Par4 = 0;
  event->Par5 = 0;

  if (GetArgv(string.c_str(), TmpStr1, 2)) { event->Par1 = CalculateParam(TmpStr1); }
  if (GetArgv(string.c_str(), TmpStr1, 3)) { event->Par2 = CalculateParam(TmpStr1); }
  if (GetArgv(string.c_str(), TmpStr1, 4)) { event->Par3 = CalculateParam(TmpStr1); }
  if (GetArgv(string.c_str(), TmpStr1, 5)) { event->Par4 = CalculateParam(TmpStr1); }
  if (GetArgv(string.c_str(), TmpStr1, 6)) { event->Par5 = CalculateParam(TmpStr1); }
  delete[] TmpStr1;
}

/********************************************************************************************\
  Clear task settings for given task
  \*********************************************************************************************/
void taskClear(byte taskIndex, boolean save)
{
  checkRAM(F("taskClear"));
  Settings.clearTask(taskIndex);
  ExtraTaskSettings.clear(); // Invalidate any cached values.
  ExtraTaskSettings.TaskIndex = taskIndex;
  if (save) {
    SaveTaskSettings(taskIndex);
    SaveSettings();
  }
}

String checkTaskSettings(byte taskIndex) {
  String err = LoadTaskSettings(taskIndex);
  if (err.length() > 0) return err;
  if (!ExtraTaskSettings.checkUniqueValueNames()) {
    return F("Use unique value names");
  }
  if (!ExtraTaskSettings.checkInvalidCharInNames()) {
    return F("Invalid character in names. Do not use ',#[]' or space.");
  }
  String deviceName = ExtraTaskSettings.TaskDeviceName;
  if (deviceName.length() == 0) {
    if (Settings.TaskDeviceEnabled[taskIndex]) {
      // Decide what to do here, for now give a warning when task is enabled.
      return F("Warning: Task Device Name is empty. It is adviced to give tasks an unique name");
    }
  }
  for (int i = 0; i < TASKS_MAX; ++i) {
    if (i != taskIndex && Settings.TaskDeviceEnabled[i]) {
      LoadTaskSettings(i);
      if (ExtraTaskSettings.TaskDeviceName[0] != 0) {
        if (strcasecmp(ExtraTaskSettings.TaskDeviceName, deviceName.c_str()) == 0) {
          err = F("Task Device Name is not unique, conflicts with task ID #");
          err += (i+1);
          return err;
        }
      }
    }
  }

  err = LoadTaskSettings(taskIndex);
  return err;
}



/********************************************************************************************\
  Find device index corresponding to task number setting
  \*********************************************************************************************/
byte getDeviceIndex(byte Number)
{
  for (byte x = 0; x <= deviceCount ; x++) {
    if (Device[x].Number == Number) {
      return x;
    }
  }
  return 0;
}

/********************************************************************************************\
  Find name of plugin given the plugin device index..
  \*********************************************************************************************/
String getPluginNameFromDeviceIndex(byte deviceIndex) {
  String deviceName = "";
  Plugin_ptr[deviceIndex](PLUGIN_GET_DEVICENAME, 0, deviceName);
  return deviceName;
}


/********************************************************************************************\
  Find protocol index corresponding to protocol setting
  \*********************************************************************************************/
byte getProtocolIndex(byte Number)
{
  for (byte x = 0; x <= protocolCount ; x++) {
    if (Protocol[x].Number == Number) {
      return x;
    }
  }
  return 0;
}

/********************************************************************************************\
  Get notificatoin protocol index (plugin index), by NPlugin_id
  \*********************************************************************************************/
byte getNotificationProtocolIndex(byte Number)
{
  for (byte x = 0; x <= notificationCount ; x++) {
    if (Notification[x].Number == Number) {
      return(x);
    }
  }
  return(NPLUGIN_NOT_FOUND);
}

/********************************************************************************************\
  Find positional parameter in a char string
  \*********************************************************************************************/
boolean GetArgv(const char *string, char *argv, unsigned int argc) {
  return GetArgv(string, argv, INPUT_COMMAND_SIZE, argc);
}

boolean GetArgv(const char *string, char *argv, unsigned int argv_size, unsigned int argc)
{
  memset(argv, 0, argv_size);
  size_t string_len = strlen(string);
  unsigned int string_pos = 0, argv_pos = 0, argc_pos = 0;
  char c, d;
  boolean parenthesis = false;
  char matching_parenthesis = '"';

  while (string_pos < string_len)
  {
    c = string[string_pos];
    d = 0;
    if ((string_pos + 1) < string_len)
      d = string[string_pos + 1];

    if       (!parenthesis && c == ' ' && d == ' ') {}
    else if  (!parenthesis && c == ' ' && d == ',') {}
    else if  (!parenthesis && c == ',' && d == ' ') {}
    else if  (!parenthesis && c == ' ' && d >= 33 && d <= 126) {}
    else if  (!parenthesis && c == ',' && d >= 33 && d <= 126) {}
    else if  (c == '"' || c == '\'' || c == '[') {
      parenthesis = true;
      matching_parenthesis = c;
      if (c == '[')
        matching_parenthesis = ']';
    }
    else
    {
      if ((argv_pos +2 ) >= argv_size) {
        if (loglevelActiveFor(LOG_LEVEL_ERROR)) {
          String log = F("GetArgv Error; argv_size exceeded. argc=");
          log += argc;
          log += F(" argv_size=");
          log += argv_size;
          log += F(" argv=");
          log += argv;
          addLog(LOG_LEVEL_ERROR, log);
        }
        return false;
      }
      argv[argv_pos++] = c;
      argv[argv_pos] = 0;

      if ((!parenthesis && (d == ' ' || d == ',' || d == 0)) || (parenthesis && (d == matching_parenthesis))) // end of word
      {
        if (d == matching_parenthesis)
          parenthesis = false;
        argv[argv_pos] = 0;
        argc_pos++;

        if (argc_pos == argc)
        {
          return true;
        }

        argv[0] = 0;
        argv_pos = 0;
        string_pos++;
      }
    }
    string_pos++;
  }
  return false;
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
  Serial.print (addr, HEX);
  Serial.print(": ");
  for (uint32_t a = addr; a < addr + 16; a++)
  {
    Serial.print ( pgm_read_byte(a), HEX);
    Serial.print (" ");
  }
  Serial.println("");
}
#endif

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

/********************************************************************************************\
  Handler for keeping ExtraTaskSettings up to date using cache
  \*********************************************************************************************/
String getTaskDeviceName(byte TaskIndex) {
  LoadTaskSettings(TaskIndex);
  return ExtraTaskSettings.TaskDeviceName;
}


/********************************************************************************************\
  Reset all settings to factory defaults
  \*********************************************************************************************/
void ResetFactory(void)
{

  checkRAM(F("ResetFactory"));
  // Direct Serial is allowed here, since this is only an emergency task.
  Serial.println(F("RESET: Resetting factory defaults..."));
  delay(1000);
  if (readFromRTC())
  {
    Serial.print(F("RESET: Warm boot, reset count: "));
    Serial.println(RTC.factoryResetCounter);
    if (RTC.factoryResetCounter >= 3)
    {
      Serial.println(F("RESET: Too many resets, protecting your flash memory (powercycle to solve this)"));
      return;
    }
  }
  else
  {
    Serial.println(F("RESET: Cold boot"));
    initRTC();
  }

  RTC.flashCounter=0; //reset flashcounter, since we're already counting the number of factory-resets. we dont want to hit a flash-count limit during reset.
  RTC.factoryResetCounter++;
  saveToRTC();

  //always format on factory reset, in case of corrupt SPIFFS
  SPIFFS.end();
  Serial.println(F("RESET: formatting..."));
  SPIFFS.format();
  Serial.println(F("RESET: formatting done..."));
  if (!SPIFFS.begin())
  {
    Serial.println(F("RESET: FORMAT SPIFFS FAILED!"));
    return;
  }


  //pad files with extra zeros for future extensions
  String fname;

  fname=FILE_CONFIG;
  InitFile(fname.c_str(), CONFIG_FILE_SIZE);

  fname=FILE_SECURITY;
  InitFile(fname.c_str(), 4096);

  fname=FILE_NOTIFICATION;
  InitFile(fname.c_str(), 4096);

  fname=FILE_RULES;
  InitFile(fname.c_str(), 0);

  Settings.clearAll();
  // now we set all parameters that need to be non-zero as default value

#if DEFAULT_USE_STATIC_IP
  str2ip((char*)DEFAULT_IP, Settings.IP);
  str2ip((char*)DEFAULT_DNS, Settings.DNS);
  str2ip((char*)DEFAULT_GW, Settings.Gateway);
  str2ip((char*)DEFAULT_SUBNET, Settings.Subnet);
#endif

  Settings.PID             = ESP_PROJECT_PID;
  Settings.Version         = VERSION;
  Settings.Unit            = UNIT;
  strcpy_P(SecuritySettings.WifiSSID, PSTR(DEFAULT_SSID));
  strcpy_P(SecuritySettings.WifiKey, PSTR(DEFAULT_KEY));
  strcpy_P(SecuritySettings.WifiAPKey, PSTR(DEFAULT_AP_KEY));
  SecuritySettings.Password[0] = 0;
  // TD-er Reset access control
  str2ip((char*)DEFAULT_IPRANGE_LOW, SecuritySettings.AllowedIPrangeLow);
  str2ip((char*)DEFAULT_IPRANGE_HIGH, SecuritySettings.AllowedIPrangeHigh);
  SecuritySettings.IPblockLevel = DEFAULT_IP_BLOCK_LEVEL;

  Settings.Delay           = DEFAULT_DELAY;
  Settings.Pin_i2c_sda     = DEFAULT_PIN_I2C_SDA;
  Settings.Pin_i2c_scl     = DEFAULT_PIN_I2C_SCL;
  Settings.Pin_status_led  = DEFAULT_PIN_STATUS_LED;
  Settings.Pin_status_led_Inversed  = DEFAULT_PIN_STATUS_LED_INVERSED;
  Settings.Pin_sd_cs       = -1;
  Settings.Pin_Reset = -1;
  Settings.Protocol[0]        = DEFAULT_PROTOCOL;
  strcpy_P(Settings.Name, PSTR(DEFAULT_NAME));
  Settings.deepSleep = false;
  Settings.CustomCSS = false;
  Settings.InitSPI = false;
  for (byte x = 0; x < TASKS_MAX; x++)
  {
    Settings.TaskDevicePin1[x] = -1;
    Settings.TaskDevicePin2[x] = -1;
    Settings.TaskDevicePin3[x] = -1;
    Settings.TaskDevicePin1PullUp[x] = true;
    Settings.TaskDevicePin1Inversed[x] = false;
    for (byte y = 0; y < CONTROLLER_MAX; y++)
      Settings.TaskDeviceSendData[y][x] = true;
    Settings.TaskDeviceTimer[x] = Settings.Delay;
  }
  Settings.Build = BUILD;

	// advanced Settings
	Settings.UseRules 		= DEFAULT_USE_RULES;

	Settings.MQTTRetainFlag	= DEFAULT_MQTT_RETAIN;
	Settings.MessageDelay	= DEFAULT_MQTT_DELAY;
	Settings.MQTTUseUnitNameAsClientId = DEFAULT_MQTT_USE_UNITNAME_AS_CLIENTID;

    Settings.UseNTP			= DEFAULT_USE_NTP;
	strcpy_P(Settings.NTPHost, PSTR(DEFAULT_NTP_HOST));
	Settings.TimeZone		= DEFAULT_TIME_ZONE;
    Settings.DST 			= DEFAULT_USE_DST;

	str2ip((char*)DEFAULT_SYSLOG_IP, Settings.Syslog_IP);

  setLogLevelFor(LOG_TO_SYSLOG, DEFAULT_SYSLOG_LEVEL);
  setLogLevelFor(LOG_TO_SERIAL, DEFAULT_SERIAL_LOG_LEVEL);
	setLogLevelFor(LOG_TO_WEBLOG, DEFAULT_WEB_LOG_LEVEL);
  setLogLevelFor(LOG_TO_SDCARD, DEFAULT_SD_LOG_LEVEL);
  Settings.SyslogFacility	= DEFAULT_SYSLOG_FACILITY;
	Settings.UseValueLogger = DEFAULT_USE_SD_LOG;

	Settings.UseSerial		= DEFAULT_USE_SERIAL;
	Settings.BaudRate		= DEFAULT_SERIAL_BAUD;

/*
	Settings.GlobalSync						= DEFAULT_USE_GLOBAL_SYNC;
	Settings.UDPPort						= DEFAULT_SYNC_UDP_PORT;

	Settings.IP_Octet						= DEFAULT_IP_OCTET;
	Settings.WDI2CAddress					= DEFAULT_WD_IC2_ADDRESS;
	Settings.UseSSDP						= DEFAULT_USE_SSDP;
	Settings.ConnectionFailuresThreshold	= DEFAULT_CON_FAIL_THRES;
	Settings.WireClockStretchLimit			= DEFAULT_I2C_CLOCK_LIMIT;
*/

#ifdef PLUGIN_DESCR
  strcpy_P(Settings.Name, PSTR(PLUGIN_DESCR));
#endif

  addPredefinedPlugins();
  addPredefinedRules();




  SaveSettings();

#if DEFAULT_CONTROLLER
  MakeControllerSettings(ControllerSettings);
  strcpy_P(ControllerSettings.Subscribe, PSTR(DEFAULT_SUB));
  strcpy_P(ControllerSettings.Publish, PSTR(DEFAULT_PUB));
  strcpy_P(ControllerSettings.MQTTLwtTopic, PSTR(DEFAULT_MQTT_LWT_TOPIC));
  strcpy_P(ControllerSettings.LWTMessageConnect, PSTR(DEFAULT_MQTT_LWT_CONNECT_MESSAGE));
  strcpy_P(ControllerSettings.LWTMessageDisconnect, PSTR(DEFAULT_MQTT_LWT_DISCONNECT_MESSAGE));
  str2ip((char*)DEFAULT_SERVER, ControllerSettings.IP);
  ControllerSettings.HostName[0]=0;
  ControllerSettings.Port = DEFAULT_PORT;
  SaveControllerSettings(0, ControllerSettings);
#endif
  checkRAM(F("ResetFactory2"));
  Serial.println(F("RESET: Succesful, rebooting. (you might need to press the reset button if you've justed flashed the firmware)"));
  //NOTE: this is a known ESP8266 bug, not our fault. :)
  delay(1000);
  WiFi.persistent(true); // use SDK storage of SSID/WPA parameters
  intent_to_reboot = true;
  WifiDisconnect(); // this will store empty ssid/wpa into sdk storage
  WiFi.persistent(false); // Do not use SDK storage of SSID/WPA parameters
  reboot();
}

void addSwitchPlugin(byte taskIndex, byte gpio, const String& name, bool activeLow) {
  setTaskDevice_to_TaskIndex(1, taskIndex);
  setBasicTaskValues(
    taskIndex,
    0,            // taskdevicetimer
    true,         // enabled
    name,         // name
    gpio,         // pin1
    -1,            // pin2
    -1);           // pin3
  Settings.TaskDevicePin1PullUp[taskIndex] = true;
  if (activeLow)
    Settings.TaskDevicePluginConfig[taskIndex][2] = 1; // PLUGIN_001_BUTTON_TYPE_PUSH_ACTIVE_LOW;
}

bool addPredefinedPlugins() {
  byte taskIndex = 0;
  #ifdef GPIO_KEY1
  // Create button switch P001
  addSwitchPlugin(taskIndex, GPIO_KEY1, F("Button1"), true);
  ++taskIndex;
  #endif // GPIO_KEY1
  #ifdef GPIO_KEY2
  // Create button switch P001
  addSwitchPlugin(taskIndex, GPIO_KEY2, F("Button2"), true);
  ++taskIndex;
  #endif // GPIO_KEY2
  #ifdef GPIO_KEY3
  // Create button switch P001
  addSwitchPlugin(taskIndex, GPIO_KEY3, F("Button3"), true);
  ++taskIndex;
  #endif // GPIO_KEY3
  #ifdef GPIO_KEY4
  // Create button switch P001
  addSwitchPlugin(taskIndex, GPIO_KEY4, F("Button4"), true);
  ++taskIndex;
  #endif // GPIO_KEY4

  #ifdef GPIO_REL1
    // Create relay switch P001
    addSwitchPlugin(taskIndex, GPIO_REL1, F("Relay1"), false);
    ++taskIndex;
  #endif // GPIO_REL1
  #ifdef GPIO_REL2
    // Create relay switch P001
    addSwitchPlugin(taskIndex, GPIO_REL2, F("Relay2"), false);
    ++taskIndex;
  #endif // GPIO_REL2
  #ifdef GPIO_REL3
    // Create relay switch P001
    addSwitchPlugin(taskIndex, GPIO_REL3, F("Relay3"), false);
    ++taskIndex;
  #endif // GPIO_REL3
  #ifdef GPIO_REL4
    // Create relay switch P001
    addSwitchPlugin(taskIndex, GPIO_REL4, F("Relay4"), false);
    ++taskIndex;
  #endif // GPIO_REL4
  return taskIndex != 0; // Indicate something was added
  // Also needed to make sure the variable is being used.
}

void addButtonRelayRule(byte buttonNumber, byte relay_gpio) {
  Settings.UseRules = true;
  String fileName;
  #if defined(ESP32)
    fileName += '/';
  #endif
  fileName += F("rules1.txt");
  String rule = F("on ButtonBNR#switch do\n  if [ButtonBNR#switch]=1\n    gpio,GNR,1\n  else\n    gpio,GNR,0\n  endif\nendon\n");
  rule.replace(F("BNR"), String(buttonNumber));
  rule.replace(F("GNR"), String(relay_gpio));
  String result = appendLineToFile(fileName, rule);
  if (result.length() > 0) {
    addLog(LOG_LEVEL_ERROR, result);
  }
}

void addPredefinedRules() {
  #ifdef GPIO_KEY1
    #ifdef GPIO_REL1
      addButtonRelayRule(1, GPIO_REL1);
    #endif // GPIO_REL1
  #endif // GPIO_KEY1
  #ifdef GPIO_KEY2
    #ifdef GPIO_REL2
      addButtonRelayRule(2, GPIO_REL2);
    #endif // GPIO_REL2
  #endif // GPIO_KEY2
  #ifdef GPIO_KEY3
    #ifdef GPIO_REL3
      addButtonRelayRule(3, GPIO_REL3);
    #endif // GPIO_REL3
  #endif // GPIO_KEY3
  #ifdef GPIO_KEY4
    #ifdef GPIO_REL4
      addButtonRelayRule(4, GPIO_REL4);
    #endif // GPIO_REL4
  #endif // GPIO_KEY4

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
      Serial.println(F("\n\n\rSystem will reset to factory defaults in 10 seconds..."));
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
  return F("Unknown");
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
  return F("Unknown");
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

uint32_t getFlashRealSizeInBytes() {
  #if defined(ESP32)
    return ESP.getFlashChipSize();
  #else
    return ESP.getFlashChipRealSize(); //ESP.getFlashChipSize();
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
  #ifdef PUYASUPPORT
    result += F(" PUYA support");
  #endif

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




/********************************************************************************************\
  Check if string is valid float
  \*********************************************************************************************/
boolean isFloat(const String& tBuf) {
  return isNumerical(tBuf, false);
}

boolean isValidFloat(float f) {
  if (f == NAN)      return false; //("NaN");
  if (f == INFINITY) return false; //("INFINITY");
  if (-f == INFINITY)return false; //("-INFINITY");
  if (isnan(f))      return false; //("isnan");
  if (isinf(f))      return false; //("isinf");
  return true;
}

boolean isInt(const String& tBuf) {
  return isNumerical(tBuf, true);
}

bool validIntFromString(const String& tBuf, int& result) {
  const String numerical = getNumerical(tBuf, true);
  const bool isvalid = isInt(numerical);
  result = numerical.toInt();
  return isvalid;
}

bool validFloatFromString(const String& tBuf, float& result) {
  const String numerical = getNumerical(tBuf, false);
  const bool isvalid = isFloat(numerical);
  result = numerical.toFloat();
  return isvalid;
}


String getNumerical(const String& tBuf, bool mustBeInteger) {
  String result = "";
  const unsigned int bufLength = tBuf.length();
  if (bufLength == 0) return result;
  boolean decPt = false;
  int firstDec = 0;
  char c = tBuf.charAt(0);
  if(c == '+' || c == '-') {
    result += c;
    firstDec = 1;
  }
  for(unsigned int x=firstDec; x < bufLength; ++x) {
    c = tBuf.charAt(x);
    if(c == '.') {
      if (mustBeInteger) return result;
      // Only one decimal point allowed
      if(decPt) return result;
      else decPt = true;
    }
    else if(c < '0' || c > '9') return result;
    result += c;
  }
  return result;
}

boolean isNumerical(const String& tBuf, bool mustBeInteger) {
  const unsigned int bufLength = tBuf.length();
  if (bufLength == 0) return false;
  boolean decPt = false;
  int firstDec = 0;
  char c = tBuf.charAt(0);
  if(c == '+' || c == '-')
    firstDec = 1;
  for(unsigned int x=firstDec; x < bufLength; ++x) {
    c = tBuf.charAt(x);
    if(c == '.') {
      if (mustBeInteger) return false;
      // Only one decimal point allowed
      if(decPt) return false;
      else decPt = true;
    }
    else if(c < '0' || c > '9') return false;
  }
  return true;
}

// convert old and new time string to nr of seconds
float timeStringToSeconds(String tBuf) {
	float sec = 0;
	int split = tBuf.indexOf(':');
	if (split < 0) { // assume only hours
		sec += tBuf.toFloat() * 60 * 60;
	} else {
		sec += tBuf.substring(0, split).toFloat() * 60 * 60;
		tBuf = tBuf.substring(split +1);
		split = tBuf.indexOf(':');
		if (split < 0) { //old format
			sec += tBuf.toFloat() * 60;
		} else { //new format
			sec += tBuf.substring(0, split).toFloat() * 60;
			sec += tBuf.substring(split +1).toFloat();
		}
	}
	return sec;
}

/********************************************************************************************\
  Init critical variables for logging (important during initial factory reset stuff )
  \*********************************************************************************************/
void initLog()
{
  //make sure addLog doesnt do any stuff before initalisation of Settings is complete.
  Settings.UseSerial=true;
  Settings.SyslogFacility=0;
  setLogLevelFor(LOG_TO_SYSLOG, 0);
  setLogLevelFor(LOG_TO_SERIAL, 2); //logging during initialisation
  setLogLevelFor(LOG_TO_WEBLOG, 2);
  setLogLevelFor(LOG_TO_SDCARD, 0);
}

/********************************************************************************************\
  Logging
  \*********************************************************************************************/
String getLogLevelDisplayString(int logLevel) {
  switch (logLevel) {
    case LOG_LEVEL_NONE:       return F("None");
    case LOG_LEVEL_ERROR:      return F("Error");
    case LOG_LEVEL_INFO:       return F("Info");
    case LOG_LEVEL_DEBUG:      return F("Debug");
    case LOG_LEVEL_DEBUG_MORE: return F("Debug More");
    case LOG_LEVEL_DEBUG_DEV:  return F("Debug dev");

    default:
      return "";
  }
}

String getLogLevelDisplayStringFromIndex(byte index, int& logLevel) {
  switch (index) {
    case 0: logLevel = LOG_LEVEL_ERROR;      break;
    case 1: logLevel = LOG_LEVEL_INFO;       break;
    case 2: logLevel = LOG_LEVEL_DEBUG;      break;
    case 3: logLevel = LOG_LEVEL_DEBUG_MORE; break;
    case 4: logLevel = LOG_LEVEL_DEBUG_DEV;  break;

    default: logLevel = -1; return "";
  }
  return getLogLevelDisplayString(logLevel);
}

void addToLog(byte loglevel, const String& string)
{
  addToLog(loglevel, string.c_str());
}

void addToLog(byte logLevel, const __FlashStringHelper* flashString)
{
    checkRAM(F("addToLog"));
    String s(flashString);
    addToLog(logLevel, s.c_str());
}

void disableSerialLog() {
  log_to_serial_disabled = true;
  setLogLevelFor(LOG_TO_SERIAL, 0);
}

void setLogLevelFor(byte destination, byte logLevel) {
  switch (destination) {
    case LOG_TO_SERIAL:
      if (!log_to_serial_disabled || logLevel == 0)
        Settings.SerialLogLevel = logLevel; break;
    case LOG_TO_SYSLOG: Settings.SyslogLevel = logLevel;    break;
    case LOG_TO_WEBLOG: Settings.WebLogLevel = logLevel;    break;
    case LOG_TO_SDCARD: Settings.SDLogLevel = logLevel;     break;
    default:
      break;
  }
  updateLogLevelCache();
}

void updateLogLevelCache() {
  byte max_lvl = 0;
  if (!log_to_serial_disabled && serialLogActiveRead()) {
    max_lvl = _max(max_lvl, Settings.SerialLogLevel);
    if (Settings.SerialLogLevel >= LOG_LEVEL_DEBUG_MORE)
      Serial.setDebugOutput(true);
  } else {
    Serial.setDebugOutput(false);
  }
  max_lvl = _max(max_lvl, Settings.SyslogLevel);
  if (Logging.logActiveRead()) {
    max_lvl = _max(max_lvl, Settings.WebLogLevel);
  }
#ifdef FEATURE_SD
  max_lvl = _max(max_lvl, Settings.SDLogLevel);
#endif
  highest_active_log_level = max_lvl;
}

bool loglevelActiveFor(byte logLevel) {
  return loglevelActive(logLevel, highest_active_log_level);
}

byte getSerialLogLevel() {
  byte logLevelSettings = 0;
  if (!Settings.UseSerial) return 0;
  if (log_to_serial_disabled || log_to_serial_disabled_temporary) return 0;
  logLevelSettings = Settings.SerialLogLevel;
  if (wifiStatus != ESPEASY_WIFI_SERVICES_INITIALIZED){
    logLevelSettings = 2;
  }
/*
  if (!serialLogActiveRead()) {
    if (logLevelSettings != 0) {
      updateLogLevelCache();
    }
    logLevelSettings = 0;
  }
*/
  return logLevelSettings;
}

byte getWebLogLevel() {
  byte logLevelSettings = 0;
  if (Logging.logActiveRead()) {
    logLevelSettings = Settings.WebLogLevel;
  } else {
    if (Settings.WebLogLevel != 0) {
      updateLogLevelCache();
    }
  }
  return logLevelSettings;
}

boolean loglevelActiveFor(byte destination, byte logLevel) {
  byte logLevelSettings = 0;
  switch (destination) {
    case LOG_TO_SERIAL: {
      logLevelSettings = getSerialLogLevel();
      break;
    }
    case LOG_TO_SYSLOG: {
      logLevelSettings = Settings.SyslogLevel;
      break;
    }
    case LOG_TO_WEBLOG: {
      logLevelSettings = getWebLogLevel();
      break;
    }
    case LOG_TO_SDCARD: {
      #ifdef FEATURE_SD
      logLevelSettings = Settings.SDLogLevel;
      #endif
      break;
    }
    default:
      return false;
  }
  return loglevelActive(logLevel, logLevelSettings);
}


boolean loglevelActive(byte logLevel, byte logLevelSettings) {
  return (logLevel <= logLevelSettings);
}

void addToLog(byte logLevel, const char *line)
{
  const size_t line_length = strlen(line);
  if (loglevelActiveFor(LOG_TO_SERIAL, logLevel)) {
    int roomLeft = ESP.getFreeHeap() - 5000;
    if (roomLeft > 0) {
      String timestamp_log(millis());
      timestamp_log += F(" : ");
      for (size_t i = 0; i < timestamp_log.length(); ++i) {
        serialLogBuffer.push_back(timestamp_log[i]);
      }
      size_t pos = 0;
      while (pos < line_length && pos < static_cast<size_t>(roomLeft)) {
        serialLogBuffer.push_back(line[pos]);
        ++pos;
      }
      serialLogBuffer.push_back('\r');
      serialLogBuffer.push_back('\n');
    }
  }
  if (loglevelActiveFor(LOG_TO_SYSLOG, logLevel)) {
    syslog(logLevel, line);
  }
  if (loglevelActiveFor(LOG_TO_WEBLOG, logLevel)) {
    Logging.add(logLevel, line);
  }

#ifdef FEATURE_SD
  if (loglevelActiveFor(LOG_TO_SDCARD, logLevel)) {
    File logFile = SD.open("log.dat", FILE_WRITE);
    if (logFile)
      logFile.println(line);
    logFile.close();
  }
#endif
}

void process_serialLogBuffer() {
  if (serialLogBuffer.size() == 0) return;
  size_t snip = 128; // Some default, ESP32 doesn't have the availableForWrite function yet.
#if defined(ESP8266)
  snip = Serial.availableForWrite();
#endif
  if (snip > 0) {
    last_serial_log_read = millis();
    size_t bytes_to_write = serialLogBuffer.size();
    if (snip < bytes_to_write) bytes_to_write = snip;
    for (size_t i = 0; i < bytes_to_write; ++i) {
      Serial.write(serialLogBuffer.front());
      serialLogBuffer.pop_front();
    }
  }
}

void tempDisableSerialLog(bool setToDisabled) {
  if (log_to_serial_disabled_temporary == setToDisabled) return;

  // FIXME TD-er: For some reason disabling serial log will not enable it again.
  //  log_to_serial_disabled_temporary = setToDisabled;
  //  updateLogLevelCache();
}

bool serialLogActiveRead() {
  if (!Settings.UseSerial) return false;
  // Some default, ESP32 doesn't have the availableForWrite function yet.
  // Not sure how to detect read activity on an ESP32.
  size_t tx_free = 128;
#if defined(ESP8266)
  tx_free = Serial.availableForWrite();
#endif
  static size_t prev_tx_free = 0;
  if (tx_free < prev_tx_free) {
    prev_tx_free = tx_free;
    tempDisableSerialLog(false);
    return true;
  }
  // Must always set it or else it will never recover from prev_tx_free == 0
  prev_tx_free = tx_free;
  if (timePassedSince(last_serial_log_read) > LOG_BUFFER_EXPIRE) {
    serialLogBuffer.clear();
    // Just add some marker to get it going again when the serial buffer is
    // read again and the serial log level was temporary set to 0 since nothing was read.
    if (Settings.SerialLogLevel > 0) {
      serialLogBuffer.push_back('\n');
    }
    tempDisableSerialLog(true);
    return false;
  }
  return true;
}

/********************************************************************************************\
  Delayed reboot, in case of issues, do not reboot with high frequency as it might not help...
  \*********************************************************************************************/
void delayedReboot(int rebootDelay)
{
  // Direct Serial is allowed here, since this is only an emergency task.
  while (rebootDelay != 0 )
  {
    Serial.print(F("Delayed Reset "));
    Serial.println(rebootDelay);
    rebootDelay--;
    delay(1000);
  }
  reboot();
}

void reboot() {
  #if defined(ESP32)
    ESP.restart();
  #else
    ESP.reset();
  #endif
}


/********************************************************************************************\
  Save RTC struct to RTC memory
  \*********************************************************************************************/
boolean saveToRTC()
{
  #if defined(ESP32)
    return false;
  #else
    if (!system_rtc_mem_write(RTC_BASE_STRUCT, (byte*)&RTC, sizeof(RTC)) || !readFromRTC())
    {
      addLog(LOG_LEVEL_ERROR, F("RTC  : Error while writing to RTC"));
      return(false);
    }
    else
    {
      return(true);
    }
  #endif
}


/********************************************************************************************\
  Initialize RTC memory
  \*********************************************************************************************/
void initRTC()
{
  memset(&RTC, 0, sizeof(RTC));
  RTC.ID1 = 0xAA;
  RTC.ID2 = 0x55;
  saveToRTC();

  memset(&UserVar, 0, sizeof(UserVar));
  saveUserVarToRTC();
}

/********************************************************************************************\
  Read RTC struct from RTC memory
  \*********************************************************************************************/
boolean readFromRTC()
{
  #if defined(ESP32)
    return false;
  #else
    if (!system_rtc_mem_read(RTC_BASE_STRUCT, (byte*)&RTC, sizeof(RTC)))
      return(false);
    return (RTC.ID1 == 0xAA && RTC.ID2 == 0x55);
  #endif
}


/********************************************************************************************\
  Save values to RTC memory
\*********************************************************************************************/
boolean saveUserVarToRTC()
{
  #if defined(ESP32)
    return false;
  #else
    //addLog(LOG_LEVEL_DEBUG, F("RTCMEM: saveUserVarToRTC"));
    byte* buffer = (byte*)&UserVar;
    size_t size = sizeof(UserVar);
    uint32_t sum = getChecksum(buffer, size);
    boolean ret = system_rtc_mem_write(RTC_BASE_USERVAR, buffer, size);
    ret &= system_rtc_mem_write(RTC_BASE_USERVAR+(size>>2), (byte*)&sum, 4);
    return ret;
  #endif
}


/********************************************************************************************\
  Read RTC struct from RTC memory
\*********************************************************************************************/
boolean readUserVarFromRTC()
{
  #if defined(ESP32)
    return false;
  #else
    //addLog(LOG_LEVEL_DEBUG, F("RTCMEM: readUserVarFromRTC"));
    byte* buffer = (byte*)&UserVar;
    size_t size = sizeof(UserVar);
    boolean ret = system_rtc_mem_read(RTC_BASE_USERVAR, buffer, size);
    uint32_t sumRAM = getChecksum(buffer, size);
    uint32_t sumRTC = 0;
    ret &= system_rtc_mem_read(RTC_BASE_USERVAR+(size>>2), (byte*)&sumRTC, 4);
    if (!ret || sumRTC != sumRAM)
    {
      addLog(LOG_LEVEL_ERROR, F("RTC  : Checksum error on reading RTC user var"));
      memset(buffer, 0, size);
    }
    return ret;
  #endif
}


uint32_t getChecksum(byte* buffer, size_t size)
{
  uint32_t sum = 0x82662342;   //some magic to avoid valid checksum on new, uninitialized ESP
  for (size_t i=0; i<size; i++)
    sum += buffer[i];
  return sum;
}


/********************************************************************************************\
  Parse string template
  \*********************************************************************************************/
String parseTemplate(String &tmpString, byte lineSize)
{
  checkRAM(F("parseTemplate"));
  String newString = "";
  //String tmpStringMid = "";
  newString.reserve(lineSize);

  parseSystemVariables(tmpString, false);

  // replace task template variables
  int leftBracketIndex = tmpString.indexOf('[');
  if (leftBracketIndex == -1)
    newString = tmpString;
  else
  {
    byte count = 0;
    byte currentTaskIndex = ExtraTaskSettings.TaskIndex;

    while (leftBracketIndex >= 0 && count < 10 - 1)
    {
      newString += tmpString.substring(0, leftBracketIndex);
      tmpString = tmpString.substring(leftBracketIndex + 1);
      int rightBracketIndex = tmpString.indexOf(']');
      if (rightBracketIndex >= 0)
      {
        String tmpStringMid = tmpString.substring(0, rightBracketIndex);
        tmpString = tmpString.substring(rightBracketIndex + 1);
        int hashtagIndex = tmpStringMid.indexOf('#');
        if (hashtagIndex >= 0) {
          String deviceName = tmpStringMid.substring(0, hashtagIndex);
          String valueName = tmpStringMid.substring(hashtagIndex + 1);
          String valueFormat = "";
          hashtagIndex = valueName.indexOf('#');
          if (hashtagIndex >= 0)
          {
            valueFormat = valueName.substring(hashtagIndex + 1);
            valueName = valueName.substring(0, hashtagIndex);
          }

          if (deviceName.equalsIgnoreCase(F("Plugin")))
          {
            String tmpString = tmpStringMid.substring(7);
            tmpString.replace('#', ',');
            if (PluginCall(PLUGIN_REQUEST, 0, tmpString))
              newString += tmpString;
          }
          else if (deviceName.equalsIgnoreCase(F("Var"))) {
            String tmpString = tmpStringMid.substring(4);
            if (tmpString.length()>0 && isDigit(tmpString[0])) {
              const int varNum = tmpString.toInt();
              if (varNum > 0 && varNum <= CUSTOM_VARS_MAX)
                newString += String(customFloatVar[varNum-1]);
            }
          }
          else
            for (byte y = 0; y < TASKS_MAX; y++)
            {
              if (Settings.TaskDeviceEnabled[y])
              {
                LoadTaskSettings(y);
                String taskDeviceName = getTaskDeviceName(y);
                if (taskDeviceName.length() != 0)
                {
                  if (deviceName.equalsIgnoreCase(taskDeviceName))
                  {
                    boolean match = false;
                    for (byte z = 0; z < VARS_PER_TASK; z++)
                      if (valueName.equalsIgnoreCase(ExtraTaskSettings.TaskDeviceValueNames[z]))
                      {
                        // here we know the task and value, so find the uservar
                        // Try to format and transform the values
                        // y = taskNr
                        // z = var_of_task
                        match = true;
                        bool isvalid;
                        String value = formatUserVar(y, z, isvalid);
                        if (isvalid) {
                          transformValue(newString, lineSize, value, valueFormat, tmpString);
                          break;
                        }
                      }
                    if (!match) // try if this is a get config request
                    {
                      struct EventStruct TempEvent;
                      TempEvent.TaskIndex = y;
                      String tmpName = valueName;
                      if (PluginCall(PLUGIN_GET_CONFIG, &TempEvent, tmpName))
                        newString += tmpName;
                    }
                    break;
                  }
                }
              }
            }
        }
      }
      leftBracketIndex = tmpString.indexOf('[');
      count++;
    }
    checkRAM(F("parseTemplate2"));
    newString += tmpString;

    if (currentTaskIndex != 255)
      LoadTaskSettings(currentTaskIndex);
  }

  //parseSystemVariables(newString, false);
  parseStandardConversions(newString, false);

  // padding spaces
  while (newString.length() < lineSize)
    newString += ' ';
  checkRAM(F("parseTemplate3"));
  return newString;
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
      const int val = value == "0" ? 0 : 1; //to be used for GPIO status (0 or 1)
      const float valFloat = value.toFloat();

      String tempValueFormat = valueFormat;
      int tempValueFormatLength = tempValueFormat.length();
      const int invertedIndex = tempValueFormat.indexOf('!');
      const bool inverted = invertedIndex >= 0 ? 1 : 0;
      if (inverted)
        tempValueFormat.remove(invertedIndex,1);

      const int rightJustifyIndex = tempValueFormat.indexOf('R');
      const bool rightJustify = rightJustifyIndex >= 0 ? 1 : 0;
      if (rightJustify)
        tempValueFormat.remove(rightJustifyIndex,1);

      tempValueFormatLength = tempValueFormat.length(); //needed because could have been changed after '!' and 'R' removal

      //Check Transformation syntax
      if (tempValueFormatLength > 0)
      {
        switch (tempValueFormat[0])
          {
          case 'V': //value = value without transformations
            break;
          case 'O':
            value = val == inverted ? F("OFF") : F(" ON"); //(equivalent to XOR operator)
            break;
          case 'C':
            value = val == inverted ? F("CLOSE") : F(" OPEN");
            break;
          case 'M':
            value = val == inverted ? F("AUTO") : F(" MAN");
            break;
          case 'm':
            value = val == inverted ? F("A") : F("M");
            break;
          case 'H':
            value = val == inverted ? F("COLD") : F(" HOT");
            break;
          case 'U':
            value = val == inverted ? F("DOWN") : F("  UP");
            break;
          case 'u':
            value = val == inverted ? F("D") : F("U");
            break;
          case 'Y':
            value = val == inverted ? F(" NO") : F("YES");
            break;
          case 'y':
            value = val == inverted ? F("N") : F("Y");
            break;
          case 'X':
            value = val == inverted ? F("O") : F("X");
            break;
          case 'I':
            value = val == inverted ? F("OUT") : F(" IN");
            break;
          case 'Z' :// return "0" or "1"
            value = val == inverted ? "0" : "1";
            break;
          case 'D' ://Dx.y min 'x' digits zero filled & 'y' decimal fixed digits
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
              int indexDot;
              indexDot = value.indexOf('.') > 0 ? value.indexOf('.') : value.length();
              for (byte f = 0; f < (x - indexDot); f++)
                value = "0" + value;
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
        if (loglevelActiveFor(LOG_LEVEL_DEBUG)) {
          String logFormatted = F("DEBUG: Formatted String='");
          logFormatted += newString;
          logFormatted += value;
          logFormatted += '\'';
          addLog(LOG_LEVEL_DEBUG, logFormatted);
        }
      }
    }
  }
  //end of changes by giig1967g - 2018-04-18

  newString += String(value);
  {
    if (loglevelActiveFor(LOG_LEVEL_DEBUG_DEV)) {
      String logParsed = F("DEBUG DEV: Parsed String='");
      logParsed += newString;
      logParsed += '\'';
      addLog(LOG_LEVEL_DEBUG_DEV, logParsed);
    }
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
    return 0.0;
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
      return round(first) % round(second);
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
  checkRAM(F("Calculate"));
  const char *strpos = input, *strend = input + strlen(input);
  char token[25];
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
        while (sl > 0)
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
        if (sl > 0)
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

int CalculateParam(char *TmpStr) {
  int returnValue;

  // Minimize calls to the Calulate function.
  // Only if TmpStr starts with '=' then call Calculate(). Otherwise do not call it
  if (TmpStr[0] != '=') {
    returnValue=str2int(TmpStr);
  } else {
    float param=0;
    TmpStr[0] = ' '; //replace '=' with space
    int returnCode=Calculate(TmpStr, &param);
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
      } else {
      if (loglevelActiveFor(LOG_LEVEL_DEBUG)) {
        String log = F("CALCULATE PARAM: ");
        log += TmpStr;
        log += F(" = ");
        log += round(param);
        addLog(LOG_LEVEL_DEBUG, log);
      }
    }
    returnValue=round(param); //return integer only as it's valid only for device and task id
  }
  return returnValue;
}

void checkRuleSets(){
for (byte x=0; x < RULESETS_MAX; x++){
  #if defined(ESP8266)
    String fileName = F("rules");
  #endif
  #if defined(ESP32)
    String fileName = F("/rules");
  #endif
  fileName += x+1;
  fileName += F(".txt");
  if (SPIFFS.exists(fileName))
    activeRuleSets[x] = true;
  else
    activeRuleSets[x] = false;

  if (Settings.SerialLogLevel == LOG_LEVEL_DEBUG_DEV){
    Serial.print(fileName);
    Serial.print(" ");
    Serial.println(activeRuleSets[x]);
    }
  }
}


/********************************************************************************************\
  Rules processing
  \*********************************************************************************************/
void rulesProcessing(String& event)
{
  if (!Settings.UseRules) return;
  checkRAM(F("rulesProcessing"));
  unsigned long timer = millis();
  if (loglevelActiveFor(LOG_LEVEL_INFO)) {
    String log = F("EVENT: ");
    log += event;
    addLog(LOG_LEVEL_INFO, log);
  }

  for (byte x = 0; x < RULESETS_MAX; x++)
  {
    #if defined(ESP8266)
      String fileName = F("rules");
    #endif
    #if defined(ESP32)
      String fileName = F("/rules");
    #endif
    fileName += x+1;
    fileName += F(".txt");
    if(activeRuleSets[x])
      rulesProcessingFile(fileName, event);
  }

  if (loglevelActiveFor(LOG_LEVEL_DEBUG)) {
    String log = F("EVENT: ");
    log += event;
    log += F(" Processing time:");
    log += timePassedSince(timer);
    log += F(" milliSeconds");
    addLog(LOG_LEVEL_DEBUG, log);
  }
  backgroundtasks();

}

/********************************************************************************************\
  Rules processing
  \*********************************************************************************************/
String rulesProcessingFile(const String& fileName, String& event)
{
  if (!Settings.UseRules) return "";
  checkRAM(F("rulesProcessingFile"));
  if (Settings.SerialLogLevel == LOG_LEVEL_DEBUG_DEV){
    Serial.print(F("RuleDebug Processing:"));
    Serial.println(fileName);
    Serial.println(F("     flags CMI  parse output:"));
  }

  static byte nestingLevel = 0;
  int data = 0;
  String log = "";

  nestingLevel++;
  if (nestingLevel > RULES_MAX_NESTING_LEVEL)
  {
    addLog(LOG_LEVEL_ERROR, F("EVENT: Error: Nesting level exceeded!"));
    nestingLevel--;
    return (log);
  }

  fs::File f = SPIFFS.open(fileName, "r+");
  SPIFFS_CHECK(f, fileName.c_str());

  String line = "";
  bool match = false;
  bool codeBlock = false;
  bool isCommand = false;
  bool condition[RULES_IF_MAX_NESTING_LEVEL];
  bool ifBranche[RULES_IF_MAX_NESTING_LEVEL];
  byte ifBlock = 0;
  byte fakeIfBlock = 0;

  byte buf[RULES_BUFFER_SIZE];
  int len = 0;
  while (f.available())
  {
    len = f.read((byte*)buf, RULES_BUFFER_SIZE);
    for (int x = 0; x < len; x++) {
      data = buf[x];

      SPIFFS_CHECK(data >= 0, fileName.c_str());

      if (data != 10) {
        line += char(data);
      }
      else
      {      // if line complete, parse this rule
        line.replace("\r", "");
        if (line.substring(0, 2) != F("//") && line.length() > 0)
        {
          parseCompleteNonCommentLine(
            line, event, log,
            match, codeBlock, isCommand,
            condition, ifBranche,
            ifBlock, fakeIfBlock);
          backgroundtasks();
        }

        line = "";
      }
    }
  }
  if (f) f.close();

  nestingLevel--;
  checkRAM(F("rulesProcessingFile2"));
  return ("");
}

void parseCompleteNonCommentLine(
    String& line,
    String& event,
    String& log,
    bool& match,
    bool& codeBlock,
    bool& isCommand,
    bool condition[],
    bool ifBranche[],
    byte& ifBlock,
    byte& fakeIfBlock)
{
  isCommand = true;

  // Strip comments
  int comment = line.indexOf(F("//"));
  if (comment > 0)
    line = line.substring(0, comment);

  if (match || !codeBlock) {
    // only parse [xxx#yyy] if we have a matching ruleblock or need to eval the "on" (no codeBlock)
    // This to avoid waisting CPU time...


    if (match && !fakeIfBlock) {
      // substitution of %eventvalue% is made here so it can be used on if statement too
      if (event.charAt(0) == '!')
      {
        line.replace(F("%eventvalue%"), event); // substitute %eventvalue% with literal event string if starting with '!'
      }
      else
      {
        int equalsPos = event.indexOf("=");
        if (equalsPos > 0)
        {
          String tmpString = event.substring(equalsPos + 1);
          //line.replace(F("%eventvalue%"), tmpString); // substitute %eventvalue% with the actual value from the event
          char* tmpParam = new char[INPUT_COMMAND_SIZE];
          tmpParam[0] = 0;

          if (GetArgv(tmpString.c_str(),tmpParam,1)) {
             line.replace(F("%eventvalue%"), tmpParam); // for compatibility issues
             line.replace(F("%eventvalue1%"), tmpParam); // substitute %eventvalue1% in actions with the actual value from the event
          }
          if (GetArgv(tmpString.c_str(),tmpParam,2)) line.replace(F("%eventvalue2%"), tmpParam); // substitute %eventvalue2% in actions with the actual value from the event
          if (GetArgv(tmpString.c_str(),tmpParam,3)) line.replace(F("%eventvalue3%"), tmpParam); // substitute %eventvalue3% in actions with the actual value from the event
          if (GetArgv(tmpString.c_str(),tmpParam,4)) line.replace(F("%eventvalue4%"), tmpParam); // substitute %eventvalue4% in actions with the actual value from the event
          delete[] tmpParam;
        }
      }
    }
    line = parseTemplate(line, line.length());
  }
  line.trim();

  String lineOrg = line; // store original line for future use
  line.toLowerCase(); // convert all to lower case to make checks easier


  String eventTrigger = "";
  String action = "";

  if (!codeBlock)  // do not check "on" rules if a block of actions is to be processed
  {
    if (line.startsWith(F("on ")))
    {
      ifBlock = 0;
      fakeIfBlock = 0;
      line = line.substring(3);
      int split = line.indexOf(F(" do"));
      if (split != -1)
      {
        eventTrigger = line.substring(0, split);
        action = lineOrg.substring(split + 7);
        action.trim();
      }
      if (eventTrigger == "*") // wildcard, always process
        match = true;
      else
        match = ruleMatch(event, eventTrigger);
      if (action.length() > 0) // single on/do/action line, no block
      {
        isCommand = true;
        codeBlock = false;
      }
      else
      {
        isCommand = false;
        codeBlock = true;
      }
    }
  }
  else
  {
    action = lineOrg;
  }

  String lcAction = action;
  lcAction.toLowerCase();
  if (lcAction == F("endon")) // Check if action block has ended, then we will wait for a new "on" rule
  {
    isCommand = false;
    codeBlock = false;
    match = false;
    ifBlock = 0;
    fakeIfBlock = 0;
  }

  if (Settings.SerialLogLevel == LOG_LEVEL_DEBUG_DEV){
    Serial.print(F("RuleDebug: "));
    Serial.print(codeBlock);
    Serial.print(match);
    Serial.print(isCommand);
    Serial.print(F(": "));
    Serial.println(line);
  }

  if (match) // rule matched for one action or a block of actions
  {
    processMatchedRule(
      lcAction, action, event, log,
      match, codeBlock, isCommand,
      condition, ifBranche,
      ifBlock, fakeIfBlock);
  }
}

void processMatchedRule(
  String& lcAction, String& action, String& event, String& log,
  bool& match,
  bool& codeBlock,
  bool& isCommand,
  bool condition[],
  bool ifBranche[],
  byte& ifBlock,
  byte& fakeIfBlock)
{
  if (fakeIfBlock)
    isCommand = false;
  else if (ifBlock)
    if (condition[ifBlock-1] != ifBranche[ifBlock-1])
      isCommand = false;
  int split = lcAction.indexOf(F("elseif ")); // check for optional "elseif" condition
  if (split != -1)
  {
    isCommand = false;
    if (ifBlock && !fakeIfBlock)
    {
      if (ifBranche[ifBlock-1])
      {
        if (condition[ifBlock-1])
          ifBranche[ifBlock-1] = false;
        else
        {
          String check = lcAction.substring(split + 7);
          condition[ifBlock-1] = conditionMatchExtended(check);
          if (loglevelActiveFor(LOG_LEVEL_DEBUG)) {
            log = F("Lev.");
            log += String(ifBlock);
            log += F(": [elseif ");
            log += check;
            log += F("]=");
            log += toString(condition[ifBlock-1]);
            addLog(LOG_LEVEL_DEBUG, log);
          }
        }
      }
    }
  }
  else
  {
    split = lcAction.indexOf(F("if ")); // check for optional "if" condition
    if (split != -1)
    {
      if (ifBlock < RULES_IF_MAX_NESTING_LEVEL)
      {
        if (isCommand)
        {
          ifBlock++;
          String check = lcAction.substring(split + 3);
          condition[ifBlock-1] = conditionMatchExtended(check);
          ifBranche[ifBlock-1] = true;
          if (loglevelActiveFor(LOG_LEVEL_DEBUG)) {
            log = F("Lev.");
            log += String(ifBlock);
            log += F(": [if ");
            log += check;
            log += F("]=");
            log += toString(condition[ifBlock-1]);
            addLog(LOG_LEVEL_DEBUG, log);
          }
        }
        else
          fakeIfBlock++;
      }
      else
      {
        fakeIfBlock++;
        if (loglevelActiveFor(LOG_LEVEL_ERROR)) {
          log = F("Lev.");
          log += String(ifBlock);
          log = F(": Error: IF Nesting level exceeded!");
          addLog(LOG_LEVEL_ERROR, log);
        }
      }
      isCommand = false;
    }
  }

  if ((lcAction == F("else")) && !fakeIfBlock) // in case of an "else" block of actions, set ifBranche to false
  {
    ifBranche[ifBlock-1] = false;
    isCommand = false;
    if (loglevelActiveFor(LOG_LEVEL_DEBUG)) {
      log = F("Lev.");
      log += String(ifBlock);
      log += F(": [else]=");
      log += toString(condition[ifBlock-1] == ifBranche[ifBlock-1]);
      addLog(LOG_LEVEL_DEBUG, log);
    }
  }

  if (lcAction == F("endif")) // conditional block ends here
  {
    if (fakeIfBlock)
      fakeIfBlock--;
    else if (ifBlock)
      ifBlock--;
    isCommand = false;
  }

  // process the action if it's a command and unconditional, or conditional and the condition matches the if or else block.
  if (isCommand)
  {
    if (event.charAt(0) == '!')
    {
      action.replace(F("%eventvalue%"), event); // substitute %eventvalue% with literal event string if starting with '!'
    }
    else
    {
      int equalsPos = event.indexOf("=");
      if (equalsPos > 0)
      {
        String tmpString = event.substring(equalsPos + 1);

        char* tmpParam = new char[INPUT_COMMAND_SIZE];
        tmpParam[0] = 0;

        if (GetArgv(tmpString.c_str(),tmpParam,1)) {
           action.replace(F("%eventvalue%"), tmpParam); // for compatibility issues
           action.replace(F("%eventvalue1%"), tmpParam); // substitute %eventvalue1% in actions with the actual value from the event
        }
        if (GetArgv(tmpString.c_str(),tmpParam,2)) action.replace(F("%eventvalue2%"), tmpParam); // substitute %eventvalue2% in actions with the actual value from the event
        if (GetArgv(tmpString.c_str(),tmpParam,3)) action.replace(F("%eventvalue3%"), tmpParam); // substitute %eventvalue3% in actions with the actual value from the event
        if (GetArgv(tmpString.c_str(),tmpParam,4)) action.replace(F("%eventvalue4%"), tmpParam); // substitute %eventvalue4% in actions with the actual value from the event
        delete[] tmpParam;
      }
    }

    if (loglevelActiveFor(LOG_LEVEL_INFO)) {
      String log = F("ACT  : ");
      log += action;
      addLog(LOG_LEVEL_INFO, log);
    }

    struct EventStruct TempEvent;
    parseCommandString(&TempEvent, action);

    // FIXME TD-er: This part seems a bit strange.
    // It can't schedule a call to PLUGIN_WRITE.
    // Maybe ExecuteCommand can be scheduled?
    delay(0);
    // Use a tmp string to call PLUGIN_WRITE, since PluginCall may inadvertenly alter the string.
    String tmpAction(action);
    if (!PluginCall(PLUGIN_WRITE, &TempEvent, tmpAction)) {
      if (!tmpAction.equals(action)) {
        if (loglevelActiveFor(LOG_LEVEL_ERROR)) {
          String log = F("PLUGIN_WRITE altered the string: ");
          log += action;
          log += F(" to: ");
          log += tmpAction;
          addLog(LOG_LEVEL_ERROR, log);
        }
        // TODO: assign here modified action???
      }
      ExecuteCommand(VALUE_SOURCE_SYSTEM, action.c_str());
    }
    delay(0);
  }
}

/********************************************************************************************\
  Check if an event matches to a given rule
  \*********************************************************************************************/
boolean ruleMatch(String& event, String& rule)
{
  checkRAM(F("ruleMatch"));
  boolean match = false;
  String tmpEvent = event;
  String tmpRule = rule;

  //Ignore escape char
  tmpRule.replace("[","");
  tmpRule.replace("]","");

  // Special handling of literal string events, they should start with '!'
  if (event.charAt(0) == '!')
  {
    int pos = rule.indexOf('*');
    if (pos != -1) // a * sign in rule, so use a'wildcard' match on message
      {
        tmpEvent = event.substring(0,pos-1);
        tmpRule = rule.substring(0,pos-1);
      }
    else
      {
        pos = rule.indexOf('#');
        if (pos == -1) // no # sign in rule, use 'wildcard' match on event 'source'
          {
            tmpEvent = event.substring(0,rule.length());
            tmpRule = rule;
          }
      }

    if (tmpEvent.equalsIgnoreCase(tmpRule))
      return true;
    else
      return false;
  }

  if (event.startsWith(F("Clock#Time"))) // clock events need different handling...
  {
    int pos1 = event.indexOf("=");
    int pos2 = rule.indexOf("=");
    if (pos1 > 0 && pos2 > 0)
    {
      tmpEvent = event.substring(0, pos1);
      tmpRule  = rule.substring(0, pos2);
      if (tmpRule.equalsIgnoreCase(tmpEvent)) // if this is a clock rule
      {
        tmpEvent = event.substring(pos1 + 1);
        tmpRule  = rule.substring(pos2 + 1);
        unsigned long clockEvent = string2TimeLong(tmpEvent);
        unsigned long clockSet = string2TimeLong(tmpRule);
        if (matchClockEvent(clockEvent, clockSet))
          return true;
        else
          return false;
      }
    }
  }


  // parse event into verb and value
  float value = 0;
  int pos = event.indexOf("=");
  if (pos)
  {
    tmpEvent = event.substring(pos + 1);
    value = tmpEvent.toFloat();
    tmpEvent = event.substring(0, pos);
  }

  // parse rule
  int comparePos = 0;
  char compare = ' ';
  comparePos = rule.indexOf(">");
  if (comparePos > 0)
  {
    compare = '>';
  }
  else
  {
    comparePos = rule.indexOf("<");
    if (comparePos > 0)
    {
      compare = '<';
    }
    else
    {
      comparePos = rule.indexOf("=");
      if (comparePos > 0)
      {
        compare = '=';
      }
    }
  }

  float ruleValue = 0;

  if (comparePos > 0)
  {
    tmpRule = rule.substring(comparePos + 1);
    ruleValue = tmpRule.toFloat();
    tmpRule = rule.substring(0, comparePos);
  }

  switch (compare)
  {
    case '>':
      if (tmpRule.equalsIgnoreCase(tmpEvent) && value > ruleValue)
        match = true;
      break;

    case '<':
      if (tmpRule.equalsIgnoreCase(tmpEvent) && value < ruleValue)
        match = true;
      break;

    case '=':
      if (tmpRule.equalsIgnoreCase(tmpEvent) && value == ruleValue)
        match = true;
      break;

    case ' ':
      if (tmpRule.equalsIgnoreCase(tmpEvent))
        match = true;
      break;
  }
  checkRAM(F("ruleMatch2"));
  return match;
}


/********************************************************************************************\
  Check expression
  \*********************************************************************************************/

boolean conditionMatchExtended(String& check) {
	int condAnd = -1;
	int condOr = -1;
	boolean rightcond = false;
	boolean leftcond = conditionMatch(check); // initial check

	do {
		condAnd = check.indexOf(F(" and "));
		condOr  = check.indexOf(F(" or "));

		if (condAnd > 0 || condOr > 0) { // we got AND/OR
			if (condAnd > 0	&& ((condOr < 0 && condOr < condAnd) || (condOr > 0 && condOr > condAnd))) { //AND is first
				check = check.substring(condAnd + 5);
				rightcond = conditionMatch(check);
				leftcond = (leftcond && rightcond);
			} else { //OR is first
				check = check.substring(condOr + 4);
				rightcond = conditionMatch(check);
				leftcond = (leftcond || rightcond);
			}
		}
	} while (condAnd > 0 || condOr > 0);
	return leftcond;
}

boolean conditionMatch(const String& check)
{
  boolean match = false;

  char compare    = ' ';

  int posStart = check.length();
  int posEnd = posStart;
  int comparePos  = 0;

  if ((comparePos = check.indexOf("!="))>0 && comparePos<posStart) {
	  posStart = comparePos;
	  posEnd = posStart+2;
	  compare = '!'+'=';
  }
  if ((comparePos = check.indexOf("<>"))>0 && comparePos<posStart) {
	  posStart = comparePos;
	  posEnd = posStart+2;
	  compare = '!'+'=';
  }
  if ((comparePos = check.indexOf(">="))>0 && comparePos<posStart) {
	  posStart = comparePos;
	  posEnd = posStart+2;
	  compare = '>'+'=';
  }
  if ((comparePos = check.indexOf("<="))>0 && comparePos<posStart) {
	  posStart = comparePos;
	  posEnd = posStart+2;
	  compare = '<'+'=';
  }
  if ((comparePos = check.indexOf("<"))>0 && comparePos<posStart) {
	  posStart = comparePos;
	  posEnd = posStart+1;
	  compare = '<';
  }
  if ((comparePos = check.indexOf(">"))>0 && comparePos<posStart) {
	  posStart = comparePos;
	  posEnd = posStart+1;
	  compare = '>';
  }
  if ((comparePos = check.indexOf("="))>0 && comparePos<posStart) {
	  posStart = comparePos;
	  posEnd = posStart+1;
	  compare = '=';
  }

  float Value1 = 0;
  float Value2 = 0;

  if (compare > ' ')
  {
    String tmpCheck1 = check.substring(0, posStart);
    String tmpCheck2 = check.substring(posEnd);
    if (!isFloat(tmpCheck1) || !isFloat(tmpCheck2)) {
        Value1 = timeStringToSeconds(tmpCheck1);
        Value2 = timeStringToSeconds(tmpCheck2);
    } else {
        Value1 = tmpCheck1.toFloat();
        Value2 = tmpCheck2.toFloat();
    }
  }
  else
    return false;

  switch (compare)
  {
  case '>'+'=':
	  if (Value1 >= Value2)
		  match = true;
	  break;

  case '<'+'=':
	  if (Value1 <= Value2)
		  match = true;
	  break;

  case '!'+'=':
	  if (Value1 != Value2)
		  match = true;
	  break;

  case '>':
	  if (Value1 > Value2)
		  match = true;
	  break;

  case '<':
	  if (Value1 < Value2)
		  match = true;
	  break;

  case '=':
	  if (Value1 == Value2)
		  match = true;
	  break;
  }
  return match;
}


/********************************************************************************************\
  Check rule timers
  \*********************************************************************************************/
void rulesTimers()
{
  if (!Settings.UseRules) return;
  for (byte x = 0; x < RULES_TIMER_MAX; x++)
  {
    if (!RulesTimer[x].paused && RulesTimer[x].timestamp != 0L) // timer active?
    {
      if (timeOutReached(RulesTimer[x].timestamp)) // timer finished?
      {
        RulesTimer[x].timestamp = 0L; // turn off this timer
        String event = F("Rules#Timer=");
        event += x + 1;
        rulesProcessing(event);
      }
    }
  }
}


/********************************************************************************************\
  Generate rule events based on task refresh
  \*********************************************************************************************/

void createRuleEvents(byte TaskIndex)
{
  if (!Settings.UseRules) return;
  LoadTaskSettings(TaskIndex);
  byte BaseVarIndex = TaskIndex * VARS_PER_TASK;
  byte DeviceIndex = getDeviceIndex(Settings.TaskDeviceNumber[TaskIndex]);
  byte sensorType = Device[DeviceIndex].VType;
  for (byte varNr = 0; varNr < Device[DeviceIndex].ValueCount; varNr++)
  {
    String eventString = getTaskDeviceName(TaskIndex);
    eventString += F("#");
    eventString += ExtraTaskSettings.TaskDeviceValueNames[varNr];
    eventString += F("=");

    if (sensorType == SENSOR_TYPE_LONG)
      eventString += (unsigned long)UserVar[BaseVarIndex] + ((unsigned long)UserVar[BaseVarIndex + 1] << 16);
    else
      eventString += UserVar[BaseVarIndex + varNr];

    rulesProcessing(eventString);
  }
}


void SendValueLogger(byte TaskIndex)
{
  bool featureSD = false;
  #ifdef FEATURE_SD
    featureSD = true;
  #endif

  String logger;
  if (featureSD || loglevelActiveFor(LOG_LEVEL_DEBUG)) {
    LoadTaskSettings(TaskIndex);
    byte DeviceIndex = getDeviceIndex(Settings.TaskDeviceNumber[TaskIndex]);
    for (byte varNr = 0; varNr < Device[DeviceIndex].ValueCount; varNr++)
    {
      logger += getDateString('-');
      logger += ' ';
      logger += getTimeString(':');
      logger += ',';
      logger += Settings.Unit;
      logger += ',';
      logger += getTaskDeviceName(TaskIndex);
      logger += ',';
      logger += ExtraTaskSettings.TaskDeviceValueNames[varNr];
      logger += ',';
      logger += formatUserVarNoCheck(TaskIndex, varNr);
      logger += F("\r\n");
    }

    addLog(LOG_LEVEL_DEBUG, logger);
  }

#ifdef FEATURE_SD
  String filename = F("VALUES.CSV");
  File logFile = SD.open(filename, FILE_WRITE);
  if (logFile)
    logFile.print(logger);
  logFile.close();
#endif
}


#define TRACES 3                                            // number of memory traces
#define TRACEENTRIES 15                                      // entries per trace

class RamTracker{
  private:
    String        traces[TRACES]  ;                         // trace of latest memory checks
    unsigned int  tracesMemory[TRACES] ;                    // lowest memory for that  trace
    unsigned int  readPtr, writePtr;                        // pointer to cyclic buffer
    String        nextAction[TRACEENTRIES];                 // buffer to record the names of functions before they are transfered to a trace
    unsigned int  nextActionStartMemory[TRACEENTRIES];      // memory levels for the functions.

    unsigned int  bestCaseTrace (void){                     // find highest the trace with the largest minimum memory (gets replaced by worse one)
       unsigned int lowestMemoryInTrace = 0;
       unsigned int lowestMemoryInTraceIndex=0;
       for (int i = 0; i<TRACES; i++) {
          if (tracesMemory[i] > lowestMemoryInTrace){
            lowestMemoryInTrace= tracesMemory[i];
            lowestMemoryInTraceIndex=i;
            }
          }
      //Serial.println(lowestMemoryInTraceIndex);
      return lowestMemoryInTraceIndex;
      }

  public:
    RamTracker(void){                                       // constructor
        readPtr=0;
        writePtr=0;
        for (int i = 0; i< TRACES; i++) {
          traces[i]="";
          tracesMemory[i]=0xffffffff;                           // init with best case memory values, so they get replaced if memory goes lower
          }
        for (int i = 0; i< TRACEENTRIES; i++) {
          nextAction[i]="startup";
          nextActionStartMemory[i] = ESP.getFreeHeap();     // init with best case memory values, so they get replaced if memory goes lower
          }
        };

    void registerRamState(String &s){    // store function
       nextAction[writePtr]=s;                              // name and mem
       nextActionStartMemory[writePtr]=ESP.getFreeHeap();   // in cyclic buffer.
       int bestCase = bestCaseTrace();                      // find best case memory trace
       if ( ESP.getFreeHeap() < tracesMemory[bestCase]){    // compare to current memory value
            traces[bestCase]="";
            readPtr = writePtr+1;                           // read out buffer, oldest value first
            if (readPtr>=TRACEENTRIES) readPtr=0;           // read pointer wrap around
            tracesMemory[bestCase] = ESP.getFreeHeap();     // store new lowest value of that trace

            for (int i = 0; i<TRACEENTRIES; i++) {          // tranfer cyclic buffer strings and mem values to this trace
              traces[bestCase]+= nextAction[readPtr];
              traces[bestCase]+= "-> ";
              traces[bestCase]+= String(nextActionStartMemory[readPtr]);
              traces[bestCase]+= ' ';
              readPtr++;
              if (readPtr >=TRACEENTRIES) readPtr=0;      // wrap around read pointer
            }
       }
       writePtr++;
       if (writePtr >= TRACEENTRIES) writePtr=0;          // inc write pointer and wrap around too.
    };
   void getTraceBuffer(){                                // return giant strings, one line per trace. Add stremToWeb method to avoid large strings.
      if (loglevelActiveFor(LOG_LEVEL_DEBUG_DEV)) {
        String retval="Memtrace\n";
        for (int i = 0; i< TRACES; i++){
          retval += String(i);
          retval += ": lowest: ";
          retval += String(tracesMemory[i]);
          retval += "  ";
          retval += traces[i];
          addLog(LOG_LEVEL_DEBUG_DEV, retval);
          retval="";
        }
      }
    }
}myRamTracker;                                              // instantiate class. (is global now)

void checkRAMtoLog(void){
  myRamTracker.getTraceBuffer();
}

void checkRAM(const __FlashStringHelper* flashString, int a ) {
 String s=String(a);
 checkRAM(flashString,s);
}

void checkRAM(const __FlashStringHelper* flashString, String &a ) {
  String s = flashString;
  checkRAM(s,a);
}

void checkRAM(String &flashString, String &a ) {
  String s = flashString;
  s+=" (";
  s+=a;
  s+=")";
  checkRAM(s);
}

void checkRAM( const __FlashStringHelper* flashString)
{
  String s = flashString;
  myRamTracker.registerRamState(s);

  uint32_t freeRAM = FreeMem();
  if (freeRAM < lowestRAM)
  {
    lowestRAM = freeRAM;
    lowestRAMfunction = flashString;
  }
  uint32_t freeStack = getFreeStackWatermark();
  if (freeStack < lowestFreeStack) {
    lowestFreeStack = freeStack;
    lowestFreeStackfunction = flashString;
  }
}

void checkRAM( String &a ) {
  myRamTracker.registerRamState(a);
}


//#ifdef PLUGIN_BUILD_TESTING

#define isdigit(n) (n >= '0' && n <= '9')

/********************************************************************************************\
  Generate a tone of specified frequency on pin
  \*********************************************************************************************/
void tone_espEasy(uint8_t _pin, unsigned int frequency, unsigned long duration) {
  #ifdef ESP32
    delay(duration);
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
    note = 0;

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
  maxSketchSize = (ESP.getFreeSketchSpace() - 0x1000) & 0xFFFFF000;
  const bool otaPossible = maxSketchSize > SMALLEST_OTA_IMAGE;
  use2step = maxSketchSize < ESP.getSketchSize();
  if (use2step) {
    const uint32_t totalSketchSpace = ESP.getFreeSketchSpace() + ESP.getSketchSize();
    maxSketchSize = totalSketchSpace - SMALLEST_OTA_IMAGE;
  }
  if (maxSketchSize > MAX_SKETCH_SIZE) maxSketchSize = MAX_SKETCH_SIZE;
  return otaPossible;
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
  ArduinoOTA.setHostname(Settings.Name);
  if (SecuritySettings.Password[0]!=0)
    ArduinoOTA.setPassword(SecuritySettings.Password);

  ArduinoOTA.onStart([]() {
      Serial.println(F("OTA  : Start upload"));
      SPIFFS.end(); //important, otherwise it fails
  });

  ArduinoOTA.onEnd([]() {
      Serial.println(F("\nOTA  : End"));
      //"dangerous": if you reset during flash you have to reflash via serial
      //so dont touch device until restart is complete
      Serial.println(F("\nOTA  : DO NOT RESET OR POWER OFF UNTIL BOOT+FLASH IS COMPLETE."));
      delay(100);
      reboot();
  });
  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {

      Serial.printf("OTA  : Progress %u%%\r", (progress / (total / 100)));
  });

  ArduinoOTA.onError([](ota_error_t error) {
      Serial.print(F("\nOTA  : Error (will reboot): "));
      if (error == OTA_AUTH_ERROR) Serial.println(F("Auth Failed"));
      else if (error == OTA_BEGIN_ERROR) Serial.println(F("Begin Failed"));
      else if (error == OTA_CONNECT_ERROR) Serial.println(F("Connect Failed"));
      else if (error == OTA_RECEIVE_ERROR) Serial.println(F("Receive Failed"));
      else if (error == OTA_END_ERROR) Serial.println(F("End Failed"));

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

int calc_CRC16(const String& text) {
  return calc_CRC16(text.c_str(), text.length());
}

int calc_CRC16(const char *ptr, int count)
{
    int  crc;
    char i;
    crc = 0;
    while (--count >= 0)
    {
        crc = crc ^ (int) *ptr++ << 8;
        i = 8;
        do
        {
            if (crc & 0x8000)
                crc = crc << 1 ^ 0x1021;
            else
                crc = crc << 1;
        } while(--i);
    }
    return crc;
}

// Compute the dew point temperature, given temperature and humidity (temp in Celcius)
// Formula: http://www.ajdesigner.com/phphumidity/dewpoint_equation_dewpoint_temperature.php
// Td = (f/100)^(1/8) * (112 + 0.9*T) + 0.1*T - 112
float compute_dew_point_temp(float temperature, float humidity_percentage) {
  return pow(humidity_percentage / 100.0, 0.125) *
         (112.0 + 0.9*temperature) + 0.1*temperature - 112.0;
}

// Compute the humidity given temperature and dew point temperature (temp in Celcius)
// Formula: http://www.ajdesigner.com/phphumidity/dewpoint_equation_relative_humidity.php
// f = 100 * ((112 - 0.1*T + Td) / (112 + 0.9 * T))^8
float compute_humidity_from_dewpoint(float temperature, float dew_temperature) {
  return 100.0 * pow((112.0 - 0.1 * temperature + dew_temperature) /
                     (112.0 + 0.9 * temperature), 8);
}
