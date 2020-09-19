#ifdef WEBSERVER_DEVICES

# include "src/Globals/Nodes.h"
# include "src/Globals/Device.h"
# include "src/Globals/CPlugins.h"
# include "src/Globals/Plugins.h"
# include "src/Static/WebStaticData.h"


void handle_devices() {
  checkRAM(F("handle_devices"));

  if (!isLoggedIn()) { return; }
  navMenuIndex = MENU_INDEX_DEVICES;
  TXBuffer.startStream();
  sendHeadandTail_stdtemplate(_HEAD);


  // char tmpString[41];


  // String taskindex = web_server.arg(F("index"));

  pluginID_t taskdevicenumber;

  if (web_server.hasArg(F("del"))) {
    taskdevicenumber = 0;
  }
  else {
    taskdevicenumber = getFormItemInt(F("TDNUM"), 0);
  }


  // String taskdeviceid[CONTROLLER_MAX];
  // String taskdevicepin1 = web_server.arg(F("taskdevicepin1"));   // "taskdevicepin*" should not be changed because it is uses by plugins
  // and expected to be saved by this code
  // String taskdevicepin2 = web_server.arg(F("taskdevicepin2"));
  // String taskdevicepin3 = web_server.arg(F("taskdevicepin3"));
  // String taskdevicepin1pullup = web_server.arg(F("TDPPU"));
  // String taskdevicepin1inversed = web_server.arg(F("TDPI"));
  // String taskdevicename = web_server.arg(F("TDN"));
  // String taskdeviceport = web_server.arg(F("TDP"));
  // String taskdeviceformula[VARS_PER_TASK];
  // String taskdevicevaluename[VARS_PER_TASK];
  // String taskdevicevaluedecimals[VARS_PER_TASK];
  // String taskdevicesenddata[CONTROLLER_MAX];
  // String taskdeviceglobalsync = web_server.arg(F("TDGS"));
  // String taskdeviceenabled = web_server.arg(F("TDE"));

  // for (byte varNr = 0; varNr < VARS_PER_TASK; varNr++)
  // {
  //   char argc[25];
  //   String arg = F("TDF");
  //   arg += varNr + 1;
  //   arg.toCharArray(argc, 25);
  //   taskdeviceformula[varNr] = web_server.arg(argc);
  //
  //   arg = F("TDVN");
  //   arg += varNr + 1;
  //   arg.toCharArray(argc, 25);
  //   taskdevicevaluename[varNr] = web_server.arg(argc);
  //
  //   arg = F("TDVD");
  //   arg += varNr + 1;
  //   arg.toCharArray(argc, 25);
  //   taskdevicevaluedecimals[varNr] = web_server.arg(argc);
  // }

  // for (controllerIndex_t controllerNr = 0; controllerNr < CONTROLLER_MAX; controllerNr++)
  // {
  //   char argc[25];
  //   String arg = F("TDID");
  //   arg += controllerNr + 1;
  //   arg.toCharArray(argc, 25);
  //   taskdeviceid[controllerNr] = web_server.arg(argc);
  //
  //   arg = F("TDSD");
  //   arg += controllerNr + 1;
  //   arg.toCharArray(argc, 25);
  //   taskdevicesenddata[controllerNr] = web_server.arg(argc);
  // }

  byte page = getFormItemInt(F("page"), 0);

  if (page == 0) {
    page = 1;
  }
  byte setpage = getFormItemInt(F("setpage"), 0);

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

  if (!taskIndexNotSet) {
    --taskIndex;
    LoadTaskSettings(taskIndex); // Make sure ExtraTaskSettings are up-to-date
  }

  // FIXME TD-er: Might have to clear any caches here.
  if ((edit != 0) && !taskIndexNotSet) // when form submitted
  {
    if (Settings.TaskDeviceNumber[taskIndex] != taskdevicenumber)
    {
      // change of device: cleanup old device and reset default settings
      setTaskDevice_to_TaskIndex(taskdevicenumber, taskIndex);
    }
    else if (taskdevicenumber != 0) // save settings
    {
      handle_devices_CopySubmittedSettings(taskIndex, taskdevicenumber);
    }

    if (taskdevicenumber != 0) {
      // Task index has a task device number, so it makes sense to save.
      // N.B. When calling delete, the settings were already saved.
      addHtmlError(SaveTaskSettings(taskIndex));
      addHtmlError(SaveSettings());

      struct EventStruct TempEvent;
      TempEvent.TaskIndex = taskIndex;
      String dummy;

      if (Settings.TaskDeviceEnabled[taskIndex]) {
        PluginCall(PLUGIN_INIT, &TempEvent, dummy);
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

  checkRAM(F("handle_devices"));
# ifndef BUILD_NO_DEBUG

  if (loglevelActiveFor(LOG_LEVEL_DEBUG_DEV)) {
    String log = F("DEBUG: String size:");
    log += String(TXBuffer.sentBytes);
    addLog(LOG_LEVEL_DEBUG_DEV, log);
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
void addDeviceSelect(const String& name,  int choice)
{
  String deviceName;

  addSelector_Head_reloadOnChange(name);
  addSelector_Item(F("- None -"), 0, false, false, "");

  for (byte x = 0; x <= deviceCount; x++)
  {
    const deviceIndex_t deviceIndex = DeviceIndex_sorted[x];

    if (validDeviceIndex(deviceIndex)) {
      const pluginID_t pluginID = DeviceIndex_to_Plugin_id[deviceIndex];

      if (validPluginID(pluginID)) {
        deviceName = getPluginNameFromDeviceIndex(deviceIndex);


  # ifdef PLUGIN_BUILD_DEV
        String plugin = "P";

        if (pluginID < 10) { plugin += '0'; }

        if (pluginID < 100) { plugin += '0'; }
        plugin    += pluginID;
        plugin    += F(" - ");
        deviceName = plugin + deviceName;
  # endif // ifdef PLUGIN_BUILD_DEV

        addSelector_Item(deviceName,
                         Device[deviceIndex].Number,
                         choice == Device[deviceIndex].Number,
                         false,
                         "");
      }
    }
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

  unsigned long taskdevicetimer = getFormItemInt(F("TDT"), 0);
  Settings.TaskDeviceNumber[taskIndex] = taskdevicenumber;


  uint8_t flags = 0;
  if (Device[DeviceIndex].Type == DEVICE_TYPE_I2C) {
    bitWrite(flags, 0, isFormItemChecked(F("taskdeviceflags0")));
  }
#ifdef FEATURE_I2CMULTIPLEXER
  if (Device[DeviceIndex].Type == DEVICE_TYPE_I2C && isI2CMultiplexerEnabled()) {
    int multipleMuxPortsOption = getFormItemInt(F("taskdeviceflags1"), 0);
    bitWrite(flags, 1, multipleMuxPortsOption == 1);
    if (multipleMuxPortsOption == 1) {
      uint8_t selectedPorts = 0;
      for (uint8_t x = 0; x < I2CMultiplexerMaxChannels(); x++) {
        String id = F("taskdeviceflag1ch");
        id += String(x);
        bitWrite(selectedPorts, x, isFormItemChecked(id));
      }
      Settings.I2C_Multiplexer_Channel[taskIndex] = selectedPorts;
    } else {
      Settings.I2C_Multiplexer_Channel[taskIndex] = getFormItemInt(F("taskdevicei2cmuxport"), 0);
    }
  }
#endif
  if (Device[DeviceIndex].Type == DEVICE_TYPE_I2C) {
    Settings.I2C_Flags[taskIndex] = flags;
  }

  int pin1 = -1;
  int pin2 = -1;
  int pin3 = -1;
  update_whenset_FormItemInt(F("taskdevicepin1"), pin1);
  update_whenset_FormItemInt(F("taskdevicepin2"), pin2);
  update_whenset_FormItemInt(F("taskdevicepin3"), pin3);
  setBasicTaskValues(taskIndex, taskdevicetimer,
                     isFormItemChecked(F("TDE")), web_server.arg(F("TDN")),
                     pin1, pin2, pin3);
  Settings.TaskDevicePort[taskIndex] = getFormItemInt(F("TDP"), 0);
  update_whenset_FormItemInt(F("remoteFeed"), Settings.TaskDeviceDataFeed[taskIndex]);

  for (controllerIndex_t controllerNr = 0; controllerNr < CONTROLLER_MAX; controllerNr++)
  {
    Settings.TaskDeviceID[controllerNr][taskIndex]       = getFormItemInt(String(F("TDID")) + (controllerNr + 1));
    Settings.TaskDeviceSendData[controllerNr][taskIndex] = isFormItemChecked(String(F("TDSD")) + (controllerNr + 1));
  }

  if (Device[DeviceIndex].PullUpOption) {
    Settings.TaskDevicePin1PullUp[taskIndex] = isFormItemChecked(F("TDPPU"));
  }

  if (Device[DeviceIndex].InverseLogicOption) {
    Settings.TaskDevicePin1Inversed[taskIndex] = isFormItemChecked(F("TDPI"));
  }

  for (byte varNr = 0; varNr < Device[DeviceIndex].ValueCount; varNr++)
  {
    strncpy_webserver_arg(ExtraTaskSettings.TaskDeviceFormula[varNr],    String(F("TDF")) + (varNr + 1));
    ExtraTaskSettings.TaskDeviceValueDecimals[varNr] = getFormItemInt(String(F("TDVD")) + (varNr + 1));
    strncpy_webserver_arg(ExtraTaskSettings.TaskDeviceValueNames[varNr], String(F("TDVN")) + (varNr + 1));

    // taskdeviceformula[varNr].toCharArray(tmpString, 41);
    // strcpy(ExtraTaskSettings.TaskDeviceFormula[varNr], tmpString);
    // ExtraTaskSettings.TaskDeviceValueDecimals[varNr] = taskdevicevaluedecimals[varNr].toInt();
    // taskdevicevaluename[varNr].toCharArray(tmpString, 41);
  }

  // // task value names handling.
  // for (byte varNr = 0; varNr < Device[DeviceIndex].ValueCount; varNr++)
  // {
  //   taskdevicevaluename[varNr].toCharArray(tmpString, 41);
  //   strcpy(ExtraTaskSettings.TaskDeviceValueNames[varNr], tmpString);
  // }

  struct EventStruct TempEvent;
  TempEvent.TaskIndex = taskIndex;

  if (ExtraTaskSettings.TaskIndex != TempEvent.TaskIndex) { // if field set empty, reload defaults
    String dummy;
    PluginCall(PLUGIN_GET_DEVICEVALUENAMES, &TempEvent, dummy);
  }

  // allow the plugin to save plugin-specific form settings.
  {
    String dummy;
    PluginCall(PLUGIN_WEBFORM_SAVE, &TempEvent, dummy);
  }

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

// ********************************************************************************
// Show table with all selected Tasks/Devices
// ********************************************************************************
void handle_devicess_ShowAllTasksTable(byte page)
{
  html_add_script(true);
  TXBuffer += DATA_UPDATE_SENSOR_VALUES_DEVICE_PAGE_JS;
  html_add_script_end();
  html_table_class_multirow();
  html_TR();
  html_table_header("", 70);

  if (TASKS_MAX != TASKS_PER_PAGE)
  {
    html_add_button_prefix();

    {
      String html;
      html.reserve(30);

      html += F("devices?setpage=");

      if (page > 1) {
        html += page - 1;
      }
      else {
        html += page;
      }
      html += F("'>&lt;</a>");
      addHtml(html);
    }
    html_add_button_prefix();
    {
      String html;
      html.reserve(30);

      html += F("devices?setpage=");

      if (page < (TASKS_MAX / TASKS_PER_PAGE)) {
        html += page + 1;
      }
      else {
        html += page;
      }
      html += F("'>&gt;</a>");
      addHtml(html);
    }
  }

  html_table_header("Task",       50);
  html_table_header(F("Enabled"), 100);
  html_table_header(F("Device"));
  html_table_header("Name");
  html_table_header("Port");
  html_table_header(F("Ctr (IDX)"), 100);
  html_table_header("GPIO",         100);
  html_table_header(F("Values"));

  String deviceName;

  for (byte x = (page - 1) * TASKS_PER_PAGE; x < ((page) * TASKS_PER_PAGE) && validTaskIndex(x); x++)
  {
    const deviceIndex_t DeviceIndex = getDeviceIndex_from_TaskIndex(x);
    const bool pluginID_set         = INVALID_PLUGIN_ID != Settings.TaskDeviceNumber[x];

    html_TR_TD();

    if (pluginID_set && !supportedPluginID(Settings.TaskDeviceNumber[x])) {
      html_add_button_prefix(F("red"), true);
    } else {
      html_add_button_prefix();
    }
    {
      String html;
      html.reserve(30);

      html += F("devices?index=");
      html += x + 1;
      html += F("&page=");
      html += page;
      html += F("'>");

      if (pluginID_set) {
        html += F("Edit");
      } else {
        html += F("Add");
      }
      html += F("</a><TD>");
      html += x + 1;
      addHtml(html);
      html_TD();
    }

    // Show table of all configured tasks
    // A task may also refer to a non supported plugin.
    // This will be shown as not supported.
    // Editing a task which has a non supported plugin will present the same as when assigning a new plugin to a task.
    if (pluginID_set)
    {
      LoadTaskSettings(x);
      struct EventStruct TempEvent;
      TempEvent.TaskIndex = x;
      addEnabled(Settings.TaskDeviceEnabled[x]  && validDeviceIndex(DeviceIndex));

      html_TD();
      addHtml(getPluginNameFromPluginID(Settings.TaskDeviceNumber[x]));
      html_TD();
      addHtml(ExtraTaskSettings.TaskDeviceName);
      html_TD();

      if (validDeviceIndex(DeviceIndex)) {
        if (Settings.TaskDeviceDataFeed[x] != 0) {
          // Show originating node number
          byte remoteUnit = Settings.TaskDeviceDataFeed[x];
          addHtml(F("Unit "));
          addHtml(String(remoteUnit));

          if (remoteUnit != 255) {
            NodesMap::iterator it = Nodes.find(remoteUnit);

            if (it != Nodes.end()) {
              addHtml(F(" - "));
              addHtml(it->second.nodeName);
            } else {
              addHtml(F(" - Not Seen recently"));
            }
          }
        } else {
          String portDescr;

          if (PluginCall(PLUGIN_WEBFORM_SHOW_CONFIG, &TempEvent, portDescr)) {
            addHtml(portDescr);
          } else {
            switch (Device[DeviceIndex].Type) {
              case DEVICE_TYPE_I2C:
                addHtml(F("I2C"));
#ifdef FEATURE_I2CMULTIPLEXER
                if (isI2CMultiplexerEnabled() && I2CMultiplexerPortSelectedForTask(x)) {
                  String mux;
                  if (bitRead(Settings.I2C_Flags[x], I2C_FLAGS_MUX_MULTICHANNEL)) {    // Multi-channel
                    mux = F("<BR>Multiplexer channel(s)");
                    uint8_t b = 0;  // For adding lineBreaks
                    for (uint8_t c = 0; c < I2CMultiplexerMaxChannels(); c++) {
                      if (bitRead(Settings.I2C_Multiplexer_Channel[x], c)) {
                        mux += b == 0 ? F("<BR>") : F(", ");
                        b++;
                        mux += String(c);
                      }
                    }
                  } else {    // Single channel
                    mux = F("<BR>Multiplexer channel ");
                    mux += String(Settings.I2C_Multiplexer_Channel[x]);
                  }
                  addHtml(mux);
                }
#endif
                break;
              case DEVICE_TYPE_SERIAL:
              case DEVICE_TYPE_SERIAL_PLUS1:
                addHtml(serialHelper_getSerialTypeLabel(&TempEvent));
                break;

              default:

                // Plugin has no custom port formatting, show default one.
                if (Device[DeviceIndex].Ports != 0)
                {
                  addHtml(formatToHex_decimal(Settings.TaskDevicePort[x]));
                }
                break;
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
                if (Protocol[ProtocolIndex].usesID && (Settings.Protocol[controllerNr] != 0))
                {
                  String html;
                  html.reserve(16);
                  html += " (";
                  html += Settings.TaskDeviceID[controllerNr][x];
                  html += ')';

                  if (Settings.TaskDeviceID[controllerNr][x] == 0) {
                    html += " ";
                    html += F(HTML_SYMBOL_WARNING);
                  }
                  addHtml(html);
                }
                doBR = true;
              }
            }
          }
        }
      }

      html_TD();

      if (validDeviceIndex(DeviceIndex)) {
        if (Settings.TaskDeviceDataFeed[x] == 0)
        {
          bool showpin1 = false;
          bool showpin2 = false;
          bool showpin3 = false;
          switch (Device[DeviceIndex].Type) {
            case DEVICE_TYPE_I2C:
            {
              String html;
              html.reserve(20);
              html += F("SDA: ");
              html += Settings.Pin_i2c_sda;
              html += F("<BR>SCL: ");
              html += Settings.Pin_i2c_scl;

              addHtml(html);
              break;
            }
            case DEVICE_TYPE_ANALOG:
            {
              #ifdef ESP8266
                #if FEATURE_ADC_VCC
                  addHtml(F("ADC (VDD)"));
                #else
                  addHtml(F("ADC (TOUT)"));
                #endif
              #endif
              #ifdef ESP32
              showpin1 = true;
              addHtml(formatGpioName_ADC(Settings.TaskDevicePin1[x]));
              html_BR();
              #endif

              break;
            }
            case DEVICE_TYPE_SERIAL_PLUS1:
              showpin3 = true;
              // fallthrough
            case DEVICE_TYPE_SERIAL:
            {
              addHtml(serialHelper_getGpioDescription(Settings.TaskDevicePin1[x], Settings.TaskDevicePin2[x], F("<BR>")));
              if (showpin3) {
                html_BR();
              }
              break;
            }
            default:
              showpin1 = true;
              showpin2 = true;
              showpin3 = true;
              break;
          }

          if (Settings.TaskDevicePin1[x] != -1 && showpin1)
          {
            String html;
            html += F("GPIO-");
            html += Settings.TaskDevicePin1[x];
            addHtml(html);
          }

          if (Settings.TaskDevicePin2[x] != -1 && showpin2)
          {
            String html;
            html += F("<BR>GPIO-");
            html += Settings.TaskDevicePin2[x];
            addHtml(html);
          }

          if (Settings.TaskDevicePin3[x] != -1 && showpin3)
          {
            String html;
            html += F("<BR>GPIO-");
            html += Settings.TaskDevicePin3[x];
            addHtml(html);
          }
        }
      }

      html_TD();

      if (validDeviceIndex(DeviceIndex)) {
        byte   customValues = false;
        String customValuesString;
        customValues = PluginCall(PLUGIN_WEBFORM_SHOW_VALUES, &TempEvent, customValuesString);

        if (!customValues)
        {
          for (byte varNr = 0; varNr < Device[DeviceIndex].ValueCount; varNr++)
          {
            if (validPluginID_fullcheck(Settings.TaskDeviceNumber[x]))
            {
              addHtml(pluginWebformShowValue(x, varNr, ExtraTaskSettings.TaskDeviceValueNames[varNr], formatUserVarNoCheck(x, varNr)));
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

// ********************************************************************************
// Show the task settings page
// ********************************************************************************
void handle_devices_TaskSettingsPage(taskIndex_t taskIndex, byte page)
{
  if (!validTaskIndex(taskIndex)) { return; }
  const deviceIndex_t DeviceIndex = getDeviceIndex_from_TaskIndex(taskIndex);

  LoadTaskSettings(taskIndex);
  struct EventStruct TempEvent;
  TempEvent.TaskIndex = taskIndex;

  html_add_form();
  html_table_class_normal();
  addFormHeader(F("Task Settings"));


  addHtml(F("<TR><TD style='width:150px;' align='left'>Device:<TD>"));

  // no (supported) device selected, this effectively checks for validDeviceIndex
  if (!supportedPluginID(Settings.TaskDeviceNumber[taskIndex]))
  {
    // takes lots of memory/time so call this only when needed.
    addDeviceSelect("TDNUM", Settings.TaskDeviceNumber[taskIndex]); // ="taskdevicenumber"
  }

  // device selected
  else
  {
    // remember selected device number
    addHtml(F("<input type='hidden' name='TDNUM' value='"));
    {
      String html;
      html += Settings.TaskDeviceNumber[taskIndex];
      html += "'>";
      addHtml(html);
    }

    // show selected device name and delete button
    addHtml(getPluginNameFromDeviceIndex(DeviceIndex));

    addHelpButton(String(F("Plugin")) + Settings.TaskDeviceNumber[taskIndex]);
    addRTDPluginButton(Settings.TaskDeviceNumber[taskIndex]);


    if ((Device[DeviceIndex].Number == 3) && (taskIndex >= 4)) // Number == 3 = PulseCounter Plugin
    {
      addFormNote(F("This plugin is only supported on task 1-4 for now"));
    }

    addFormTextBox(F("Name"), F("TDN"), ExtraTaskSettings.TaskDeviceName, NAME_FORMULA_LENGTH_MAX); // ="taskdevicename"

    addFormCheckBox(F("Enabled"), F("TDE"), Settings.TaskDeviceEnabled[taskIndex]);                 // ="taskdeviceenabled"

    // section: Sensor / Actuator
    if (!Device[DeviceIndex].Custom && (Settings.TaskDeviceDataFeed[taskIndex] == 0) &&
        ((Device[DeviceIndex].Ports != 0) ||
         (Device[DeviceIndex].PullUpOption) ||
         (Device[DeviceIndex].InverseLogicOption) ||
         (Device[DeviceIndex].connectedToGPIOpins())))
    {
      addFormSubHeader((Device[DeviceIndex].SendDataOption) ? F("Sensor") : F("Actuator"));

      if (Device[DeviceIndex].Ports != 0) {
        addFormNumericBox(F("Port"), F("TDP"), Settings.TaskDevicePort[taskIndex]); // ="taskdeviceport"
      }

      if (Device[DeviceIndex].PullUpOption)
      {
        addFormCheckBox(F("Internal PullUp"), F("TDPPU"), Settings.TaskDevicePin1PullUp[taskIndex]); // ="taskdevicepin1pullup"
          # if defined(ESP8266)

        if ((Settings.TaskDevicePin1[taskIndex] == 16) || (Settings.TaskDevicePin2[taskIndex] == 16) ||
            (Settings.TaskDevicePin3[taskIndex] == 16)) {
          addFormNote(F("PullDown for GPIO-16 (D0)"));
        }
          # endif // if defined(ESP8266)
      }

      if (Device[DeviceIndex].InverseLogicOption)
      {
        addFormCheckBox(F("Inversed Logic"), F("TDPI"), Settings.TaskDevicePin1Inversed[taskIndex]); // ="taskdevicepin1inversed"
        addFormNote(F("Will go into effect on next input change."));
      }

      // get descriptive GPIO-names from plugin
      TempEvent.String1 = F("1st GPIO");
      TempEvent.String2 = F("2nd GPIO");
      TempEvent.String3 = F("3rd GPIO");
      String dummy;
      PluginCall(PLUGIN_GET_DEVICEGPIONAMES, &TempEvent, dummy);

      if (Device[DeviceIndex].connectedToGPIOpins()) {
        if (Device[DeviceIndex].usesTaskDevicePin(1)) {
          addFormPinSelect(TempEvent.String1, F("taskdevicepin1"), Settings.TaskDevicePin1[taskIndex]);
        }

        if (Device[DeviceIndex].usesTaskDevicePin(2)) {
          addFormPinSelect(TempEvent.String2, F("taskdevicepin2"), Settings.TaskDevicePin2[taskIndex]);
        }

        if (Device[DeviceIndex].usesTaskDevicePin(3)) {
          addFormPinSelect(TempEvent.String3, F("taskdevicepin3"), Settings.TaskDevicePin3[taskIndex]);
        }
      }
    }

    if (Device[DeviceIndex].Type == DEVICE_TYPE_I2C) {
      addFormSubHeader(F("I2C options"));
      String dummy;
      PluginCall(PLUGIN_WEBFORM_SHOW_I2C_PARAMS, &TempEvent, dummy);
      addFormCheckBox(F("Force Slow I2C speed"), F("taskdeviceflags0"), bitRead(Settings.I2C_Flags[taskIndex], I2C_FLAGS_SLOW_SPEED));
    }
#ifdef FEATURE_I2CMULTIPLEXER
    // Show selector for an I2C multiplexer port if a multiplexer is configured
    if (Device[DeviceIndex].Type == DEVICE_TYPE_I2C && isI2CMultiplexerEnabled()) {
      bool multipleMuxPorts = bitRead(Settings.I2C_Flags[taskIndex], I2C_FLAGS_MUX_MULTICHANNEL);
      {
        String i2c_mux_channels[2];
        int    i2c_mux_channelOptions[2];
        int    i2c_mux_channelCount = 1;
        i2c_mux_channels[0] = F("Single channel");
        i2c_mux_channelOptions[0] = 0;
        if (Settings.I2C_Multiplexer_Type == I2C_MULTIPLEXER_PCA9540) {
          multipleMuxPorts = false; // force off
        } else {
          i2c_mux_channels[1] = F("Multiple channels");
          i2c_mux_channelOptions[1] = 1;
          i2c_mux_channelCount++;
        }
        addFormSelector(F("Multiplexer channels"),F("taskdeviceflags1"), i2c_mux_channelCount, i2c_mux_channels, i2c_mux_channelOptions, multipleMuxPorts ? 1 : 0, true);
      }
      if (multipleMuxPorts) {
        addRowLabel(F("Select connections"), F(""));
        html_table(F(""), false);  // Sub-table
        html_table_header(F("Channel"));
        html_table_header(F("Enable"));
        html_table_header(F("Channel"));
        html_table_header(F("Enable"));
        for (uint8_t x = 0; x < I2CMultiplexerMaxChannels(); x++) {
          String label = F("Channel ");
          label += String(x);
          String id = F("taskdeviceflag1ch");
          id += String(x);
          if (x % 2 == 0) { html_TR(); }  // Start a new row for every 2 channels
          html_TD();
          addHtml(label);
          html_TD();
          addCheckBox(id, bitRead(Settings.I2C_Multiplexer_Channel[taskIndex], x), false);
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
        for (int8_t x = 0; x < mux_max; x++) {
          mux_opt++;
          i2c_mux_portoptions[mux_opt]  = F("Channel ");
          i2c_mux_portoptions[mux_opt] += String(x);

          i2c_mux_portchoices[mux_opt]  = x;
        }
        if (taskDeviceI2CMuxPort >= mux_max) { taskDeviceI2CMuxPort = -1; } // Reset if out of range
        addFormSelector(F("Connected to"), F("taskdevicei2cmuxport"), mux_opt + 1, i2c_mux_portoptions, i2c_mux_portchoices, taskDeviceI2CMuxPort);
      }
    }
#endif
    
    if (Device[DeviceIndex].Type == DEVICE_TYPE_I2C) {
      addFormSubHeader(F("Device settings"));
    }
    // add plugins content
    if (Settings.TaskDeviceDataFeed[taskIndex] == 0) { // only show additional config for local connected sensors
      String webformLoadString;
      PluginCall(PLUGIN_WEBFORM_LOAD, &TempEvent, webformLoadString);

      if (webformLoadString.length() > 0) {
        String errorMessage;
        PluginCall(PLUGIN_GET_DEVICENAME, &TempEvent, errorMessage);
        errorMessage += F(": Bug in PLUGIN_WEBFORM_LOAD, should not append to string, use addHtml() instead");
        addHtmlError(errorMessage);
      }
    }
    else {
      // Show remote feed information.
      addFormSubHeader(F("Data Source"));
      byte remoteUnit = Settings.TaskDeviceDataFeed[taskIndex];
      addFormNumericBox(F("Remote Unit"), F("RemoteUnit"), remoteUnit, 0, 255);

      if (remoteUnit != 255) {
        NodesMap::iterator it = Nodes.find(remoteUnit);

        if (it != Nodes.end()) {
          addUnit(it->second.nodeName);
        } else {
          addUnit(F("Unknown Unit Name"));
        }
      }
      addFormNote(F("0 = disable remote feed, 255 = broadcast")); // FIXME TD-er: Must verify if broadcast can be set.
    }


    // section: Data Acquisition
    if (Device[DeviceIndex].SendDataOption)
    {
      addFormSubHeader(F("Data Acquisition"));

      for (controllerIndex_t controllerNr = 0; controllerNr < CONTROLLER_MAX; controllerNr++)
      {
        if (Settings.Protocol[controllerNr] != 0)
        {
          String id = F("TDSD"); // ="taskdevicesenddata"
          id += controllerNr + 1;

          html_TR_TD();
          addHtml(F("Send to Controller "));
          addHtml(getControllerSymbol(controllerNr));
          html_TD();
          addCheckBox(id, Settings.TaskDeviceSendData[controllerNr][taskIndex]);

          protocolIndex_t ProtocolIndex = getProtocolIndex_from_ControllerIndex(controllerNr);

          if (validProtocolIndex(ProtocolIndex)) {
            if (Protocol[ProtocolIndex].usesID && (Settings.Protocol[controllerNr] != 0))
            {
              addRowLabel(F("IDX"));
              id  = F("TDID"); // ="taskdeviceid"
              id += controllerNr + 1;
              addNumericBox(id, Settings.TaskDeviceID[controllerNr][taskIndex], 0, DOMOTICZ_MAX_IDX);
            }
          }
        }
      }
    }

    addFormSeparator(2);

    if (Device[DeviceIndex].TimerOption)
    {
      // FIXME: shoudn't the max be ULONG_MAX because Settings.TaskDeviceTimer is an unsigned long? addFormNumericBox only supports ints
      // for min and max specification
      addFormNumericBox(F("Interval"), F("TDT"), Settings.TaskDeviceTimer[taskIndex], 0, 65535); // ="taskdevicetimer"
      addUnit(F("sec"));

      if (Device[DeviceIndex].TimerOptional) {
        addHtml(F(" (Optional for this Device)"));
      }
    }

    // section: Values
    if (!Device[DeviceIndex].Custom && (Device[DeviceIndex].ValueCount > 0))
    {
      addFormSubHeader(F("Values"));
      html_end_table();
      html_table_class_normal();

      // table header
      addHtml(F("<TR><TH style='width:30px;' align='center'>#"));
      html_table_header("Name");

      if (Device[DeviceIndex].FormulaOption)
      {
        html_table_header(F("Formula"), F("EasyFormula"), 0);
      }

      if (Device[DeviceIndex].FormulaOption || Device[DeviceIndex].DecimalsOnly)
      {
        html_table_header(F("Decimals"), 30);
      }

      // table body
      for (byte varNr = 0; varNr < Device[DeviceIndex].ValueCount; varNr++)
      {
        html_TR_TD();
        addHtml(String(varNr + 1));
        html_TD();
        String id = F("TDVN"); // ="taskdevicevaluename"
        id += (varNr + 1);
        addTextBox(id, ExtraTaskSettings.TaskDeviceValueNames[varNr], NAME_FORMULA_LENGTH_MAX);

        if (Device[DeviceIndex].FormulaOption)
        {
          html_TD();
          String id = F("TDF"); // ="taskdeviceformula"
          id += (varNr + 1);
          addTextBox(id, ExtraTaskSettings.TaskDeviceFormula[varNr], NAME_FORMULA_LENGTH_MAX);
        }

        if (Device[DeviceIndex].FormulaOption || Device[DeviceIndex].DecimalsOnly)
        {
          html_TD();
          String id = F("TDVD"); // ="taskdevicevaluedecimals"
          id += (varNr + 1);
          addNumericBox(id, ExtraTaskSettings.TaskDeviceValueDecimals[varNr], 0, 6);
        }
      }
    }
  }

  addFormSeparator(4);

  html_TR_TD();
  addHtml(F("<TD colspan='3'>"));
  html_add_button_prefix();
  {
    String html;
    html.reserve(32);

    html += F("devices?setpage=");
    html += page;
    html += F("'>Close</a>");
    addHtml(html);
  }
  addSubmitButton();
  addHtml(F("<input type='hidden' name='edit' value='1'>"));
  addHtml(F("<input type='hidden' name='page' value='1'>"));

  // if user selected a device, add the delete button
  if (validPluginID_fullcheck(Settings.TaskDeviceNumber[taskIndex])) {
    addSubmitButton(F("Delete"), F("del"));
  }

  html_end_table();
  html_end_form();
}

#endif // ifdef WEBSERVER_DEVICES

