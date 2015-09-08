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
  WebServer.on("/json.htm", handle_json);
#ifdef ESP_CONNEXIO
  WebServer.on("/eventlist", handle_eventlist);
#endif
  WebServer.on("/log", handle_log);
  WebServer.on("/tools", handle_tools);
  WebServer.on("/i2cscanner", handle_i2cscanner);
  WebServer.on("/wifiscanner", handle_wifiscanner);
  WebServer.on("/login", handle_login);
  WebServer.on("/control", handle_control);
  WebServer.begin();
}


//********************************************************************************
// Add top menu
//********************************************************************************
void addMenu(String& str)
{
  // Inline style definitions
  str += F("<head><title>");
  str += Settings.Name;
  str += F("</title></head>");
  str += F("<style>");
  str += F("* {font-family:sans-serif; font-size:12pt;}");
  str += F("h1 {font-size:16pt; border:1px solid #333; color:#ffffff; background:#27f;}");
  str += F(".button-link {padding:2px 10px; background:#5bf; color:#fff; border-radius:4px; border:solid 1px #258; text-decoration:none}");
  str += F(".button-link:hover {background:#369;}");
  str += F("</style>");

  str += F("<h1>Welcome to ESP ");
#ifdef ESP_CONNEXIO
  str += F("Connexio : ");
#endif
#ifdef ESP_EASY
  str += F("Easy : ");
#endif
  str += Settings.Name;
  str += F("</h1><a class=\"button-link\" href=\".\">Main</a>");
  str += F("<a class=\"button-link\" href=\"config\">Config</a>");
  str += F("<a class=\"button-link\" href=\"hardware\">Hardware</a>");
  str += F("<a class=\"button-link\" href=\"devices\">Devices</a>");
#ifdef ESP_CONNEXIO
  str += F("<a class=\"button-link\" href=\"eventlist\">Eventlist</a>");
#endif
  str += F("<a class=\"button-link\" href=\"log\">Log</a>");
  str += F("<a class=\"button-link\" href=\"tools\">Tools</a><BR><BR>");
}


//********************************************************************************
// Add footer to web page
//********************************************************************************
void addFooter(String& str)
{
  str += F("<h1>Powered by www.esp8266.nu</h1></body>");
}


//********************************************************************************
// Web Interface root page
//********************************************************************************
void handle_root() {
  if (!isLoggedIn()) return;

  int freeMem = ESP.getFreeHeap();
  if (Settings.SerialLogLevel >= LOG_LEVEL_DEBUG)
    Serial.print(F("HTTP : Webrequest : "));
  String webrequest = WebServer.arg("cmd");
  webrequest.replace("%20", " ");
  if (Settings.SerialLogLevel >= LOG_LEVEL_DEBUG)
    Serial.println(webrequest);
  char command[80];
  command[0] = 0;
  webrequest.toCharArray(command, 80);

  if ((strcasecmp(command, "wifidisconnect") != 0) && (strcasecmp(command, "reboot") != 0))
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
    reply += F("<table bgcolor='#ddeeff'><tr bgcolor='#55bbff'><td>System Info<TD><TD><TR><TD>");

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

    reply += F("<TR bgcolor='#55bbff'><TD>Node List:<TD>IP<TD>Age<TR><TD><TD>");
    for (byte x = 0; x < 32; x++)
    {
      if (Nodes[x].ip[0] != 0)
      {
        if (x == Settings.Unit)
          reply += "<font color='blue'>";
        char url[80];
        sprintf_P(url, PSTR("<a href='http://%u.%u.%u.%u'>%u.%u.%u.%u</a>"), Nodes[x].ip[0], Nodes[x].ip[1], Nodes[x].ip[2], Nodes[x].ip[3], Nodes[x].ip[0], Nodes[x].ip[1], Nodes[x].ip[2], Nodes[x].ip[3]);
        reply += "<TR><TD>Unit ";
        reply += x;
        reply += ":<TD>";
        reply += url;
        reply += "<TD>";
        reply += Nodes[x].age;
        if (x == Settings.Unit)
          reply += "</font color>";
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
    if (strcasecmp(command, "wifidisconnect") == 0)
    {
      Serial.println(F("WIFI : Disconnecting..."));
      cmd_within_mainloop = CMD_WIFI_DISCONNECT;
    }

    if (strcasecmp(command, "reboot") == 0)
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

  char tmpstring[64];

  if (Settings.SerialLogLevel >= LOG_LEVEL_DEBUG)
    Serial.println(F("HTTP : Webconfig"));

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
  String messagedelay = WebServer.arg("messagedelay");
  String ip = WebServer.arg("ip");
  String espip = WebServer.arg("espip");
  String espgateway = WebServer.arg("espgateway");
  String espsubnet = WebServer.arg("espsubnet");
  String unit = WebServer.arg("unit");
  String apkey = WebServer.arg("apkey");
  String syslogip = WebServer.arg("syslogip");
  String sysloglevel = WebServer.arg("sysloglevel");
  String udpport = WebServer.arg("udpport");
  String serialloglevel = WebServer.arg("serialloglevel");
  String webloglevel = WebServer.arg("webloglevel");
  String baudrate = WebServer.arg("baudrate");

  if (ssid[0] != 0)
  {
    name.replace("+", " ");
    name.toCharArray(tmpstring, 26);
    strcpy(Settings.Name, tmpstring);
    password.toCharArray(tmpstring, 26);
    strcpy(Settings.Password, tmpstring);
    ssid.toCharArray(tmpstring, 26);
    strcpy(Settings.WifiSSID, tmpstring);
    key.toCharArray(tmpstring, 64);
    strcpy(Settings.WifiKey, tmpstring);
    apkey.toCharArray(tmpstring, 64);
    strcpy(Settings.WifiAPKey, tmpstring);
    controllerip.toCharArray(tmpstring, 26);
    str2ip(tmpstring, Settings.Controller_IP);
    Settings.ControllerPort = controllerport.toInt();
    controlleruser.toCharArray(tmpstring, 26);
    strcpy(Settings.ControllerUser, tmpstring);
    controllerpassword.toCharArray(tmpstring, 26);
    strcpy(Settings.ControllerPassword, tmpstring);
    Settings.Protocol = protocol.toInt();
    Settings.Delay = sensordelay.toInt();
    Settings.MessageDelay = messagedelay.toInt();
    Settings.IP_Octet = ip.toInt();
    espip.toCharArray(tmpstring, 26);
    str2ip(tmpstring, Settings.IP);
    espgateway.toCharArray(tmpstring, 26);
    str2ip(tmpstring, Settings.Gateway);
    espsubnet.toCharArray(tmpstring, 26);
    str2ip(tmpstring, Settings.Subnet);
    Settings.Unit = unit.toInt();
    syslogip.toCharArray(tmpstring, 26);
    str2ip(tmpstring, Settings.Syslog_IP);
    Settings.UDPPort = udpport.toInt();
    Settings.SyslogLevel = sysloglevel.toInt();
    Settings.SerialLogLevel = serialloglevel.toInt();
    Settings.WebLogLevel = webloglevel.toInt();
    Settings.BaudRate = baudrate.toInt();
    Save_Settings();
    LoadSettings();
  }

  String reply = "";
  addMenu(reply);

  reply += F("<form  method='post'><table bgcolor='#ddeeff'>");
  reply += F("<tr bgcolor='#55bbff'><td>Main Settings<td><TR><TD>Name:<TD><input type='text' name='name' value='");
  Settings.Name[25] = 0;
  reply += Settings.Name;
  reply += F("'><TR><TD>Admin Password:<TD><input type='text' name='password' value='");
  Settings.Password[25] = 0;
  reply += Settings.Password;
  reply += F("'><TR><TD>SSID:<TD><input type='text' name='ssid' value='");
  reply += Settings.WifiSSID;
  reply += F("'><TR><TD>WPA Key:<TD><input type='text' maxlength='63' name='key' value='");
  reply += Settings.WifiKey;

  reply += F("'><TR><TD>WPA AP Mode Key:<TD><input type='text' maxlength='63' name='apkey' value='");
  reply += Settings.WifiAPKey;

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
  reply += F("<TD><select name='protocol'>");
  for (byte x = 0; x < 7; x++)
  {
    reply += F("<option value='");
    reply += x;
    reply += "'";
    if (choice == x)
      reply += " selected";
    reply += ">";
    reply += options[x];
    reply += "</option>";
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
      reply += Settings.ControllerUser;
    }
    
  if (Settings.Protocol == PROTOCOL_NODO_TELNET or Settings.Protocol == PROTOCOL_THINGSPEAK)
    {
      reply += F("'><TR><TD>Controller Password:<TD><input type='text' name='controllerpassword' value='");
      reply += Settings.ControllerPassword;
    }

  reply += F("'><TR><TD>Sensor Delay:<TD><input type='text' name='delay' value='");
  reply += Settings.Delay;
  reply += F("'><TR><TD>Message Delay (ms):<TD><input type='text' name='messagedelay' value='");
  reply += Settings.MessageDelay;

  reply += F("'><TR bgcolor='#55bbff'><TD>Optional Settings<TD><TR><TD>Fixed IP Octet:<TD><input type='text' name='ip' value='");
  reply += Settings.IP_Octet;

  reply += F("'><TR><TD>ESP IP:<TD><input type='text' name='espip' value='");
  sprintf_P(str, PSTR("%u.%u.%u.%u"), Settings.IP[0], Settings.IP[1], Settings.IP[2], Settings.IP[3]);
  reply += str;

  reply += F("'><TR><TD>ESP GW:<TD><input type='text' name='espgateway' value='");
  sprintf_P(str, PSTR("%u.%u.%u.%u"), Settings.Gateway[0], Settings.Gateway[1], Settings.Gateway[2], Settings.Gateway[3]);
  reply += str;

  reply += F("'><TR><TD>ESP Subnet:<TD><input type='text' name='espsubnet' value='");
  sprintf_P(str, PSTR("%u.%u.%u.%u"), Settings.Subnet[0], Settings.Subnet[1], Settings.Subnet[2], Settings.Subnet[3]);
  reply += str;

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

  char tmpstring[26];

  if (Settings.SerialLogLevel >= LOG_LEVEL_DEBUG)
    Serial.println(F("HTTP : Webtasks"));

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
      for (byte varNr=0; varNr < VARS_PER_TASK; varNr++)
      {
        String arg = "taskdeviceformula";
        arg += varNr + 1;
        char argc[20];
        arg.toCharArray(argc,20);
        taskdeviceformula[varNr] = WebServer.arg(argc);
      }
  
  String edit = WebServer.arg("edit");
  byte index = taskindex.toInt();

  byte DeviceIndex=0;
   
  if (edit.toInt() != 0)
  {
    Settings.TaskDeviceNumber[index-1] = taskdevicenumber.toInt();
    DeviceIndex = getDeviceIndex(Settings.TaskDeviceNumber[index-1]);
    taskdevicename.replace("+", " ");
    taskdevicename.toCharArray(tmpstring, 26);
    strcpy(Settings.TaskDeviceName[index-1], tmpstring);
    Settings.TaskDevicePort[index-1] = taskdeviceport.toInt();
    if (Settings.TaskDeviceNumber[index-1] !=0)
      Settings.TaskDeviceID[index-1] = taskdeviceid.toInt();
    else
      Settings.TaskDeviceID[index-1]=0;
    Settings.TaskDevicePin1[index-1] = taskdevicepin1.toInt();
    Settings.TaskDevicePin2[index-1] = taskdevicepin2.toInt();
    if (Device[DeviceIndex].Type != DEVICE_TYPE_SINGLE && Device[DeviceIndex].Type != DEVICE_TYPE_DUAL)
      Settings.TaskDevicePin1[index-1] = -1;
    if (Device[DeviceIndex].Type != DEVICE_TYPE_DUAL)
      Settings.TaskDevicePin2[index-1] = -1;

    if (Device[DeviceIndex].PullUpOption)
      Settings.TaskDevicePin1PullUp[index-1] = (taskdevicepin1pullup == "on");

    if (Device[DeviceIndex].InverseLogicOption)
      Settings.TaskDevicePin1Inversed[index-1] = (taskdevicepin1inversed == "on");

    for (byte varNr=0; varNr < VARS_PER_TASK; varNr++)
      {
        taskdeviceformula[varNr].replace("%25", "%");
        taskdeviceformula[varNr].replace("%28", "(");
        taskdeviceformula[varNr].replace("%29", ")");
        taskdeviceformula[varNr].replace("%2B", "+");
        taskdeviceformula[varNr].replace("%2F", "/");
        taskdeviceformula[varNr].toCharArray(tmpstring, 26);
        strcpy(Settings.TaskDeviceFormula[index-1][varNr], tmpstring);
      }
   
    #ifdef ESP_EASY
      struct EventStruct TempEvent;
      TempEvent.TaskIndex = index-1;
      PluginCall(PLUGIN_WEBFORM_SAVE, &TempEvent, dummyString);
      PluginCall(PLUGIN_INIT, &TempEvent, dummyString);
    #endif
    #ifdef ESP_CONNEXIO
      struct NodoEventStruct TempEvent;
      TempEvent.Par1 = index-1;
      PluginCall(PLUGIN_WEBFORM_SAVE, &TempEvent, 0);
      PluginCall(PLUGIN_INIT, &TempEvent, 0);
      createEventlist();
    #endif
    
    Save_Settings();
  }

  String reply = "";
  addMenu(reply);

  // show all tasks as table
  reply += F("<table border='1' bgcolor='#ddeeff'><tr bgcolor='#55bbff'><td><TD>Task<TD>Device<td>Name<TD>Port<TD>IDX/Variable<TD>1st GPIO<TD>2nd GPIO");
  for (byte varNr=0; varNr < VARS_PER_TASK; varNr++)
  {
    reply += F("<TD>Value ");
    reply += varNr + 1;
  }
  
  for (byte x=0; x < TASKS_MAX; x++)
  {
    DeviceIndex = getDeviceIndex(Settings.TaskDeviceNumber[x]);
    reply += F("<TR><TD>");
    reply += F("<a class=\"button-link\" href=\"devices?index=");
    reply += x+1;
    reply += F("\">Edit</a>");
    reply += F("<TD>");
    reply += x+1;
    reply += F("<TD>");
    reply += Device[DeviceIndex].Name;
    reply += F("<TD>");
    reply += Settings.TaskDeviceName[x];
    reply += F("<TD>");
    if (Device[DeviceIndex].Ports != 0)
      reply += Settings.TaskDevicePort[x];
    reply += F("<TD>");
    if (Settings.TaskDeviceID[x] != 0)
      reply += Settings.TaskDeviceID[x];
    reply += F("<TD>");

    if(Device[DeviceIndex].Type == DEVICE_TYPE_I2C)
      {
        reply += F("GPIO-");
        reply += Settings.Pin_i2c_sda;
      }
    if(Device[DeviceIndex].Type == DEVICE_TYPE_ANALOG)
        reply += F("ADC (TOUT)");

    if (Settings.TaskDevicePin1[x] != -1)
      {
        reply += F("GPIO-");
        reply += Settings.TaskDevicePin1[x];
      }
    reply += F("<TD>");

    if(Device[DeviceIndex].Type == DEVICE_TYPE_I2C)
      {
        reply += F("GPIO-");
        reply += Settings.Pin_i2c_scl;
      }
      
    if (Settings.TaskDevicePin2[x] != -1)
      {
        reply += F("GPIO-");
        reply += Settings.TaskDevicePin2[x];
      }

    byte customValues=false;  
    #ifdef ESP_EASY
      struct EventStruct TempEvent;
      TempEvent.TaskIndex = x;
      customValues = PluginCall(PLUGIN_WEBFORM_VALUES, &TempEvent, reply);
    #endif
    #ifdef ESP_CONNEXIO
      struct NodoEventStruct TempEvent;
      char tmpString[256];
      tmpString[0]=0;
      TempEvent.Par1 = x;
      customValues = PluginCall(PLUGIN_WEBFORM_LOAD, &TempEvent, tmpString);
      if (tmpString[0] != 0)
        reply += tmpString;
    #endif

    if (!customValues)
      {
        for (byte varNr=0; varNr < VARS_PER_TASK; varNr++)
          {
            reply += F("<TD>");
            if (varNr < Device[DeviceIndex].ValueCount)
              {
                reply += Device[DeviceIndex].ValueNames[varNr];
                reply += ":";
                reply += UserVar[x * VARS_PER_TASK + varNr];
              }
          }
      }
  }
  reply += F("</table>");

  // Show edit form if a specific entry is chosen with the edit button
  if (index != 0)
  {
    DeviceIndex = getDeviceIndex(Settings.TaskDeviceNumber[index-1]);

    reply += F("<BR><BR><form  method='post'><table bgcolor='#ddeeff'><tr bgcolor='#55bbff'><td>Task Settings<td>Value");

    reply += "<TR><TD>Device:<TD>";
    addDeviceSelect(reply,"taskdevicenumber",Settings.TaskDeviceNumber[index-1]);

    reply += F("<TR><TD>Name:<TD><input type='text' maxlength='25' name='taskdevicename' value='");
    reply += Settings.TaskDeviceName[index-1];

    if (Device[DeviceIndex].Ports != 0)
      {
        reply += F("'><TR><TD>Port:<TD><input type='text' name='taskdeviceport' value='");
        reply += Settings.TaskDevicePort[index-1];
      }
    reply += F("'><TR><TD>IDX / Var:<TD><input type='text' name='taskdeviceid' value='");
    reply += Settings.TaskDeviceID[index-1];
    reply += F("'>");

    if (Device[DeviceIndex].Type == DEVICE_TYPE_SINGLE || Device[DeviceIndex].Type == DEVICE_TYPE_DUAL)
      {
        reply += F("<TR><TD>1st GPIO:<TD>");
        addPinSelect(false, reply,"taskdevicepin1",Settings.TaskDevicePin1[index-1]);
      }
    if (Device[DeviceIndex].Type == DEVICE_TYPE_DUAL)
      {
        reply += F("<TR><TD>2nd GPIO:<TD>");
        addPinSelect(false, reply,"taskdevicepin2",Settings.TaskDevicePin2[index-1]);
      }

    if (Device[DeviceIndex].PullUpOption)
    {
      reply += F("<TR><TD>Pull UP:<TD>");
      if (Settings.TaskDevicePin1PullUp[index-1])
        reply += F("<input type=checkbox name=taskdevicepin1pullup checked>");
      else
        reply += F("<input type=checkbox name=taskdevicepin1pullup>");
    }

    if (Device[DeviceIndex].InverseLogicOption)
    {
      reply += F("<TR><TD>Inversed:<TD>");
      if (Settings.TaskDevicePin1Inversed[index-1])
        reply += F("<input type=checkbox name=taskdevicepin1inversed checked>");
      else
        reply += F("<input type=checkbox name=taskdevicepin1inversed>");
    }

    if (Device[DeviceIndex].FormulaOption)
    {
      for (byte varNr=0; varNr < Device[DeviceIndex].ValueCount; varNr++)
        {
          reply += F("<TR><TD>Formula ");
          reply += Device[DeviceIndex].ValueNames[varNr];
          reply += F(":<TD><input type='text' maxlength='25' name='taskdeviceformula");
          reply += varNr + 1;
          reply += F("' value='");
          reply += Settings.TaskDeviceFormula[index-1][varNr];
         reply += F("'>");
       }
    }
    
    #ifdef ESP_EASY
      struct EventStruct TempEvent;
      TempEvent.TaskIndex = index-1;
      PluginCall(PLUGIN_WEBFORM_LOAD, &TempEvent, reply);
    #endif

    #ifdef ESP_CONNEXIO
      struct NodoEventStruct TempEvent;
      char tmpString[256];
      tmpString[0]=0;
      TempEvent.Par1 = index-1;
      PluginCall(PLUGIN_WEBFORM_LOAD, &TempEvent, tmpString);
      if (tmpString[0] != 0)
        reply += tmpString;
    #endif

    reply += F("<TR><TD><TD><a class=\"button-link\" href=\"devices\">Cancel</a>");
    reply += F("<input class=\"button-link\" type='submit' value='Submit'><TR><TD>");
    reply += F("<input type='hidden' name='edit' value='1'>");
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

  if (Settings.SerialLogLevel >= LOG_LEVEL_DEBUG)
    Serial.println(F("HTTP : Hardware"));

  String pin_i2c_sda = WebServer.arg("pini2csda");
  String pin_i2c_scl = WebServer.arg("pini2cscl");

  if (pin_i2c_sda.length() != 0)
  {
    Settings.Pin_i2c_sda     = pin_i2c_sda.toInt();
    Settings.Pin_i2c_scl     = pin_i2c_scl.toInt();
    Save_Settings();
  }

  String reply = "";
  addMenu(reply);

  reply += F("<form  method='post'><table bgcolor='#ddeeff'><tr bgcolor='#55bbff'><td>Hardware Settings<td><TR><TD>");
  reply += F("<TR><TD>SDA:<TD>");
  addPinSelect(true, reply,"pini2csda",Settings.Pin_i2c_sda);
  reply += F("<TR><TD>SCL:<TD>");
  addPinSelect(true, reply,"pini2cscl",Settings.Pin_i2c_scl);

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
  str += F("<select name='");
  str += name;
  str += "'>";

  for (byte x = 0; x <= deviceCount; x++)
  {
    str += F("<option value='");
    str += Device[x].Number;
    str += "'";
    if (choice == Device[x].Number)
      str += " selected";
    str += ">";
    str += Device[x].Name;
    str += "</option>";
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
      str += " disabled";    
    if (choice == optionValues[x])
      str += " selected";
    str += ">";
    str += options[x];
    str += "</option>";
  }
  str += F("</select>");
}


//********************************************************************************
// Nodo proof of concept. send json query as nodo event on I2C to mega
// Compatible with Nodo 3.8 only, tested on R818
// set used variables to global on the Mega.
//********************************************************************************

#define NODO_VERSION_MAJOR   3
#define TARGET_NODO          5

struct TransmissionStruct
{
  byte Type;
  byte Command;
  byte Par1;
  byte Dummy;
  unsigned long Par2;
  byte P1;
  byte P2;
  byte SourceUnit;
  byte DestinationUnit;
  byte Flags;
  byte Checksum;
};

void handle_json() {
  Serial.print(F("HTTP : Web json : idx: "));
  String idx = WebServer.arg("idx");
  String svalue = WebServer.arg("svalue");
  Serial.print(idx);
  Serial.print(" svalue: ");
  Serial.println(svalue);
  char c_idx[10];
  c_idx[0] = 0;
  idx.toCharArray(c_idx, 10);
  char c_svalue[40];
  c_svalue[0] = 0;
  svalue.toCharArray(c_svalue, 40);

  struct TransmissionStruct event;
  event.Type = 1;
  event.Command = 4;
  event.Par1 = str2int(c_idx);
  event.Par2 = float2ul(atof(c_svalue));
  event.P1 = 0;
  event.P2 = 0;
  event.SourceUnit = 1;
  event.DestinationUnit = 0;
  event.Flags = 0;
  event.Checksum = 0;

  // due to padding of structs in memory on this MCU, we need to shift some bytes
  byte data[13];
  memcpy((byte*)&data, (byte*)&event, 3);
  memcpy((byte*)&data + 3, (byte*)&event + 4, 10);

  // calculate xor checksum
  byte NewChecksum = NODO_VERSION_MAJOR;
  for (byte x = 0; x < sizeof(data); x++)
    NewChecksum ^= data[x];
  data[12] = NewChecksum;

  // Send data to Nodo through I2C bus
  // Currently the target Nodo nr is fixed
  // I2C implementation is still incomplete, scanning does not work, slave mode not supported yet...
  Wire.beginTransmission(TARGET_NODO);
  for (byte x = 0; x < sizeof(data); x++)
    Wire.write(data[x]);
  Wire.endTransmission();

  WebServer.send(200, "text/html", "OK");
}

#ifdef ESP_CONNEXIO
//********************************************************************************
// Web Interface eventlist page
//********************************************************************************
void handle_eventlist() {
  if (!isLoggedIn()) return;

  if (Settings.SerialLogLevel >= LOG_LEVEL_DEBUG)
    Serial.println(F("HTTP : Eventlist request"));

  char *TempString = (char*)malloc(80);
  String reply = "";
  addMenu(reply);

  reply += F("<table bgcolor='#ddeeff'><tr bgcolor='#55bbff'><td>Eventlist<td><TR><TD>");

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
      line.replace("+", " ");
      line.replace("%25", "%");
      line.replace("%28", "(");
      line.replace("%29", ")");
      line.replace("%2B", "+");
      line.replace("%2F", "/");
      line.replace("%2C", ",");
      line.replace("%3B", ";");
      String strCommand = F("eventlistwrite 0,");
      strCommand += line;
      strCommand.toCharArray(TempString, 80);
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

  if (Settings.SerialLogLevel >= LOG_LEVEL_DEBUG)
    Serial.println(F("HTTP : Log request"));

  char *TempString = (char*)malloc(80);

  String reply = "";
  addMenu(reply);
  reply += F("<script language='JavaScript'>function RefreshMe(){window.location = window.location}setTimeout('RefreshMe()', 3000);</script>");
  reply += F("<table bgcolor='#ddeeff'><tr bgcolor='#55bbff'><td>Log<TR><TD>");

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
        reply += "<BR>";
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
  
  if (Settings.SerialLogLevel >= LOG_LEVEL_DEBUG)
    Serial.print(F("HTTP : Tools request : "));
    
  String webrequest = WebServer.arg("cmd");
  webrequest.replace("%3B", ";");
  webrequest.replace("%2C", ",");
  webrequest.replace("+", " ");
  if (Settings.SerialLogLevel >= LOG_LEVEL_DEBUG)
    Serial.println(webrequest);
  char command[80];
  command[0] = 0;
  webrequest.toCharArray(command, 80);

  String reply = "";
  addMenu(reply);

  // second menu
  reply += F("<a class=\"button-link\" href=\"/?cmd=reboot\">Reboot</a>");
  reply += F("<a class=\"button-link\" href=\"/?cmd=wificonnect\">Connect</a>");
  reply += F("<a class=\"button-link\" href=\"/?cmd=wifidisconnect\">Disconnect</a>");
  reply += F("<a class=\"button-link\" href=\"/i2cscanner\">I2C Scanner</a>");
  reply += F("<a class=\"button-link\" href=\"/wifiscanner\">Wifi Scanner</a><BR><BR>");

  reply += F("<form>");
  reply += F("<table bgcolor='#ddeeff'><tr bgcolor='#55bbff'><td>Command<TD>");
  reply += F("<input type='text' name='cmd' value='");
  reply += webrequest;
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

  if (Settings.SerialLogLevel >= LOG_LEVEL_DEBUG)
    Serial.println(F("HTTP : I2C Scanner"));

  char *TempString = (char*)malloc(80);

  String reply = "";
  addMenu(reply);
  reply += F("<table bgcolor='#ddeeff'><tr bgcolor='#55bbff'><td>I2C Addresses in use<TD>Known devices");

  byte error, address;
  int nDevices;
  nDevices = 0;
  for(address = 1; address <= 127; address++ ) 
    {
      Wire.beginTransmission(address);
      error = Wire.endTransmission();
      if (error == 0)
        {
          reply += "<TR><TD>0x";
          reply += String(address,HEX);
          reply += "<TD>";
          switch(address)
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
      else if (error==4) 
        {
          reply += F("<TR><TD>Unknow error at address 0x");
          reply += String(address,HEX);
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

  if (Settings.SerialLogLevel >= LOG_LEVEL_DEBUG)
    Serial.println(F("HTTP : Wifi Scanner"));

  char *TempString = (char*)malloc(80);

  String reply = "";
  addMenu(reply);
  reply += F("<table bgcolor='#ddeeff'><tr bgcolor='#55bbff'><td>Access Points:<TD>RSSI");

  int n = WiFi.scanNetworks();
  if (n == 0)
    reply += F("No Access Points found");
  else
  {
    for (int i = 0; i < n; ++i)
    {
      reply += "<TR><TD>";
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
  if (Settings.SerialLogLevel >= LOG_LEVEL_DEBUG)
    Serial.print(F("HTTP : Login request : "));
    
  String webrequest = WebServer.arg("password");
  if (Settings.SerialLogLevel >= LOG_LEVEL_DEBUG)
    Serial.println(webrequest);
  char command[80];
  command[0] = 0;
  webrequest.toCharArray(command, 80);

  String reply = "";
  reply += F("<form method='post'>");
  reply += F("<table bgcolor='#ddeeff'><tr bgcolor='#55bbff'><td>Password<TD>");
  reply += F("<input type='password' name='password' value='");
  reply += webrequest;
  reply += F("'><TR><TD><TD><input class=\"button-link\" type='submit' value='Submit'><TR><TD>");
  reply += F("</table></form>");

  if (webrequest.length() != 0)
  {
    // compare with stored password and set timer if there's a match
    if ((strcasecmp(command, Settings.Password) == 0) || (Settings.Password[0] == 0))
    {
       WebLoggedIn = true;
       WebLoggedInTimer=0;
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

  if (Settings.SerialLogLevel >= LOG_LEVEL_DEBUG)
    Serial.print(F("HTTP : Webrequest : "));
  String webrequest = WebServer.arg("cmd");
  webrequest.replace("%20", " ");
  if (Settings.SerialLogLevel >= LOG_LEVEL_DEBUG)
    Serial.println(webrequest);
  char command[80];
  command[0] = 0;
  webrequest.toCharArray(command, 80);
  boolean validCmd = false;
  
  char Cmd[40];
  Cmd[0] = 0;
  GetArgv(command, Cmd, 1);

  if ((strcasecmp(Cmd, "gpio") == 0) || (strcasecmp(Cmd, "pwm") == 0))
    validCmd = true;
    
  String reply = "";

  printToWeb = true;
  printWebString = "";
  
  if (validCmd)
    {
      #ifdef ESP_CONNEXIO
        ExecuteLine(command, VALUE_SOURCE_SERIAL);
      #endif
      #ifdef ESP_EASY
        ExecuteCommand(command);
      #endif
    }
    else
      reply += F("Unknown or restricted command!");
    
  reply += printWebString;
  reply += F("</table></form>");
  WebServer.send(200, "text/html", reply);
  printWebString = "";
  printToWeb = false;
}


//********************************************************************************
// Login state check
//********************************************************************************
boolean isLoggedIn()
{
  if (Settings.Password[0] == 0)
    WebLoggedIn = true;
    
  if (!WebLoggedIn)
    {
      String reply = F("<a class=\"button-link\" href=\"login\">Login</a>");
      WebServer.send(200, "text/html", reply);
    }
  else
    {
      WebLoggedInTimer=0;
    }
  
  return WebLoggedIn;
}

