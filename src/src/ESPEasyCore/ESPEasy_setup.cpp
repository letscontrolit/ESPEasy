#include "../ESPEasyCore/ESPEasy_setup.h"

#include "../../ESPEasy_fdwdecl.h" // Needed for PluginInit() and CPluginInit()

#include "../../ESPEasy-Globals.h"
#include "../../_Plugin_Helper.h"
#include "../Commands/InternalCommands_decoder.h"
#include "../CustomBuild/CompiletimeDefines.h"
#include "../ESPEasyCore/ESPEasyGPIO.h"
#include "../ESPEasyCore/ESPEasyNetwork.h"
#include "../ESPEasyCore/ESPEasyRules.h"
#include "../ESPEasyCore/ESPEasyWifi.h"
#include "../ESPEasyCore/ESPEasyWifi_ProcessEvent.h"
#include "../ESPEasyCore/Serial.h"
#include "../Globals/Cache.h"
#include "../Globals/ESPEasy_Console.h"
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
#include "../Helpers/Hardware_device_info.h"
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

# if ESP_IDF_VERSION_MAJOR < 5
#  include <esp_pm.h>
#  include <soc/efuse_reg.h>
#  include <soc/boot_mode.h>
#  include <soc/gpio_reg.h>
# else // if ESP_IDF_VERSION_MAJOR < 5
#  include <hal/efuse_hal.h>
#  include <rom/gpio.h>
#  include <esp_pm.h>
# endif // if ESP_IDF_VERSION_MAJOR < 5

# if CONFIG_IDF_TARGET_ESP32
#  if ESP_IDF_VERSION_MAJOR < 5
#   include "hal/efuse_ll.h"
#   include "hal/efuse_hal.h"
#  else // if ESP_IDF_VERSION_MAJOR < 5
#   include <soc/efuse_defs.h>
#   include <bootloader_common.h>

#   if ESP_IDF_VERSION > ESP_IDF_VERSION_VAL(5, 2, 0)

// IDF5.3 fix esp_gpio_reserve used in init PSRAM.
#    include "esp_private/esp_gpio_reserve.h"
#   endif // if ESP_IDF_VERSION > ESP_IDF_VERSION_VAL(5, 2, 0)
#  endif // if ESP_IDF_VERSION_MAJOR < 5
# endif // if CONFIG_IDF_TARGET_ESP32

#endif // ifdef ESP32


#ifdef USE_RTOS_MULTITASKING

void RTOS_TaskServers(void *parameter)
{
  while (true) {
    delay(100);
    web_server.handleClient();
    # if FEATURE_ESPEASY_P2P
    checkUDP();
    # endif // if FEATURE_ESPEASY_P2P
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

  //  disable_extra4k_at_link_time();
#endif // if defined(ESP8266_DISABLE_EXTRA4K) || defined(USE_SECOND_HEAP)
#ifdef PHASE_LOCKED_WAVEFORM
  enablePhaseLockedWaveform();
#endif // ifdef PHASE_LOCKED_WAVEFORM
#ifdef USE_SECOND_HEAP
  HeapSelectDram ephemeral;
#endif // ifdef USE_SECOND_HEAP
#ifdef ESP32
# ifdef DISABLE_ESP32_BROWNOUT
  DisableBrownout(); // Workaround possible weak LDO resulting in brownout detection during Wifi connection
# endif  // DISABLE_ESP32_BROWNOUT

# ifdef BOARD_HAS_PSRAM
  psramInit();
# endif // ifdef BOARD_HAS_PSRAM

# if CONFIG_IDF_TARGET_ESP32

  // restore GPIO16/17 if no PSRAM is found
  if (!FoundPSRAM()) {
    // test if the CPU is not pico
    #  if ESP_IDF_VERSION_MAJOR < 5
    uint32_t chip_ver    = REG_GET_FIELD(EFUSE_BLK0_RDATA3_REG, EFUSE_RD_CHIP_VER_PKG);
    uint32_t pkg_version = chip_ver & 0x7;
    #  else // if ESP_IDF_VERSION_MAJOR < 5
    uint32_t pkg_version = bootloader_common_get_chip_ver_pkg();
    uint32_t chip_ver    = REG_GET_FIELD(EFUSE_BLK0_RDATA3_REG, EFUSE_RD_CHIP_PACKAGE);
    #  endif // if ESP_IDF_VERSION_MAJOR < 5

    if (pkg_version <= 7) { // D0WD, S0WD, D2WD
#ifdef CORE32SOLO1
      gpio_num_t PSRAM_CLK = GPIO_NUM_17;
      gpio_num_t PSRAM_CS  = GPIO_NUM_16;
#else
      gpio_num_t PSRAM_CLK = static_cast<gpio_num_t>(CONFIG_D0WD_PSRAM_CLK_IO);
      gpio_num_t PSRAM_CS  = static_cast<gpio_num_t>(CONFIG_D0WD_PSRAM_CS_IO);

      switch (pkg_version)
      {
        case EFUSE_RD_CHIP_VER_PKG_ESP32D2WDQ5:
          PSRAM_CLK = static_cast<gpio_num_t>(CONFIG_D2WD_PSRAM_CLK_IO);
          PSRAM_CS  = static_cast<gpio_num_t>(CONFIG_D2WD_PSRAM_CS_IO);
          break;
        case EFUSE_RD_CHIP_VER_PKG_ESP32PICOD4:
        case EFUSE_RD_CHIP_VER_PKG_ESP32PICOV302:
          PSRAM_CLK = static_cast<gpio_num_t>(GPIO_NUM_NC);
          PSRAM_CS  = static_cast<gpio_num_t>(CONFIG_PICO_PSRAM_CS_IO);
          break;
      }
#endif
#  if ESP_IDF_VERSION > ESP_IDF_VERSION_VAL(5, 2, 0)

      // Thanks Theo Arends from Tasmota
      if (PSRAM_CLK != GPIO_NUM_NC) {
        esp_gpio_revoke(BIT64(PSRAM_CLK) | BIT64(PSRAM_CS));
      } else {
        esp_gpio_revoke(BIT64(PSRAM_CS));
      }
#  endif // if ESP_IDF_VERSION > ESP_IDF_VERSION_VAL(5, 2, 0)

      if (PSRAM_CLK != GPIO_NUM_NC) {
        gpio_reset_pin(PSRAM_CLK);
      }
      gpio_reset_pin(PSRAM_CS);
    }
  }
# endif  // if CONFIG_IDF_TARGET_ESP32
  initADC();
#endif  // ESP32
#ifndef BUILD_NO_RAM_TRACKER
  lowestFreeStack = getFreeStackWatermark();
  lowestRAM       = FreeMem();
#endif // ifndef BUILD_NO_RAM_TRACKER

  /*
   #ifdef ESP32
     {
     ESPEasy_NVS_Helper preferences;
     ResetFactoryDefaultPreference.init(preferences);
     }
   #endif
   */
#ifndef BUILD_NO_DEBUG

  //  checkAll_internalCommands();
#endif // ifndef BUILD_NO_DEBUG

  PluginSetup();
  CPluginSetup();

  initWiFi();
  WiFiEventData.clearAll();

#ifndef BUILD_MINIMAL_OTA
  run_compiletime_checks();
#endif // ifndef BUILD_MINIMAL_OTA
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
  ESPEasy_Console.begin(115200);

  // serialPrint("\n\n\nBOOOTTT\n\n\n");

  initLog();
  #ifndef BUILD_NO_RAM_TRACKER
  logMemUsageAfter(F("initLog()"));
  #endif // ifndef BUILD_NO_RAM_TRACKER
  #ifdef BOARD_HAS_PSRAM

  if (FoundPSRAM()) {
    if (UsePSRAM()) {
      addLog(LOG_LEVEL_INFO, F("Using PSRAM"));
    } else {
      addLog(LOG_LEVEL_ERROR, F("PSRAM found, unable to use"));
    }
  }
  #endif // ifdef BOARD_HAS_PSRAM

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
    String log = F("INIT : ");
    log += getLastBootCauseString();

    if (readFromRTC())
    {
      RTC.bootFailedCount++;
      RTC.bootCounter++;
      lastMixedSchedulerId_beforereboot.mixed_id = RTC.lastMixedSchedulerId;
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
  #endif // ifndef BUILD_NO_RAM_TRACKER

  fileSystemCheck();
  #ifndef BUILD_NO_RAM_TRACKER
  logMemUsageAfter(F("fileSystemCheck()"));
  #endif // ifndef BUILD_NO_RAM_TRACKER

  //  progMemMD5check();
  LoadSettings();
#if FEATURE_DEFINE_SERIAL_CONSOLE_PORT
  ESPEasy_Console.reInit();
#endif // if FEATURE_DEFINE_SERIAL_CONSOLE_PORT

  #ifndef BUILD_NO_RAM_TRACKER
  logMemUsageAfter(F("LoadSettings()"));
  #endif // ifndef BUILD_NO_RAM_TRACKER

#ifdef ESP32

  // Configure dynamic frequency scaling:
  // maximum and minimum frequencies are set in sdkconfig,
  // automatic light sleep is enabled if tickless idle support is enabled.
  ESP_PM_CONFIG_T pm_config = {
    .max_freq_mhz = getCPU_MaxFreqMHz(),
    .min_freq_mhz = Settings.EcoPowerMode() ? getCPU_MinFreqMHz() : getCPU_MaxFreqMHz(),
# if CONFIG_FREERTOS_USE_TICKLESS_IDLE
    .light_sleep_enable = Settings.EcoPowerMode()
# else
    .light_sleep_enable = false
# endif // if CONFIG_FREERTOS_USE_TICKLESS_IDLE
  };
  esp_pm_configure(&pm_config);
#endif // ifdef ESP32


  #ifndef BUILD_NO_RAM_TRACKER
  checkRAM(F("hardwareInit"));
  #endif // ifndef BUILD_NO_RAM_TRACKER
  hardwareInit();
  #ifndef BUILD_NO_RAM_TRACKER
  logMemUsageAfter(F("hardwareInit()"));
  #endif // ifndef BUILD_NO_RAM_TRACKER

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
    #endif // if FEATURE_NOTIFIER

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
#endif // if FEATURE_NOTIFIER
  }
  #if FEATURE_ETHERNET

  // This ensures, that changing WIFI OR ETHERNET MODE happens properly only after reboot. Changing without reboot would not be a good idea.
  // This only works after LoadSettings();
  // Do not call setNetworkMedium here as that may try to clean up settings.
  active_network_medium = Settings.NetworkMedium;
  #else // if FEATURE_ETHERNET

  if (Settings.NetworkMedium == NetworkMedium_t::Ethernet) {
    Settings.NetworkMedium = NetworkMedium_t::WIFI;
  }
  #endif // if FEATURE_ETHERNET

  setNetworkMedium(Settings.NetworkMedium);

  bool initWiFi = active_network_medium == NetworkMedium_t::WIFI;

  // FIXME TD-er: Must add another check for 'delayed start WiFi' for poorly designed ESP8266 nodes.


  if (initWiFi) {
    WiFi_AP_Candidates.clearCache();
    WiFi_AP_Candidates.load_knownCredentials();
    setSTA(true);

    if (!WiFi_AP_Candidates.hasCandidates()) {
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
    if (!WiFi_AP_Candidates.hasCandidates()) {
      WifiScan(false, RTC.lastWiFiChannel);
    }
    WiFi_AP_Candidates.clearCache();
    processScanDone();
    WiFi_AP_Candidates.load_knownCredentials();

    if (!WiFi_AP_Candidates.hasCandidates()) {
      addLog(LOG_LEVEL_INFO, F("Setup: Scan all channels"));
      WifiScan(false);
    }

    //    setWifiMode(WIFI_OFF);
  }
  #ifndef BUILD_NO_RAM_TRACKER
  logMemUsageAfter(F("WifiScan()"));
  #endif // ifndef BUILD_NO_RAM_TRACKER


  //  setWifiMode(WIFI_STA);
  checkRuleSets();
  #ifndef BUILD_NO_RAM_TRACKER
  logMemUsageAfter(F("checkRuleSets()"));
  #endif // ifndef BUILD_NO_RAM_TRACKER


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
  #endif // ifndef BUILD_NO_RAM_TRACKER

  if (loglevelActiveFor(LOG_LEVEL_INFO)) {
    addLogMove(LOG_LEVEL_INFO, concat(F("INIT : Free RAM:"), FreeMem()));
  }

#ifndef BUILD_NO_DEBUG

  if (Settings.UseSerial && (Settings.SerialLogLevel >= LOG_LEVEL_DEBUG_MORE)) {
    ESPEasy_Console.setDebugOutput(true);
  }
#endif // ifndef BUILD_NO_DEBUG

  timermqtt_interval      = 100; // Interval for checking MQTT
  timerAwakeFromDeepSleep = millis();
  CPluginInit();
  #ifndef BUILD_NO_RAM_TRACKER
  logMemUsageAfter(F("CPluginInit()"));
  #endif // ifndef BUILD_NO_RAM_TRACKER
  #if FEATURE_NOTIFIER
  NPluginInit();
  # ifndef BUILD_NO_RAM_TRACKER
  logMemUsageAfter(F("NPluginInit()"));
  # endif
  #endif // if FEATURE_NOTIFIER

  PluginInit();

  initSerial(); // Plugins may have altered serial, so re-init serial

  #ifndef BUILD_NO_RAM_TRACKER
  logMemUsageAfter(F("PluginInit()"));
  #endif

  if (loglevelActiveFor(LOG_LEVEL_INFO)) {
    String log;
    log.reserve(80);
    log += concat(F("INFO : Plugins: "), getDeviceCount() + 1);
    log += ' ';
    log += getPluginDescriptionString();
    log += F(" (");
    log += getSystemLibraryString();
    log += ')';
    addLogMove(LOG_LEVEL_INFO, log);
  }

  /*
     if ((getDeviceCount() + 1) >= PLUGIN_MAX) {
      addLog(LOG_LEVEL_ERROR, concat(F("Programming error! - Increase PLUGIN_MAX ("), getDeviceCount()) + ')');
     }
   */

  clearAllCaches();
  #ifndef BUILD_NO_RAM_TRACKER
  logMemUsageAfter(F("clearAllCaches()"));
  #endif // ifndef BUILD_NO_RAM_TRACKER

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

    // Event values:
    // ESP32   :  GPIO-5, GPIO-15, GPIO-4, GPIO-2, GPIO-0, GPIO-12
    // ESP32-C3:  bit 0: GPIO2, bit 2: GPIO8, bit 3: GPIO9
    // ESP32-S2: Unclear what bits represent which strapping state.
    // ESP32-S3: bit5 ~ bit2 correspond to strapping pins GPIO3, GPIO45, GPIO0, and GPIO46 respectively.
    String event = F("System#BootMode=");
    event += bitRead(gpio_strap, 0);
    event += ',';
    event += bitRead(gpio_strap, 1);
    event += ',';
    event += bitRead(gpio_strap, 2);
    event += ',';
    event += bitRead(gpio_strap, 3);
    event += ',';
    event += bitRead(gpio_strap, 4);
    event += ',';
    event += bitRead(gpio_strap, 5);
    rulesProcessing(event);
  }
  #endif // ifdef ESP32

  #if FEATURE_ETHERNET

  if (Settings.ETH_Pin_power_rst != -1) {
    GPIO_Write(PLUGIN_GPIO, Settings.ETH_Pin_power_rst, 1);
  }

  #endif // if FEATURE_ETHERNET

  NetworkConnectRelaxed();
  #ifndef BUILD_NO_RAM_TRACKER
  logMemUsageAfter(F("NetworkConnectRelaxed()"));
  #endif // ifndef BUILD_NO_RAM_TRACKER

  setWebserverRunning(true);
  #ifndef BUILD_NO_RAM_TRACKER
  logMemUsageAfter(F("setWebserverRunning()"));
  #endif // ifndef BUILD_NO_RAM_TRACKER


  #if FEATURE_REPORTING
  ReportStatus();
  #endif // if FEATURE_REPORTING

  #if FEATURE_ARDUINO_OTA
  ArduinoOTAInit();
  # ifndef BUILD_NO_RAM_TRACKER
  logMemUsageAfter(F("ArduinoOTAInit()"));
  # endif
  #endif // if FEATURE_ARDUINO_OTA

  if (node_time.systemTimePresent()) {
    node_time.initTime();
    #ifndef BUILD_NO_RAM_TRACKER
    logMemUsageAfter(F("node_time.initTime()"));
    #endif // ifndef BUILD_NO_RAM_TRACKER
  }

  if (Settings.UseRules)
  {
    String event = F("System#Boot");
    rulesProcessing(event); // TD-er: Process events in the setup() now.
    #ifndef BUILD_NO_RAM_TRACKER
    logMemUsageAfter(F("rulesProcessing(System#Boot)"));
    #endif // ifndef BUILD_NO_RAM_TRACKER
  }

  writeDefaultCSS();
  #ifndef BUILD_NO_RAM_TRACKER
  logMemUsageAfter(F("writeDefaultCSS()"));
  #endif // ifndef BUILD_NO_RAM_TRACKER


  #ifdef USE_RTOS_MULTITASKING
  UseRTOSMultitasking = Settings.UseRTOSMultitasking;

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
      nullptr,               /* Task input parameter */
      1,                     /* Priority of the task */
      nullptr,               /* Task handle. */
      1);                    /* Core where the task should run */
  }
  #endif // ifdef USE_RTOS_MULTITASKING

  // Start the interval timers at N msec from now.
  // Make sure to start them at some time after eachother,
  // since they will keep running at the same interval.
  Scheduler.setIntervalTimerOverride(SchedulerIntervalTimer_e::TIMER_20MSEC,     5);    // timer for periodic actions 50 x per/sec
  Scheduler.setIntervalTimerOverride(SchedulerIntervalTimer_e::TIMER_100MSEC,    66);   // timer for periodic actions 10 x per/sec
  Scheduler.setIntervalTimerOverride(SchedulerIntervalTimer_e::TIMER_1SEC,       777);  // timer for periodic actions once per/sec
  Scheduler.setIntervalTimerOverride(SchedulerIntervalTimer_e::TIMER_30SEC,      1333); // timer for watchdog once per 30 sec
  Scheduler.setIntervalTimerOverride(SchedulerIntervalTimer_e::TIMER_MQTT,       88);   // timer for interaction with MQTT
  Scheduler.setIntervalTimerOverride(SchedulerIntervalTimer_e::TIMER_STATISTICS, 2222);
  #ifndef BUILD_NO_RAM_TRACKER
  logMemUsageAfter(F("Scheduler.setIntervalTimerOverride"));
  #endif // ifndef BUILD_NO_RAM_TRACKER
}
