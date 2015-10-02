//********************************************************************************
// Web Interface init
//********************************************************************************
void WebServerInit()
{
  // Prepare webserver pages
  WebServer.on("/", handle_root);
  WebServer.on("/config", handle_config);
  WebServer.on("/hardware", handle_hardware);
  WebServer.on("/devices", handle_devices);
#ifdef ESP_CONNEXIO
  WebServer.on("/eventlist", handle_eventlist);
#endif
  WebServer.on("/log", handle_log);
  WebServer.on("/tools", handle_tools);
  WebServer.on("/i2cscanner", handle_i2cscanner);
  WebServer.on("/wifiscanner", handle_wifiscanner);
  WebServer.on("/login", handle_login);
  WebServer.on("/control", handle_control);
  WebServer.on("/download", handle_download);
  WebServer.on("/upload", handle_upload);
  WebServer.onFileUpload(handleFileUpload);
  #if FEATURE_SPIFFS
    WebServer.on("/filelist", handle_filelist);
    WebServer.onNotFound(handleNotFound);
  #else
    WebServer.on("/esp.css", handle_css);
  #endif
  WebServer.on("/advanced", handle_advanced);
  WebServer.begin();
}


//********************************************************************************
// Add top menu
//********************************************************************************
void addMenu(String& str)
{
  boolean cssfile = false;
  
  str += F("<script language=\"javascript\"><!--\n");
  str += F("function dept_onchange(frmselect) {frmselect.submit();}\n");
  str += F("//--></script>");
  str += F("<head><title>");
  str += Settings.Name;
  str += F("</title>");

  #if FEATURE_SPIFFS
  File f = SPIFFS.open("esp.css", "r");
  if (f)
  {
    cssfile = true;
    f.close();
  }
  #else
  if (Settings.CustomCSS)
    cssfile = true;
  #endif

  if (!cssfile)
  {
    str += F("<style>");
    str += F("* {font-family:sans-serif; font-size:12pt;}");
    str += F("h1 {font-size:16pt; color:black;}");
    str += F("h6 {font-size:10pt; color:black; text-align:center;}");
    str += F(".button-menu {background-color:#ffffff; color:blue; margin: 10px; text-decoration:none}");
    str += F(".button-link {padding:5px 15px; background-color:#0077dd; color:#fff; border:solid 1px #fff; text-decoration:none}");
    str += F(".button-menu:hover {background:#ddddff;}");
    str += F(".button-link:hover {background:#369;}");
    str += F("th {padding:10px; background-color:black; color:#ffffff;}");
    str += F("td {padding:7px;}");
    str += F("table {color:black;}");
    str += F(".div_l {float: left;}");
    str += F(".div_r {float: right; margin: 2px; padding: 1px 10px; border-radius: 7px; background-color:#080; color:white;}");
    str += F(".div_br {clear: both;}");
    str += F("</style>");
  }
  else
    str += F("<link rel=\"stylesheet\" type=\"text/css\" href=\"esp.css\">");

  str += F("</head>");

  str += F("<h1>Welcome to ESP ");
#ifdef ESP_CONNEXIO
  str += F("Connexio : ");
#endif
#ifdef ESP_EASY
  str += F("Easy : ");
#endif
  str += Settings.Name;

  #if FEATURE_SPIFFS
  f = SPIFFS.open("esp.png", "r");
  if (f)
  {
    str += F("<img src=\"esp.png\" width=50 height=50 align=right >");
    f.close();
  }  
  #endif
  
  str += F("</h1><BR><a class=\"button-menu\" href=\".\">Main</a>");
  str += F("<a class=\"button-menu\" href=\"config\">Config</a>");
  str += F("<a class=\"button-menu\" href=\"hardware\">Hardware</a>");
  str += F("<a class=\"button-menu\" href=\"devices\">Devices</a>");
#ifdef ESP_CONNEXIO
  str += F("<a class=\"button-menu\" href=\"eventlist\">Eventlist</a>");
#endif
  str += F("<a class=\"button-menu\" href=\"tools\">Tools</a><BR><BR>");
}


//********************************************************************************
// Add footer to web page
//********************************************************************************
void addFooter(String& str)
{
  str += F("<h6>Powered by www.esp8266.nu</h6></body>");
}


//********************************************************************************
// Web Interface root page
//********************************************************************************
void handle_root() {
  if (!isLoggedIn()) return;

  int freeMem = ESP.getFreeHeap();
  String webrequest = WebServer.arg("cmd");
  char command[80];
  command[0] = 0;
  webrequest.toCharArray(command, 80);
  urlDecode(command);

  if ((strcasecmp_P(command, PSTR("wifidisconnect")) != 0) && (strcasecmp_P(command, PSTR("reboot")) != 0))
  {
    String reply = "";
    addMenu(reply);

    printToWeb = true;
    printWebString = "";
#ifdef ESP_CONNEXIO
    ExecuteLine(command, VALUE_SOURCE_SERIAL);
#endif
#ifdef ESP_EASY
    ExecuteCommand(command);
#endif

    reply += printWebString;
    reply += F("<form>");
    reply += F("<table><TH>System Info<TH><TH><TR><TD>");

    IPAddress ip = WiFi.localIP();
    IPAddress gw = WiFi.gatewayIP();

    reply += F("Uptime:<TD>");
    reply += wdcounter / 2;
    reply += F(" minutes");

    char str[20];
    sprintf_P(str, PSTR("%u.%u.%u.%u"), ip[0], ip[1], ip[2], ip[3]);
    reply += F("<TR><TD>IP:<TD>");
    reply += str;

    sprintf_P(str, PSTR("%u.%u.%u.%u"), gw[0], gw[1], gw[2], gw[3]);
    reply += F("<TR><TD>GW:<TD>");
    reply += str;

    reply += F("<TR><TD>Build:<TD>");
    reply += BUILD;

    reply += F("<TR><TD>Unit:<TD>");
    reply += Settings.Unit;

    reply += F("<TR><TD>STA MAC:<TD>");
    uint8_t mac[] = {0, 0, 0, 0, 0, 0};
    uint8_t* macread = WiFi.macAddress(mac);
    char macaddress[20];
    sprintf_P(macaddress, PSTR("%02x:%02x:%02x:%02x:%02x:%02x"), macread[0], macread[1], macread[2], macread[3], macread[4], macread[5]);
    reply += macaddress;

    reply += F("<TR><TD>AP MAC:<TD>");
    macread = WiFi.softAPmacAddress(mac);
    sprintf_P(macaddress, PSTR("%02x:%02x:%02x:%02x:%02x:%02x"), macread[0], macread[1], macread[2], macread[3], macread[4], macread[5]);
    reply += macaddress;

    reply += F("<TR><TD>ESP Chip ID:<TD>");
    reply += ESP.getChipId();

    reply += F("<TR><TD>ESP Flash Size:<TD>");
    reply += ESP.getFlashChipSize();

    reply += F("<TR><TD>Free Mem:<TD>");
    reply += freeMem;

    reply += F("<TR><TH>Node List:<TH>IP<TH>Age<TR><TD><TD>");
    for (byte x = 0; x < 32; x++)
    {
      if (Nodes[x].ip[0] != 0)
      {
        if (x == Settings.Unit)
          reply += F("<font color='blue'>");
        char url[80];
        sprintf_P(url, PSTR("<a href='http://%u.%u.%u.%u'>%u.%u.%u.%u</a>"), Nodes[x].ip[0], Nodes[x].ip[1], Nodes[x].ip[2], Nodes[x].ip[3], Nodes[x].ip[0], Nodes[x].ip[1], Nodes[x].ip[2], Nodes[x].ip[3]);
        reply += F("<TR><TD>Unit ");
        reply += x;
        reply += F(":<TD>");
        reply += url;
        reply += F("<TD>");
        reply += Nodes[x].age;
        if (x == Settings.Unit)
          reply += F("</font color>");
      }
    }

    reply += F("</table></form>");
    addFooter(reply);
    WebServer.send(200, "text/html", reply);
    printWebString = "";
    printToWeb = false;
  }
  else
  {
    // have to disconnect or reboot from within the main loop
    // because the webconnection is still active at this point
    // disconnect here could result into a crash/reboot...
    if (strcasecmp_P(command, PSTR("wifidisconnect")) == 0)
    {
      Serial.println(F("WIFI : Disconnecting..."));
      cmd_within_mainloop = CMD_WIFI_DISCONNECT;
    }

    if (strcasecmp_P(command, PSTR("reboot")) == 0)
    {
      Serial.println(F("     : Rebooting..."));
      cmd_within_mainloop = CMD_REBOOT;
    }

    WebServer.send(200, "text/html", "OK");
  }
}


//********************************************************************************
// Web Interface config page
//********************************************************************************
void handle_config() {
  if (!isLoggedIn()) return;

  char tmpString[64];

  String name = WebServer.arg("name");
  String password = WebServer.arg("password");
  String ssid = WebServer.arg("ssid");
  String key = WebServer.arg("key");
  String controllerip = WebServer.arg("controllerip");
  String controllerport = WebServer.arg("controllerport");
  String protocol = WebServer.arg("protocol");
  String controlleruser = WebServer.arg("controlleruser");
  String controllerpassword = WebServer.arg("controllerpassword");
  String sensordelay = WebServer.arg("delay");
  String deepsleep = WebServer.arg("deepsleep");
  String espip = WebServer.arg("espip");
  String espgateway = WebServer.arg("espgateway");
  String espsubnet = WebServer.arg("espsubnet");
  String unit = WebServer.arg("unit");
  String apkey = WebServer.arg("apkey");

  if (ssid[0] != 0)
  {
    name.toCharArray(tmpString, 26);
    urlDecode(tmpString);
    strcpy(Settings.Name, tmpString);
    password.toCharArray(tmpString, 26);
    urlDecode(tmpString);
    strcpy(SecuritySettings.Password, tmpString);
    ssid.toCharArray(tmpString, 26);
    urlDecode(tmpString);
    strcpy(SecuritySettings.WifiSSID, tmpString);
    key.toCharArray(tmpString, 64);
    urlDecode(tmpString);
    strcpy(SecuritySettings.WifiKey, tmpString);
    apkey.toCharArray(tmpString, 64);
    urlDecode(tmpString);
    strcpy(SecuritySettings.WifiAPKey, tmpString);
    controllerip.toCharArray(tmpString, 26);
    str2ip(tmpString, Settings.Controller_IP);
    Settings.ControllerPort = controllerport.toInt();
    controlleruser.toCharArray(tmpString, 26);
    urlDecode(tmpString);
    strcpy(SecuritySettings.ControllerUser, tmpString);
    controllerpassword.toCharArray(tmpString, 26);
    urlDecode(tmpString);
    strcpy(SecuritySettings.ControllerPassword, tmpString);
    Settings.Protocol = protocol.toInt();
    switch (Settings.Protocol)
    {
      case PROTOCOL_DOMOTICZ_MQTT:
        strcpy_P(Settings.MQTTsubscribe, PSTR("domoticz/in"));
        strcpy_P(Settings.MQTTpublish, PSTR("domoticz/out"));
        break;
      case PROTOCOL_OPENHAB_MQTT:
        strcpy_P(Settings.MQTTsubscribe, PSTR("/%sysname%/#"));
        strcpy_P(Settings.MQTTpublish, PSTR("/%sysname%/%tskname%/%valname%"));
        break;
      case PROTOCOL_PIDOME_MQTT:
        strcpy_P(Settings.MQTTsubscribe, PSTR("/Home/#"));
        strcpy_P(Settings.MQTTpublish, PSTR("/hooks/devices/%id%/SensorData/%valname%"));
        break;
    }
    Settings.Delay = sensordelay.toInt();
    Settings.deepSleep = (deepsleep == "on");
    espip.toCharArray(tmpString, 26);
    str2ip(tmpString, Settings.IP);
    espgateway.toCharArray(tmpString, 26);
    str2ip(tmpString, Settings.Gateway);
    espsubnet.toCharArray(tmpString, 26);
    str2ip(tmpString, Settings.Subnet);
    Settings.Unit = unit.toInt();
    SaveSettings();
  }

  String reply = "";
  addMenu(reply);

  reply += F("<form name='frmselect' method='post'><table>");
  reply += F("<TH>Main Settings<TH><TR><TD>Name:<TD><input type='text' name='name' value='");
  Settings.Name[25] = 0;
  reply += Settings.Name;
  reply += F("'><TR><TD>Admin Password:<TD><input type='text' name='password' value='");
  SecuritySettings.Password[25] = 0;
  reply += SecuritySettings.Password;
  reply += F("'><TR><TD>SSID:<TD><input type='text' name='ssid' value='");
  reply += SecuritySettings.WifiSSID;
  reply += F("'><TR><TD>WPA Key:<TD><input type='text' maxlength='63' name='key' value='");
  reply += SecuritySettings.WifiKey;

  reply += F("'><TR><TD>WPA AP Mode Key:<TD><input type='text' maxlength='63' name='apkey' value='");
  reply += SecuritySettings.WifiAPKey;

  reply += F("'><TR><TD>Unit nr:<TD><input type='text' name='unit' value='");
  reply += Settings.Unit;

  reply += F("'><TR><TD>Protocol:");
  byte choice = Settings.Protocol;
  String options[7];
  options[0] = F("");
  options[1] = F("Domoticz HTTP");
  options[2] = F("Domoticz MQTT");
  options[3] = F("Nodo Telnet");
  options[4] = F("ThingsSpeak HTTP");
  options[5] = F("OpenHAB MQTT");
  options[6] = F("PiDome MQTT");
  reply += F("<TD><select name='protocol' LANGUAGE=javascript onchange=\"return dept_onchange(frmselect)\" >");
  for (byte x = 0; x < 7; x++)
  {
    reply += F("<option value='");
    reply += x;
    reply += "'";
    if (choice == x)
      reply += F(" selected");
    reply += ">";
    reply += options[x];
    reply += F("</option>");
  }
  reply += F("</select>");

  reply += F("<TR><TD>Controller IP:<TD><input type='text' name='controllerip' value='");
  char str[20];
  sprintf_P(str, PSTR("%u.%u.%u.%u"), Settings.Controller_IP[0], Settings.Controller_IP[1], Settings.Controller_IP[2], Settings.Controller_IP[3]);
  reply += str;

  reply += F("'><TR><TD>Controller Port:<TD><input type='text' name='controllerport' value='");
  reply += Settings.ControllerPort;

  if (Settings.Protocol == 9999)
  {
    reply += F("'><TR><TD>Controller User:<TD><input type='text' name='controlleruser' value='");
    reply += SecuritySettings.ControllerUser;
  }

  if (Settings.Protocol == PROTOCOL_NODO_TELNET or Settings.Protocol == PROTOCOL_THINGSPEAK)
  {
    reply += F("'><TR><TD>Controller Password:<TD><input type='text' name='controllerpassword' value='");
    reply += SecuritySettings.ControllerPassword;
  }

  reply += F("'><TR><TD>Sensor Delay:<TD><input type='text' name='delay' value='");
  reply += Settings.Delay;
  reply += F("'><TR><TD>Sleep Mode:<TD>");
  if (Settings.deepSleep)
    reply += F("<input type=checkbox name='deepsleep' checked>");
  else
    reply += F("<input type=checkbox name='deepsleep'>");

  reply += F("<a class=\"button-link\" href=\"http://www.esp8266.nu/index.php/SleepMode\" target=\"_blank\">?</a>");

  reply += F("<TR><TH>Optional Settings<TH>");

  reply += F("<TR><TD>ESP IP:<TD><input type='text' name='espip' value='");
  sprintf_P(str, PSTR("%u.%u.%u.%u"), Settings.IP[0], Settings.IP[1], Settings.IP[2], Settings.IP[3]);
  reply += str;

  reply += F("'><TR><TD>ESP GW:<TD><input type='text' name='espgateway' value='");
  sprintf_P(str, PSTR("%u.%u.%u.%u"), Settings.Gateway[0], Settings.Gateway[1], Settings.Gateway[2], Settings.Gateway[3]);
  reply += str;

  reply += F("'><TR><TD>ESP Subnet:<TD><input type='text' name='espsubnet' value='");
  sprintf_P(str, PSTR("%u.%u.%u.%u"), Settings.Subnet[0], Settings.Subnet[1], Settings.Subnet[2], Settings.Subnet[3]);
  reply += str;

  reply += F("'><TR><TD><TD><input class=\"button-link\" type='submit' value='Submit'>");
  reply += F("</table></form>");
  addFooter(reply);
  WebServer.send(200, "text/html", reply);
}


//********************************************************************************
// Web Interface device page
//********************************************************************************
void handle_devices() {
  if (!isLoggedIn()) return;

  char tmpString[41];
  struct EventStruct TempEvent;

  String taskindex = WebServer.arg("index");
  String taskdevicenumber = WebServer.arg("taskdevicenumber");
  String taskdeviceid = WebServer.arg("taskdeviceid");
  String taskdevicepin1 = WebServer.arg("taskdevicepin1");
  String taskdevicepin2 = WebServer.arg("taskdevicepin2");
  String taskdevicepin1pullup = WebServer.arg("taskdevicepin1pullup");
  String taskdevicepin1inversed = WebServer.arg("taskdevicepin1inversed");
  String taskdevicename = WebServer.arg("taskdevicename");
  String taskdeviceport = WebServer.arg("taskdeviceport");
  String taskdeviceformula[VARS_PER_TASK];
  String taskdevicevaluename[VARS_PER_TASK];
  for (byte varNr = 0; varNr < VARS_PER_TASK; varNr++)
  {
    char argc[25];
    String arg = "taskdeviceformula";
    arg += varNr + 1;
    arg.toCharArray(argc, 25);
    taskdeviceformula[varNr] = WebServer.arg(argc);

    arg = "taskdevicevaluename";
    arg += varNr + 1;
    arg.toCharArray(argc, 25);
    taskdevicevaluename[varNr] = WebServer.arg(argc);
  }

  String edit = WebServer.arg("edit");
  byte page = WebServer.arg("page").toInt();
  if (page == 0)
    page = 1;
  byte setpage = WebServer.arg("setpage").toInt();
  if (setpage > 0)
  {
    if (setpage <= (TASKS_MAX / 4))
      page = setpage;
    else
      page = TASKS_MAX / 4;
  }

  byte index = taskindex.toInt();

  byte DeviceIndex = 0;

  if (edit.toInt() != 0)
  {
    if (Settings.TaskDeviceNumber[index - 1] != taskdevicenumber.toInt()) // change of device, clear all other values
    {
      Settings.TaskDeviceNumber[index - 1] = taskdevicenumber.toInt();
      ExtraTaskSettings.TaskDeviceName[0] = 0;
      Settings.TaskDeviceID[index - 1] = 0;
      Settings.TaskDevicePin1[index - 1] = -1;
      Settings.TaskDevicePin2[index - 1] = -1;
      Settings.TaskDevicePort[index - 1] = 0;
      for (byte varNr = 0; varNr < VARS_PER_TASK; varNr++)
      {
        ExtraTaskSettings.TaskDeviceFormula[varNr][0] = 0;
        ExtraTaskSettings.TaskDeviceValueNames[varNr][0] = 0;
      }
    }
    else if (taskdevicenumber.toInt() != 0)
    {
      Settings.TaskDeviceNumber[index - 1] = taskdevicenumber.toInt();
      DeviceIndex = getDeviceIndex(Settings.TaskDeviceNumber[index - 1]);
      taskdevicename.toCharArray(tmpString, 26);
      urlDecode(tmpString);
      strcpy(ExtraTaskSettings.TaskDeviceName, tmpString);
      Settings.TaskDevicePort[index - 1] = taskdeviceport.toInt();
      if (Settings.TaskDeviceNumber[index - 1] != 0)
        Settings.TaskDeviceID[index - 1] = taskdeviceid.toInt();
      else
        Settings.TaskDeviceID[index - 1] = 0;
      if (Device[DeviceIndex].Type == DEVICE_TYPE_SINGLE)
      {
        Settings.TaskDevicePin1[index - 1] = taskdevicepin1.toInt();
      }
      if (Device[DeviceIndex].Type == DEVICE_TYPE_DUAL)
      {
        Settings.TaskDevicePin1[index - 1] = taskdevicepin1.toInt();
        Settings.TaskDevicePin2[index - 1] = taskdevicepin2.toInt();
      }

      if (Device[DeviceIndex].PullUpOption)
        Settings.TaskDevicePin1PullUp[index - 1] = (taskdevicepin1pullup == "on");

      if (Device[DeviceIndex].InverseLogicOption)
        Settings.TaskDevicePin1Inversed[index - 1] = (taskdevicepin1inversed == "on");

      for (byte varNr = 0; varNr < Device[DeviceIndex].ValueCount; varNr++)
      {
        taskdeviceformula[varNr].toCharArray(tmpString, 41);
        urlDecode(tmpString);
        strcpy(ExtraTaskSettings.TaskDeviceFormula[varNr], tmpString);
      }

      // task value names handling.
      TempEvent.TaskIndex = index - 1;
      for (byte varNr = 0; varNr < Device[DeviceIndex].ValueCount; varNr++)
      {
        taskdevicevaluename[varNr].toCharArray(tmpString, 26);
        urlDecode(tmpString);
        strcpy(ExtraTaskSettings.TaskDeviceValueNames[varNr], tmpString);
      }

#ifdef ESP_EASY
      TempEvent.TaskIndex = index - 1;
      PluginCall(PLUGIN_WEBFORM_SAVE, &TempEvent, dummyString);
      PluginCall(PLUGIN_INIT, &TempEvent, dummyString);
#endif
#ifdef ESP_CONNEXIO
      struct NodoEventStruct TempEvent;
      TempEvent.Par1 = index - 1;
      PluginCall(PLUGIN_WEBFORM_SAVE, &TempEvent, 0);
      PluginCall(PLUGIN_INIT, &TempEvent, 0);
      createEventlist();
#endif
    }
    SaveTaskSettings(index - 1);
    SaveSettings();
  }

  String reply = "";
  addMenu(reply);


  // show all tasks as table
  reply += F("<table cellpadding='4' border='1' frame='box' rules='all'><TH>");
  reply += F("<a class=\"button-link\" href=\"devices?setpage=");
  if (page > 1)
    reply += page - 1;
  else
    reply += page;
  reply += F("\"><</a>");
  reply += F("<a class=\"button-link\" href=\"devices?setpage=");
  if (page < 4)
    reply += page + 1;
  else
    reply += page;
  reply += F("\">></a>");

  reply += F("<TH>Task<TH>Device<TH>Name<TH>Port<TH>IDX/Variable<TH>GPIO<TH>Values");

  String deviceName;

  for (byte x = (page - 1) * 4; x < ((page) * 4); x++)
  {
    LoadTaskSettings(x);
    DeviceIndex = getDeviceIndex(Settings.TaskDeviceNumber[x]);
    TempEvent.TaskIndex = x;

    if (ExtraTaskSettings.TaskDeviceValueNames[0][0] == 0)
      PluginCall(PLUGIN_GET_DEVICEVALUENAMES, &TempEvent, dummyString);

    deviceName = "";
    if (Settings.TaskDeviceNumber[x] != 0)
      Plugin_ptr[DeviceIndex](PLUGIN_GET_DEVICENAME, &TempEvent, deviceName);

    reply += F("<TR><TD>");
    reply += F("<a class=\"button-link\" href=\"devices?index=");
    reply += x + 1;
    reply += F("&page=");
    reply += page;
    reply += F("\">Edit</a>");
    reply += F("<TD>");
    reply += x + 1;
    reply += F("<TD>");
    reply += deviceName;
    reply += F("<TD>");
    reply += ExtraTaskSettings.TaskDeviceName;
    reply += F("<TD>");
    if (Device[DeviceIndex].Ports != 0)
      reply += Settings.TaskDevicePort[x];
    reply += F("<TD>");
    if (Settings.TaskDeviceID[x] != 0)
      reply += Settings.TaskDeviceID[x];

    reply += F("<TD>");
    if (Device[DeviceIndex].Type == DEVICE_TYPE_I2C)
    {
      reply += F("GPIO-");
      reply += Settings.Pin_i2c_sda;
      reply += F("<BR>GPIO-");
      reply += Settings.Pin_i2c_scl;
    }
    if (Device[DeviceIndex].Type == DEVICE_TYPE_ANALOG)
      reply += F("ADC (TOUT)");

    if (Settings.TaskDevicePin1[x] != -1)
    {
      reply += F("GPIO-");
      reply += Settings.TaskDevicePin1[x];
    }

    if (Settings.TaskDevicePin2[x] != -1)
    {
      reply += F("<BR>GPIO-");
      reply += Settings.TaskDevicePin2[x];
    }

    reply += F("<TD>");
    byte customValues = false;
#ifdef ESP_EASY
    customValues = PluginCall(PLUGIN_WEBFORM_VALUES, &TempEvent, reply);
#endif
#ifdef ESP_CONNEXIO
    struct NodoEventStruct TempEvent;
    char tmpString[256];
    tmpString[0] = 0;
    TempEvent.Par1 = x;
    customValues = PluginCall(PLUGIN_WEBFORM_VALUES, &TempEvent, tmpString);
    if (tmpString[0] != 0)
      reply += tmpString;
#endif

    if (!customValues)
    {
      for (byte varNr = 0; varNr < VARS_PER_TASK; varNr++)
      {
        if ((Settings.TaskDeviceNumber[x] != 0) and (varNr < Device[DeviceIndex].ValueCount))
        {
          if (varNr > 0)
            reply += F("<div class=\"div_br\"></div>");
          reply += F("<div class=\"div_l\">");
          reply += ExtraTaskSettings.TaskDeviceValueNames[varNr];
          reply += ":</div>";
          reply += F("<div class=\"div_r\">");
          reply += UserVar[x * VARS_PER_TASK + varNr];
          reply += "</div>";
        }
      }
    }
  }
  reply += F("</table>");

  // Show edit form if a specific entry is chosen with the edit button
  if (index != 0)
  {
    LoadTaskSettings(index - 1);
    DeviceIndex = getDeviceIndex(Settings.TaskDeviceNumber[index - 1]);
    TempEvent.TaskIndex = index - 1;

    if (ExtraTaskSettings.TaskDeviceValueNames[0][0] == 0)
      PluginCall(PLUGIN_GET_DEVICEVALUENAMES, &TempEvent, dummyString);

    reply += F("<BR><BR><form name='frmselect' method='post'><table><TH>Task Settings<TH>Value");

    reply += F("<TR><TD>Device:<TD>");
    addDeviceSelect(reply, "taskdevicenumber", Settings.TaskDeviceNumber[index - 1]);

    if (Settings.TaskDeviceNumber[index - 1] != 0 )
    {
      reply += F("<a class=\"button-link\" href=\"http://www.esp8266.nu/index.php/plugin");
      reply += Settings.TaskDeviceNumber[index - 1];
      reply += F("\" target=\"_blank\">?</a>");

      reply += F("<TR><TD>Name:<TD><input type='text' maxlength='25' name='taskdevicename' value='");
      reply += ExtraTaskSettings.TaskDeviceName;

      if (Device[DeviceIndex].Ports != 0)
      {
        reply += F("'><TR><TD>Port:<TD><input type='text' name='taskdeviceport' value='");
        reply += Settings.TaskDevicePort[index - 1];
      }
      reply += F("'><TR><TD>IDX / Var:<TD><input type='text' name='taskdeviceid' value='");
      reply += Settings.TaskDeviceID[index - 1];
      reply += F("'>");

      if (Device[DeviceIndex].Type == DEVICE_TYPE_SINGLE || Device[DeviceIndex].Type == DEVICE_TYPE_DUAL)
      {
        reply += F("<TR><TD>1st GPIO:<TD>");
        addPinSelect(false, reply, "taskdevicepin1", Settings.TaskDevicePin1[index - 1]);
      }
      if (Device[DeviceIndex].Type == DEVICE_TYPE_DUAL)
      {
        reply += F("<TR><TD>2nd GPIO:<TD>");
        addPinSelect(false, reply, "taskdevicepin2", Settings.TaskDevicePin2[index - 1]);
      }

      if (Device[DeviceIndex].PullUpOption)
      {
        reply += F("<TR><TD>Pull UP:<TD>");
        if (Settings.TaskDevicePin1PullUp[index - 1])
          reply += F("<input type=checkbox name=taskdevicepin1pullup checked>");
        else
          reply += F("<input type=checkbox name=taskdevicepin1pullup>");
      }

      if (Device[DeviceIndex].InverseLogicOption)
      {
        reply += F("<TR><TD>Inversed:<TD>");
        if (Settings.TaskDevicePin1Inversed[index - 1])
          reply += F("<input type=checkbox name=taskdevicepin1inversed checked>");
        else
          reply += F("<input type=checkbox name=taskdevicepin1inversed>");
      }

#ifdef ESP_EASY
      PluginCall(PLUGIN_WEBFORM_LOAD, &TempEvent, reply);
#endif

#ifdef ESP_CONNEXIO
      struct NodoEventStruct TempEvent;
      char tmpString[256];
      tmpString[0] = 0;
      TempEvent.Par1 = index - 1;
      PluginCall(PLUGIN_WEBFORM_LOAD, &TempEvent, tmpString);
      if (tmpString[0] != 0)
        reply += tmpString;
#endif

      reply += F("<TR><TH>Optional Settings<TH>Value");

      if (Device[DeviceIndex].FormulaOption)
      {
        for (byte varNr = 0; varNr < Device[DeviceIndex].ValueCount; varNr++)
        {
          reply += F("<TR><TD>Formula ");
          reply += ExtraTaskSettings.TaskDeviceValueNames[varNr];
          reply += F(":<TD><input type='text' maxlength='25' name='taskdeviceformula");
          reply += varNr + 1;
          reply += F("' value='");
          reply += ExtraTaskSettings.TaskDeviceFormula[varNr];
          reply += F("'>");
          if (varNr == 0)
            reply += F("<a class=\"button-link\" href=\"http://www.esp8266.nu/index.php/EasyFormula\" target=\"_blank\">?</a>");
        }
      }

      for (byte varNr = 0; varNr < Device[DeviceIndex].ValueCount; varNr++)
      {
        reply += F("<TR><TD>Value Name ");
        reply += varNr + 1;
        reply += F(":<TD><input type='text' maxlength='25' name='taskdevicevaluename");
        reply += varNr + 1;
        reply += F("' value='");
        reply += ExtraTaskSettings.TaskDeviceValueNames[varNr];
        reply += F("'>");
      }


    }
    reply += F("<TR><TD><TD><a class=\"button-link\" href=\"devices\">Close</a>");
    reply += F("<input class=\"button-link\" type='submit' value='Submit'><TR><TD>");
    reply += F("<input type='hidden' name='edit' value='1'>");
    reply += F("<input type='hidden' name='page' value='1'>");
    reply += F("</table></form>");
  }

  addFooter(reply);
  WebServer.send(200, "text/html", reply);
}


//********************************************************************************
// Web Interface hardware page
//********************************************************************************
void handle_hardware() {
  if (!isLoggedIn()) return;

  String pin_i2c_sda = WebServer.arg("pini2csda");
  String pin_i2c_scl = WebServer.arg("pini2cscl");

  if (pin_i2c_sda.length() != 0)
  {
    Settings.Pin_i2c_sda     = pin_i2c_sda.toInt();
    Settings.Pin_i2c_scl     = pin_i2c_scl.toInt();
    SaveSettings();
  }

  String reply = "";
  addMenu(reply);

  reply += F("<form  method='post'><table><TH>Hardware Settings<TH><TR><TD>");
  reply += F("<TR><TD>SDA:<TD>");
  addPinSelect(true, reply, "pini2csda", Settings.Pin_i2c_sda);
  reply += F("<TR><TD>SCL:<TD>");
  addPinSelect(true, reply, "pini2cscl", Settings.Pin_i2c_scl);

  reply += F("<TR><TD><TD><input class=\"button-link\" type='submit' value='Submit'><TR><TD>");

  reply += F("</table></form>");
  addFooter(reply);
  WebServer.send(200, "text/html", reply);
}


//********************************************************************************
// Add a device select dropdown list
//********************************************************************************
void addDeviceSelect(String& str, String name,  int choice)
{
  struct EventStruct TempEvent;
  String deviceName;

  str += F("<select name='");
  str += name;
  //str += "'>";
  str += "' LANGUAGE=javascript onchange=\"return dept_onchange(frmselect)\">";


  str += F("<option value='0'></option>");
  for (byte x = 0; x <= deviceCount; x++)
  {
    if (Plugin_id[x] != 0)
      Plugin_ptr[x](PLUGIN_GET_DEVICENAME, &TempEvent, deviceName);
    str += F("<option value='");
    str += Device[x].Number;
    str += "'";
    if (choice == Device[x].Number)
      str += F(" selected");
    str += ">";
    str += deviceName;
    str += F("</option>");
  }
  str += F("</select>");
}


//********************************************************************************
// Add a GPIO pin select dropdown list
//********************************************************************************
void addPinSelect(boolean forI2C, String& str, String name,  int choice)
{
  String options[12];
  options[0] = F(" ");
  options[1] = F("GPIO-0");
  options[2] = F("GPIO-2");
  options[3] = F("GPIO-4");
  options[4] = F("GPIO-5");
  options[5] = F("GPIO-9");
  options[6] = F("GPIO-10");
  options[7] = F("GPIO-12");
  options[8] = F("GPIO-13");
  options[9] = F("GPIO-14");
  options[10] = F("GPIO-15");
  options[11] = F("GPIO-16");
  int optionValues[12];
  optionValues[0] = -1;
  optionValues[1] = 0;
  optionValues[2] = 2;
  optionValues[3] = 4;
  optionValues[4] = 5;
  optionValues[5] = 9;
  optionValues[6] = 10;
  optionValues[7] = 12;
  optionValues[8] = 13;
  optionValues[9] = 14;
  optionValues[10] = 15;
  optionValues[11] = 16;
  str += F("<select name='");
  str += name;
  str += "'>";
  for (byte x = 0; x < 12; x++)
  {
    str += F("<option value='");
    str += optionValues[x];
    str += "'";
    if (!forI2C && ((optionValues[x] == Settings.Pin_i2c_sda) || (optionValues[x] == Settings.Pin_i2c_scl)))
      str += F(" disabled");
    if (choice == optionValues[x])
      str += F(" selected");
    str += ">";
    str += options[x];
    str += F("</option>");
  }
  str += F("</select>");
}


#ifdef ESP_CONNEXIO
//********************************************************************************
// Web Interface eventlist page
//********************************************************************************
void handle_eventlist() {
  if (!isLoggedIn()) return;

  char *TempString = (char*)malloc(80);
  String reply = "";
  addMenu(reply);

  reply += F("<table><TH>Eventlist<td><TR><TD>");

  if (WebServer.args() == 1)
  {

    struct NodoEventStruct TempEvent;
    ClearEvent(&TempEvent);
    byte x = 1;
    while (Eventlist_Write(x++, 0, &TempEvent, &TempEvent)) delay(1);

    String eventlist = WebServer.arg("eventlist");
    eventlist.replace("%0D%0A", "\n");
    int NewLineIndex = eventlist.indexOf('\n');
    byte limit = 0;
    byte messagecode = 0;
    while ((NewLineIndex > 0) && (limit < EventlistMax))
    {
      limit++;
      String line = eventlist.substring(0, NewLineIndex);
      String strCommand = F("eventlistwrite 0,");
      strCommand += line;
      strCommand.toCharArray(TempString, 80);
      urlDecode(TempString);

      messagecode = ExecuteLine(TempString, VALUE_SOURCE_SERIAL);
      if (messagecode > 0)
      {
        reply += TempString;
        reply += " : ";
        reply += MessageText_tabel[messagecode];
        reply += "<BR>";
      }
      eventlist = eventlist.substring(NewLineIndex + 1);
      NewLineIndex = eventlist.indexOf('\n');
    }
    EEPROM.commit();
  }

  reply += F("<form method='post'>");
  reply += F("<TD><textarea name='eventlist' rows='15' cols='80' wrap='on'>");
  byte x = 1;
  while (EventlistEntry2str(x++, 0, TempString, false))
    if (TempString[0] != 0)
    {
      reply += TempString;
      reply += '\n';
    }

  reply += F("</textarea>");

  reply += F("<TR><TD><TD><input class=\"button-link\" type='submit' value='Submit'>");
  reply += F("</table></form>");
  addFooter(reply);
  WebServer.send(200, "text/html", reply);
  free(TempString);
}
#endif


//********************************************************************************
// Web Interface log page
//********************************************************************************
void handle_log() {
  if (!isLoggedIn()) return;

  char *TempString = (char*)malloc(80);

  String reply = "";
  addMenu(reply);
  reply += F("<script language='JavaScript'>function RefreshMe(){window.location = window.location}setTimeout('RefreshMe()', 3000);</script>");
  reply += F("<table><TH>Log<TR><TD>");

  if (logcount != -1)
  {
    byte counter = logcount;
    do
    {
      counter++;
      if (counter > 9)
        counter = 0;
      if (Logging[counter].timeStamp > 0)
      {
        reply += Logging[counter].timeStamp;
        reply += " : ";
        reply += Logging[counter].Message;
        reply += F("<BR>");
      }
    }  while (counter != logcount);
  }
  reply += F("</table>");
  addFooter(reply);
  WebServer.send(200, "text/html", reply);
  free(TempString);
}


//********************************************************************************
// Web Interface debug page
//********************************************************************************
void handle_tools() {
  if (!isLoggedIn()) return;

  String webrequest = WebServer.arg("cmd");
  char command[80];
  command[0] = 0;
  webrequest.toCharArray(command, 80);
  urlDecode(command);

  String reply = "";
  addMenu(reply);

  reply += F("<form>");
  reply += F("<table><TH>Tools<TH>");
  reply += F("<TR><TD>System<TD><a class=\"button-link\" href=\"/?cmd=reboot\">Reboot</a>");
  reply += F("<a class=\"button-link\" href=\"log\">Log</a>");
  reply += F("<a class=\"button-link\" href=\"advanced\">Advanced</a><BR><BR>");
  reply += F("<TR><TD>Wifi<TD><a class=\"button-link\" href=\"/?cmd=wificonnect\">Connect</a>");
  reply += F("<a class=\"button-link\" href=\"/?cmd=wifidisconnect\">Disconnect</a>");
  reply += F("<a class=\"button-link\" href=\"/wifiscanner\">Scan</a><BR><BR>");
  reply += F("<TR><TD>Interfaces<TD><a class=\"button-link\" href=\"/i2cscanner\">I2C Scan</a><BR><BR>");
  reply += F("<TR><TD>Settings<TD><a class=\"button-link\" href=\"/upload\">Load</a>");
  reply += F("<a class=\"button-link\" href=\"/download\">Save</a>");

  #if FEATURE_SPIFFS
    reply += F("<a class=\"button-link\" href=\"/filelist\">List</a><BR><BR>");
  #else
    reply += F("<BR><BR>");
  #endif

  reply += F("<TR><TD>Command<TD>");
  reply += F("<input type='text' name='cmd' value='");
  reply += command;
  reply += F("'><TR><TD><TD><input class=\"button-link\" type='submit' value='Submit'><TR><TD>");

  printToWeb = true;
  printWebString = "<BR>";
#ifdef ESP_CONNEXIO
  ExecuteLine(command, VALUE_SOURCE_SERIAL);
#endif
#ifdef ESP_EASY
  ExecuteCommand(command);
#endif
  reply += printWebString;
  reply += F("</table></form>");
  addFooter(reply);
  WebServer.send(200, "text/html", reply);
  printWebString = "";
  printToWeb = false;
}


//********************************************************************************
// Web Interface I2C scanner
//********************************************************************************
void handle_i2cscanner() {
  if (!isLoggedIn()) return;

  char *TempString = (char*)malloc(80);

  String reply = "";
  addMenu(reply);
  reply += F("<table><TH>I2C Addresses in use<TH>Known devices");

  byte error, address;
  int nDevices;
  nDevices = 0;
  for (address = 1; address <= 127; address++ )
  {
    Wire.beginTransmission(address);
    error = Wire.endTransmission();
    if (error == 0)
    {
      reply += "<TR><TD>0x";
      reply += String(address, HEX);
      reply += "<TD>";
      switch (address)
      {
        case 0x20:
        case 0x27:
          reply += F("PCF8574, MCP23017, LCD Modules");
          break;
        case 0x23:
          reply += F("BH1750 Lux Sensor");
          break;
        case 0x48:
          reply += F("PCF8591 ADC");
          break;
        case 0x68:
          reply += F("DS1307 RTC");
          break;
        case 0x77:
          reply += F("BMP085");
          break;
        case 0x7f:
          reply += F("Arduino Pro Mini IO Extender");
          break;
      }
      nDevices++;
    }
    else if (error == 4)
    {
      reply += F("<TR><TD>Unknow error at address 0x");
      reply += String(address, HEX);
    }
  }

  if (nDevices == 0)
    reply += F("<TR>No I2C devices found");

  reply += F("</table>");
  addFooter(reply);
  WebServer.send(200, "text/html", reply);
  free(TempString);
}


//********************************************************************************
// Web Interface Wifi scanner
//********************************************************************************
void handle_wifiscanner() {
  if (!isLoggedIn()) return;

  char *TempString = (char*)malloc(80);

  String reply = "";
  addMenu(reply);
  reply += F("<table><TH>Access Points:<TH>RSSI");

  int n = WiFi.scanNetworks();
  if (n == 0)
    reply += F("No Access Points found");
  else
  {
    for (int i = 0; i < n; ++i)
    {
      reply += F("<TR><TD>");
      reply += WiFi.SSID(i);
      reply += "<TD>";
      reply += WiFi.RSSI(i);
    }
  }


  reply += F("</table>");
  addFooter(reply);
  WebServer.send(200, "text/html", reply);
  free(TempString);
}


//********************************************************************************
// Web Interface login page
//********************************************************************************
void handle_login() {

  String webrequest = WebServer.arg("password");
  char command[80];
  command[0] = 0;
  webrequest.toCharArray(command, 80);
  urlDecode(command);

  String reply = "";
  reply += F("<form method='post'>");
  reply += F("<table><TR><TD>Password<TD>");
  reply += F("<input type='password' name='password' value='");
  reply += webrequest;
  reply += F("'><TR><TD><TD><input class=\"button-link\" type='submit' value='Submit'><TR><TD>");
  reply += F("</table></form>");

  if (webrequest.length() != 0)
  {
    // compare with stored password and set timer if there's a match
    if ((strcasecmp(command, SecuritySettings.Password) == 0) || (SecuritySettings.Password[0] == 0))
    {
      WebLoggedIn = true;
      WebLoggedInTimer = 0;
      reply = F("<script language='JavaScript'>window.location = '.'</script>");
    }
    else
    {
      reply += F("Invalid password!");
    }
  }

  WebServer.send(200, "text/html", reply);
  printWebString = "";
  printToWeb = false;
}


//********************************************************************************
// Web Interface control page (no password!)
//********************************************************************************
void handle_control() {

  String webrequest = WebServer.arg("cmd");
  char command[80];
  command[0] = 0;
  webrequest.toCharArray(command, 80);
  urlDecode(command);
  boolean validCmd = false;

  struct EventStruct TempEvent;
  char TmpStr1[80];
  TmpStr1[0] = 0;
  TempEvent.Par1 = 0;
  TempEvent.Par2 = 0;
  TempEvent.Par3 = 0;

  char Cmd[40];
  Cmd[0] = 0;
  GetArgv(command, Cmd, 1);
  if (GetArgv(command, TmpStr1, 2)) TempEvent.Par1 = str2int(TmpStr1);
  if (GetArgv(command, TmpStr1, 3)) TempEvent.Par2 = str2int(TmpStr1);
  if (GetArgv(command, TmpStr1, 4)) TempEvent.Par3 = str2int(TmpStr1);

  printToWeb = true;
  printWebString = "";
  String reply = "";

  if (!PluginCall(PLUGIN_WRITE, &TempEvent, webrequest))
    reply += F("Unknown or restricted command!");

  reply += printWebString;
  reply += F("</table></form>");
  WebServer.send(200, "text/html", reply);
  printWebString = "";
  printToWeb = false;
}


//********************************************************************************
// Web Interface config page
//********************************************************************************
void handle_advanced() {
  if (!isLoggedIn()) return;

  char tmpString[81];

  String mqttsubscribe = WebServer.arg("mqttsubscribe");
  String mqttpublish = WebServer.arg("mqttpublish");
  String messagedelay = WebServer.arg("messagedelay");
  String ip = WebServer.arg("ip");
  String syslogip = WebServer.arg("syslogip");
  String sysloglevel = WebServer.arg("sysloglevel");
  String udpport = WebServer.arg("udpport");
  String serialloglevel = WebServer.arg("serialloglevel");
  String webloglevel = WebServer.arg("webloglevel");
  String baudrate = WebServer.arg("baudrate");
  #if !FEATURE_SPIFFS
    String customcss = WebServer.arg("customcss");
  #endif
  String edit = WebServer.arg("edit");

  if (edit.length() != 0)
  {
    mqttsubscribe.toCharArray(tmpString, 81);
    urlDecode(tmpString);
    strcpy(Settings.MQTTsubscribe, tmpString);
    mqttpublish.toCharArray(tmpString, 81);
    urlDecode(tmpString);
    strcpy(Settings.MQTTpublish, tmpString);
    Settings.MessageDelay = messagedelay.toInt();
    Settings.IP_Octet = ip.toInt();
    syslogip.toCharArray(tmpString, 26);
    str2ip(tmpString, Settings.Syslog_IP);
    Settings.UDPPort = udpport.toInt();
    Settings.SyslogLevel = sysloglevel.toInt();
    Settings.SerialLogLevel = serialloglevel.toInt();
    Settings.WebLogLevel = webloglevel.toInt();
    Settings.BaudRate = baudrate.toInt();
    #if !FEATURE_SPIFFS
      Settings.CustomCSS = (customcss == "on");
    #endif
    SaveSettings();
  }

  String reply = "";
  addMenu(reply);

  char str[20];

  reply += F("<form  method='post'><table>");
  reply += F("<TH>Advanced Settings<TH>Value");

  reply += F("<TR><TD>MQTT Subscribe Template:<TD><input type='text' name='mqttsubscribe' size=80 value='");
  reply += Settings.MQTTsubscribe;

  reply += F("'><TR><TD>MQTT Publish Template:<TD><input type='text' name='mqttpublish' size=80 value='");
  reply += Settings.MQTTpublish;

  reply += F("'><TR><TD>Message Delay (ms):<TD><input type='text' name='messagedelay' value='");
  reply += Settings.MessageDelay;

  reply += F("'><TR><TD>Fixed IP Octet:<TD><input type='text' name='ip' value='");
  reply += Settings.IP_Octet;

  reply += F("'><TR><TD>Syslog IP:<TD><input type='text' name='syslogip' value='");
  str[0] = 0;
  sprintf_P(str, PSTR("%u.%u.%u.%u"), Settings.Syslog_IP[0], Settings.Syslog_IP[1], Settings.Syslog_IP[2], Settings.Syslog_IP[3]);
  reply += str;

  reply += F("'><TR><TD>Syslog Level:<TD><input type='text' name='sysloglevel' value='");
  reply += Settings.SyslogLevel;

  reply += F("'><TR><TD>UDP port:<TD><input type='text' name='udpport' value='");
  reply += Settings.UDPPort;

  reply += F("'><TR><TD>Serial log Level:<TD><input type='text' name='serialloglevel' value='");
  reply += Settings.SerialLogLevel;

  reply += F("'><TR><TD>Web log Level:<TD><input type='text' name='webloglevel' value='");
  reply += Settings.WebLogLevel;

  reply += F("'><TR><TD>Baud Rate:<TD><input type='text' name='baudrate' value='");
  reply += Settings.BaudRate;
  reply += F("'>");

  #if !FEATURE_SPIFFS
  reply += F("<TR><TD>Custom CSS:<TD>");
  if (Settings.CustomCSS)
    reply += F("<input type=checkbox name='customcss' checked>");
  else
    reply += F("<input type=checkbox name='customcss'>");
  #endif
  
  reply += F("<TR><TD><TD><input class=\"button-link\" type='submit' value='Submit'>");
  reply += F("<input type='hidden' name='edit' value='1'>");
  reply += F("</table></form>");
  addFooter(reply);
  WebServer.send(200, "text/html", reply);
}


//********************************************************************************
// Login state check
//********************************************************************************
boolean isLoggedIn()
{
  if (SecuritySettings.Password[0] == 0)
    WebLoggedIn = true;

  if (!WebLoggedIn)
  {
    String reply = F("<a class=\"button-link\" href=\"login\">Login</a>");
    WebServer.send(200, "text/html", reply);
  }
  else
  {
    WebLoggedInTimer = 0;
  }

  return WebLoggedIn;
}

//********************************************************************************
// Decode special characters in URL of get/post data
//********************************************************************************
void urlDecode(char *src)
{
  char* dst = src;
  char a, b;
  while (*src) {

    if (*src == '+')
      *src = ' ';

    if ((*src == '%') &&
        ((a = src[1]) && (b = src[2])) &&
        (isxdigit(a) && isxdigit(b))) {
      if (a >= 'a')
        a -= 'a' - 'A';
      if (a >= 'A')
        a -= ('A' - 10);
      else
        a -= '0';
      if (b >= 'a')
        b -= 'a' - 'A';
      if (b >= 'A')
        b -= ('A' - 10);
      else
        b -= '0';
      *dst++ = 16 * a + b;
      src += 3;
    }
    else {
      *dst++ = *src++;
    }
  }
  *dst++ = '\0';
}

#if FEATURE_SPIFFS
//********************************************************************************
// Web Interface download page
//********************************************************************************
void handle_download()
{
  if (!isLoggedIn()) return;

  File dataFile = SPIFFS.open("config.txt", "r");
  if (!dataFile)
    return;

  WebServer.sendHeader("Content-Disposition", "attachment; filename=config.txt");
  WebServer.streamFile(dataFile, "application/octet-stream");
}


//********************************************************************************
// Web Interface upload page
//********************************************************************************
byte uploadResult = 0;
void handle_upload() {
  if (!isLoggedIn()) return;

  String edit = WebServer.arg("edit");

  String reply = "";
  addMenu(reply);

  if (edit.length() != 0)
  {
    if (uploadResult == 1)
    {
      reply += F("Upload OK!<BR>You may need to reboot to apply all settings...");
      LoadSettings();
    }

    if (uploadResult == 2)
      reply += F("<font color=\"red\">Upload file invalid!</font>");

    if (uploadResult == 3)
      reply += F("<font color=\"red\">No filename!</font>");
  }

  reply += F("<form enctype=\"multipart/form-data\" method=\"post\"><p>Upload settings file:<br><input type=\"file\" name=\"datafile\" size=\"40\"></p><div><input class=\"button-link\" type='submit' value='Upload'></div><input type='hidden' name='edit' value='1'></form>");
  addFooter(reply);
  WebServer.send(200, "text/html", reply);
  printWebString = "";
  printToWeb = false;
}

//********************************************************************************
// Web Interface upload handler
//********************************************************************************
File uploadFile;
void handleFileUpload() {
  if (!isLoggedIn()) return;

  static boolean valid = false;

  HTTPUpload& upload = WebServer.upload();

  if (upload.filename.c_str()[0] == 0)
  {
    uploadResult = 3;
    return;
  }

  if (upload.status == UPLOAD_FILE_START)
  {
    Serial.print(F("Upload: START, filename: ")); Serial.println(upload.filename);
    valid = false;
    uploadResult = 0;
  }
  else if (upload.status == UPLOAD_FILE_WRITE)
  {
    // first data block, if this is the config file, check PID/Version
    if (upload.totalSize == 0)
    {
      if (strcasecmp(upload.filename.c_str(), "config.txt") == 0)
      {
        struct TempStruct {
          unsigned long PID;
          int Version;
        } Temp;
        for (int x = 0; x < sizeof(struct TempStruct); x++)
        {
          byte b = upload.buf[x];
          memcpy((byte*)&Temp + x, &b, 1);
        }
        Serial.println(Temp.PID);
        Serial.println(Temp.Version);
        if (Temp.Version == VERSION && Temp.PID == ESP_PROJECT_PID)
          valid = true;
      }
      else
      {
        // other files are always valid...
        valid = true;
      }
      if (valid)
      {
        Serial.println(F("Create file"));
        // once we're safe, remove file and create empty one...
        SPIFFS.remove((char *)upload.filename.c_str());
        uploadFile = SPIFFS.open(upload.filename.c_str(), "w");
      }
    }
    if (uploadFile) uploadFile.write(upload.buf, upload.currentSize);
    Serial.print(F("Upload: WRITE, Bytes: ")); Serial.println(upload.currentSize);
  }
  else if (upload.status == UPLOAD_FILE_END)
  {
    if (uploadFile) uploadFile.close();
    Serial.print(F("Upload: END, Size: ")); Serial.println(upload.totalSize);
  }

  if (valid)
    uploadResult = 1;
  else
    uploadResult = 2;

}


//********************************************************************************
// Web Interface server web file from SPIFFS
//********************************************************************************
bool loadFromSPIFFS(String path) {
  if (!isLoggedIn()) return false;

  String dataType = F("text/plain");
  if (path.endsWith("/")) path += "index.htm";

  if (path.endsWith(".src")) path = path.substring(0, path.lastIndexOf("."));
  else if (path.endsWith(".htm")) dataType = F("text/html");
  else if (path.endsWith(".css")) dataType = F("text/css");
  else if (path.endsWith(".js")) dataType = F("application/javascript");
  else if (path.endsWith(".png")) dataType = F("image/png");
  else if (path.endsWith(".gif")) dataType = F("image/gif");
  else if (path.endsWith(".jpg")) dataType = F("image/jpeg");
  else if (path.endsWith(".ico")) dataType = F("image/x-icon");
  else if (path.endsWith(".txt")) dataType = F("application/octet-stream");

  path = path.substring(1);
  File dataFile = SPIFFS.open(path.c_str(), "r");

  if (!dataFile)
    return false;

  if (path.endsWith(".txt"))
    WebServer.sendHeader("Content-Disposition", "attachment;");
  WebServer.streamFile(dataFile, dataType);

  dataFile.close();
  return true;
}

//********************************************************************************
// Web Interface handle other requests
//********************************************************************************
void handleNotFound() {
  if (!isLoggedIn()) return;
  if (loadFromSPIFFS(WebServer.uri())) return;
  String message = "URI: ";
  message += WebServer.uri();
  message += "\nMethod: ";
  message += (WebServer.method() == HTTP_GET) ? "GET" : "POST";
  message += "\nArguments: ";
  message += WebServer.args();
  message += "\n";
  for (uint8_t i = 0; i < WebServer.args(); i++) {
    message += " NAME:" + WebServer.argName(i) + "\n VALUE:" + WebServer.arg(i) + "\n";
  }
  WebServer.send(404, "text/plain", message);
}


//********************************************************************************
// Web Interface file list)
//********************************************************************************
void handle_filelist() {

  String fdelete = WebServer.arg("delete");

  if (fdelete.length() >0)
    {
      SPIFFS.remove(fdelete);
    }
    
  String reply = "";
  addMenu(reply);
  reply += F("<table border='1'><TH><TH>Filename:<TH>Size");

  Dir dir = SPIFFS.openDir("/");
  while (dir.next())
  {
    reply += F("<TR><TD>");
    if (dir.fileName() != "config.txt" && dir.fileName() !="security.txt")
      {
      reply += F("<a class=\"button-link\" href=\"filelist?delete=");
      reply += dir.fileName();
      reply += F("\">Del</a>");
      }
      
    reply += F("<TD><a href=\"");
    reply += dir.fileName();
    reply += F("\">");
    reply += dir.fileName();
    reply += F("</a>");
    File f = dir.openFile("r");
    reply += F("<TD>");
    reply += f.size();
  }
  reply += F("</table></form>");
  addFooter(reply);
  WebServer.send(200, "text/html", reply);
}

#else

// Without spiffs support, we will use our own handlers to manage a 33kb flash area
// it uses the same space as where SPIFFS would reside
// first 32kB is used to store settings as uses in "config.txt"
// last 4kB block is used for security settings. These cannot be downloaded/uploaded.
// the config.txt can be interchanged between using spiffs or this custom method

//********************************************************************************
// Web Interface download page
//********************************************************************************
void handle_download() {
  if (!isLoggedIn()) return;

  uint32_t _sectorStart = ((uint32_t)&_SPIFFS_start - 0x40200000) / SPI_FLASH_SEC_SIZE;
  uint32_t _sectorEnd = _sectorStart + 32; //((uint32_t)&_SPIFFS_end - 0x40200000) / SPI_FLASH_SEC_SIZE;
  uint8_t* data = new uint8_t[FLASH_EEPROM_SIZE];

  WiFiClient client = WebServer.client();
  /*client.print("HTTP/1.1 200 OK\r\n");
  client.print("Content-Disposition: attachment; filename=config.txt\r\n");
  client.print("Content-Type: application/octet-stream\r\n");
  client.print("Content-Length: 32768\r\n");
  client.print("Connection: close\r\n");
  client.print("Access-Control-Allow-Origin: *\r\n");
  client.print("\r\n");
  */
  WebServer.setContentLength(32768);
  WebServer.sendHeader("Content-Disposition", "attachment; filename=config.txt");
  WebServer.send(200, "application/octet-stream", "");
  
  for (uint32_t _sector=_sectorStart; _sector < _sectorEnd; _sector++)
  {
    // load entire sector from flash into memory
    noInterrupts();
    spi_flash_read(_sector * SPI_FLASH_SEC_SIZE, reinterpret_cast<uint32_t*>(data), FLASH_EEPROM_SIZE);
    interrupts();
    client.write((const char*)data, 2048);
    client.write((const char*)data+2028, 2048);
  }
  delete [] data;
}


//********************************************************************************
// Web Interface download page
//********************************************************************************
void handle_css() {
  if (!isLoggedIn()) return;

  int size=0;
  uint32_t _sector = ((uint32_t)&_SPIFFS_start - 0x40200000) / SPI_FLASH_SEC_SIZE;
  uint8_t* data = new uint8_t[FLASH_EEPROM_SIZE];
  _sector += 9;

  // load entire sector from flash into memory
  noInterrupts();
  spi_flash_read(_sector * SPI_FLASH_SEC_SIZE, reinterpret_cast<uint32_t*>(data), FLASH_EEPROM_SIZE);
  interrupts();

  // check size of css file content
  for (int x=0; x < 4096; x++)
    if (data[x] == 0)
      {
        size=x;
        break;
      }
  WiFiClient client = WebServer.client();
  WebServer.setContentLength(size);
  WebServer.send(200, "text/css", "");
  client.write((const char*)data, size);
  delete [] data;
}


//********************************************************************************
// Web Interface upload page
//********************************************************************************
void handle_upload() {
  if (!isLoggedIn()) return;

  String reply = "";
  addMenu(reply);
  reply += F("<form enctype=\"multipart/form-data\" method=\"post\"><p>Upload settings:<br><input type=\"file\" name=\"datafile\" size=\"40\"></p><div><input class=\"button-link\" type='submit' value='Upload'></div></form>");
  addFooter(reply);
  WebServer.send(200, "text/html", reply);
  printWebString = "";
  printToWeb = false;
}


//********************************************************************************
// Upload handler
//********************************************************************************
void handleFileUpload()
{
  if (!isLoggedIn()) return;

  static byte filetype = 0;
  static byte page = 0;
  static uint8_t* data;
  int uploadSize = 0;
  HTTPUpload& upload = WebServer.upload();

  if (upload.status == UPLOAD_FILE_START)
  {
    filetype=0;
    if (strcasecmp(upload.filename.c_str(), "config.txt") == 0)
      filetype=1;
    if (strcasecmp(upload.filename.c_str(), "esp.css") == 0)
      {
        filetype=2;
        Settings.CustomCSS=true;
      }
    Serial.print(F("Upload start "));
    Serial.println((char *)upload.filename.c_str());
    page = 0;
    data = new uint8_t[FLASH_EEPROM_SIZE];
  }

  if (upload.status == UPLOAD_FILE_WRITE && filetype !=0)
  {
    uploadSize = upload.currentSize;
    
    int base=0;
    if (page % 2)
      base +=2048;
    memcpy((byte*)data + base, upload.buf, upload.currentSize);
    if (filetype == 2)
      data[upload.currentSize + upload.totalSize]=0; // eof marker

    if ((page % 2) || (filetype == 2))
    {
      byte sectorOffset = 0;
      if (filetype == 2)
        sectorOffset = 9;
      uint32_t _sector = ((uint32_t)&_SPIFFS_start - 0x40200000) / SPI_FLASH_SEC_SIZE;
      _sector += page/2;
      _sector += sectorOffset;
      noInterrupts();
      if(spi_flash_erase_sector(_sector) == SPI_FLASH_RESULT_OK)
        if(spi_flash_write(_sector * SPI_FLASH_SEC_SIZE, reinterpret_cast<uint32_t*>(data), FLASH_EEPROM_SIZE) == SPI_FLASH_RESULT_OK)
          {
            //Serial.println("flash save ok");
          }
      interrupts();
      delay(10);
    }
    page++;
  }

  if (upload.status == UPLOAD_FILE_END)
  {
    Serial.println(F("Upload end"));
    delete [] data;
    if (filetype == 1)
      LoadSettings();
    if (filetype == 2)
      SaveSettings();
  }
}
#endif

