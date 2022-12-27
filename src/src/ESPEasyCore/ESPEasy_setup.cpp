#include "../ESPEasyCore/ESPEasy_setup.h"

#include "../../ESPEasy_fdwdecl.h" // Needed for PluginInit() and CPluginInit()

#include "../../ESPEasy-Globals.h"
#include "../../_Plugin_Helper.h"
#include "../CustomBuild/CompiletimeDefines.h"
#include "../ESPEasyCore/ESPEasyNetwork.h"
#include "../ESPEasyCore/ESPEasyRules.h"
#include "../ESPEasyCore/ESPEasyWifi.h"
#include "../ESPEasyCore/Serial.h"
#include "../Globals/Cache.h"
#include "../Globals/ESPEasyWiFiEvent.h"
#include "../Globals/ESPEasy_time.h"
#include "../Globals/NetworkState.h"
#include "../Globals/RTC.h"
#include "../Globals/Statistics.h"
#include "../Globals/WiFi_AP_Candidates.h"
#include "../Helpers/_CPlugin_init.h"
#include "../Helpers/_NPlugin_init.h"
#include "../Helpers/_Plugin_init.h"
#include "../Helpers/DeepSleep.h"
#include "../Helpers/ESPEasyRTC.h"
#include "../Helpers/ESPEasy_FactoryDefault.h"
#include "../Helpers/ESPEasy_Storage.h"
#include "../Helpers/ESPEasy_checks.h"
#include "../Helpers/Hardware.h"
#include "../Helpers/Memory.h"
#include "../Helpers/Misc.h"
#include "../Helpers/StringGenerator_System.h"
#include "../WebServer/ESPEasy_WebServer.h"


#ifdef USE_RTOS_MULTITASKING
# include "../Helpers/Networking.h"
# include "../Helpers/PeriodicalActions.h"
#endif // ifdef USE_RTOS_MULTITASKING

#if FEATURE_ARDUINO_OTA
# include "../Helpers/OTA.h"
#endif // if FEATURE_ARDUINO_OTA

#ifdef ESP32
#include <soc/boot_mode.h>
#include <soc/gpio_reg.h>
#include <soc/efuse_reg.h>

#include <esp_pm.h>

#endif


#ifdef USE_RTOS_MULTITASKING
void RTOS_TaskServers(void *parameter)
{
  while (true) {
    delay(100);
    web_server.handleClient();
    #if FEATURE_ESPEASY_P2P
    checkUDP();
    #endif
  }
}

void RTOS_TaskSerial(void *parameter)
{
  while (true) {
    delay(100);
    serial();
  }
}

void RTOS_Task10ps(void *parameter)
{
  while (true) {
    delay(100);
    run10TimesPerSecond();
  }
}

void RTOS_HandleSchedule(void *parameter)
{
  while (true) {
    Scheduler.handle_schedule();
  }
}

#endif // ifdef USE_RTOS_MULTITASKING


/*********************************************************************************************\
* ISR call back function for handling the watchdog.
\*********************************************************************************************/
void sw_watchdog_callback(void *arg)
{
  yield(); // feed the WD
  ++sw_watchdog_callback_count;
}

/*********************************************************************************************\
* SETUP
\*********************************************************************************************/
void ESPEasy_setup()
{
#if defined(ESP8266_DISABLE_EXTRA4K) || defined(USE_SECOND_HEAP)
  disable_extra4k_at_link_time();
#endif
#ifdef PHASE_LOCKED_WAVEFORM
  enablePhaseLockedWaveform();
#endif
#ifdef USE_SECOND_HEAP
  HeapSelectDram ephemeral;
#endif
#ifdef ESP32
#ifdef DISABLE_ESP32_BROWNOUT
  DisableBrownout();      // Workaround possible weak LDO resulting in brownout detection during Wifi connection
#endif  // DISABLE_ESP32_BROWNOUT

#ifdef BOARD_HAS_PSRAM
  psramInit();
#endif

#if CONFIG_IDF_TARGET_ESP32
  // restore GPIO16/17 if no PSRAM is found
  if (!FoundPSRAM()) {
    // test if the CPU is not pico
    uint32_t chip_ver = REG_GET_FIELD(EFUSE_BLK0_RDATA3_REG, EFUSE_RD_CHIP_VER_PKG);
    uint32_t pkg_version = chip_ver & 0x7;
    if (pkg_version <= 3) {   // D0WD, S0WD, D2WD
      gpio_reset_pin(GPIO_NUM_16);
      gpio_reset_pin(GPIO_NUM_17);
    }
  }
#endif  // if CONFIG_IDF_TARGET_ESP32
  initADC();
#endif  // ESP32
#ifndef BUILD_NO_RAM_TRACKER
  lowestFreeStack = getFreeStackWatermark();
  lowestRAM       = FreeMem();
#endif // ifndef BUILD_NO_RAM_TRACKER

  initWiFi();
  WiFiEventData.clearAll();

#ifndef BUILD_MINIMAL_OTA
  run_compiletime_checks();
#endif
#ifdef ESP8266

  //  ets_isr_attach(8, sw_watchdog_callback, nullptr);  // Set a callback for feeding the watchdog.
#endif // ifdef ESP8266


  // Read ADC at boot, before WiFi tries to connect.
  // see https://github.com/letscontrolit/ESPEasy/issues/2646
#if FEATURE_ADC_VCC
  vcc = ESP.getVcc() / 1000.0f;
#endif // if FEATURE_ADC_VCC
#ifdef ESP8266
  espeasy_analogRead(A0);
#endif // ifdef ESP8266

  initAnalogWrite();

  resetPluginTaskData();

  #ifndef BUILD_NO_RAM_TRACKER
  checkRAM(F("setup"));
  #endif // ifndef BUILD_NO_RAM_TRACKER

  Serial.begin(115200);

  // serialPrint("\n\n\nBOOOTTT\n\n\n");

  initLog();
  #ifndef BUILD_NO_RAM_TRACKER
  logMemUsageAfter(F("initLog()"));
  #endif
  #ifdef BOARD_HAS_PSRAM
  if (FoundPSRAM()) {
    if (UsePSRAM()) {
      addLog(LOG_LEVEL_INFO, F("Using PSRAM"));
    } else {
      addLog(LOG_LEVEL_ERROR, F("PSRAM found, unable to use"));
    }
  }
  #endif

  if (SpiffsSectors() < 32)
  {
    serialPrintln(F("\nNo (or too small) FS area..\nSystem Halted\nPlease reflash with 128k FS minimum!"));

    while (true) {
      delay(1);
    }
  }

  emergencyReset();

  if (loglevelActiveFor(LOG_LEVEL_INFO)) {
    String log = F("\n\n\rINIT : Booting version: ");
    log += getValue(LabelType::BINARY_FILENAME);
    log += F(", (");
    log += get_build_origin();
    log += F(") ");
    log += getValue(LabelType::GIT_BUILD);
    log += F(" (");
    log += getSystemLibraryString();
    log += ')';
    addLogMove(LOG_LEVEL_INFO, log);
    log  = F("INIT : Free RAM:");
    log += FreeMem();
    addLogMove(LOG_LEVEL_INFO, log);
  }

  readBootCause();

  {
    String log  = F("INIT : ");
    log += getLastBootCauseString();

    if (readFromRTC())
    {
      RTC.bootFailedCount++;
      RTC.bootCounter++;
      lastMixedSchedulerId_beforereboot = RTC.lastMixedSchedulerId;
      readUserVarFromRTC();

      log += F(" #");
      log += RTC.bootCounter;

      #ifndef BUILD_NO_DEBUG
      log += F(" Last Action before Reboot: ");
      log += ESPEasy_Scheduler::decodeSchedulerId(lastMixedSchedulerId_beforereboot);
      log += F(" Last systime: ");
      log += RTC.lastSysTime;
      #endif // ifndef BUILD_NO_DEBUG
    }

    // cold boot (RTC memory empty)
    else
    {
      initRTC();

      // cold boot situation
      if (lastBootCause == BOOT_CAUSE_MANUAL_REBOOT) { // only set this if not set earlier during boot stage.
        lastBootCause = BOOT_CAUSE_COLD_BOOT;
      }
      log = F("INIT : Cold Boot");
    }

    log += F(" - Restart Reason: ");
    log += getResetReasonString();

    RTC.deepSleepState = 0;
    saveToRTC();

    addLogMove(LOG_LEVEL_INFO, log);
  }
  #ifndef BUILD_NO_RAM_TRACKER
  logMemUsageAfter(F("RTC init"));
  #endif

  fileSystemCheck();
  #ifndef BUILD_NO_RAM_TRACKER
  logMemUsageAfter(F("fileSystemCheck()"));
  #endif

  //  progMemMD5check();
  LoadSettings();
  #ifndef BUILD_NO_RAM_TRACKER
  logMemUsageAfter(F("LoadSettings()"));
  #endif

#ifdef ESP32
  if (Settings.EcoPowerMode()) {
    // Configure dynamic frequency scaling:
    // maximum and minimum frequencies are set in sdkconfig,
    // automatic light sleep is enabled if tickless idle support is enabled.
#if CONFIG_IDF_TARGET_ESP32
    esp_pm_config_esp32_t pm_config = {
            .max_freq_mhz = 240,
#elif CONFIG_IDF_TARGET_ESP32S2
    esp_pm_config_esp32s2_t pm_config = {
            .max_freq_mhz = 240,
#elif CONFIG_IDF_TARGET_ESP32C3
    esp_pm_config_esp32c3_t pm_config = {
            .max_freq_mhz = 160,
#elif CONFIG_IDF_TARGET_ESP32S3
    esp_pm_config_esp32s3_t pm_config = {
            .max_freq_mhz = 240,
#elif CONFIG_IDF_TARGET_ESP32C2
    esp_pm_config_esp32c2_t pm_config = {
            .max_freq_mhz = 120,
#endif
            .min_freq_mhz = 80,
#if CONFIG_FREERTOS_USE_TICKLESS_IDLE
            .light_sleep_enable = true
#endif
    };
    esp_pm_configure(&pm_config);
  }
#endif


  #ifndef BUILD_NO_RAM_TRACKER
  checkRAM(F("hardwareInit"));
  #endif // ifndef BUILD_NO_RAM_TRACKER
  hardwareInit();
  #ifndef BUILD_NO_RAM_TRACKER
  logMemUsageAfter(F("hardwareInit()"));
  #endif

  node_time.restoreFromRTC();

  Settings.UseRTOSMultitasking = false; // For now, disable it, we experience heap corruption.

  if ((RTC.bootFailedCount > 10) && (RTC.bootCounter > 10)) {
    uint8_t toDisable = RTC.bootFailedCount - 10;
    toDisable = disablePlugin(toDisable);

    if (toDisable != 0) {
      toDisable = disableController(toDisable);
    }
    #if FEATURE_NOTIFIER
    if (toDisable != 0) {
      toDisable = disableNotification(toDisable);
    }
    #endif

    if (toDisable != 0) {
      toDisable = disableRules(toDisable);
    }

    if (toDisable != 0) {
      toDisable = disableAllPlugins(toDisable);
    }

    if (toDisable != 0) {
      toDisable = disableAllControllers(toDisable);
    }
#if FEATURE_NOTIFIER
    if (toDisable != 0) {
      toDisable = disableAllNotifications(toDisable);
    }
#endif
  }
  #if FEATURE_ETHERNET

  // This ensures, that changing WIFI OR ETHERNET MODE happens properly only after reboot. Changing without reboot would not be a good idea.
  // This only works after LoadSettings();
  // Do not call setNetworkMedium here as that may try to clean up settings.
  active_network_medium = Settings.NetworkMedium;
  #endif // if FEATURE_ETHERNET

  if (active_network_medium == NetworkMedium_t::WIFI) {
    WiFi_AP_Candidates.load_knownCredentials();
    setSTA(true);
    if (!WiFi_AP_Candidates.hasKnownCredentials()) {
      WiFiEventData.wifiSetup = true;
      RTC.clearLastWiFi(); // Must scan all channels
      // Wait until scan has finished to make sure as many as possible are found
      // We're still in the setup phase, so nothing else is taking resources of the ESP.
      WifiScan(false);
      WiFiEventData.lastScanMoment.clear();
    }

    // Always perform WiFi scan
    // It appears reconnecting from RTC may take just as long to be able to send first packet as performing a scan first and then connect.
    // Perhaps the WiFi radio needs some time to stabilize first?
    WifiScan(false);
    setWifiMode(WIFI_OFF);
  }
  #ifndef BUILD_NO_RAM_TRACKER
  logMemUsageAfter(F("WifiScan()"));
  #endif


  //  setWifiMode(WIFI_STA);
  checkRuleSets();
  #ifndef BUILD_NO_RAM_TRACKER
  logMemUsageAfter(F("checkRuleSets()"));
  #endif


  // if different version, eeprom settings structure has changed. Full Reset needed
  // on a fresh ESP module eeprom values are set to 255. Version results into -1 (signed int)
  if ((Settings.Version != VERSION) || (Settings.PID != ESP_PROJECT_PID))
  {
    // Direct Serial is allowed here, since this is only an emergency task.
    serialPrint(F("\nPID:"));
    serialPrintln(String(Settings.PID));
    serialPrint(F("Version:"));
    serialPrintln(String(Settings.Version));
    serialPrintln(F("INIT : Incorrect PID or version!"));
    delay(1000);
    ResetFactory();
  }

  initSerial();
  #ifndef BUILD_NO_RAM_TRACKER
  logMemUsageAfter(F("initSerial()"));
  #endif


  if (Settings.Build != get_build_nr()) {
    BuildFixes();
  }

  if (loglevelActiveFor(LOG_LEVEL_INFO)) {
    String log  = F("INIT : Free RAM:");
    log += FreeMem();
    addLogMove(LOG_LEVEL_INFO, log);
  }

# ifndef BUILD_NO_DEBUG
  if (Settings.UseSerial && (Settings.SerialLogLevel >= LOG_LEVEL_DEBUG_MORE)) {
    Serial.setDebugOutput(true);
  }
#endif

  timermqtt_interval      = 250; // Interval for checking MQTT
  timerAwakeFromDeepSleep = millis();
  CPluginInit();
  #ifndef BUILD_NO_RAM_TRACKER
  logMemUsageAfter(F("CPluginInit()"));
  #endif
  #if FEATURE_NOTIFIER
  NPluginInit();
  #ifndef BUILD_NO_RAM_TRACKER
  logMemUsageAfter(F("NPluginInit()"));
  #endif
  #endif // if FEATURE_NOTIFIER

  PluginInit();
  #ifndef BUILD_NO_RAM_TRACKER
  logMemUsageAfter(F("PluginInit()"));
  #endif
  if (loglevelActiveFor(LOG_LEVEL_INFO)) {
    String log;
    log.reserve(80);
    log += concat(F("INFO : Plugins: "), deviceCount + 1);
    log += ' ';
    log += getPluginDescriptionString();
    log += F(" (");
    log += getSystemLibraryString();
    log += ')';
    addLogMove(LOG_LEVEL_INFO, log);
  }

  if (deviceCount + 1 >= PLUGIN_MAX) {
    addLog(LOG_LEVEL_ERROR, concat(F("Programming error! - Increase PLUGIN_MAX ("), deviceCount) + ')');
  }

  clearAllCaches();
  #ifndef BUILD_NO_RAM_TRACKER
  logMemUsageAfter(F("clearAllCaches()"));
  #endif

  if (Settings.UseRules && isDeepSleepEnabled())
  {
    String event = F("System#NoSleep=");
    event += Settings.deepSleep_wakeTime;
    rulesProcessing(event); // TD-er: Process events in the setup() now.
  }

  if (Settings.UseRules)
  {
    String event = F("System#Wake");
    rulesProcessing(event); // TD-er: Process events in the setup() now.
  }
  #ifdef ESP32
  if (Settings.UseRules)
  {
    const uint32_t gpio_strap =   GPIO_REG_READ(GPIO_STRAP_REG);
//    BOOT_MODE_GET();

    // Event values: GPIO-5, GPIO-15, GPIO-4, GPIO-2
    String event = F("System#BootMode=");
    event += bitRead(gpio_strap, 0); // GPIO-5
    event += ',';
    event += bitRead(gpio_strap, 1); // GPIO-15
    event += ',';
    event += bitRead(gpio_strap, 2); // GPIO-4
    event += ',';
    event += bitRead(gpio_strap, 3); // GPIO-2
    rulesProcessing(event);
  }
  #endif

  NetworkConnectRelaxed();
  #ifndef BUILD_NO_RAM_TRACKER
  logMemUsageAfter(F("NetworkConnectRelaxed()"));
  #endif

  setWebserverRunning(true);
  #ifndef BUILD_NO_RAM_TRACKER
  logMemUsageAfter(F("setWebserverRunning()"));
  #endif


  #if FEATURE_REPORTING
  ReportStatus();
  #endif // if FEATURE_REPORTING

  #if FEATURE_ARDUINO_OTA
  ArduinoOTAInit();
  #ifndef BUILD_NO_RAM_TRACKER
  logMemUsageAfter(F("ArduinoOTAInit()"));
  #endif
  #endif // if FEATURE_ARDUINO_OTA

  if (node_time.systemTimePresent()) {
    node_time.initTime();
    #ifndef BUILD_NO_RAM_TRACKER
    logMemUsageAfter(F("node_time.initTime()"));
    #endif
  }

  if (Settings.UseRules)
  {
    String event = F("System#Boot");
    rulesProcessing(event); // TD-er: Process events in the setup() now.
    #ifndef BUILD_NO_RAM_TRACKER
    logMemUsageAfter(F("rulesProcessing(System#Boot)"));
    #endif
  }

  writeDefaultCSS();
  #ifndef BUILD_NO_RAM_TRACKER
  logMemUsageAfter(F("writeDefaultCSS()"));
  #endif


  UseRTOSMultitasking = Settings.UseRTOSMultitasking;
  #ifdef USE_RTOS_MULTITASKING

  if (UseRTOSMultitasking) {
    if (loglevelActiveFor(LOG_LEVEL_INFO)) {
      addLog(LOG_LEVEL_INFO, F("RTOS : Launching tasks"));
    }
    xTaskCreatePinnedToCore(RTOS_TaskServers, "RTOS_TaskServers", 16384, nullptr, 1, nullptr, 1);
    xTaskCreatePinnedToCore(RTOS_TaskSerial,  "RTOS_TaskSerial",  8192,  nullptr, 1, nullptr, 1);
    xTaskCreatePinnedToCore(RTOS_Task10ps,    "RTOS_Task10ps",    8192,  nullptr, 1, nullptr, 1);
    xTaskCreatePinnedToCore(
      RTOS_HandleSchedule,   /* Function to implement the task */
      "RTOS_HandleSchedule", /* Name of the task */
      16384,                 /* Stack size in words */
      nullptr,                  /* Task input parameter */
      1,                     /* Priority of the task */
      nullptr,                  /* Task handle. */
      1);                    /* Core where the task should run */
  }
  #endif // ifdef USE_RTOS_MULTITASKING

  // Start the interval timers at N msec from now.
  // Make sure to start them at some time after eachother,
  // since they will keep running at the same interval.
  Scheduler.setIntervalTimerOverride(ESPEasy_Scheduler::IntervalTimer_e::TIMER_20MSEC,     5);    // timer for periodic actions 50 x per/sec
  Scheduler.setIntervalTimerOverride(ESPEasy_Scheduler::IntervalTimer_e::TIMER_100MSEC,    66);   // timer for periodic actions 10 x per/sec
  Scheduler.setIntervalTimerOverride(ESPEasy_Scheduler::IntervalTimer_e::TIMER_1SEC,       777);  // timer for periodic actions once per/sec
  Scheduler.setIntervalTimerOverride(ESPEasy_Scheduler::IntervalTimer_e::TIMER_30SEC,      1333); // timer for watchdog once per 30 sec
  Scheduler.setIntervalTimerOverride(ESPEasy_Scheduler::IntervalTimer_e::TIMER_MQTT,       88);   // timer for interaction with MQTT
  Scheduler.setIntervalTimerOverride(ESPEasy_Scheduler::IntervalTimer_e::TIMER_STATISTICS, 2222);
  #ifndef BUILD_NO_RAM_TRACKER
  logMemUsageAfter(F("Scheduler.setIntervalTimerOverride"));
  #endif

}
