#include "../WebServer/JSON.h"

#include "../WebServer/WebServer.h"
#include "../WebServer/JSON.h"
#include "../WebServer/Markup_Forms.h"

#include "../Globals/Nodes.h"
#include "../Globals/Device.h"
#include "../Globals/Plugins.h"
#include "../Helpers/ESPEasy_Storage.h"
#include "../Helpers/Hardware.h"
#include "../Helpers/Numerical.h"
#include "../Helpers/StringConverter.h"
#include "../Helpers/StringProvider.h"

#include "../../_Plugin_Helper.h"
#include "../../ESPEasy-Globals.h"



// ********************************************************************************
// Web Interface get CSV value from task
// ********************************************************************************
void handle_csvval()
{
  String htmlData;
  htmlData.reserve(25); // Reserve for error message
  const int printHeader = getFormItemInt(F("header"), 1);
  bool printHeaderValid = true;
  if (printHeader != 1 && printHeader != 0)
  {
    htmlData = F("ERROR: Header not valid!\n");
    printHeaderValid = false;
  }

  const taskIndex_t taskNr    = getFormItemInt(F("tasknr"), INVALID_TASK_INDEX);
  const bool taskValid = validTaskIndex(taskNr);
  if (!taskValid)
  {
    htmlData = F("ERROR: TaskNr not valid!\n");
  }

  const int INVALID_VALUE_NUM = INVALID_TASKVAR_INDEX + 1;
  const taskVarIndex_t valNr    = getFormItemInt(F("valnr"), INVALID_VALUE_NUM);
  bool valueNumberValid = true;
  if (valNr != INVALID_VALUE_NUM && !validTaskVarIndex(valNr))
  {
    htmlData = F("ERROR: ValueId not valid!\n");
    valueNumberValid = false;
  }

  TXBuffer.startJsonStream();
  if (taskValid && valueNumberValid && printHeaderValid)
  {
    const deviceIndex_t DeviceIndex = getDeviceIndex_from_TaskIndex(taskNr);

    if (validDeviceIndex(DeviceIndex))
    {
      LoadTaskSettings(taskNr);
      const byte taskValCount = getValueCountForTask(taskNr);
      uint16_t stringReserveSize = (valNr == INVALID_VALUE_NUM ? 1 : taskValCount) * 24;
      htmlData.reserve(stringReserveSize);

      if (printHeader)
      {
        for (byte x = 0; x < taskValCount; x++)
        {
          if (valNr == INVALID_VALUE_NUM || valNr == x)
          {
            htmlData += String(ExtraTaskSettings.TaskDeviceValueNames[x]);
            if (x != taskValCount - 1)
            {
              htmlData += ';';
            }
          }
        }
        htmlData += '\n';
        addHtml(htmlData);
        htmlData = "";
      }

      for (byte x = 0; x < taskValCount; x++)
      {
        if (valNr == INVALID_VALUE_NUM || valNr == x)
        {
          htmlData += formatUserVarNoCheck(taskNr, x);
          if (x != taskValCount - 1)
          {
            htmlData += ';';
          }
        }
      }
      htmlData += '\n';
    }
  }
  addHtml(htmlData);
  TXBuffer.endStream();
}
// ********************************************************************************
// Web Interface JSON page (no password!)
// ********************************************************************************
void handle_json()
{
  const taskIndex_t taskNr    = getFormItemInt(F("tasknr"), INVALID_TASK_INDEX);
  const bool showSpecificTask = validTaskIndex(taskNr);
  bool showSystem             = true;
  bool showWifi               = true;
  #ifdef HAS_ETHERNET
  bool showEthernet           = true;
  #endif
  bool showDataAcquisition    = true;
  bool showTaskDetails        = true;
  bool showNodes              = true;
  {
    String view = web_server.arg("view");

    if (view.length() != 0) {
      if (view == F("sensorupdate")) {
        showSystem          = false;
        showWifi            = false;
        #ifdef HAS_ETHERNET
        showEthernet        = false;
        #endif
        showDataAcquisition = false;
        showTaskDetails     = false;
        showNodes           = false;
      }
    }
  }

  TXBuffer.startJsonStream();

  if (!showSpecificTask)
  {
    addHtml("{");

    if (showSystem) {
      addHtml(F("\"System\":{\n"));
      stream_next_json_object_value(LabelType::BUILD_DESC);
      stream_next_json_object_value(LabelType::GIT_BUILD);
      stream_next_json_object_value(LabelType::SYSTEM_LIBRARIES);
      stream_next_json_object_value(LabelType::PLUGIN_COUNT);
      stream_next_json_object_value(LabelType::PLUGIN_DESCRIPTION);
      stream_next_json_object_value(LabelType::LOCAL_TIME);
      stream_next_json_object_value(LabelType::UNIT_NR);
      stream_next_json_object_value(LabelType::UNIT_NAME);
      stream_next_json_object_value(LabelType::UPTIME);
      stream_next_json_object_value(LabelType::BOOT_TYPE);
      stream_next_json_object_value(LabelType::RESET_REASON);

      if (wdcounter > 0)
      {
        stream_next_json_object_value(LabelType::LOAD_PCT);
        stream_next_json_object_value(LabelType::LOOP_COUNT);
      }
      stream_next_json_object_value(LabelType::CPU_ECO_MODE);

      #ifdef CORE_POST_2_5_0
      stream_next_json_object_value(LabelType::HEAP_MAX_FREE_BLOCK);
      stream_next_json_object_value(LabelType::HEAP_FRAGMENTATION);
      #endif // ifdef CORE_POST_2_5_0
      stream_last_json_object_value(LabelType::FREE_MEM);
      addHtml(F(",\n"));
    }

    if (showWifi) {
      addHtml(F("\"WiFi\":{\n"));
      #if defined(ESP8266)
      stream_next_json_object_value(LabelType::HOST_NAME);
      #endif // if defined(ESP8266)
      #ifdef FEATURE_MDNS
      stream_next_json_object_value(LabelType::M_DNS);
      #endif // ifdef FEATURE_MDNS
      stream_next_json_object_value(LabelType::IP_CONFIG);
      stream_next_json_object_value(LabelType::IP_ADDRESS);
      stream_next_json_object_value(LabelType::IP_SUBNET);
      stream_next_json_object_value(LabelType::GATEWAY);
      stream_next_json_object_value(LabelType::STA_MAC);
      stream_next_json_object_value(LabelType::DNS_1);
      stream_next_json_object_value(LabelType::DNS_2);
      stream_next_json_object_value(LabelType::SSID);
      stream_next_json_object_value(LabelType::BSSID);
      stream_next_json_object_value(LabelType::CHANNEL);
      stream_next_json_object_value(LabelType::CONNECTED_MSEC);
      stream_next_json_object_value(LabelType::LAST_DISCONNECT_REASON);
      stream_next_json_object_value(LabelType::LAST_DISC_REASON_STR);
      stream_next_json_object_value(LabelType::NUMBER_RECONNECTS);
      stream_next_json_object_value(LabelType::FORCE_WIFI_BG);
      stream_next_json_object_value(LabelType::RESTART_WIFI_LOST_CONN);
#ifdef ESP8266
      stream_next_json_object_value(LabelType::FORCE_WIFI_NOSLEEP);
#endif // ifdef ESP8266
#ifdef SUPPORT_ARP
      stream_next_json_object_value(LabelType::PERIODICAL_GRAT_ARP);
#endif // ifdef SUPPORT_ARP
      stream_next_json_object_value(LabelType::CONNECTION_FAIL_THRESH);
      stream_last_json_object_value(LabelType::WIFI_RSSI);
      // TODO: PKR: Add ETH Objects
      addHtml(F(",\n"));
    }

    #ifdef HAS_ETHERNET
    if (showEthernet) {
      addHtml(F("\"Ethernet\":{\n"));
      stream_next_json_object_value(LabelType::ETH_WIFI_MODE);
      stream_next_json_object_value(LabelType::ETH_CONNECTED);
      stream_next_json_object_value(LabelType::ETH_DUPLEX);
      stream_next_json_object_value(LabelType::ETH_SPEED);
      stream_next_json_object_value(LabelType::ETH_STATE);
      stream_last_json_object_value(LabelType::ETH_SPEED_STATE);
      addHtml(F(",\n"));
    }
    #endif

    if (showNodes) {
      bool comma_between = false;

      for (NodesMap::iterator it = Nodes.begin(); it != Nodes.end(); ++it)
      {
        if (it->second.ip[0] != 0)
        {
          if (comma_between) {
            addHtml(",");
          } else {
            comma_between = true;
            addHtml(F("\"nodes\":[\n")); // open json array if >0 nodes
          }

          addHtml("{");
          stream_next_json_object_value(F("nr"), String(it->first));
          stream_next_json_object_value(F("name"),
                                        (it->first != Settings.Unit) ? it->second.nodeName : Settings.Name);

          if (it->second.build) {
            stream_next_json_object_value(F("build"), String(it->second.build));
          }

          if (it->second.nodeType) {
            String platform = getNodeTypeDisplayString(it->second.nodeType);

            if (platform.length() > 0) {
              stream_next_json_object_value(F("platform"), platform);
            }
          }
          stream_next_json_object_value(F("ip"), it->second.ip.toString());
          stream_last_json_object_value(F("age"), String(it->second.age));
        } // if node info exists
      }   // for loop

      if (comma_between) {
        addHtml(F("],\n")); // close array if >0 nodes
      }
    }
  }

  taskIndex_t firstTaskIndex = 0;
  taskIndex_t lastTaskIndex  = TASKS_MAX - 1;

  if (showSpecificTask)
  {
    firstTaskIndex = taskNr - 1;
    lastTaskIndex  = taskNr - 1;
  }
  taskIndex_t lastActiveTaskIndex = 0;

  for (taskIndex_t TaskIndex = firstTaskIndex; TaskIndex <= lastTaskIndex; TaskIndex++) {
    if (validPluginID_fullcheck(Settings.TaskDeviceNumber[TaskIndex])) {
      lastActiveTaskIndex = TaskIndex;
    }
  }

  if (!showSpecificTask) {
    addHtml(F("\"Sensors\":[\n"));
  }

  // Keep track of the lowest reported TTL and use that as refresh interval.
  unsigned long lowest_ttl_json = 60;

  for (taskIndex_t TaskIndex = firstTaskIndex; TaskIndex <= lastActiveTaskIndex && validTaskIndex(TaskIndex); TaskIndex++)
  {
    const deviceIndex_t DeviceIndex = getDeviceIndex_from_TaskIndex(TaskIndex);

    if (validDeviceIndex(DeviceIndex))
    {
      const unsigned long taskInterval = Settings.TaskDeviceTimer[TaskIndex];
      LoadTaskSettings(TaskIndex);
      addHtml(F("{\n"));

      unsigned long ttl_json = 60; // Default value

      // For simplicity, do the optional values first.
      const byte valueCount = getValueCountForTask(TaskIndex);
      if (valueCount != 0) {
        if ((taskInterval > 0) && Settings.TaskDeviceEnabled[TaskIndex]) {
          ttl_json = taskInterval;

          if (ttl_json < lowest_ttl_json) {
            lowest_ttl_json = ttl_json;
          }
        }
        addHtml(F("\"TaskValues\": [\n"));

        for (byte x = 0; x < valueCount; x++)
        {
          addHtml("{");
          stream_next_json_object_value(F("ValueNumber"), String(x + 1));
          stream_next_json_object_value(F("Name"),        String(ExtraTaskSettings.TaskDeviceValueNames[x]));
          stream_next_json_object_value(F("NrDecimals"),  String(ExtraTaskSettings.TaskDeviceValueDecimals[x]));
          stream_last_json_object_value(F("Value"), formatUserVarNoCheck(TaskIndex, x));

          if (x < (valueCount - 1)) {
            addHtml(F(",\n"));
          }
        }
        addHtml(F("],\n"));
      }

      if (showSpecificTask) {
        stream_next_json_object_value(F("TTL"), String(ttl_json * 1000));
      }

      if (showDataAcquisition) {
        addHtml(F("\"DataAcquisition\": [\n"));

        for (controllerIndex_t x = 0; x < CONTROLLER_MAX; x++)
        {
          addHtml("{");
          stream_next_json_object_value(F("Controller"), String(x + 1));
          stream_next_json_object_value(F("IDX"),        String(Settings.TaskDeviceID[x][TaskIndex]));
          stream_last_json_object_value(F("Enabled"), jsonBool(Settings.TaskDeviceSendData[x][TaskIndex]));

          if (x < (CONTROLLER_MAX - 1)) {
            addHtml(F(",\n"));
          }
        }
        addHtml(F("],\n"));
      }

      if (showTaskDetails) {
        stream_next_json_object_value(F("TaskInterval"),     String(taskInterval));
        stream_next_json_object_value(F("Type"),             getPluginNameFromDeviceIndex(DeviceIndex));
        stream_next_json_object_value(F("TaskName"),         String(ExtraTaskSettings.TaskDeviceName));
        stream_next_json_object_value(F("TaskDeviceNumber"), String(Settings.TaskDeviceNumber[TaskIndex]));
#ifdef FEATURE_I2CMULTIPLEXER
        if (Device[DeviceIndex].Type == DEVICE_TYPE_I2C && isI2CMultiplexerEnabled()) {
          int8_t channel = Settings.I2C_Multiplexer_Channel[TaskIndex];
          if (bitRead(Settings.I2C_Flags[TaskIndex], I2C_FLAGS_MUX_MULTICHANNEL)) {
            addHtml(F("\"I2CBus\" : ["));
            uint8_t b = 0;
            for (uint8_t c = 0; c < I2CMultiplexerMaxChannels(); c++) {
              if (bitRead(channel, c)) {
                if (b > 0) { addHtml(F(",\n")); }
                b++;
                String i2cChannel = F("\"Multiplexer channel ");
                i2cChannel += String(c);
                i2cChannel += F("\"");
                addHtml(i2cChannel);
              }
            }
            addHtml(F("],\n"));
          } else {
            if (channel == -1){
              stream_next_json_object_value(F("I2Cbus"),       F("Standard I2C bus"));
            } else {
              String i2cChannel = F("Multiplexer channel ");
              i2cChannel += String(channel);
              stream_next_json_object_value(F("I2Cbus"),       i2cChannel);
            }
          }
        }
#endif
      }
      stream_next_json_object_value(F("TaskEnabled"), jsonBool(Settings.TaskDeviceEnabled[TaskIndex]));
      stream_last_json_object_value(F("TaskNumber"), String(TaskIndex + 1));

      if (TaskIndex != lastActiveTaskIndex) {
        addHtml(",");
      }
      addHtml("\n");
    }
  }

  if (!showSpecificTask) {
    addHtml(F("],\n"));
    stream_last_json_object_value(F("TTL"), String(lowest_ttl_json * 1000));
  }

  TXBuffer.endStream();
}

// ********************************************************************************
// JSON formatted timing statistics
// ********************************************************************************

#ifdef WEBSERVER_NEW_UI
void handle_timingstats_json() {
  TXBuffer.startJsonStream();
  json_init();
  json_open();
  # ifdef USES_TIMING_STATS
  jsonStatistics(false);
  # endif // ifdef USES_TIMING_STATS
  json_close();
  TXBuffer.endStream();
}

#endif // WEBSERVER_NEW_UI

#ifdef WEBSERVER_NEW_UI
void handle_nodes_list_json() {
  if (!isLoggedIn()) { return; }
  TXBuffer.startJsonStream();
  json_init();
  json_open(true);

  for (NodesMap::iterator it = Nodes.begin(); it != Nodes.end(); ++it)
  {
    if (it->second.ip[0] != 0)
    {
      json_open();
      bool isThisUnit = it->first == Settings.Unit;

      if (isThisUnit) {
        json_number(F("thisunit"), String(1));
      }

      json_number(F("first"), String(it->first));
      json_prop(F("name"), isThisUnit ? Settings.Name : it->second.nodeName);

      if (it->second.build) { json_prop(F("build"), String(it->second.build)); }
      json_prop(F("type"), getNodeTypeDisplayString(it->second.nodeType));
      json_prop(F("ip"),   it->second.ip.toString());
      json_number(F("age"), String(it->second.age));
      json_close();
    }
  }
  json_close(true);
  TXBuffer.endStream();
}

void handle_buildinfo() {
  if (!isLoggedIn()) { return; }
  TXBuffer.startJsonStream();
  json_init();
  json_open();
  {
    json_open(true, F("plugins"));

    for (deviceIndex_t x = 0; x <= deviceCount; x++) {
      if (validPluginID(DeviceIndex_to_Plugin_id[x])) {
        json_open();
        json_number(F("id"), String(DeviceIndex_to_Plugin_id[x]));
        json_prop(F("name"), getPluginNameFromDeviceIndex(x));
        json_close();
      }
    }
    json_close(true);
  }
  {
    json_open(true, F("controllers"));

    for (protocolIndex_t x = 0; x < CPLUGIN_MAX; x++) {
      if (getCPluginID_from_ProtocolIndex(x) != INVALID_C_PLUGIN_ID) {
        json_open();
        json_number(F("id"), String(x + 1));
        json_prop(F("name"), getCPluginNameFromProtocolIndex(x));
        json_close();
      }
    }
    json_close(true);
  }
  {
    json_open(true, F("notifications"));

    for (byte x = 0; x < NPLUGIN_MAX; x++) {
      if (validNPluginID(NPlugin_id[x])) {
        json_open();
        json_number(F("id"), String(x + 1));
        json_prop(F("name"), getNPluginNameFromNotifierIndex(x));
        json_close();
      }
    }
    json_close(true);
  }
  json_prop(LabelType::BUILD_DESC);
  json_prop(LabelType::GIT_BUILD);
  json_prop(LabelType::SYSTEM_LIBRARIES);
  json_prop(LabelType::PLUGIN_COUNT);
  json_prop(LabelType::PLUGIN_DESCRIPTION);
  json_close();
  TXBuffer.endStream();
}

#endif // WEBSERVER_NEW_UI


/*********************************************************************************************\
   Streaming versions directly to TXBuffer
\*********************************************************************************************/
void stream_to_json_value(const String& value) {
  if ((value.length() == 0) || !isFloat(value)) {
    String html;
    html.reserve(value.length() + 2);
    html += '\"';
    html += value;
    html += '\"';
    addHtml(html);
  } else {
    addHtml(value);
  }
}

void stream_to_json_object_value(const String& object, const String& value) {
  String html;

  html.reserve(object.length() + 4);

  html += '\"';
  html += object;
  html += "\":";
  addHtml(html);
  stream_to_json_value(value);
}

String jsonBool(bool value) {
  return boolToString(value);
}

// Add JSON formatted data directly to the TXbuffer, including a trailing comma.
void stream_next_json_object_value(const String& object, const String& value) {
  addHtml(to_json_object_value(object, value));
  addHtml(F(",\n"));
}

// Add JSON formatted data directly to the TXbuffer, including a closing '}'
void stream_last_json_object_value(const String& object, const String& value) {
  addHtml(to_json_object_value(object, value));
  addHtml(F("\n}"));
}

void stream_next_json_object_value(LabelType::Enum label) {
  stream_next_json_object_value(getLabel(label), getValue(label));
}

void stream_last_json_object_value(LabelType::Enum label) {
  stream_last_json_object_value(getLabel(label), getValue(label));
}
