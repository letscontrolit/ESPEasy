#include "../WebServer/DevicesPage.h"

#ifdef WEBSERVER_DEVICES

# include "../WebServer/ESPEasy_WebServer.h"
# include "../WebServer/HTML_wrappers.h"
# include "../WebServer/Markup.h"
# include "../WebServer/Markup_Buttons.h"
# include "../WebServer/Markup_Forms.h"

# include "../DataStructs/NodeStruct.h"

# include "../Globals/CPlugins.h"
# include "../Globals/Device.h"
# include "../Globals/ExtraTaskSettings.h"
# include "../Globals/Nodes.h"
# include "../Globals/Plugins.h"

# include "../Static/WebStaticData.h"

# include "../Helpers/_CPlugin_init.h"
# include "../Helpers/_Plugin_init.h"
# include "../Helpers/_Plugin_SensorTypeHelper.h"
# include "../Helpers/_Plugin_Helper_serial.h"
# include "../Helpers/ESPEasy_Storage.h"
# include "../Helpers/Hardware.h"
# include "../Helpers/I2C_Plugin_Helper.h"
# include "../Helpers/StringConverter.h"
# include "../Helpers/StringGenerator_GPIO.h"



# include "../../_Plugin_Helper.h"

# include <ESPeasySerial.h>


void handle_devices() {
  # ifndef BUILD_NO_RAM_TRACKER
  checkRAM(F("handle_devices"));
  # endif // ifndef BUILD_NO_RAM_TRACKER

  if (!isLoggedIn()) { return; }
  navMenuIndex = MENU_INDEX_DEVICES;
  TXBuffer.startStream();
  sendHeadandTail_stdtemplate(_HEAD);


  // char tmpString[41];


  // String taskindex = webArg(F("index"));

  pluginID_t taskdevicenumber;

  if (hasArg(F("del"))) {
    taskdevicenumber.setInvalid();
  }
  else {
    taskdevicenumber = pluginID_t::toPluginID(getFormItemInt(F("TDNUM"), 0));
  }


  // String taskdeviceid[CONTROLLER_MAX];
  // String taskdevicepin1 = webArg(F("taskdevicepin1"));   // "taskdevicepin*" should not be changed because it is uses by plugins
  // and expected to be saved by this code
  // String taskdevicepin2 = webArg(F("taskdevicepin2"));
  // String taskdevicepin3 = webArg(F("taskdevicepin3"));
  // String taskdevicepin1pullup = webArg(F("TDPPU"));
  // String taskdevicepin1inversed = webArg(F("TDPI"));
  // String taskdevicename = webArg(F("TDN"));
  // String taskdeviceport = webArg(F("TDP"));
  // String taskdeviceformula[VARS_PER_TASK];
  // String taskdevicevaluename[VARS_PER_TASK];
  // String taskdevicevaluedecimals[VARS_PER_TASK];
  // String taskdevicesenddata[CONTROLLER_MAX];
  // String taskdeviceglobalsync = webArg(F("TDGS"));
  // String taskdeviceenabled = webArg(F("TDE"));

  // for (uint8_t varNr = 0; varNr < VARS_PER_TASK; varNr++)
  // {
  //   char argc[25];
  //   String arg = F("TDF");
  //   arg += varNr + 1;
  //   arg.toCharArray(argc, 25);
  //   taskdeviceformula[varNr] = webArg(argc);
  //
  //   arg = F("TDVN");
  //   arg += varNr + 1;
  //   arg.toCharArray(argc, 25);
  //   taskdevicevaluename[varNr] = webArg(argc);
  //
  //   arg = F("TDVD");
  //   arg += varNr + 1;
  //   arg.toCharArray(argc, 25);
  //   taskdevicevaluedecimals[varNr] = webArg(argc);
  // }

  // for (controllerIndex_t controllerNr = 0; controllerNr < CONTROLLER_MAX; controllerNr++)
  // {
  //   char argc[25];
  //   String arg = F("TDID");
  //   arg += controllerNr + 1;
  //   arg.toCharArray(argc, 25);
  //   taskdeviceid[controllerNr] = webArg(argc);
  //
  //   arg = F("TDSD");
  //   arg += controllerNr + 1;
  //   arg.toCharArray(argc, 25);
  //   taskdevicesenddata[controllerNr] = webArg(argc);
  // }

  uint8_t page = getFormItemInt(F("page"), 0);

  if (page == 0) {
    page = 1;
  }
  uint8_t setpage = getFormItemInt(F("setpage"), 0);

  if (setpage > 0)
  {
    if (setpage <= (TASKS_MAX / TASKS_PER_PAGE)) {
      page = setpage;
    }
    else {
      page = TASKS_MAX / TASKS_PER_PAGE;
    }
  }
  const int edit = getFormItemInt(F("edit"), 0);

  // taskIndex in the URL is 1 ... TASKS_MAX
  // For use in other functions, set it to 0 ... (TASKS_MAX - 1)
  taskIndex_t taskIndex       = getFormItemInt(F("index"), 0);
  boolean     taskIndexNotSet = taskIndex == 0;

  const bool nosave = isFormItemChecked(F("nosave"));

  if (!taskIndexNotSet) {
    --taskIndex;
//    LoadTaskSettings(taskIndex); // Make sure ExtraTaskSettings are up-to-date
  }

  // FIXME TD-er: Might have to clear any caches here.
  if ((edit != 0) && !taskIndexNotSet) // when form submitted
  {
    if (Settings.getPluginID_for_task(taskIndex) != taskdevicenumber)
    {
      // change of device: cleanup old device and reset default settings
      setTaskDevice_to_TaskIndex(taskdevicenumber, taskIndex);
    }
    else if (taskdevicenumber != INVALID_PLUGIN_ID) // save settings
    {
      handle_devices_CopySubmittedSettings(taskIndex, taskdevicenumber);
    }

    if (taskdevicenumber != INVALID_PLUGIN_ID) {
      // Task index has a task device number, so it makes sense to save.
      // N.B. When calling delete, the settings were already saved.
      if (nosave) {
        Cache.updateExtraTaskSettingsCache();
      } else {
        addHtmlError(SaveTaskSettings(taskIndex));
        addHtmlError(SaveSettings());
      }

      struct EventStruct TempEvent(taskIndex);
      String dummy;

      if (Settings.TaskDeviceEnabled[taskIndex]) {
        if (PluginCall(PLUGIN_INIT, &TempEvent, dummy)) {
          PluginCall(PLUGIN_READ, &TempEvent, dummy);
        }
      } else {
        PluginCall(PLUGIN_EXIT, &TempEvent, dummy);
      }
    }
  }

  // show all tasks as table
  if (taskIndexNotSet)
  {
    handle_devicess_ShowAllTasksTable(page);
  }

  // Show edit form if a specific entry is chosen with the edit button
  else
  {
    handle_devices_TaskSettingsPage(taskIndex, page);
  }

  # ifndef BUILD_NO_RAM_TRACKER
  checkRAM(F("handle_devices"));
  # endif // ifndef BUILD_NO_RAM_TRACKER
# ifndef BUILD_NO_DEBUG

  if (loglevelActiveFor(LOG_LEVEL_DEBUG_DEV)) {
    addLogMove(LOG_LEVEL_DEBUG_DEV, concat(F("DEBUG: String size:"), static_cast<int>(TXBuffer.sentBytes)));
  }
# endif // ifndef BUILD_NO_DEBUG
  sendHeadandTail_stdtemplate(_TAIL);
  TXBuffer.endStream();
}

// ********************************************************************************
// Add a device select dropdown list
// TODO TD-er: Add JavaScript filter:
//             https://www.w3schools.com/howto/howto_js_filter_dropdown.asp
// ********************************************************************************
void addDeviceSelect(const __FlashStringHelper *name,  pluginID_t choice)
{
  String deviceName;

  addSelector_Head_reloadOnChange(name);
  addSelector_Item(F("- None -"), 0, false);

  deviceIndex_t x;
  bool done = false;
  while (!done) {
    const deviceIndex_t deviceIndex = getDeviceIndex_sorted(x);
    if (!validDeviceIndex(deviceIndex)) {
      done = true;
    } else {
      const pluginID_t pluginID = getPluginID_from_DeviceIndex(deviceIndex);

      if (validPluginID(pluginID)) {
        deviceName = getPluginNameFromDeviceIndex(deviceIndex);


        # if defined(PLUGIN_BUILD_DEV) || defined(PLUGIN_SET_MAX)
        deviceName = concat(get_formatted_Plugin_number(pluginID), F(" - ")) + deviceName;
        # endif // if defined(PLUGIN_BUILD_DEV) || defined(PLUGIN_SET_MAX)

        addSelector_Item(deviceName,
                          pluginID.value,
                          choice == pluginID);
      }
    }
    ++x;
  }

  addSelector_Foot();
}

// ********************************************************************************
// Collect all submitted form data and store the task settings
// ********************************************************************************
void handle_devices_CopySubmittedSettings(taskIndex_t taskIndex, pluginID_t taskdevicenumber)
{
  if (!validTaskIndex(taskIndex)) { return; }
  const deviceIndex_t DeviceIndex = getDeviceIndex(taskdevicenumber);

  if (!validDeviceIndex(DeviceIndex)) { return; }

  const DeviceStruct& device = Device[DeviceIndex];

  unsigned long taskdevicetimer = getFormItemInt(F("TDT"), 0);

  Settings.TaskDeviceNumber[taskIndex] = taskdevicenumber.value;

  if (device.Type == DEVICE_TYPE_I2C) {
    uint8_t flags = 0;
    bitWrite(flags, I2C_FLAGS_SLOW_SPEED, isFormItemChecked(F("taskdeviceflags0")));

# if FEATURE_I2CMULTIPLEXER

    if (isI2CMultiplexerEnabled()) {
      int multipleMuxPortsOption = getFormItemInt(F("taskdeviceflags1"), 0);
      bitWrite(flags, I2C_FLAGS_MUX_MULTICHANNEL, multipleMuxPortsOption == 1);

      if (multipleMuxPortsOption == 1) {
        uint8_t selectedPorts = 0;

        for (int x = 0; x < I2CMultiplexerMaxChannels(); ++x) {
          bitWrite(selectedPorts, x, isFormItemChecked(concat(F("taskdeviceflag1ch"), x)));
        }
        Settings.I2C_Multiplexer_Channel[taskIndex] = selectedPorts;
      } else {
        Settings.I2C_Multiplexer_Channel[taskIndex] = getFormItemInt(F("taskdevicei2cmuxport"), 0);
      }
    }

# endif // if FEATURE_I2CMULTIPLEXER

    Settings.I2C_Flags[taskIndex] = flags;
  }

  // Must load from file system to make sure all caches and checksums match.
  ExtraTaskSettings.clear();
  ExtraTaskSettings.TaskIndex = taskIndex;
  Cache.clearTaskCache(taskIndex);

  struct EventStruct TempEvent(taskIndex);

  // Save selected output type.
  switch (device.OutputDataType) {
    case Output_Data_type_t::Default:
    {
      String dummy;
      PluginCall(PLUGIN_GET_DEVICEVALUENAMES, &TempEvent, dummy);
      break;
    }
    case Output_Data_type_t::Simple:
    case Output_Data_type_t::All:
    {
      int pconfigIndex = checkDeviceVTypeForTask(&TempEvent);
      Sensor_VType VType = TempEvent.sensorType;

      if ((pconfigIndex >= 0) && (pconfigIndex < PLUGIN_CONFIGVAR_MAX)) {
        VType = static_cast<Sensor_VType>(getFormItemInt(PCONFIG_LABEL(pconfigIndex), 0));
        Settings.TaskDevicePluginConfig[taskIndex][pconfigIndex] = static_cast<int>(VType);
      }
      ExtraTaskSettings.clearUnusedValueNames(getValueCountFromSensorType(VType));
      break;
    }
  }

  {
    int pins[] = {-1, -1, -1};
    for (int i = 0; i < 3; ++i) {
      update_whenset_FormItemInt(concat(F("taskdevicepin"), i + 1), pins[i]);
    }

    bool taskEnabled = isFormItemChecked(F("TDE"));
    setBasicTaskValues(taskIndex, taskdevicetimer,
                      taskEnabled, webArg(F("TDN")),
                      pins);
  }

  #if FEATURE_PLUGIN_PRIORITY
  if (device.PowerManager // Check extra priority device flags when available
      ) {
    bool disablePrio = false;
    for (taskIndex_t t = 0; t < TASKS_MAX && !disablePrio; t++) {
      if (t != taskIndex) {
        disablePrio = Settings.isPriorityTask(t);
      }
    }
    bool statePriority = isFormItemChecked(F("TPRE"));
    if (device.PowerManager) {
      Settings.setPowerManagerTask(taskIndex, statePriority);
    }
    // Set alternative Priority flags
    // Set to readonly if set as Priority task
    Settings.setTaskEnableReadonly(taskIndex, statePriority);
  }
  #endif // if FEATURE_PLUGIN_PRIORITY
  Settings.TaskDevicePort[taskIndex] = getFormItemInt(F("TDP"), 0);
  update_whenset_FormItemInt(F("remoteFeed"), Settings.TaskDeviceDataFeed[taskIndex]);
  Settings.CombineTaskValues_SingleEvent(taskIndex, isFormItemChecked(F("TVSE")));

  for (controllerIndex_t controllerNr = 0; controllerNr < CONTROLLER_MAX; controllerNr++)
  {
    Settings.TaskDeviceID[controllerNr][taskIndex]       = getFormItemInt(getPluginCustomArgName(F("TDID"), controllerNr));
    Settings.TaskDeviceSendData[controllerNr][taskIndex] = isFormItemChecked(getPluginCustomArgName(F("TDSD"), controllerNr));
  }

  if (device.PullUpOption) {
    Settings.TaskDevicePin1PullUp[taskIndex] = isFormItemChecked(F("TDPPU"));
  }

  if (device.InverseLogicOption) {
    Settings.TaskDevicePin1Inversed[taskIndex] = isFormItemChecked(F("TDPI"));
  }

  if (device.isSerial())
  {
    # ifdef PLUGIN_USES_SERIAL
    serialHelper_webformSave(&TempEvent);
    # else // ifdef PLUGIN_USES_SERIAL
    addLog(LOG_LEVEL_ERROR, F("PLUGIN_USES_SERIAL not defined"));
    # endif // ifdef PLUGIN_USES_SERIAL
  }

  const uint8_t valueCount = getValueCountForTask(taskIndex);

  for (uint8_t varNr = 0; varNr < valueCount; varNr++)
  {
    strncpy_webserver_arg(ExtraTaskSettings.TaskDeviceFormula[varNr], getPluginCustomArgName(F("TDF"), varNr));
    update_whenset_FormItemInt(getPluginCustomArgName(F("TDVD"), varNr), ExtraTaskSettings.TaskDeviceValueDecimals[varNr]);
    strncpy_webserver_arg(ExtraTaskSettings.TaskDeviceValueNames[varNr], getPluginCustomArgName(F("TDVN"), varNr));
#if FEATURE_PLUGIN_FILTER
    ExtraTaskSettings.enablePluginFilter(varNr, isFormItemChecked(getPluginCustomArgName(F("TDFIL"), varNr)));
#endif
#if FEATURE_PLUGIN_STATS
    ExtraTaskSettings.enablePluginStats(varNr, isFormItemChecked(getPluginCustomArgName(F("TDS"), varNr)));
#endif
  }
  ExtraTaskSettings.clearUnusedValueNames(valueCount);

  // ExtraTaskSettings has changed.
  // The content of it is needed for sending CPLUGIN_TASK_CHANGE_NOTIFICATION and TaskInit/TaskExit events
  Cache.updateExtraTaskSettingsCache();

  // allow the plugin to save plugin-specific form settings.
  {
    String dummy;
    if (device.ExitTaskBeforeSave) {
      PluginCall(PLUGIN_EXIT, &TempEvent, dummy);
    }

    PluginCall(PLUGIN_WEBFORM_SAVE, &TempEvent, dummy);

    if (device.ErrorStateValues) {
      // FIXME TD-er: Must collect these from the web page.
      PluginCall(DeviceIndex, PLUGIN_INIT_VALUE_RANGES, &TempEvent, dummy);
    }

    // Make sure the task needs to reload using the new settings.
    if (!device.ExitTaskBeforeSave) {
      PluginCall(PLUGIN_EXIT, &TempEvent, dummy);
    }    
  }

  // Store all PCONFIG values on the web page
  // Must be done after PLUGIN_WEBFORM_SAVE, to allow tasks to clear the default task value names
  // Output type selectors are typically stored in PCONFIG 
  for (int pconfigIndex = 0; pconfigIndex < PLUGIN_CONFIGVAR_MAX; ++pconfigIndex) {
    pconfig_webformSave(&TempEvent, pconfigIndex);
  }
  // ExtraTaskSettings may have changed during PLUGIN_WEBFORM_SAVE, so again update the cache.
  Cache.updateExtraTaskSettingsCache();

  loadDefaultTaskValueNames_ifEmpty(taskIndex);
  Cache.updateExtraTaskSettingsCache();

  // notify controllers: CPlugin::Function::CPLUGIN_TASK_CHANGE_NOTIFICATION
  for (controllerIndex_t x = 0; x < CONTROLLER_MAX; x++)
  {
    TempEvent.ControllerIndex = x;

    if (Settings.TaskDeviceSendData[TempEvent.ControllerIndex][TempEvent.TaskIndex] &&
        Settings.ControllerEnabled[TempEvent.ControllerIndex] && Settings.Protocol[TempEvent.ControllerIndex])
    {
      protocolIndex_t ProtocolIndex = getProtocolIndex_from_ControllerIndex(TempEvent.ControllerIndex);
      String dummy;
      CPluginCall(ProtocolIndex, CPlugin::Function::CPLUGIN_TASK_CHANGE_NOTIFICATION, &TempEvent, dummy);
    }
  }
}


void html_add_setPage(uint8_t page, bool isLinkToPrev) {
  addHtml(F("devices?setpage="));
  addHtmlInt(page);
  addHtml(F("'>&"));
  addHtml(isLinkToPrev ? 'l' : 'g');
  addHtml(F("t;</a>"));
}

// ********************************************************************************
// Show table with all selected Tasks/Devices
// ********************************************************************************
void handle_devicess_ShowAllTasksTable(uint8_t page)
{
  serve_JS(JSfiles_e::UpdateSensorValuesDevicePage);
  html_table_class_multirow();
  html_TR();
  html_table_header(F(""), 70);

  if (TASKS_MAX != TASKS_PER_PAGE)
  {
    html_add_button_prefix();

    html_add_setPage((page > 1) ? page - 1 : page, true);
    html_add_button_prefix();
    html_add_setPage((page < (TASKS_MAX / TASKS_PER_PAGE)) ? page + 1 : page, false);
  }

  html_table_header(F("Task"),    50);
  html_table_header(F("Enabled"), 100);
  html_table_header(F("Device"));
  html_table_header(F("Name"));
  html_table_header(F("Port"));
  html_table_header(F("Ctr (IDX)"), 100);
  html_table_header(F("GPIO"));
  html_table_header(F("Values"));

  String deviceName;

  for (taskIndex_t x = (page - 1) * TASKS_PER_PAGE; x < ((page) * TASKS_PER_PAGE) && validTaskIndex(x); x++)
  {
    const deviceIndex_t DeviceIndex = getDeviceIndex_from_TaskIndex(x);
    const bool pluginID_set         = INVALID_PLUGIN_ID != Settings.getPluginID_for_task(x);

    html_TR_TD();

    if (pluginID_set && !supportedPluginID(Settings.getPluginID_for_task(x))) {
      html_add_button_prefix(F("red"), true);
    } else {
      html_add_button_prefix();
    }
    {
      addHtml(concat(F("devices?index="), static_cast<int>(x + 1)));
      addHtml(concat(F("&page="), static_cast<int>(page)));
      addHtml('\'', '>');
      addHtml(pluginID_set ? F("Edit") : F("Add"));
      addHtml(concat(F("</a><TD>"), static_cast<int>(x + 1)));
      html_TD();
    }

    // Show table of all configured tasks
    // A task may also refer to a non supported plugin.
    // This will be shown as not supported.
    // Editing a task which has a non supported plugin will present the same as when assigning a new plugin to a task.
    if (pluginID_set)
    {
      //LoadTaskSettings(x);
      int8_t spi_gpios[3] { -1, -1, -1 };
      struct EventStruct TempEvent(x);
      addEnabled(Settings.TaskDeviceEnabled[x]  && validDeviceIndex(DeviceIndex));

      html_TD();
      addHtml(getPluginNameFromPluginID(Settings.getPluginID_for_task(x)));
      html_TD();
      addHtml(getTaskDeviceName(x));
      html_TD();

      if (validDeviceIndex(DeviceIndex)) {
        if (Settings.TaskDeviceDataFeed[x] != 0) {
          #if FEATURE_ESPEASY_P2P
          // Show originating node number
          const uint8_t remoteUnit = Settings.TaskDeviceDataFeed[x];
          format_originating_node(remoteUnit);
          #endif
        } else {
          String portDescr;

          if (PluginCall(PLUGIN_WEBFORM_SHOW_CONFIG, &TempEvent, portDescr)) {
            addHtml(portDescr);
          } else {
            const DeviceStruct& device = Device[DeviceIndex];
            if (device.Type == DEVICE_TYPE_I2C) {
              format_I2C_port_description(x);
            } else if (device.isSPI()) {
              format_SPI_port_description(spi_gpios);
            } else if (device.isSerial()) {
                # ifdef PLUGIN_USES_SERIAL
                addHtml(serialHelper_getSerialTypeLabel(&TempEvent));
                # else // ifdef PLUGIN_USES_SERIAL
                addHtml(F("PLUGIN_USES_SERIAL not defined"));
                # endif // ifdef PLUGIN_USES_SERIAL
            } else {
                // Plugin has no custom port formatting, show default one.
                if (device.Ports != 0)
                {
                  addHtml(formatToHex_decimal(Settings.TaskDevicePort[x]));
                }
            }
          }
        }
      }

      html_TD();

      if (validDeviceIndex(DeviceIndex)) {
        if (Device[DeviceIndex].SendDataOption)
        {
          boolean doBR = false;

          for (controllerIndex_t controllerNr = 0; controllerNr < CONTROLLER_MAX; controllerNr++)
          {
            if (Settings.TaskDeviceSendData[controllerNr][x])
            {
              if (doBR) {
                html_BR();
              }
              addHtml(getControllerSymbol(controllerNr));
              protocolIndex_t ProtocolIndex = getProtocolIndex_from_ControllerIndex(controllerNr);

              if (validProtocolIndex(ProtocolIndex)) {
                if (getProtocolStruct(ProtocolIndex).usesID && (Settings.Protocol[controllerNr] != 0))
                {
                  addHtml(concat(F(" ("), static_cast<int>(Settings.TaskDeviceID[controllerNr][x])));
                  addHtml(')');

                  if (Settings.TaskDeviceID[controllerNr][x] == 0) {
                    addHtml(' ');
                    addHtml(F(HTML_SYMBOL_WARNING));
                  }
                }
                doBR = true;
              }
            }
          }
        }
      }

      html_TD();

      if (validDeviceIndex(DeviceIndex)) {
        const DeviceStruct& device = Device[DeviceIndex];
        if (Settings.TaskDeviceDataFeed[x] == 0)
        {
          String description;
          bool pluginHasGPIODescription = pluginWebformShowGPIOdescription(x, F("<BR>"), description);

          bool showpin1 = false;
          bool showpin2 = false;
          bool showpin3 = false;

          switch (device.Type) {
            case DEVICE_TYPE_I2C:
            {
              format_I2C_pin_description(x);
              html_BR();
              break;
            }
            case DEVICE_TYPE_SPI3:
              showpin3 = !pluginHasGPIODescription;

            // Fall Through
            case DEVICE_TYPE_SPI2:
              showpin2 = !pluginHasGPIODescription;

            // Fall Through
            case DEVICE_TYPE_SPI:
              format_SPI_pin_description(spi_gpios, x, !pluginHasGPIODescription);
              break;
            case DEVICE_TYPE_ANALOG:
            {
              # ifdef ESP8266
              #  if FEATURE_ADC_VCC
              addHtml(F("ADC (VCC)"));
              #  else // if FEATURE_ADC_VCC
              addHtml(F("ADC (TOUT)"));
              #  endif // if FEATURE_ADC_VCC
              # endif // ifdef ESP8266
              # ifdef ESP32
              showpin1 = true;
              addHtml(formatGpioName_ADC(Settings.TaskDevicePin1[x]));
              html_BR();
              # endif // ifdef ESP32

              break;
            }
            case DEVICE_TYPE_SERIAL_PLUS1:
              showpin3 = true;

            // fallthrough
            case DEVICE_TYPE_SERIAL:
            {
              # ifdef PLUGIN_USES_SERIAL
              const String serialDescription = serialHelper_getGpioDescription(static_cast<ESPEasySerialPort>(Settings.TaskDevicePort[x]), Settings.TaskDevicePin1[x],
                                                      Settings.TaskDevicePin2[x], F("<BR>"));
              addHtml(serialDescription);
              # else // ifdef PLUGIN_USES_SERIAL
              addHtml(F("PLUGIN_USES_SERIAL not defined"));
              # endif // ifdef PLUGIN_USES_SERIAL

              if (
#ifdef PLUGIN_USES_SERIAL
                serialDescription.length() || 
#endif
                showpin3) {
                html_BR();
              }
              break;
            }
            case DEVICE_TYPE_CUSTOM3:
              showpin3 = true;

            // fallthrough
            case DEVICE_TYPE_CUSTOM2:
              showpin2 = true;

            // fallthrough
            case DEVICE_TYPE_CUSTOM1:
            case DEVICE_TYPE_CUSTOM0:
            {
              showpin1 = true;
              if (pluginHasGPIODescription || (device.Type == DEVICE_TYPE_CUSTOM0)) {
                addHtml(description);
                showpin1 = false;
                showpin2 = false;
                showpin3 = false;
              }
              break;
            }

            default:
              showpin1 = true;
              showpin2 = true;
              showpin3 = true;
              break;
          }

          if (showpin1)
          {
            addGpioHtml(Settings.getTaskDevicePin(x, 1));
          }

          if (showpin2)
          {
            html_BR();
            addGpioHtml(Settings.getTaskDevicePin(x, 2));
          }

          if (showpin3)
          {
            html_BR();
            addGpioHtml(Settings.getTaskDevicePin(x, 3));
          }

          // Allow for tasks to show their own specific GPIO pins.
          if (!device.isCustom() &&
              pluginHasGPIODescription) {
            if (showpin1 || showpin2 || showpin3) {
              html_BR();
            }
            addHtml(description);
          }
        }
      }

      html_TD();

      if (validDeviceIndex(DeviceIndex)) {
        String customValuesString;
        const bool customValues = PluginCall(PLUGIN_WEBFORM_SHOW_VALUES, &TempEvent, customValuesString);

        if (!customValues)
        {
          const uint8_t valueCount = getValueCountForTask(x);

          for (uint8_t varNr = 0; varNr < valueCount; varNr++)
          {
            if (validPluginID_fullcheck(Settings.getPluginID_for_task(x)))
            {
              pluginWebformShowValue(x, varNr, getTaskValueName(x, varNr), formatUserVarNoCheck(x, varNr));
            }
          }
        }
      }
    }
    else {
      html_TD(6);
    }
  } // next
  html_end_table();
  html_end_form();
}

#if FEATURE_ESPEASY_P2P
void format_originating_node(uint8_t remoteUnit) {
  addHtml(F("Unit "));
  addHtmlInt(remoteUnit);

  if (remoteUnit != 255) {
    const NodeStruct *node = Nodes.getNode(remoteUnit);

    if (node != nullptr) {
      addHtml(F(" - "));
      addHtml(node->getNodeName());
    } else {
      addHtml(F(" - Not Seen recently"));
    }
  }
}
#endif

void format_I2C_port_description(taskIndex_t x)
{
  addHtml(F("I2C"));
  # if FEATURE_I2C_GET_ADDRESS
  const uint8_t i2cAddr = getTaskI2CAddress(x);
  if (i2cAddr > 0) {
    addHtml(' ');
    addHtml(formatToHex(i2cAddr, 2));
  }
  # endif // if FEATURE_I2C_GET_ADDRESS
  # if FEATURE_I2CMULTIPLEXER

  if (isI2CMultiplexerEnabled() && I2CMultiplexerPortSelectedForTask(x)) {
    String mux;

    if (bitRead(Settings.I2C_Flags[x], I2C_FLAGS_MUX_MULTICHANNEL)) { // Multi-channel
      mux = F("<BR>Multiplexer channel(s)");
      uint8_t b = 0;                                                  // For adding lineBreaks

      for (uint8_t c = 0; c < I2CMultiplexerMaxChannels(); c++) {
        if (bitRead(Settings.I2C_Multiplexer_Channel[x], c)) {
          mux += b == 0 ? F("<BR>") : F(", ");
          b++;
          mux += String(c);
        }
      }
    } else { // Single channel
      mux  = concat(F("<BR>Multiplexer channel "), static_cast<int>(Settings.I2C_Multiplexer_Channel[x]));
    }
    addHtml(mux);
  }
  # endif // if FEATURE_I2CMULTIPLEXER
}

void format_SPI_port_description(int8_t spi_gpios[3])
{
  if (!Settings.getSPI_pins(spi_gpios)) {
    addHtml(F("SPI (Not enabled)"));
    return;
  }
  # ifdef ESP32
  addHtml(getSPI_optionToShortString(static_cast<SPI_Options_e>(Settings.InitSPI)));
  # endif // ifdef ESP32
  # ifdef ESP8266
  addHtml(F("SPI"));
  # endif // ifdef ESP8266
}

void format_I2C_pin_description(taskIndex_t x)
{
  if (checkI2CConfigValid_toHtml(x)) {
    Label_Gpio_toHtml(F("SDA"), formatGpioLabel(Settings.Pin_i2c_sda, false));
    html_BR();
    Label_Gpio_toHtml(F("SCL"), formatGpioLabel(Settings.Pin_i2c_scl, false));
  }
}

void format_SPI_pin_description(int8_t spi_gpios[3], taskIndex_t x, bool showCSpin)
{
  if (Settings.InitSPI > static_cast<int>(SPI_Options_e::None)) {
    const __FlashStringHelper* labels[] = { F("CLK"), F("MISO"), F("MOSI") };
    for (int i = 0; i < 3; ++i) {
      if (i != 0)       
        html_BR();

      Label_Gpio_toHtml(labels[i], formatGpioLabel(spi_gpios[i], false));
    }
    if (showCSpin) {
      html_BR();
      Label_Gpio_toHtml(F("CS"), formatGpioLabel(Settings.TaskDevicePin1[x], false));
    }
  }
}

// ********************************************************************************
// Show the task settings page
// ********************************************************************************
void handle_devices_TaskSettingsPage(taskIndex_t taskIndex, uint8_t page)
{
  if (!validTaskIndex(taskIndex)) { return; }

  const deviceIndex_t DeviceIndex = getDeviceIndex_from_TaskIndex(taskIndex);

  //LoadTaskSettings(taskIndex);

  html_add_form();
  html_table_class_normal();
  addFormHeader(F("Task Settings"));


  addHtml(F("<TR><TD style='width:150px;' align='left'>Device:<TD>"));

  // no (supported) device selected, this effectively checks for validDeviceIndex
  if (!supportedPluginID(Settings.getPluginID_for_task(taskIndex)))
  {
    // takes lots of memory/time so call this only when needed.
    addDeviceSelect(F("TDNUM"), Settings.getPluginID_for_task(taskIndex)); // ="taskdevicenumber"
    addFormSeparator(4);
  }

  // device selected
  else
  {
    const DeviceStruct& device = Device[DeviceIndex];
    // remember selected device number
    addHtml(F("<input "));
    addHtmlAttribute(F("type"),  F("hidden"));
    addHtmlAttribute(F("name"),  F("TDNUM"));
    addHtmlAttribute(F("value"), Settings.getPluginID_for_task(taskIndex).value);
    addHtml('>');

    // show selected device name and delete button
    addHtml(getPluginNameFromDeviceIndex(DeviceIndex));

    addHelpButton(concat(F("Plugin"), Settings.getPluginID_for_task(taskIndex).value));
    addRTDPluginButton(Settings.getPluginID_for_task(taskIndex));

    addFormTextBox(F("Name"), F("TDN"), getTaskDeviceName(taskIndex), NAME_FORMULA_LENGTH_MAX); // ="taskdevicename"

    addFormCheckBox(F("Enabled"), F("TDE"), 
      Settings.TaskDeviceEnabled[taskIndex], 
//    Settings.TaskDeviceEnabled[taskIndex].enabled, 
      Settings.isTaskEnableReadonly(taskIndex)); // ="taskdeviceenabled"

    #if FEATURE_PLUGIN_PRIORITY
    if (device.PowerManager) { // Check extra priority device flags when available
      bool disablePrio = !Settings.TaskDeviceEnabled[taskIndex];
      for (taskIndex_t t = 0; t < TASKS_MAX && !disablePrio; t++) {
        if (t != taskIndex) { // Ignore current device
          if (device.PowerManager && Settings.isPowerManagerTask(t)) {
            disablePrio = true; // Allow only a single PowerManager plugin
          }
          // Add other Priority options checks
        }
      }
      addFormSubHeader(F("Priority task"));
      addFormCheckBox(F("Priority task"), F("TPRE"), Settings.isPriorityTask(taskIndex), disablePrio); // ="taskpriorityenabled"
      if (!disablePrio) {
        addFormNote(F("After enabling a Priority task, a reboot is required to activate. See documentation."));
      }
    }
    #endif // if FEATURE_PLUGIN_PRIORITY

    bool addPinConfig = false;

    // section: Sensor / Actuator
    if (!device.Custom && (Settings.TaskDeviceDataFeed[taskIndex] == 0) &&
        ((device.Ports != 0) ||
         (device.PullUpOption) ||
         (device.InverseLogicOption) ||
         (device.connectedToGPIOpins())))
    {
      addFormSubHeader((device.SendDataOption) ? F("Sensor") : F("Actuator"));

      if (device.Ports != 0) {
        addFormNumericBox(F("Port"), F("TDP"), Settings.TaskDevicePort[taskIndex]); // ="taskdeviceport"
      }

      addPinConfig = true;
    }

    if (addPinConfig || (device.Type == DEVICE_TYPE_I2C)) {
      if (device.isSerial()) {
          # ifdef PLUGIN_USES_SERIAL
          devicePage_show_serial_config(taskIndex);
          # else // ifdef PLUGIN_USES_SERIAL
          addHtml(F("PLUGIN_USES_SERIAL not defined"));
          # endif // ifdef PLUGIN_USES_SERIAL

          devicePage_show_pin_config(taskIndex, DeviceIndex);
          addPinConfig = false;

          html_add_script(F("document.getElementById('serPort').onchange();"), false);
      } else if (device.Type == DEVICE_TYPE_I2C) {
          devicePage_show_pin_config(taskIndex, DeviceIndex);
          addPinConfig = false;

          if (Settings.TaskDeviceDataFeed[taskIndex] == 0) {
            devicePage_show_I2C_config(taskIndex);
          }
      }

      if (addPinConfig) {
        devicePage_show_pin_config(taskIndex, DeviceIndex);
      }
    }
    if (DEVICE_TYPE_DUMMY != device.Type) {
      addFormSubHeader(F("Device Settings"));
    }

    String webformLoadString;
    struct EventStruct TempEvent(taskIndex);
    // add plugins content
    if (Settings.TaskDeviceDataFeed[taskIndex] == 0) { // only show additional config for local connected sensors
      PluginCall(PLUGIN_WEBFORM_LOAD, &TempEvent, webformLoadString);
      #ifndef BUILD_NO_DEBUG
      if (webformLoadString.length() > 0) {
        String errorMessage;
        PluginCall(PLUGIN_GET_DEVICENAME, &TempEvent, errorMessage);
        errorMessage += F(": Bug in PLUGIN_WEBFORM_LOAD, should not append to string, use addHtml() instead");
        addHtmlError(errorMessage);
      }
      #endif

      PluginCall(PLUGIN_WEBFORM_LOAD_ALWAYS, &TempEvent, webformLoadString); // Load settings also useful for remote-datafeed devices
    }
    else {
      #if FEATURE_ESPEASY_P2P
      // Show remote feed information.
      addFormSubHeader(F("Data Source"));
      const uint8_t remoteUnit = Settings.TaskDeviceDataFeed[taskIndex];
      addFormNumericBox(F("Remote Unit"), F("RemoteUnit"), remoteUnit, 0, 255);

      if (remoteUnit != 255) {
        const NodeStruct* node = Nodes.getNode(remoteUnit);

        if (node != nullptr) {
          addUnit(node->getNodeName());
        } else {
          addUnit(F("Unknown Unit Name"));
        }
      }
      addFormNote(F("0 = disable remote feed, 255 = broadcast")); // FIXME TD-er: Must verify if broadcast can be set.
      #endif

      PluginCall(PLUGIN_WEBFORM_LOAD_ALWAYS, &TempEvent, webformLoadString); // Load settings also useful for remote-datafeed devices
    }

    devicePage_show_output_data_type(taskIndex, DeviceIndex);

    #if FEATURE_PLUGIN_STATS
    // Task statistics and historic data in a chart
    devicePage_show_task_statistics(taskIndex, DeviceIndex);
    #endif // if FEATURE_PLUGIN_STATS

    // section: Data Acquisition
    devicePage_show_controller_config(taskIndex, DeviceIndex);

    addFormSeparator(2);

    devicePage_show_interval_config(taskIndex, DeviceIndex);

    devicePage_show_task_values(taskIndex, DeviceIndex);
  }

  html_TR_TD();
  addHtml(F("<TD colspan='3'>"));
  html_add_button_prefix();
  addHtml(F("devices?setpage="));
  addHtmlInt(page);
  addHtml(F("'>Close</a>"));
  #if FEATURE_PLUGIN_PRIORITY
  if (!Settings.isPriorityTask(taskIndex))
  #endif // if FEATURE_PLUGIN_PRIORITY
  {
    addSubmitButton();
  }
  addHtml(F("<input type='hidden' name='edit' value='1'>"));
  addHtml(F("<input type='hidden' name='page' value='1'>"));

  // if user selected a device, add the delete button, except for Priority tasks
  if (validPluginID_fullcheck(Settings.getPluginID_for_task(taskIndex))
      #if FEATURE_PLUGIN_PRIORITY
      && !Settings.isPriorityTask(taskIndex)
      #endif // if FEATURE_PLUGIN_PRIORITY
     ) {
    addSubmitButton(F("Delete"), F("del"));
  }

  html_end_table();
  #if FEATURE_PLUGIN_PRIORITY
  if (Settings.isPriorityTask(taskIndex)) {
    addFormNote(F("A Priority task can't be updated or deleted. See documentation."));
  }
  #endif // if FEATURE_PLUGIN_PRIORITY
  html_end_form();
  serve_JS(JSfiles_e::SplitPasteInput);
}

void devicePage_show_pin_config(taskIndex_t taskIndex, deviceIndex_t DeviceIndex)
{
  const DeviceStruct &device = Device[DeviceIndex];
  if (device.PullUpOption)
  {
    addFormCheckBox(F("Internal PullUp"), F("TDPPU"), Settings.TaskDevicePin1PullUp[taskIndex]); // ="taskdevicepin1pullup"
      # if defined(ESP8266)

    if ((Settings.TaskDevicePin1[taskIndex] == 16) || (Settings.TaskDevicePin2[taskIndex] == 16) ||
        (Settings.TaskDevicePin3[taskIndex] == 16)) {
      addFormNote(F("PullDown for GPIO-16 (D0)"));
    }
      # endif // if defined(ESP8266)
  }

  if (device.InverseLogicOption)
  {
    addFormCheckBox(F("Inversed Logic"), F("TDPI"), Settings.TaskDevicePin1Inversed[taskIndex]); // ="taskdevicepin1inversed"
    addFormNote(F("Will go into effect on next input change."));
  }

  if (device.isSPI()
      && (Settings.InitSPI == static_cast<int>(SPI_Options_e::None))) {
    addFormNote(F("SPI Interface is not configured yet (Hardware page)."));
  }

  if (device.connectedToGPIOpins()) {
    // get descriptive GPIO-names from plugin
    struct EventStruct TempEvent(taskIndex);

    TempEvent.String1 = F("1st GPIO");
    TempEvent.String2 = F("2nd GPIO");
    TempEvent.String3 = F("3rd GPIO");
    String dummy;
    PluginCall(PLUGIN_GET_DEVICEGPIONAMES, &TempEvent, dummy);

    if (device.usesTaskDevicePin(1)) {
      PinSelectPurpose purpose = PinSelectPurpose::Generic;

      if (device.isSerial())
      {
        // Pin1 = GPIO <--- TX
        purpose = PinSelectPurpose::Serial_input;
      } else if (device.isSPI())
      {
        // All selectable SPI pins are output only
        purpose = PinSelectPurpose::Generic_output;
      }

      addFormPinSelect(purpose, TempEvent.String1, F("taskdevicepin1"), Settings.TaskDevicePin1[taskIndex]);
    }

    if (device.usesTaskDevicePin(2)) {
      PinSelectPurpose purpose = PinSelectPurpose::Generic;

      if (device.isSerial())
      {
        // Serial Pin2 = GPIO ---> RX
        purpose = PinSelectPurpose::Serial_output;
      }
      if (device.isSPI())
      {
        // SPI only needs output pins
        purpose = PinSelectPurpose::Generic_output;
      }
      addFormPinSelect(purpose, TempEvent.String2, F("taskdevicepin2"), Settings.TaskDevicePin2[taskIndex]);
    }

    if (device.usesTaskDevicePin(3)) {
      PinSelectPurpose purpose = PinSelectPurpose::Generic;

      if (device.isSPI())
      {
        // SPI only needs output pins
        purpose = PinSelectPurpose::Generic_output;
      }
      addFormPinSelect(purpose, TempEvent.String3, F("taskdevicepin3"), Settings.TaskDevicePin3[taskIndex]);
    }
  }
}

#ifdef PLUGIN_USES_SERIAL
void devicePage_show_serial_config(taskIndex_t taskIndex)
{
  struct EventStruct TempEvent(taskIndex);

  serialHelper_webformLoad(&TempEvent);
  String webformLoadString;

  PluginCall(PLUGIN_WEBFORM_SHOW_SERIAL_PARAMS, &TempEvent, webformLoadString);
}
#endif

void devicePage_show_I2C_config(taskIndex_t taskIndex)
{
  struct EventStruct TempEvent(taskIndex);

  addFormSubHeader(F("I2C options"));

  if (!Settings.isI2CEnabled()) {
    addFormNote(F("I2C Interface is not configured yet (Hardware page)."));
  }

  String dummy;

  PluginCall(PLUGIN_WEBFORM_SHOW_I2C_PARAMS, &TempEvent, dummy);
  addFormCheckBox(F("Force Slow I2C speed"), F("taskdeviceflags0"), bitRead(Settings.I2C_Flags[taskIndex], I2C_FLAGS_SLOW_SPEED));

  # if FEATURE_I2CMULTIPLEXER

  // Show selector for an I2C multiplexer port if a multiplexer is configured
  if (isI2CMultiplexerEnabled()) {
    bool multipleMuxPorts = bitRead(Settings.I2C_Flags[taskIndex], I2C_FLAGS_MUX_MULTICHANNEL);
    {
      const __FlashStringHelper *i2c_mux_channels[] = {
        F("Single channel"),
        F("Multiple channels")};
      constexpr int i2c_mux_channelOptions[] = { 0, 1};
      int i2c_mux_channelCount = 1;

      if (Settings.I2C_Multiplexer_Type == I2C_MULTIPLEXER_PCA9540) {
        multipleMuxPorts = false; // force off
      } else {
        i2c_mux_channelCount++;
      }
      addFormSelector(F("Multiplexer channels"),
                      F("taskdeviceflags1"),
                      i2c_mux_channelCount,
                      i2c_mux_channels,
                      i2c_mux_channelOptions,
                      multipleMuxPorts ? 1 : 0,
                      true);
    }

    if (multipleMuxPorts) {
      addRowLabel(F("Select connections"), EMPTY_STRING);
      html_table(EMPTY_STRING, false); // Sub-table
      html_table_header(F("Channel"), 100);
      html_table_header(F("Enable"),  80);
      html_table_header(F("Channel"), 100);
      html_table_header(F("Enable"),  80);

      for (int x = 0; x < I2CMultiplexerMaxChannels(); x++) {
        if (x % 2 == 0) { html_TR(); } // Start a new row for every 2 channels
        html_TD();
        addHtml(concat(F("Channel "), x));
        html_TD();
        addCheckBox(concat(F("taskdeviceflag1ch"), x), bitRead(Settings.I2C_Multiplexer_Channel[taskIndex], x), false);
      }
      html_end_table();
    } else {
      int taskDeviceI2CMuxPort = Settings.I2C_Multiplexer_Channel[taskIndex];
      String  i2c_mux_portoptions[9];
      int     i2c_mux_portchoices[9];
      uint8_t mux_opt = 0;
      i2c_mux_portoptions[mux_opt] = F("(Not connected via multiplexer)");
      i2c_mux_portchoices[mux_opt] = -1;
      uint8_t mux_max = I2CMultiplexerMaxChannels();

      for (int x = 0; x < mux_max; x++) {
        mux_opt++;
        i2c_mux_portoptions[mux_opt] = concat(F("Channel "), x);
        i2c_mux_portchoices[mux_opt] = x;
      }

      if (taskDeviceI2CMuxPort >= mux_max) { taskDeviceI2CMuxPort = -1; } // Reset if out of range
      addFormSelector(F("Connected to"),
                      F("taskdevicei2cmuxport"),
                      mux_opt + 1,
                      i2c_mux_portoptions,
                      i2c_mux_portchoices,
                      taskDeviceI2CMuxPort);
    }
  }
  # endif // if FEATURE_I2CMULTIPLEXER
}

void devicePage_show_output_data_type(taskIndex_t taskIndex, deviceIndex_t DeviceIndex)
{
  struct EventStruct TempEvent(taskIndex);
  int pconfigIndex = checkDeviceVTypeForTask(&TempEvent);

  switch (Device[DeviceIndex].OutputDataType) {
    case Output_Data_type_t::Default:
      return;
    case Output_Data_type_t::Simple:

      if (pconfigIndex >= 0) {
        sensorTypeHelper_webformLoad_simple(&TempEvent, pconfigIndex);
        return;
      }
      break;
    case Output_Data_type_t::All:
    {
      if (pconfigIndex >= 0) {
        sensorTypeHelper_webformLoad_allTypes(&TempEvent, pconfigIndex);
        return;
      }
      break;
    }
  }
  addFormSubHeader(F("Output Configuration"));
  String dummy;
  PluginCall(PLUGIN_WEBFORM_LOAD_OUTPUT_SELECTOR, &TempEvent, dummy);
}

#if FEATURE_PLUGIN_STATS
void devicePage_show_task_statistics(taskIndex_t taskIndex, deviceIndex_t DeviceIndex)
{
  if (Device[DeviceIndex].PluginStats)
  {
    PluginTaskData_base *taskData = getPluginTaskDataBaseClassOnly(taskIndex);

    if (taskData != nullptr) {
      if (taskData->hasPluginStats()) {
        addFormSubHeader(F("Statistics"));
      }
      #if FEATURE_CHART_JS
      if (taskData->nrSamplesPresent() > 0) {
        addRowLabel(F("Historic data"));
        taskData->plot_ChartJS();
      }
      #endif // if FEATURE_CHART_JS

      struct EventStruct TempEvent(taskIndex);
      String dummy;
      bool   somethingAdded = false;

      if (!PluginCall(PLUGIN_WEBFORM_LOAD_SHOW_STATS, &TempEvent, dummy)) {
        somethingAdded = taskData->webformLoad_show_stats(&TempEvent);
      } else { somethingAdded = true; }

      if (somethingAdded) {
        if (taskData->hasPeaks()) {
          addFormNote(strformat(
            F("Peak values recorded since last \"%s.resetpeaks\"."),
            getTaskDeviceName(taskIndex).c_str()));
        }
      }
    }
  }
}
#endif // if FEATURE_PLUGIN_STATS



void devicePage_show_controller_config(taskIndex_t taskIndex, deviceIndex_t DeviceIndex)
{
  if (!validDeviceIndex(DeviceIndex)) return;

  const DeviceStruct& device = Device[DeviceIndex];

  if (device.SendDataOption)
  {
    addFormSubHeader(F("Data Acquisition"));

    if (device.ErrorStateValues) {
      struct EventStruct TempEvent(taskIndex);
      String dummy;

      PluginCall(PLUGIN_WEBFORM_SHOW_ERRORSTATE_OPT, &TempEvent, dummy); // Show extra settings for Error State Value options
    }

    addRowLabel(F("Single event with all values"));
    addCheckBox(F("TVSE"), Settings.CombineTaskValues_SingleEvent(taskIndex));
    addFormNote(strformat(
      F("Unchecked: Send event per value. Checked: Send single event (%s#All) containing all values"),
      getTaskDeviceName(taskIndex).c_str()));

    bool separatorAdded = false;
    for (controllerIndex_t controllerNr = 0; controllerNr < CONTROLLER_MAX; controllerNr++)
    {
      if (Settings.Protocol[controllerNr] != 0)
      {
        if (!separatorAdded) {
          addFormSeparator(2);
        }
        separatorAdded = true;
        html_TR_TD();
        addHtml(F("Send to Controller "));
        addHtml(getControllerSymbol(controllerNr));
        addHtmlDiv(F("note"), wrap_braces(getCPluginNameFromCPluginID(Settings.Protocol[controllerNr])));
        html_TD();

        addHtml(F("<table style='padding-left:0;'>")); // remove left padding 2x to align vertically with other inputs
        html_TD(F("width:50px;padding-left:0"));
        addCheckBox(
          getPluginCustomArgName(F("TDSD"), controllerNr), // ="taskdevicesenddata"
          Settings.TaskDeviceSendData[controllerNr][taskIndex]);

        protocolIndex_t ProtocolIndex = getProtocolIndex_from_ControllerIndex(controllerNr);

        if (validProtocolIndex(ProtocolIndex) && 
            getProtocolStruct(ProtocolIndex).usesID && (Settings.Protocol[controllerNr] != 0)) {
          html_TD();
          addHtml(F("IDX:"));
          html_TD();
          addNumericBox(
            getPluginCustomArgName(F("TDID"), controllerNr), // ="taskdeviceid"
            Settings.TaskDeviceID[controllerNr][taskIndex], 0, DOMOTICZ_MAX_IDX);
        }
        html_end_table();
      }
    }
  }
}

void devicePage_show_interval_config(taskIndex_t taskIndex, deviceIndex_t DeviceIndex)
{
  if (!validDeviceIndex(DeviceIndex)) return;

  const DeviceStruct& device = Device[DeviceIndex];

  if (device.TimerOption)
  {
    // FIXME: shoudn't the max be ULONG_MAX because Settings.TaskDeviceTimer is an unsigned long? addFormNumericBox only supports ints
    // for min and max specification
    addFormNumericBox(F("Interval"), F("TDT"), Settings.TaskDeviceTimer[taskIndex], 0, 65535); // ="taskdevicetimer"
    addUnit(F("sec"));

    if (device.TimerOptional) {
      addHtml(F(" (Optional for this Device)"));
    }
  }
}

void devicePage_show_task_values(taskIndex_t taskIndex, deviceIndex_t DeviceIndex)
{
  if (!validDeviceIndex(DeviceIndex)) return;
  // section: Values
  const uint8_t valueCount = getValueCountForTask(taskIndex);

  const DeviceStruct& device = Device[DeviceIndex];

  if (!device.Custom && (valueCount > 0))
  {
    int colCount = 2;
    addFormSubHeader(F("Values"));
    html_end_table();
    html_table_class_normal();

    // table header
    addHtml(F("<TR><TH style='width:30px;' align='center'>#"));
    html_table_header(F("Name"),500);

    if (device.FormulaOption)
    {
      html_table_header(F("Formula"), F("EasyFormula"), 500);
      ++colCount;
    }

#if FEATURE_PLUGIN_STATS
    if (device.PluginStats)
    {
      html_table_header(F("Stats"), 30);
      ++colCount;
    }
#endif

    if (device.configurableDecimals())
    {
      html_table_header(F("Decimals"), 30);
      ++colCount;
    }

    //placeholder header
    html_table_header(F(""));
    ++colCount;
    
    // table body
    for (uint8_t varNr = 0; varNr < valueCount; varNr++)
    {
      html_TR_TD();
      addHtmlInt(varNr + 1);
      html_TD();
      {
        const String id = getPluginCustomArgName(F("TDVN"), varNr); // ="taskdevicevaluename"
        addTextBox(id, Cache.getTaskDeviceValueName(taskIndex, varNr), NAME_FORMULA_LENGTH_MAX);
      }

      if (device.FormulaOption)
      {
        html_TD();
        const String id = getPluginCustomArgName(F("TDF"), varNr); // ="taskdeviceformula"
        addTextBox(id, Cache.getTaskDeviceFormula(taskIndex, varNr), NAME_FORMULA_LENGTH_MAX);
      }

#if FEATURE_PLUGIN_STATS
      if (device.PluginStats)
      {
        html_TD();
        const String id = getPluginCustomArgName(F("TDS"), varNr); // ="taskdevicestats"
        addCheckBox(id, Cache.enabledPluginStats(taskIndex, varNr));
      }
#endif

      if (device.configurableDecimals())
      {
        html_TD();
        const String id = getPluginCustomArgName(F("TDVD"), varNr); // ="taskdevicevaluedecimals"
        addNumericBox(id, Cache.getTaskDeviceValueDecimals(taskIndex, varNr), 0, 6);
      }
    }
    addFormSeparator(colCount);
  }
}

#endif // ifdef WEBSERVER_DEVICES