//********************************************************************************
// Web Interface init
//********************************************************************************
void WebServerInit()
{
  // Prepare webserver pages
  WebServer.on("/", handle_root);
  WebServer.on("/config", handle_config);
  WebServer.on("/controllers", handle_controllers);
  WebServer.on("/hardware", handle_hardware);
  WebServer.on("/devices", handle_devices);
  WebServer.on("/notifications", handle_notifications);
  WebServer.on("/log", handle_log);
  WebServer.on("/tools", handle_tools);
  WebServer.on("/i2cscanner", handle_i2cscanner);
  WebServer.on("/wifiscanner", handle_wifiscanner);
  WebServer.on("/login", handle_login);
  WebServer.on("/control", handle_control);
  WebServer.on("/download", handle_download);
  WebServer.on("/upload", HTTP_GET, handle_upload);
  WebServer.on("/upload", HTTP_POST, handle_upload_post, handleFileUpload);
  WebServer.onNotFound(handleNotFound);
  WebServer.on("/filelist", handle_filelist);
  WebServer.on("/SDfilelist", handle_SDfilelist);
  WebServer.on("/advanced", handle_advanced);
  WebServer.on("/setup", handle_setup);
  WebServer.on("/json", handle_json);
  WebServer.on("/rules", handle_rules);
  WebServer.on("/sysinfo", handle_sysinfo);

  if (ESP.getFlashChipRealSize() > 524288)
    httpUpdater.setup(&WebServer);

  if (Settings.UseSSDP)
  {
    WebServer.on("/ssdp.xml", HTTP_GET, []() {
      SSDP_schema(WebServer.client());
    });
    SSDP_begin();
  }

  WebServer.begin();
}


//********************************************************************************
// Add top menu
//********************************************************************************
void addHeader(boolean showMenu, String& str)
{
  boolean cssfile = false;

  str += F("<script language=\"javascript\"><!--\n");
  str += F("function dept_onchange(frmselect) {frmselect.submit();}\n");
  str += F("//--></script>");
  str += F("<head><title>");
  str += Settings.Name;
  str += F("</title>");

  fs::File f = SPIFFS.open("esp.css", "r");
  if (f)
  {
    cssfile = true;
    f.close();
  }

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

  str += F("<h1>Welcome to ESP Easy Mega: ");
  str += Settings.Name;

  f = SPIFFS.open("esp.png", "r");
  if (f)
  {
    str += F("<img src=\"esp.png\" width=50 height=50 align=right >");
    f.close();
  }

  str += F("</h1>");

  if (showMenu)
  {
    str += F("<BR><a class=\"button-menu\" href=\".\">Main</a>");
    str += F("<a class=\"button-menu\" href=\"config\">Config</a>");
    str += F("<a class=\"button-menu\" href=\"controllers\">Controllers</a>");
    str += F("<a class=\"button-menu\" href=\"hardware\">Hardware</a>");
    str += F("<a class=\"button-menu\" href=\"devices\">Devices</a>");
    if (Settings.UseRules)
      str += F("<a class=\"button-menu\" href=\"rules\">Rules</a>");
    str += F("<a class=\"button-menu\" href=\"notifications\">Notifications</a>");
    str += F("<a class=\"button-menu\" href=\"tools\">Tools</a><BR><BR>");
  }
}


//********************************************************************************
// Add footer to web page
//********************************************************************************
void addFooter(String& str)
{
  str += F("<h6>Powered by www.letscontrolit.com</h6></body>");
}


//********************************************************************************
// Web Interface root page
//********************************************************************************
void handle_root() {

  // if Wifi setup, launch setup wizard
  if (wifiSetup)
  {
    WebServer.send(200, "text/html", F("<meta HTTP-EQUIV='REFRESH' content='0; url=/setup'>"));
    return;
  }

  if (!isLoggedIn()) return;

  int freeMem = ESP.getFreeHeap();
  String sCommand = WebServer.arg(F("cmd"));

  if ((strcasecmp_P(sCommand.c_str(), PSTR("wifidisconnect")) != 0) && (strcasecmp_P(sCommand.c_str(), PSTR("reboot")) != 0))
  {
    String reply = "";
    addHeader(true, reply);

    printToWeb = true;
    printWebString = "";
    if (sCommand.length() > 0)
      ExecuteCommand(VALUE_SOURCE_HTTP, sCommand.c_str());

    IPAddress ip = WiFi.localIP();
    IPAddress gw = WiFi.gatewayIP();

    reply += printWebString;
    reply += F("<form>");
    reply += F("<table><TH>System Info<TH>Value<TH><TH>System Info<TH>Value<TH>");

    reply += F("<TR><TD>Unit:<TD>");
    reply += Settings.Unit;

    reply += F("<TD><TD>GIT version:<TD>");
    reply += BUILD_GIT;

    reply += F("<TR><TD>Local Time:<TD>");
    if (Settings.UseNTP)
    {
      reply += year();
      reply += F("-");
      if (month() < 10)
        reply += "0";
      reply += month();
      reply += F("-");
      if (day() < 10)
      	reply += F("0");
      reply += day();
      reply += F(" ");
      reply += hour();
      reply += F(":");
      if (minute() < 10)
        reply += F("0");
      reply += minute();
    }
    else
      reply += F("NTP disabled");

    reply += F("<TD><TD>Uptime:<TD>");
    char strUpTime[40];
    int minutes = wdcounter / 2;
    int days = minutes / 1440;
    minutes = minutes % 1440;
    int hrs = minutes / 60;
    minutes = minutes % 60;
    sprintf_P(strUpTime, PSTR("%d days %d hours %d minutes"), days, hrs, minutes);
    reply += strUpTime;

    reply += F("<TR><TD>Load:<TD>");
    if (wdcounter > 0)
    {
      reply += 100 - (100 * loopCounterLast / loopCounterMax);
      reply += F("% (LC=");
      reply += int(loopCounterLast / 30);
      reply += F(")");
    }

    reply += F("<TD><TD>Free Mem:<TD>");
    reply += freeMem;
    reply += F(" (");
    reply += lowestRAM;
    reply += F(")");

    char str[20];
    sprintf_P(str, PSTR("%u.%u.%u.%u"), ip[0], ip[1], ip[2], ip[3]);
    reply += F("<TR><TD>IP:<TD>");
    reply += str;

    reply += F("<TD><TD>Wifi RSSI:<TD>");
    if (WiFi.status() == WL_CONNECTED)
    {
      reply += WiFi.RSSI();
      reply += F(" dB");
    }

    reply += F("<TR><TH>Node List:<TH>Name<TH>Build<TH>Type<TH>IP<TH>Age<TR><TD><TD>");
    for (byte x = 0; x < UNIT_MAX; x++)
    {
      if (Nodes[x].ip[0] != 0)
      {
        char url[80];
        sprintf_P(url, PSTR("<a href='http://%u.%u.%u.%u'>%u.%u.%u.%u</a>"), Nodes[x].ip[0], Nodes[x].ip[1], Nodes[x].ip[2], Nodes[x].ip[3], Nodes[x].ip[0], Nodes[x].ip[1], Nodes[x].ip[2], Nodes[x].ip[3]);
        reply += F("<TR><TD>Unit ");
        reply += x;
        reply += F("<TD>");
        if (x != Settings.Unit)
          reply += Nodes[x].nodeName;
        else
          reply += Settings.Name;
        reply += F("<TD>");
        if (Nodes[x].build)
          reply += Nodes[x].build;
        reply += F("<TD>");
        if (Nodes[x].nodeType)
          switch (Nodes[x].nodeType)
          {
            case NODE_TYPE_ID_ESP_EASY_STD:
              reply += F("ESP Easy");
              break;
            case NODE_TYPE_ID_ESP_EASYM_STD:
              reply += F("ESP Easy Mega");
              break;
            case NODE_TYPE_ID_ESP_EASY32_STD:
              reply += F("ESP Easy 32");
              break;
            case NODE_TYPE_ID_ARDUINO_EASY_STD:
              reply += F("Arduino Easy");
              break;
            case NODE_TYPE_ID_NANO_EASY_STD:
              reply += F("Nano Easy");
              break;
          }
        reply += F("<TD>");
        reply += url;
        reply += F("<TD>");
        reply += Nodes[x].age;
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
    if (strcasecmp_P(sCommand.c_str(), PSTR("wifidisconnect")) == 0)
    {
      String log = F("WIFI : Disconnecting...");
      addLog(LOG_LEVEL_INFO, log);
      cmd_within_mainloop = CMD_WIFI_DISCONNECT;
    }

    if (strcasecmp_P(sCommand.c_str(), PSTR("reboot")) == 0)
    {
      String log = F("     : Rebooting...");
      addLog(LOG_LEVEL_INFO, log);
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

  String name = WebServer.arg(F("name"));
  String password = WebServer.arg(F("password"));
  String ssid = WebServer.arg(F("ssid"));
  String key = WebServer.arg(F("key"));
  String ssid2 = WebServer.arg(F("ssid2"));
  String key2 = WebServer.arg(F("key2"));
  String sensordelay = WebServer.arg(F("delay"));
  String deepsleep = WebServer.arg(F("deepsleep"));
  String espip = WebServer.arg(F("espip"));
  String espgateway = WebServer.arg(F("espgateway"));
  String espsubnet = WebServer.arg(F("espsubnet"));
  String espdns = WebServer.arg(F("espdns"));
  String unit = WebServer.arg(F("unit"));
  String apkey = WebServer.arg(F("apkey"));

  String reply = "";
  addHeader(true, reply);

  if (ssid[0] != 0)
  {
    strncpy(Settings.Name, name.c_str(), sizeof(Settings.Name));
    strncpy(SecuritySettings.Password, password.c_str(), sizeof(SecuritySettings.Password));
    strncpy(SecuritySettings.WifiSSID, ssid.c_str(), sizeof(SecuritySettings.WifiSSID));
    strncpy(SecuritySettings.WifiKey, key.c_str(), sizeof(SecuritySettings.WifiKey));
    strncpy(SecuritySettings.WifiSSID2, ssid2.c_str(), sizeof(SecuritySettings.WifiSSID2));
    strncpy(SecuritySettings.WifiKey2, key2.c_str(), sizeof(SecuritySettings.WifiKey2));
    strncpy(SecuritySettings.WifiAPKey, apkey.c_str(), sizeof(SecuritySettings.WifiAPKey));

    Settings.Delay = sensordelay.toInt();
    Settings.deepSleep = (deepsleep == "on");
    espip.toCharArray(tmpString, 26);
    str2ip(tmpString, Settings.IP);
    espgateway.toCharArray(tmpString, 26);
    str2ip(tmpString, Settings.Gateway);
    espsubnet.toCharArray(tmpString, 26);
    str2ip(tmpString, Settings.Subnet);
    espdns.toCharArray(tmpString, 26);
    str2ip(tmpString, Settings.DNS);
    Settings.Unit = unit.toInt();
    if (!SaveSettings())
      reply += F("<span style=\"color:red\">Error saving to flash!</span>");
  }

  reply += F("<form name='frmselect' method='post'><table>");
  reply += F("<TH>Main Settings<TH><TR><TD>Name:<TD><input type='text'  maxlength='25' name='name' value='");
  Settings.Name[25] = 0;
  reply += Settings.Name;
  reply += F("'><TR><TD>Admin Password:<TD><input type='password' maxlength='25'  name='password' value='");
  SecuritySettings.Password[25] = 0;
  reply += SecuritySettings.Password;
  reply += F("'><TR><TD>SSID:<TD><input type='text' name='ssid' value='");
  reply += SecuritySettings.WifiSSID;
  reply += F("'><TR><TD>WPA Key:<TD><input type='password' maxlength='63' name='key' value='");
  reply += SecuritySettings.WifiKey;
  reply += F("'><TR><TD>Fallback SSID:<TD><input type='text' name='ssid2' value='");
  reply += SecuritySettings.WifiSSID2;
  reply += F("'><TR><TD>Fallback WPA Key:<TD><input type='password' maxlength='63' name='key2' value='");
  reply += SecuritySettings.WifiKey2;

  reply += F("'><TR><TD>WPA AP Mode Key:<TD><input type='password' maxlength='63' name='apkey' value='");
  reply += SecuritySettings.WifiAPKey;

  reply += F("'><TR><TD>Unit nr:<TD><input type='text' name='unit' value='");
  reply += Settings.Unit;

  reply += F("'>");

  char str[20];

  reply += F("<TR><TD>Sleep Delay:<TD><input type='text' name='delay' value='");
  reply += Settings.Delay;
  reply += F("'><TR><TD>Sleep enabled:<TD>");
  if (Settings.deepSleep)
    reply += F("<input type=checkbox name='deepsleep' checked>");
  else
    reply += F("<input type=checkbox name='deepsleep'>");

  reply += F("<a class=\"button-link\" href=\"http://www.letscontrolit.com/wiki/index.php/SleepMode\" target=\"_blank\">?</a>");

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

  reply += F("'><TR><TD>ESP DNS:<TD><input type='text' name='espdns' value='");
  sprintf_P(str, PSTR("%u.%u.%u.%u"), Settings.DNS[0], Settings.DNS[1], Settings.DNS[2], Settings.DNS[3]);
  reply += str;

  reply += F("'><TR><TD><TD><input class=\"button-link\" type='submit' value='Submit'>");
  reply += F("</table></form>");
  addFooter(reply);
  WebServer.send(200, "text/html", reply);
}


//********************************************************************************
// Web Interface controller page
//********************************************************************************
void handle_controllers() {
  if (!isLoggedIn()) return;

  struct EventStruct TempEvent;
  char tmpString[64];

  String controllerindex = WebServer.arg(F("index"));
  String usedns = WebServer.arg(F("usedns"));
  String controllerip = WebServer.arg(F("controllerip"));
  String controllerhostname = WebServer.arg(F("controllerhostname"));
  String controllerport = WebServer.arg(F("controllerport"));
  String protocol = WebServer.arg(F("protocol"));
  String controlleruser = WebServer.arg(F("controlleruser"));
  String controllerpassword = WebServer.arg(F("controllerpassword"));
  String controllersubscribe = WebServer.arg(F("controllersubscribe"));
  String controllerpublish = WebServer.arg(F("controllerpublish"));
  String controllerenabled = WebServer.arg(F("controllerenabled"));

  String reply = "";
  addHeader(true, reply);

  byte index = controllerindex.toInt();

  if (protocol.length() != 0)
  {
    ControllerSettingsStruct ControllerSettings;
    if (Settings.Protocol[index - 1] != protocol.toInt())
    {
      Settings.Protocol[index - 1] = protocol.toInt();
      byte ProtocolIndex = getProtocolIndex(Settings.Protocol[index - 1]);
      ControllerSettings.Port = Protocol[ProtocolIndex].defaultPort;
      if (Protocol[ProtocolIndex].usesTemplate)
        CPlugin_ptr[ProtocolIndex](CPLUGIN_PROTOCOL_TEMPLATE, &TempEvent, dummyString);
      strncpy(ControllerSettings.Subscribe, TempEvent.String1.c_str(), sizeof(ControllerSettings.Subscribe));
      strncpy(ControllerSettings.Publish, TempEvent.String2.c_str(), sizeof(ControllerSettings.Publish));
      TempEvent.String1 = "";
      TempEvent.String2 = "";
    }
    else
    {
      if (Settings.Protocol != 0)
      {
        byte ProtocolIndex = getProtocolIndex(Settings.Protocol[index - 1]);
        CPlugin_ptr[ProtocolIndex](CPLUGIN_WEBFORM_SAVE, 0, dummyString);
        ControllerSettings.UseDNS = usedns.toInt();
        if (ControllerSettings.UseDNS)
        {
          strncpy(ControllerSettings.HostName, controllerhostname.c_str(), sizeof(ControllerSettings.HostName));
          IPAddress IP;
          WiFi.hostByName(ControllerSettings.HostName, IP);
          for (byte x = 0; x < 4; x++)
            ControllerSettings.IP[x] = IP[x];
        }
        else
        {
          if (controllerip.length() != 0)
          {
            controllerip.toCharArray(tmpString, 26);
            str2ip(tmpString, ControllerSettings.IP);
          }
        }

        Settings.ControllerEnabled[index - 1] = (controllerenabled == "on");
        ControllerSettings.Port = controllerport.toInt();
        strncpy(SecuritySettings.ControllerUser[index - 1], controlleruser.c_str(), sizeof(SecuritySettings.ControllerUser[0]));
        strncpy(SecuritySettings.ControllerPassword[index - 1], controllerpassword.c_str(), sizeof(SecuritySettings.ControllerPassword[0]));
        strncpy(ControllerSettings.Subscribe, controllersubscribe.c_str(), sizeof(ControllerSettings.Subscribe));
        strncpy(ControllerSettings.Publish, controllerpublish.c_str(), sizeof(ControllerSettings.Publish));
      }
    }
    SaveControllerSettings(index - 1, (byte*)&ControllerSettings, sizeof(ControllerSettings));
    if (!SaveSettings())
      reply += F("<span style=\"color:red\">Error saving to flash!</span>");
  }

  reply += F("<form name='frmselect' method='post'><table>");

  if (index == 0)
  {
    reply += F("<table cellpadding='4' border='1' frame='box' rules='all'><TH>");
    reply += F("<TH>Nr<TH>Enabled<TH>Protocol<TH>IP<TH>Port");

    ControllerSettingsStruct ControllerSettings;
    for (byte x = 0; x < CONTROLLER_MAX; x++)
    {
      LoadControllerSettings(x, (byte*)&ControllerSettings, sizeof(ControllerSettings));
      reply += F("<TR><TD>");
      reply += F("<a class=\"button-link\" href=\"controllers?index=");
      reply += x + 1;
      reply += F("\">Edit</a>");
      reply += F("<TD>");
      reply += x + 1;
      reply += F("<TD>");
      if (Settings.Protocol[x] != 0)
      {
        if (Settings.ControllerEnabled[x])
          reply += F("<span style=\"color:green\">Yes</span>");
        else
          reply += F("<span style=\"color:red\">No</span>");

        reply += F("<TD>");
        byte ProtocolIndex = getProtocolIndex(Settings.Protocol[x]);
        String ProtocolName = "";
        CPlugin_ptr[ProtocolIndex](CPLUGIN_GET_DEVICENAME, 0, ProtocolName);
        reply += ProtocolName;

        char str[20];
        reply += F("<TD>");
        sprintf_P(str, PSTR("%u.%u.%u.%u"), ControllerSettings.IP[0], ControllerSettings.IP[1], ControllerSettings.IP[2], ControllerSettings.IP[3]);
        reply += str;
        reply += F("<TD>");
        reply += ControllerSettings.Port;
      }
      else
        reply += F("<TD><TD><TD>");
    }
    reply += F("</table>");
  }
  else
  {
    reply += F("<TH>Controller Settings<TH>");
    reply += F("<TR><TD>Protocol:");
    byte choice = Settings.Protocol[index - 1];
    reply += F("<TD><select name='protocol' LANGUAGE=javascript onchange=\"return dept_onchange(frmselect)\" >");
    reply += F("<option value='0'>- Standalone -</option>");
    for (byte x = 0; x <= protocolCount; x++)
    {
      if ((index == 1) || !Protocol[x].usesMQTT) {  // Only show MQTT capable controllers for slot "0"
        reply += F("<option value='");
        reply += Protocol[x].Number;
        reply += "'";
        if (choice == Protocol[x].Number)
          reply += F(" selected");
        reply += ">";
        String ProtocolName = "";
        CPlugin_ptr[x](CPLUGIN_GET_DEVICENAME, 0, ProtocolName);
        reply += ProtocolName;
        reply += F("</option>");
      }
    }
    reply += F("</select>");
    reply += F("<a class=\"button-link\" href=\"http://www.letscontrolit.com/wiki/index.php/EasyProtocols\" target=\"_blank\">?</a>");


    char str[20];

    if (Settings.Protocol[index - 1])
    {
      ControllerSettingsStruct ControllerSettings;
      LoadControllerSettings(index - 1, (byte*)&ControllerSettings, sizeof(ControllerSettings));
      byte choice = ControllerSettings.UseDNS;
      String options[2];
      options[0] = F("Use IP address");
      options[1] = F("Use Hostname");
      int optionValues[2];
      optionValues[0] = 0;
      optionValues[1] = 1;
      reply += F("<TR><TD>Locate Controller:<TD><select name='usedns' LANGUAGE=javascript onchange=\"return dept_onchange(frmselect)\" >");
      for (byte x = 0; x < 2; x++)
      {
        reply += F("<option value='");
        reply += optionValues[x];
        reply += "'";
        if (choice == optionValues[x])
          reply += F(" selected");
        reply += ">";
        reply += options[x];
        reply += F("</option>");
      }
      reply += F("</select>");

      if (ControllerSettings.UseDNS)
      {
        reply += F("<TR><TD>Controller Hostname:<TD><input type='text' name='controllerhostname' size='64' value='");
        reply += ControllerSettings.HostName;
      }
      else
      {
        reply += F("<TR><TD>Controller IP:<TD><input type='text' name='controllerip' value='");
        sprintf_P(str, PSTR("%u.%u.%u.%u"), ControllerSettings.IP[0], ControllerSettings.IP[1], ControllerSettings.IP[2], ControllerSettings.IP[3]);
        reply += str;
      }

      reply += F("'><TR><TD>Controller Port:<TD><input type='text' name='controllerport' value='");
      reply += ControllerSettings.Port;
      reply += F("'>");

      byte ProtocolIndex = getProtocolIndex(Settings.Protocol[index - 1]);
      if (Protocol[ProtocolIndex].usesAccount)
      {
        reply += F("<TR><TD>Controller User:<TD><input type='text' name='controlleruser' value='");
        reply += SecuritySettings.ControllerUser[index - 1];
        reply += F("'>");
      }

      if (Protocol[ProtocolIndex].usesPassword)
      {
        reply += F("<TR><TD>Controller Password:<TD><input type='password' name='controllerpassword' value='");
        reply += SecuritySettings.ControllerPassword[index - 1];
        reply += F("'>");
      }

      if (Protocol[ProtocolIndex].usesTemplate || Protocol[ProtocolIndex].usesMQTT)
      {
        reply += F("<TR><TD>Controller Subscribe:<TD><input type='text' name='controllersubscribe' size=64 value='");
        reply += ControllerSettings.Subscribe;
        reply += F("'>");
      }

      if (Protocol[ProtocolIndex].usesTemplate || Protocol[ProtocolIndex].usesMQTT)
      {
        reply += F("<TR><TD>Controller Publish:<TD><input type='text' name='controllerpublish'  size=64 value='");
        reply += ControllerSettings.Publish;
        reply += F("'>");
      }

      reply += F("<TR><TD>Enabled:<TD>");
      if (Settings.ControllerEnabled[index - 1])
        reply += F("<input type=checkbox name=controllerenabled checked>");
      else
        reply += F("<input type=checkbox name=controllerenabled>");

      TempEvent.ProtocolIndex = index - 1;
      CPlugin_ptr[ProtocolIndex](CPLUGIN_WEBFORM_LOAD, &TempEvent, reply);

    }
    reply += F("<TR><TD><TD><a class=\"button-link\" href=\"controllers\">Close</a>");
    reply += F("<input class=\"button-link\" type='submit' value='Submit'>");
    reply += F("</table></form>");
  }
  addFooter(reply);
  WebServer.send(200, "text/html", reply);
}


//********************************************************************************
// Web Interface notifcations page
//********************************************************************************
void handle_notifications() {
  if (!isLoggedIn()) return;

  struct EventStruct TempEvent;
  char tmpString[64];

  String notificationindex = WebServer.arg(F("index"));
  String notification = WebServer.arg(F("notification"));
  String domain = WebServer.arg(F("domain"));
  String server = WebServer.arg(F("server"));
  String port = WebServer.arg(F("port"));
  String sender = WebServer.arg(F("sender"));
  String receiver = WebServer.arg(F("receiver"));
  String subject = WebServer.arg(F("subject"));
  String body = WebServer.arg(F("body"));
  String pin1 = WebServer.arg(F("pin1"));
  String pin2 = WebServer.arg(F("pin2"));
  String notificationenabled = WebServer.arg(F("notificationenabled"));

  String reply = "";
  addHeader(true, reply);

  byte index = notificationindex.toInt();

  if (notification.length() != 0)
  {
    NotificationSettingsStruct NotificationSettings;
    if (Settings.Notification[index - 1] != notification.toInt())
    {
      Settings.Notification[index - 1] = notification.toInt();
      NotificationSettings.Domain[0] = 0;
      NotificationSettings.Server[0] = 0;
      NotificationSettings.Port = 0;
      NotificationSettings.Sender[0] = 0;
      NotificationSettings.Receiver[0] = 0;
      NotificationSettings.Subject[0] = 0;
      NotificationSettings.Body[0] = 0;
    }
    else
    {
      if (Settings.Notification != 0)
      {
        byte NotificationProtocolIndex = getNotificationIndex(Settings.Notification[index - 1]);
        NPlugin_ptr[NotificationProtocolIndex](NPLUGIN_WEBFORM_SAVE, 0, dummyString);
        NotificationSettings.Port = port.toInt();
        NotificationSettings.Pin1 = pin1.toInt();
        NotificationSettings.Pin2 = pin2.toInt();
        Settings.NotificationEnabled[index - 1] = (notificationenabled == "on");
        strncpy(NotificationSettings.Domain, domain.c_str(), sizeof(NotificationSettings.Domain));
        strncpy(NotificationSettings.Server, server.c_str(), sizeof(NotificationSettings.Server));
        strncpy(NotificationSettings.Sender, sender.c_str(), sizeof(NotificationSettings.Sender));
        strncpy(NotificationSettings.Receiver, receiver.c_str(), sizeof(NotificationSettings.Receiver));
        strncpy(NotificationSettings.Subject, subject.c_str(), sizeof(NotificationSettings.Subject));
        strncpy(NotificationSettings.Body, body.c_str(), sizeof(NotificationSettings.Body));
      }
    }
    SaveNotificationSettings(index - 1, (byte*)&NotificationSettings, sizeof(NotificationSettings));
    if (!SaveSettings())
      reply += F("<span style=\"color:red\">Error saving to flash!</span>");
  }

  reply += F("<form name='frmselect' method='post'><table>");

  if (index == 0)
  {
    reply += F("<table cellpadding='4' border='1' frame='box' rules='all'><TH>");
    reply += F("<TH>Nr<TH>Enabled<TH>Service<TH>Server<TH>Port");

    NotificationSettingsStruct NotificationSettings;
    for (byte x = 0; x < NOTIFICATION_MAX; x++)
    {
      LoadNotificationSettings(x, (byte*)&NotificationSettings, sizeof(NotificationSettings));
      reply += F("<TR><TD>");
      reply += F("<a class=\"button-link\" href=\"notifications?index=");
      reply += x + 1;
      reply += F("\">Edit</a>");
      reply += F("<TD>");
      reply += x + 1;
      reply += F("<TD>");
      if (Settings.Notification[x] != 0)
      {
        if (Settings.NotificationEnabled[x])
          reply += F("<span style=\"color:green\">Yes</span>");
        else
          reply += F("<span style=\"color:red\">No</span>");

        reply += F("<TD>");
        byte NotificationProtocolIndex = getNotificationIndex(Settings.Notification[x]);
        String NotificationName = "";
        NPlugin_ptr[NotificationProtocolIndex](NPLUGIN_GET_DEVICENAME, 0, NotificationName);
        reply += NotificationName;
        reply += F("<TD>");
        reply += NotificationSettings.Server;
        reply += F("<TD>");
        reply += NotificationSettings.Port;
      }
      else
        reply += F("<TD><TD><TD>");
    }
    reply += F("</table>");
  }
  else
  {
    reply += F("<TH>Notification Settings<TH>");
    reply += F("<TR><TD>Notification:");
    byte choice = Settings.Notification[index - 1];
    reply += F("<TD><select name='notification' LANGUAGE=javascript onchange=\"return dept_onchange(frmselect)\" >");
    reply += F("<option value='0'>- None -</option>");
    for (byte x = 0; x <= notificationCount; x++)
    {
      reply += F("<option value='");
      reply += Notification[x].Number;
      reply += "'";
      if (choice == Notification[x].Number)
        reply += F(" selected");
      reply += ">";

      String NotificationName = "";
      NPlugin_ptr[x](NPLUGIN_GET_DEVICENAME, 0, NotificationName);
      reply += NotificationName;
      reply += F("</option>");
    }
    reply += F("</select>");
    reply += F("<a class=\"button-link\" href=\"http://www.letscontrolit.com/wiki/index.php/EasyNotifications\" target=\"_blank\">?</a>");


    char str[20];

    if (Settings.Notification[index - 1])
    {
      NotificationSettingsStruct NotificationSettings;
      LoadNotificationSettings(index - 1, (byte*)&NotificationSettings, sizeof(NotificationSettings));

      byte NotificationProtocolIndex = getNotificationIndex(Settings.Notification[index - 1]);

      if (Notification[NotificationProtocolIndex].usesMessaging)
      {
        reply += F("<TR><TD>Domain:<TD><input type='text' name='domain' size=64 value='");
        reply += NotificationSettings.Domain;
        reply += F("'>");

        reply += F("<TR><TD>Server:<TD><input type='text' name='server' size=64 value='");
        reply += NotificationSettings.Server;
        reply += F("'>");

        reply += F("<TR><TD>Port:<TD><input type='text' name='port' value='");
        reply += NotificationSettings.Port;
        reply += F("'>");

        reply += F("<TR><TD>Sender:<TD><input type='text' name='sender' size=64 value='");
        reply += NotificationSettings.Sender;
        reply += F("'>");

        reply += F("<TR><TD>Receiver:<TD><input type='text' name='receiver' size=64 value='");
        reply += NotificationSettings.Receiver;
        reply += F("'>");

        reply += F("<TR><TD>Subject:<TD><input type='text' name='subject' size=64 value='");
        reply += NotificationSettings.Subject;
        reply += F("'>");

        reply += F("<TR><TD>Body:<TD><textarea name='body' rows='5' cols='80' size=512 wrap='off'>");
        reply += NotificationSettings.Body;
        reply += F("</textarea>");
      }

      if (Notification[NotificationProtocolIndex].usesGPIO > 0)
      {
        reply += F("<TR><TD>1st GPIO:<TD>");
        addPinSelect(false, reply, "pin1", NotificationSettings.Pin1);
      }

      reply += F("<TR><TD>Enabled:<TD>");
      if (Settings.NotificationEnabled[index - 1])
        reply += F("<input type=checkbox name=notificationenabled checked>");
      else
        reply += F("<input type=checkbox name=notificationenabled>");

      TempEvent.NotificationIndex = index - 1;
      NPlugin_ptr[NotificationProtocolIndex](NPLUGIN_WEBFORM_LOAD, &TempEvent, reply);

    }
    reply += F("<TR><TD><TD><a class=\"button-link\" href=\"notifications\">Close</a>");
    reply += F("<input class=\"button-link\" type='submit' value='Submit'>");
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

  String pin_i2c_sda = WebServer.arg(F("psda"));
  String pin_i2c_scl = WebServer.arg(F("pscl"));
  String pin_status_led = WebServer.arg(F("pled"));
  String pin_sd_cs = WebServer.arg(F("sd"));

  String reply = "";
  addHeader(true, reply);

  if (pin_i2c_sda.length() != 0)
  {
    Settings.Pin_i2c_sda     = pin_i2c_sda.toInt();
    Settings.Pin_i2c_scl     = pin_i2c_scl.toInt();
    Settings.Pin_status_led  = pin_status_led.toInt();
    Settings.Pin_sd_cs  = pin_sd_cs.toInt();
    Settings.PinBootStates[0]  =  WebServer.arg(F("p0")).toInt();
    Settings.PinBootStates[2]  =  WebServer.arg(F("p2")).toInt();
    Settings.PinBootStates[4]  =  WebServer.arg(F("p4")).toInt();
    Settings.PinBootStates[5]  =  WebServer.arg(F("p5")).toInt();
    Settings.PinBootStates[9]  =  WebServer.arg(F("p9")).toInt();
    Settings.PinBootStates[10] =  WebServer.arg(F("p10")).toInt();
    Settings.PinBootStates[12] =  WebServer.arg(F("p12")).toInt();
    Settings.PinBootStates[13] =  WebServer.arg(F("p13")).toInt();
    Settings.PinBootStates[14] =  WebServer.arg(F("p14")).toInt();
    Settings.PinBootStates[15] =  WebServer.arg(F("p15")).toInt();
    Settings.PinBootStates[16] =  WebServer.arg(F("p16")).toInt();

    Settings.InitSPI = WebServer.arg(F("initspi")) == "on";      // SPI Init

    if (!SaveSettings())
      reply += F("<span style=\"color:red\">Error saving to flash!</span>");

  }

  reply += F("<form  method='post'><table><TH>Hardware Settings<TH><TR><TD>");
  reply += F("<TR><TD>Wifi Status Led:<TD>");
  addPinSelect(false, reply, "pled", Settings.Pin_status_led);
  reply += F("<TR><TD>SDA:<TD>");
  addPinSelect(true, reply, "psda", Settings.Pin_i2c_sda);
  reply += F("<TR><TD>SCL:<TD>");
  addPinSelect(true, reply, "pscl", Settings.Pin_i2c_scl);

  // SPI Init
  reply += F("<TR><TD>Init SPI:<TD>");
  if (Settings.InitSPI)
    reply += F("<input type=checkbox id='initspi'  name='initspi' checked>&nbsp;");
  else
    reply += F("<input type=checkbox id='initspi' name='initspi'>&nbsp;");
  reply += F("(Note : Chip Select (CS) config must be done in the plugin)");

  reply += F("<TR><TD>SD Card CS Pin:<TD>");
  addPinSelect(false, reply, "sd", Settings.Pin_sd_cs);

  reply += F("<TR><TD>GPIO boot states:<TD>");
  reply += F("<TR><TD>Pin mode 0 (D3):<TD>");
  addPinStateSelect(reply, "p0", Settings.PinBootStates[0]);
  reply += F("<TR><TD>Pin mode 2 (D4):<TD>");
  addPinStateSelect(reply, "p2", Settings.PinBootStates[2]);
  reply += F("<TR><TD>Pin mode 4 (D2):<TD>");
  addPinStateSelect(reply, "p4", Settings.PinBootStates[4]);
  reply += F("<TR><TD>Pin mode 5 (D1):<TD>");
  addPinStateSelect(reply, "p5", Settings.PinBootStates[5]);
  reply += F("<TR><TD>Pin mode 9 (D11):<TD>");
  addPinStateSelect(reply, "p9", Settings.PinBootStates[9]);
  reply += F("<TR><TD>Pin mode 10 (D12):<TD>");
  addPinStateSelect(reply, "p10", Settings.PinBootStates[10]);
  reply += F("<TR><TD>Pin mode 12 (D6):<TD>");
  addPinStateSelect(reply, "p12", Settings.PinBootStates[12]);
  reply += F("<TR><TD>Pin mode 13 (D7):<TD>");
  addPinStateSelect(reply, "p13", Settings.PinBootStates[13]);
  reply += F("<TR><TD>Pin mode 14 (D5):<TD>");
  addPinStateSelect(reply, "p14", Settings.PinBootStates[14]);
  reply += F("<TR><TD>Pin mode 15 (D8):<TD>");
  addPinStateSelect(reply, "p15", Settings.PinBootStates[15]);
  reply += F("<TR><TD>Pin mode 16 (D0):<TD>");
  addPinStateSelect(reply, "p16", Settings.PinBootStates[16]);

  reply += F("<TR><TD><TD><input class=\"button-link\" type='submit' value='Submit'><TR><TD>");

  reply += F("</table></form>");
  addFooter(reply);
  WebServer.send(200, "text/html", reply);
}


//********************************************************************************
// Add a GPIO pin select dropdown list
//********************************************************************************
void addPinStateSelect(String& str, String name,  int choice)
{
  String options[4];
  options[0] = F("Default");
  options[1] = F("Output Low");
  options[2] = F("Output High");
  options[3] = F("Input");
  int optionValues[4];
  optionValues[0] = 0;
  optionValues[1] = 1;
  optionValues[2] = 2;
  optionValues[3] = 3;

  str += F("<select name='");
  str += name;
  str += "'>";
  for (byte x = 0; x < 4; x++)
  {
    str += F("<option value='");
    str += optionValues[x];
    str += "'";
    if (choice == optionValues[x])
      str += F(" selected");
    str += ">";
    str += options[x];
    str += F("</option>");
  }
  str += F("</select>");
}


//********************************************************************************
// Web Interface device page
//********************************************************************************
void handle_devices() {
  if (!isLoggedIn()) return;

  char tmpString[41];
  struct EventStruct TempEvent;

  String taskindex = WebServer.arg(F("index"));
  String taskdevicenumber = WebServer.arg(F("taskdevicenumber"));
  String taskdevicetimer = WebServer.arg(F("taskdevicetimer"));
  String taskdeviceid[CONTROLLER_MAX];
  String taskdevicepin1 = WebServer.arg(F("taskdevicepin1"));
  String taskdevicepin2 = WebServer.arg(F("taskdevicepin2"));
  String taskdevicepin3 = WebServer.arg(F("taskdevicepin3"));
  String taskdevicepin1pullup = WebServer.arg(F("taskdevicepin1pullup"));
  String taskdevicepin1inversed = WebServer.arg(F("taskdevicepin1inversed"));
  String taskdevicename = WebServer.arg(F("taskdevicename"));
  String taskdeviceport = WebServer.arg(F("taskdeviceport"));
  String taskdeviceformula[VARS_PER_TASK];
  String taskdevicevaluename[VARS_PER_TASK];
  String taskdevicevaluedecimals[VARS_PER_TASK];
  String taskdevicesenddata[CONTROLLER_MAX];
  String taskdeviceglobalsync = WebServer.arg(F("taskdeviceglobalsync"));
  String taskdeviceenabled = WebServer.arg(F("taskdeviceenabled"));

  for (byte varNr = 0; varNr < VARS_PER_TASK; varNr++)
  {
    char argc[25];
    String arg = F("taskdeviceformula");
    arg += varNr + 1;
    arg.toCharArray(argc, 25);
    taskdeviceformula[varNr] = WebServer.arg(argc);

    arg = F("taskdevicevaluename");
    arg += varNr + 1;
    arg.toCharArray(argc, 25);
    taskdevicevaluename[varNr] = WebServer.arg(argc);

    arg = F("taskdevicevaluedecimals");
    arg += varNr + 1;
    arg.toCharArray(argc, 25);
    taskdevicevaluedecimals[varNr] = WebServer.arg(argc);
  }

  for (byte controllerNr = 0; controllerNr < CONTROLLER_MAX; controllerNr++)
  {
    char argc[25];
    String arg = F("taskdeviceid");
    arg += controllerNr + 1;
    arg.toCharArray(argc, 25);
    taskdeviceid[controllerNr] = WebServer.arg(argc);

    arg = F("taskdevicesenddata");
    arg += controllerNr + 1;
    arg.toCharArray(argc, 25);
    taskdevicesenddata[controllerNr] = WebServer.arg(argc);
  }

  String edit = WebServer.arg(F("edit"));
  byte page = WebServer.arg(F("page")).toInt();
  if (page == 0)
    page = 1;
  byte setpage = WebServer.arg(F("setpage")).toInt();
  if (setpage > 0)
  {
    if (setpage <= (TASKS_MAX / 4))
      page = setpage;
    else
      page = TASKS_MAX / 4;
  }

  String reply = "";
  //reply.reserve(8192);
  addHeader(true, reply);

  byte index = taskindex.toInt();

  byte DeviceIndex = 0;

  if (edit.toInt() != 0) // when form submitted
  {
    if (Settings.TaskDeviceNumber[index - 1] != taskdevicenumber.toInt()) // change of device, clear all other values
    {
      taskClear(index - 1, false); // clear settings, but do not save
      Settings.TaskDeviceNumber[index - 1] = taskdevicenumber.toInt();
      if (taskdevicenumber.toInt() != 0) // preload valuenames
      {
        TempEvent.TaskIndex = index - 1;
        if (ExtraTaskSettings.TaskDeviceValueNames[0][0] == 0) // if field set empty, reload defaults
          PluginCall(PLUGIN_GET_DEVICEVALUENAMES, &TempEvent, dummyString);
      }
    }
    else if (taskdevicenumber.toInt() != 0)
    {
      Settings.TaskDeviceNumber[index - 1] = taskdevicenumber.toInt();
      DeviceIndex = getDeviceIndex(Settings.TaskDeviceNumber[index - 1]);

      if (taskdevicetimer.toInt() > 0)
        Settings.TaskDeviceTimer[index - 1] = taskdevicetimer.toInt();
      else
      {
        if (!Device[DeviceIndex].TimerOptional) // Set default delay, unless it's optional...
          Settings.TaskDeviceTimer[index - 1] = Settings.Delay;
        else
          Settings.TaskDeviceTimer[index - 1] = 0;
      }

      taskdevicename.toCharArray(tmpString, 41);
      Settings.TaskDeviceEnabled[index - 1] = (taskdeviceenabled == "on");
      strcpy(ExtraTaskSettings.TaskDeviceName, tmpString);
      Settings.TaskDevicePort[index - 1] = taskdeviceport.toInt();

      for (byte controllerNr = 0; controllerNr < CONTROLLER_MAX; controllerNr++)
      {
        Settings.TaskDeviceID[controllerNr][index - 1] = taskdeviceid[controllerNr].toInt();
        Settings.TaskDeviceSendData[controllerNr][index - 1] = (taskdevicesenddata[controllerNr] == "on");
      }

      if (taskdevicepin1.length() != 0)
        Settings.TaskDevicePin1[index - 1] = taskdevicepin1.toInt();

      if (taskdevicepin2.length() != 0)
        Settings.TaskDevicePin2[index - 1] = taskdevicepin2.toInt();

      if (taskdevicepin3.length() != 0)
        Settings.TaskDevicePin3[index - 1] = taskdevicepin3.toInt();

      if (Device[DeviceIndex].PullUpOption)
        Settings.TaskDevicePin1PullUp[index - 1] = (taskdevicepin1pullup == "on");

      if (Device[DeviceIndex].InverseLogicOption)
        Settings.TaskDevicePin1Inversed[index - 1] = (taskdevicepin1inversed == "on");

      if (Settings.GlobalSync)
      {
        if (Device[DeviceIndex].GlobalSyncOption)
          Settings.TaskDeviceGlobalSync[index - 1] = (taskdeviceglobalsync == "on");

        // Send task info if set global
        if (Settings.TaskDeviceGlobalSync[index - 1])
        {
          SendUDPTaskInfo(0, index - 1, index - 1);
        }
      }

      for (byte varNr = 0; varNr < Device[DeviceIndex].ValueCount; varNr++)
      {
        taskdeviceformula[varNr].toCharArray(tmpString, 41);
        strcpy(ExtraTaskSettings.TaskDeviceFormula[varNr], tmpString);
        ExtraTaskSettings.TaskDeviceValueDecimals[varNr] = taskdevicevaluedecimals[varNr].toInt();
      }

      // task value names handling.
      for (byte varNr = 0; varNr < Device[DeviceIndex].ValueCount; varNr++)
      {
        taskdevicevaluename[varNr].toCharArray(tmpString, 41);
        strcpy(ExtraTaskSettings.TaskDeviceValueNames[varNr], tmpString);
      }

      TempEvent.TaskIndex = index - 1;
      if (ExtraTaskSettings.TaskDeviceValueNames[0][0] == 0) // if field set empty, reload defaults
        PluginCall(PLUGIN_GET_DEVICEVALUENAMES, &TempEvent, dummyString);

      PluginCall(PLUGIN_WEBFORM_SAVE, &TempEvent, dummyString);
    }
    SaveTaskSettings(index - 1);
    if (!SaveSettings())
      reply += F("<span style=\"color:red\">Error saving to flash!</span>");
    if (taskdevicenumber.toInt() != 0 && Settings.TaskDeviceEnabled[index - 1])
      PluginCall(PLUGIN_INIT, &TempEvent, dummyString);
  }

  // show all tasks as table
  if (index == 0)
  {
    reply += F("<table cellpadding='4' border='1' frame='box' rules='all'><TH>");
    reply += F("<a class=\"button-link\" href=\"devices?setpage=");
    if (page > 1)
      reply += page - 1;
    else
      reply += page;
    reply += F("\"><</a>");
    reply += F("<a class=\"button-link\" href=\"devices?setpage=");
    if (page < (TASKS_MAX / 4))
      reply += page + 1;
    else
      reply += page;
    reply += F("\">></a>");

    reply += F("<TH>Task<TH>Enabled<TH>Device<TH>Name<TH>Port<TH>IDX/Variable<TH>GPIO<TH>Values");

    String deviceName;

    for (byte x = (page - 1) * 4; x < ((page) * 4); x++)
    {
      reply += F("<TR><TD>");
      reply += F("<a class=\"button-link\" href=\"devices?index=");
      reply += x + 1;
      reply += F("&page=");
      reply += page;
      reply += F("\">Edit</a>");
      reply += F("<TD>");
      reply += x + 1;
      reply += F("<TD>");

      if (Settings.TaskDeviceNumber[x] != 0)
      {
        LoadTaskSettings(x);
        DeviceIndex = getDeviceIndex(Settings.TaskDeviceNumber[x]);
        TempEvent.TaskIndex = x;

        deviceName = "";
        Plugin_ptr[DeviceIndex](PLUGIN_GET_DEVICENAME, &TempEvent, deviceName);

        if (Settings.TaskDeviceEnabled[x])
          reply += F("<span style=\"color:green\">Yes</span>");
        else
          reply += F("<span style=\"color:red\">No</span>");

        reply += F("<TD>");
        reply += deviceName;
        reply += F("<TD>");
        reply += ExtraTaskSettings.TaskDeviceName;
        reply += F("<TD>");

        byte customConfig = false;
        customConfig = PluginCall(PLUGIN_WEBFORM_SHOW_CONFIG, &TempEvent, reply);
        if (!customConfig)
          if (Device[DeviceIndex].Ports != 0)
            reply += Settings.TaskDevicePort[x];

        reply += F("<TD>");

        for (byte controllerNr = 0; controllerNr < CONTROLLER_MAX; controllerNr++)
        {
          if (Settings.TaskDeviceID[controllerNr][x] != 0)
            reply += Settings.TaskDeviceID[controllerNr][x];
          if (x < CONTROLLER_MAX)
            reply += F("<BR>");
        }

        reply += F("<TD>");

        if (Settings.TaskDeviceDataFeed[x] == 0)
        {
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

          if (Settings.TaskDevicePin3[x] != -1)
          {
            reply += F("<BR>GPIO-");
            reply += Settings.TaskDevicePin3[x];
          }
        }

        reply += F("<TD>");
        byte customValues = false;
        customValues = PluginCall(PLUGIN_WEBFORM_SHOW_VALUES, &TempEvent, reply);
        if (!customValues)
        {
          if (Device[DeviceIndex].VType == SENSOR_TYPE_LONG)
          {
            reply  += F("<div class=\"div_l\">");
            reply  += ExtraTaskSettings.TaskDeviceValueNames[0];
            reply  += F(":</div><div class=\"div_r\">");
            reply  += (unsigned long)UserVar[x * VARS_PER_TASK] + ((unsigned long)UserVar[x * VARS_PER_TASK + 1] << 16);
            reply  += F("</div>");
          }
          else
          {
            for (byte varNr = 0; varNr < VARS_PER_TASK; varNr++)
            {
              if ((Settings.TaskDeviceNumber[x] != 0) and (varNr < Device[DeviceIndex].ValueCount))
              {
                if (varNr > 0)
                  reply += F("<div class=\"div_br\"></div>");
                reply += F("<div class=\"div_l\">");
                reply += ExtraTaskSettings.TaskDeviceValueNames[varNr];
                reply += F(":</div><div class=\"div_r\">");
                reply += String(UserVar[x * VARS_PER_TASK + varNr], ExtraTaskSettings.TaskDeviceValueDecimals[varNr]);
                reply += "</div>";
              }
            }
          }
        }
      }
      else
        reply += F("<TD><TD><TD><TD><TD><TD>");

    } // next
    reply += F("</table>");
  }
  // Show edit form if a specific entry is chosen with the edit button
  else
  {
    LoadTaskSettings(index - 1);
    DeviceIndex = getDeviceIndex(Settings.TaskDeviceNumber[index - 1]);
    TempEvent.TaskIndex = index - 1;

    reply += F("<form name='frmselect' method='post'><table><TH>Task Settings<TH>Value");

    reply += F("<TR><TD>Device:<TD>");
    addDeviceSelect(reply, "taskdevicenumber", Settings.TaskDeviceNumber[index - 1]);

    if (Settings.TaskDeviceNumber[index - 1] != 0 )
    {
      reply += F("<a class=\"button-link\" href=\"http://www.letscontrolit.com/wiki/index.php/Plugin");
      reply += Settings.TaskDeviceNumber[index - 1];
      reply += F("\" target=\"_blank\">?</a>");

      reply += F("<TR><TD>Name:<TD><input type='text' maxlength='40' name='taskdevicename' value='");
      reply += ExtraTaskSettings.TaskDeviceName;
      reply += F("'>");

      if (Device[DeviceIndex].TimerOption)
      {
        reply += F("<TR><TD>Delay:<TD><input type='text' name='taskdevicetimer' value='");
        reply += Settings.TaskDeviceTimer[index - 1];
        reply += F("'>");
        if (Device[DeviceIndex].TimerOptional)
          reply += F(" (Optional for this device)");
      }

      if (!Device[DeviceIndex].Custom)
      {
        for (byte controllerNr = 0; controllerNr < CONTROLLER_MAX; controllerNr++)
        {
          byte ProtocolIndex = getProtocolIndex(Settings.Protocol[controllerNr]);
          if (Protocol[ProtocolIndex].usesID && Settings.Protocol[controllerNr] != 0)
          {
            reply += F("<TR><TD>IDX / Var for controller ");
            reply += controllerNr + 1;
            reply += F("<TD><input type='text' name='taskdeviceid");
            reply += controllerNr + 1;
            reply += F("' value='");
            reply += Settings.TaskDeviceID[controllerNr][index - 1];
            reply += F("'>");
          }
        }
      }

      if (!Device[DeviceIndex].Custom && Settings.TaskDeviceDataFeed[index - 1] == 0)
      {
        if (Device[DeviceIndex].Ports != 0)
        {
          reply += F("<TR><TD>Port:<TD><input type='text' name='taskdeviceport' value='");
          reply += Settings.TaskDevicePort[index - 1];
          reply += F("'>");
        }

        if (Device[DeviceIndex].Type >= DEVICE_TYPE_SINGLE && Device[DeviceIndex].Type <= DEVICE_TYPE_TRIPLE)
        {
          reply += F("<TR><TD>1st GPIO:<TD>");
          addPinSelect(false, reply, "taskdevicepin1", Settings.TaskDevicePin1[index - 1]);
        }
        if (Device[DeviceIndex].Type >= DEVICE_TYPE_DUAL && Device[DeviceIndex].Type <= DEVICE_TYPE_TRIPLE)
        {
          reply += F("<TR><TD>2nd GPIO:<TD>");
          addPinSelect(false, reply, "taskdevicepin2", Settings.TaskDevicePin2[index - 1]);
        }
        if (Device[DeviceIndex].Type == DEVICE_TYPE_TRIPLE)
        {
          reply += F("<TR><TD>3rd GPIO:<TD>");
          addPinSelect(false, reply, "taskdevicepin3", Settings.TaskDevicePin3[index - 1]);
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
      }

      if (Settings.TaskDeviceDataFeed[index - 1] == 0) // only show additional config for local connected sensors
        PluginCall(PLUGIN_WEBFORM_LOAD, &TempEvent, reply);

      if (Device[DeviceIndex].SendDataOption)
      {
        for (byte controllerNr = 0; controllerNr < CONTROLLER_MAX; controllerNr++)
        {
          if (Settings.Protocol[controllerNr] != 0)
          {
            reply += F("<TR><TD>Send Data to controller ");
            reply += controllerNr + 1;
            reply += F("<TD><input type=checkbox name='taskdevicesenddata");
            reply += controllerNr + 1;
            reply += F("'");
            if (Settings.TaskDeviceSendData[controllerNr][index - 1])
              reply += F(" checked>");
            else
              reply += F(">");
          }
        }
      }

      if (Settings.GlobalSync && Device[DeviceIndex].GlobalSyncOption && Settings.TaskDeviceDataFeed[index - 1] == 0 && Settings.UDPPort != 0)
      {
        reply += F("<TR><TD>Global Sync:<TD>");
        if (Settings.TaskDeviceGlobalSync[index - 1])
          reply += F("<input type=checkbox name=taskdeviceglobalsync checked>");
        else
          reply += F("<input type=checkbox name=taskdeviceglobalsync>");
      }

      reply += F("<TR><TD>Enabled:<TD>");
      if (Settings.TaskDeviceEnabled[index - 1])
        reply += F("<input type=checkbox name=taskdeviceenabled checked>");
      else
        reply += F("<input type=checkbox name=taskdeviceenabled>");

      if (!Device[DeviceIndex].Custom)
      {
        reply += F("<TR><TH>Optional Settings<TH>Value");

        if (Device[DeviceIndex].FormulaOption)
        {
          for (byte varNr = 0; varNr < Device[DeviceIndex].ValueCount; varNr++)
          {
            reply += F("<TR><TD>Formula ");
            reply += ExtraTaskSettings.TaskDeviceValueNames[varNr];
            reply += F(":<TD><input type='text' maxlength='40' name='taskdeviceformula");
            reply += varNr + 1;
            reply += F("' value='");
            reply += ExtraTaskSettings.TaskDeviceFormula[varNr];
            reply += F("'>");

            reply += F(" Decimals: <input type='text' name='taskdevicevaluedecimals");
            reply += varNr + 1;
            reply += F("' value='");
            reply += ExtraTaskSettings.TaskDeviceValueDecimals[varNr];
            reply += F("'>");

            if (varNr == 0)
              reply += F("<a class=\"button-link\" href=\"http://www.letscontrolit.com/wiki/index.php/EasyFormula\" target=\"_blank\">?</a>");
          }
        }
        else
        {
          if (Device[DeviceIndex].DecimalsOnly)
            for (byte varNr = 0; varNr < Device[DeviceIndex].ValueCount; varNr++)
            {
              reply += F("<TR><TD>Decimals ");
              reply += ExtraTaskSettings.TaskDeviceValueNames[varNr];
              reply += F(":<TD><input type='text' name='taskdevicevaluedecimals");
              reply += varNr + 1;
              reply += F("' value='");
              reply += ExtraTaskSettings.TaskDeviceValueDecimals[varNr];
              reply += F("'>");
            }
        }

        for (byte varNr = 0; varNr < Device[DeviceIndex].ValueCount; varNr++)
        {
          reply += F("<TR><TD>Value Name ");
          reply += varNr + 1;
          reply += F(":<TD><input type='text' maxlength='40' name='taskdevicevaluename");
          reply += varNr + 1;
          reply += F("' value='");
          reply += ExtraTaskSettings.TaskDeviceValueNames[varNr];
          reply += F("'>");
        }
      }

    }
    reply += F("<TR><TD><TD><a class=\"button-link\" href=\"devices?setpage=");
    reply += page;
    reply += F("\">Close</a>");
    reply += F("<input class=\"button-link\" type='submit' value='Submit'>");
    reply += F("<input type='hidden' name='edit' value='1'>");
    reply += F("<input type='hidden' name='page' value='1'>");
    reply += F("</table></form>");
  }

  addFooter(reply);
  checkRAM(9);
  String log = F("DEBUG: String size:");
  log += reply.length();
  addLog(LOG_LEVEL_DEBUG_MORE, log);
  WebServer.send(200, "text/html", reply);
}


byte sortedIndex[DEVICES_MAX + 1];
//********************************************************************************
// Add a device select dropdown list
//********************************************************************************
void addDeviceSelect(String& str, String name,  int choice)
{
  // first get the list in alphabetic order
  for (byte x = 0; x <= deviceCount; x++)
    sortedIndex[x] = x;
  sortDeviceArray();

  String deviceName;

  str += F("<select name='");
  str += name;
  //str += "'>";
  str += "' LANGUAGE=javascript onchange=\"return dept_onchange(frmselect)\">";

  str += F("<option value='0'></option>");
  for (byte x = 0; x <= deviceCount; x++)
  {
    byte index = sortedIndex[x];
    if (Plugin_id[index] != 0)
      Plugin_ptr[index](PLUGIN_GET_DEVICENAME, 0, deviceName);
    str += F("<option value='");
    str += Device[index].Number;
    str += "'";
    if (choice == Device[index].Number)
      str += F(" selected");
    str += ">";
    str += deviceName;
    str += F("</option>");
  }
  str += F("</select>");
}


//********************************************************************************
// Device Sort routine, switch array entries
//********************************************************************************
void switchArray(byte value)
{
  byte temp;
  temp = sortedIndex[value - 1];
  sortedIndex[value - 1] = sortedIndex[value];
  sortedIndex[value] = temp;
}


//********************************************************************************
// Device Sort routine, compare two array entries
//********************************************************************************
byte arrayLessThan(char *ptr_1, char *ptr_2)
{
  char check1;
  char check2;

  int i = 0;
  while (i < strlen(ptr_1))    // For each character in string 1, starting with the first:
  {
    check1 = (char)ptr_1[i];  // get the same char from string 1 and string 2

    if (strlen(ptr_2) < i)    // If string 2 is shorter, then switch them
    {
      return 1;
    }
    else
    {
      check2 = (char)ptr_2[i];

      if (check2 > check1)
      {
        return 1;       // String 2 is greater; so switch them
      }
      if (check2 < check1)
      {
        return 0;       // String 2 is LESS; so DONT switch them
      }
      // OTHERWISE they're equal so far; check the next char !!
      i++;
    }
  }

  return 0;
}


//********************************************************************************
// Device Sort routine, actual sorting
//********************************************************************************
void sortDeviceArray()
{
  String deviceName;
  char deviceName1[41];
  char deviceName2[41];
  int innerLoop ;
  int mainLoop ;

  for ( mainLoop = 1; mainLoop <= deviceCount; mainLoop++)
  {
    innerLoop = mainLoop;
    while (innerLoop  >= 1)
    {
      Plugin_ptr[sortedIndex[innerLoop]](PLUGIN_GET_DEVICENAME, 0, deviceName);
      deviceName.toCharArray(deviceName1, 26);
      Plugin_ptr[sortedIndex[innerLoop - 1]](PLUGIN_GET_DEVICENAME, 0, deviceName);
      deviceName.toCharArray(deviceName2, 26);

      if (arrayLessThan(deviceName1, deviceName2) == 1)
      {
        switchArray(innerLoop);
      }
      innerLoop--;
    }
  }
}


//********************************************************************************
// Add a GPIO pin select dropdown list for both 8266 and 8285
//********************************************************************************
#ifdef ESP8285
// Code for the ESP8285

//********************************************************************************
// Add a GPIO pin select dropdown list
//********************************************************************************
void addPinSelect(boolean forI2C, String& str, String name,  int choice)
{
  String options[18];
  options[0] = F(" ");
  options[1] = F("GPIO-0 (D3)");
  options[2] = F("GPIO-1 (D10)");
  options[3] = F("GPIO-2 (D4)");
  options[4] = F("GPIO-3 (D9)");
  options[5] = F("GPIO-4 (D2)");
  options[6] = F("GPIO-5 (D1)");
  options[7] = F("GPIO-6");
  options[8] = F("GPIO-7");
  options[9] = F("GPIO-8");
  options[10] = F("GPIO-9 (D11)");
  options[11] = F("GPIO-10 (D12)");
  options[12] = F("GPIO-11");
  options[13] = F("GPIO-12 (D6)");
  options[14] = F("GPIO-13 (D7)");
  options[15] = F("GPIO-14 (D5)");
  options[16] = F("GPIO-15 (D8)");
  options[17] = F("GPIO-16 (D0)");
  int optionValues[18];
  optionValues[0] = -1;
  optionValues[1] = 0;
  optionValues[2] = 1;
  optionValues[3] = 2;
  optionValues[4] = 3;
  optionValues[5] = 4;
  optionValues[6] = 5;
  optionValues[7] = 7;
  optionValues[8] = 7;
  optionValues[9] = 8;
  optionValues[10] = 9;
  optionValues[11] = 10;
  optionValues[12] = 11;
  optionValues[13] = 12;
  optionValues[14] = 13;
  optionValues[15] = 14;
  optionValues[16] = 15;
  optionValues[17] = 16;
  renderHTMLForPinSelect(options, optionValues, forI2C, str, name, choice, 18);
}


#else
// Code for the ESP8266

//********************************************************************************
// Add a GPIO pin select dropdown list
//********************************************************************************
void addPinSelect(boolean forI2C, String& str, String name,  int choice)
{
  String options[14];
  options[0] = F(" ");
  options[1] = F("GPIO-0 (D3)");
  options[2] = F("GPIO-1 (D10)");
  options[3] = F("GPIO-2 (D4)");
  options[4] = F("GPIO-3 (D9)");
  options[5] = F("GPIO-4 (D2)");
  options[6] = F("GPIO-5 (D1)");
  options[7] = F("GPIO-9 (D11)");
  options[8] = F("GPIO-10 (D12)");
  options[9] = F("GPIO-12 (D6)");
  options[10] = F("GPIO-13 (D7)");
  options[11] = F("GPIO-14 (D5)");
  options[12] = F("GPIO-15 (D8)");
  options[13] = F("GPIO-16 (D0)");
  int optionValues[14];
  optionValues[0] = -1;
  optionValues[1] = 0;
  optionValues[2] = 1;
  optionValues[3] = 2;
  optionValues[4] = 3;
  optionValues[5] = 4;
  optionValues[6] = 5;
  optionValues[7] = 9;
  optionValues[8] = 10;
  optionValues[9] = 12;
  optionValues[10] = 13;
  optionValues[11] = 14;
  optionValues[12] = 15;
  optionValues[13] = 16;
  renderHTMLForPinSelect(options, optionValues, forI2C, str, name, choice, 14);
}

#endif

//********************************************************************************
// Helper function actually rendering dropdown list for addPinSelect()
//********************************************************************************
void renderHTMLForPinSelect(String options[], int optionValues[], boolean forI2C, String& str, String name,  int choice, int count) {
    str += F("<select name='");
    str += name;
    str += "'>";
    for (byte x = 0; x < count; x++)
    {
      str += F("<option value='");
      str += optionValues[x];
      str += "'";
      if (optionValues[x] != -1) // empty selection can never be disabled...
      {
        if (!forI2C && ((optionValues[x] == Settings.Pin_i2c_sda) || (optionValues[x] == Settings.Pin_i2c_scl)))
          str += F(" disabled");
        if (Settings.UseSerial && ((optionValues[x] == 1) || (optionValues[x] == 3)))
          str += F(" disabled");
      }
      if (choice == optionValues[x])
        str += F(" selected");
      str += ">";
      str += options[x];
      str += F("</option>");
    }
    str += F("</select>");

}

//********************************************************************************
// Add a task select dropdown list
//********************************************************************************
void addTaskSelect(String& str, String name,  int choice)
{
  struct EventStruct TempEvent;
  String deviceName;

  str += F("<select name='");
  str += name;
  str += F("' LANGUAGE=javascript onchange=\"return dept_onchange(frmselect)\">");

  for (byte x = 0; x < TASKS_MAX; x++)
  {
    deviceName = "";
    if (Settings.TaskDeviceNumber[x] != 0 )
    {
      byte DeviceIndex = getDeviceIndex(Settings.TaskDeviceNumber[x]);

      if (Plugin_id[DeviceIndex] != 0)
        Plugin_ptr[DeviceIndex](PLUGIN_GET_DEVICENAME, &TempEvent, deviceName);
    }
    LoadTaskSettings(x);
    str += F("<option value='");
    str += x;
    str += "'";
    if (choice == x)
      str += " selected";
    if (Settings.TaskDeviceNumber[x] == 0)
      str += " disabled";
    str += ">";
    str += x + 1;
    str += " - ";
    str += deviceName;
    str += " - ";
    str += ExtraTaskSettings.TaskDeviceName;
    str += "</option>";
  }
}


//********************************************************************************
// Add a Value select dropdown list, based on TaskIndex
//********************************************************************************
void addTaskValueSelect(String& str, String name,  int choice, byte TaskIndex)
{
  str += F("<select name='");
  str += name;
  str += "'>";

  byte DeviceIndex = getDeviceIndex(Settings.TaskDeviceNumber[TaskIndex]);

  for (byte x = 0; x < Device[DeviceIndex].ValueCount; x++)
  {
    str += F("<option value='");
    str += x;
    str += "'";
    if (choice == x)
      str += " selected";
    str += ">";
    str += ExtraTaskSettings.TaskDeviceValueNames[x];
    str += "</option>";
  }
}


//********************************************************************************
// Web Interface log page
//********************************************************************************
void handle_log() {
  if (!isLoggedIn()) return;

  char *TempString = (char*)malloc(80);

  String reply = "";
  addHeader(true, reply);
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

  String webrequest = WebServer.arg(F("cmd"));

  String reply = "";
  addHeader(true, reply);

  reply += F("<form>");
  reply += F("<table><TH>Tools<TH>");
  reply += F("<TR><TD>System<TD><a class=\"button-link\" href=\"/?cmd=reboot\">Reboot</a>");
  reply += F("<a class=\"button-link\" href=\"log\">Log</a>");
  reply += F("<a class=\"button-link\" href=\"sysinfo\">Info</a>");
  reply += F("<a class=\"button-link\" href=\"advanced\">Advanced</a><BR><BR>");
  reply += F("<TR><TD>Wifi<TD><a class=\"button-link\" href=\"/?cmd=wificonnect\">Connect</a>");
  reply += F("<a class=\"button-link\" href=\"/?cmd=wifidisconnect\">Disconnect</a>");
  reply += F("<a class=\"button-link\" href=\"/wifiscanner\">Scan</a><BR><BR>");
  reply += F("<TR><TD>Interfaces<TD><a class=\"button-link\" href=\"/i2cscanner\">I2C Scan</a><BR><BR>");
  reply += F("<TR><TD>Settings<TD><a class=\"button-link\" href=\"/upload\">Load</a>");
  reply += F("<a class=\"button-link\" href=\"/download\">Save</a> (If you change filename, load will not work!!)");
  if (ESP.getFlashChipRealSize() > 524288)
  {
    reply += F("<TR><TD>Firmware<TD><a class=\"button-link\" href=\"/update\">Load</a>");
    reply += F("<a class=\"button-link\" href=\"http://www.letscontrolit.com/wiki/index.php/EasyOTA\" target=\"_blank\">?</a>");
  }
  reply += F("<TR><TD>Filesystem<TD><a class=\"button-link\" href=\"/filelist\">Flash</a>");
  reply += F("<a class=\"button-link\" href=\"/SDfilelist\">SD Card</a><BR><BR>");

  reply += F("<TR><TD>Command<TD>");
  reply += F("<input type='text' name='cmd' value='");
  reply += webrequest;
  reply += F("'><TR><TD><TD><input class=\"button-link\" type='submit' value='Submit'><TR><TD>");

  printToWeb = true;
  printWebString = "";

  if (webrequest.length() > 0)
  {
    struct EventStruct TempEvent;
    parseCommandString(&TempEvent, webrequest);
    TempEvent.Source = VALUE_SOURCE_HTTP;
    if (!PluginCall(PLUGIN_WRITE, &TempEvent, webrequest))
      ExecuteCommand(VALUE_SOURCE_HTTP, webrequest.c_str());
  }

  if (printWebString.length() > 0)
  {
    reply += F("<TR><TD>Command Output<TD><textarea readonly rows='10' cols='60' wrap='on'>");
    reply += printWebString;
    reply += F("</textarea>");
  }
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
  addHeader(true, reply);
  reply += F("<table cellpadding='4' border='1' frame='box' rules='all'><TH>I2C Addresses in use<TH>Supported devices");

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
        case 0x21:
        case 0x22:
        case 0x25:
        case 0x26:
        case 0x27:
          reply += F("PCF8574<BR>MCP23017<BR>LCD");
          break;
        case 0x23:
          reply += F("PCF8574<BR>MCP23017<BR>LCD<BR>BH1750");
          break;
        case 0x24:
          reply += F("PCF8574<BR>MCP23017<BR>LCD<BR>PN532");
          break;
        case 0x29:
          reply += F("TSL2561");
          break;
        case 0x38:
        case 0x3A:
        case 0x3B:
        case 0x3E:
        case 0x3F:
          reply += F("PCF8574A");
          break;
        case 0x39:
          reply += F("PCF8574A<BR>TSL2561");
          break;
        case 0x3C:
        case 0x3D:
          reply += F("PCF8574A<BR>OLED");
          break;
        case 0x40:
          reply += F("SI7021<BR>HTU21D<BR>INA219<BR>PCA9685");
          break;
        case 0x41:
        case 0x42:
        case 0x43:
          reply += F("INA219");
          break;
        case 0x48:
        case 0x4A:
        case 0x4B:
          reply += F("PCF8591<BR>ADS1115");
          break;
        case 0x49:
          reply += F("PCF8591<BR>ADS1115<BR>TSL2561");
          break;
        case 0x4C:
        case 0x4D:
        case 0x4E:
        case 0x4F:
          reply += F("PCF8591");
          break;
        case 0x5A:
          reply += F("MLX90614");
          break;
        case 0x5C:
          reply += F("DHT12<BR>BH1750");
          break;
        case 0x60:
          reply += F("Adafruit Motorshield v2<BR>SI1145");
          break;
        case 0x70:
          reply += F("Adafruit Motorshield v2 (Catchall)");
          break;
        case 0x76:
          reply += F("BME280<BR>BMP280<BR>MS5607<BR>MS5611");
          break;
        case 0x77:
          reply += F("BMP085<BR>BMP180<BR>BME280<BR>BMP280<BR>MS5607<BR>MS5611");
          break;
        case 0x7f:
          reply += F("Arduino PME");
          break;
      }
      nDevices++;
    }
    else if (error == 4)
    {
      reply += F("<TR><TD>Unknown error at address 0x");
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
  addHeader(true, reply);
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

  String webrequest = WebServer.arg(F("password"));
  char command[80];
  command[0] = 0;
  webrequest.toCharArray(command, 80);

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

  String webrequest = WebServer.arg(F("cmd"));

  // in case of event, store to buffer and return...
  String command = parseString(webrequest, 1);
  if (command == F("event"))
  {
    eventBuffer = webrequest.substring(6);
    WebServer.send(200, "text/html", "OK");
  }

  struct EventStruct TempEvent;
  parseCommandString(&TempEvent, webrequest);
  TempEvent.Source = VALUE_SOURCE_HTTP;

  printToWeb = true;
  printWebString = "";
  String reply = "";

  if (PluginCall(PLUGIN_WRITE, &TempEvent, webrequest));
  else if (remoteConfig(&TempEvent, webrequest));
  else
    reply += F("Unknown or restricted command!");

  reply += printWebString;

  if (printToWebJSON)
    WebServer.send(200, "application/json", reply);
  else
    WebServer.send(200, "text/html", reply);

  printWebString = "";
  printToWeb = false;
  printToWebJSON = false;
}


//********************************************************************************
// Web Interface JSON page (no password!)
//********************************************************************************

boolean handle_json()
{
  String tasknr = WebServer.arg(F("tasknr"));
  String reply = "";

  if (tasknr.length() == 0)
  {
    reply += F("{\"System\":{\n");
    reply += F("\"Build\": ");
    reply += BUILD;
    reply += F(",\n\"Unit\": ");
    reply += Settings.Unit;
    reply += F(",\n\"Uptime\": ");
    reply += wdcounter / 2;
    reply += F(",\n\"Free RAM\": ");
    reply += ESP.getFreeHeap();
    reply += F("\n},\n");
  }

  byte taskNr = tasknr.toInt();
  byte firstTaskIndex = 0;
  byte lastTaskIndex = TASKS_MAX - 1;
  if (taskNr != 0 )
  {
    firstTaskIndex = taskNr - 1;
    lastTaskIndex = taskNr - 1;
  }

  byte lastActiveTaskIndex = 0;
  for (byte TaskIndex = firstTaskIndex; TaskIndex <= lastTaskIndex; TaskIndex++)
    if (Settings.TaskDeviceNumber[TaskIndex])
      lastActiveTaskIndex = TaskIndex;

  if (taskNr == 0 )
    reply += F("\"Sensors\":[\n");
  for (byte TaskIndex = firstTaskIndex; TaskIndex <= lastTaskIndex; TaskIndex++)
  {
    if (Settings.TaskDeviceNumber[TaskIndex])
    {
      byte BaseVarIndex = TaskIndex * VARS_PER_TASK;
      byte DeviceIndex = getDeviceIndex(Settings.TaskDeviceNumber[TaskIndex]);
      LoadTaskSettings(TaskIndex);
      reply += F("{\n");

      reply += F("\"TaskName\": \"");
      reply += ExtraTaskSettings.TaskDeviceName;
      reply += F("\"");
      if (Device[DeviceIndex].ValueCount != 0)
        reply += F(",");
      reply += F("\n");

      for (byte x = 0; x < Device[DeviceIndex].ValueCount; x++)
      {
        reply += F("\"");
        reply += ExtraTaskSettings.TaskDeviceValueNames[x];
        reply += F("\": ");
        reply += UserVar[BaseVarIndex + x];
        if (x < (Device[DeviceIndex].ValueCount - 1))
          reply += F(",");
        reply += F("\n");
      }
      reply += F("}");
      if (TaskIndex != lastActiveTaskIndex)
        reply += F(",");
      reply += F("\n");
    }
  }
  if (taskNr == 0 )
    reply += F("]}\n");

  WebServer.send(200, "application/json", reply);
}


//********************************************************************************
// Web Interface config page
//********************************************************************************
void handle_advanced() {
  if (!isLoggedIn()) return;

  char tmpString[81];

  String messagedelay = WebServer.arg(F("messagedelay"));
  String ip = WebServer.arg(F("ip"));
  String syslogip = WebServer.arg(F("syslogip"));
  String ntphost = WebServer.arg(F("ntphost"));
  String timezone = WebServer.arg(F("timezone"));
  String dst = WebServer.arg(F("dst"));
  String sysloglevel = WebServer.arg(F("sysloglevel"));
  String udpport = WebServer.arg(F("udpport"));
  String useserial = WebServer.arg(F("useserial"));
  String serialloglevel = WebServer.arg(F("serialloglevel"));
  String webloglevel = WebServer.arg(F("webloglevel"));
  String sdloglevel = WebServer.arg(F("sdloglevel"));
  String baudrate = WebServer.arg(F("baudrate"));
  String usentp = WebServer.arg(F("usentp"));
  String wdi2caddress = WebServer.arg(F("wdi2caddress"));
  String usessdp = WebServer.arg(F("usessdp"));
  String edit = WebServer.arg(F("edit"));
  String wireclockstretchlimit = WebServer.arg(F("wireclockstretchlimit"));
  String globalsync = WebServer.arg(F("globalsync"));
  String userules = WebServer.arg(F("userules"));
  String cft = WebServer.arg(F("cft"));
  String MQTTRetainFlag = WebServer.arg(F("mqttretainflag"));

  String reply = "";
  addHeader(true, reply);

  if (edit.length() != 0)
  {
    Settings.MessageDelay = messagedelay.toInt();
    Settings.IP_Octet = ip.toInt();
    ntphost.toCharArray(tmpString, 64);
    strcpy(Settings.NTPHost, tmpString);
    Settings.TimeZone = timezone.toInt();
    syslogip.toCharArray(tmpString, 26);
    str2ip(tmpString, Settings.Syslog_IP);
    Settings.UDPPort = udpport.toInt();
    Settings.SyslogLevel = sysloglevel.toInt();
    Settings.UseSerial = (useserial == "on");
    Settings.SerialLogLevel = serialloglevel.toInt();
    Settings.WebLogLevel = webloglevel.toInt();
    Settings.SDLogLevel = sdloglevel.toInt();
    Settings.BaudRate = baudrate.toInt();
    Settings.UseNTP = (usentp == "on");
    Settings.DST = (dst == "on");
    Settings.WDI2CAddress = wdi2caddress.toInt();
    Settings.UseSSDP = (usessdp == "on");
    Settings.WireClockStretchLimit = wireclockstretchlimit.toInt();
    Settings.UseRules = (userules == "on");
    Settings.GlobalSync = (globalsync == "on");
    Settings.ConnectionFailuresThreshold = cft.toInt();
    Settings.MQTTRetainFlag = (MQTTRetainFlag == "on");
    if (!SaveSettings())
      reply += F("<span style=\"color:red\">Error saving to flash!</span>");
    if (Settings.UseNTP)
      initTime();
  }

  char str[20];

  reply += F("<form  method='post'><table>");
  reply += F("<TH>Advanced Settings<TH>Value");

  reply += F("<TR><TD>MQTT Retain Msg:<TD>");
  if (Settings.MQTTRetainFlag)
    reply += F("<input type=checkbox name='mqttretainflag' checked>");
  else
    reply += F("<input type=checkbox name='mqttretainflag'>");

  reply += F("<TR><TD>Message Delay (ms):<TD><input type='text' name='messagedelay' value='");
  reply += Settings.MessageDelay;

  reply += F("'><TR><TD>Fixed IP Octet:<TD><input type='text' name='ip' value='");
  reply += Settings.IP_Octet;
  reply += F("'>");

  reply += F("<TR><TD>Use NTP:<TD>");
  if (Settings.UseNTP)
    reply += F("<input type=checkbox name='usentp' checked>");
  else
    reply += F("<input type=checkbox name='usentp'>");

  reply += F("<TR><TD>NTP Hostname:<TD><input type='text' name='ntphost' size=64 value='");
  reply += Settings.NTPHost;

  reply += F("'><TR><TD>Timezone Offset: (Minutes)<TD><input type='text' name='timezone' size=2 value='");
  reply += Settings.TimeZone;
  reply += F("'>");

  reply += F("<TR><TD>DST:<TD>");
  if (Settings.DST)
    reply += F("<input type=checkbox name='dst' checked>");
  else
    reply += F("<input type=checkbox name='dst'>");

  reply += F("<TR><TD>Syslog IP:<TD><input type='text' name='syslogip' value='");
  str[0] = 0;
  sprintf_P(str, PSTR("%u.%u.%u.%u"), Settings.Syslog_IP[0], Settings.Syslog_IP[1], Settings.Syslog_IP[2], Settings.Syslog_IP[3]);
  reply += str;

  reply += F("'><TR><TD>Syslog Level:<TD><input type='text' name='sysloglevel' value='");
  reply += Settings.SyslogLevel;

  reply += F("'><TR><TD>UDP port:<TD><input type='text' name='udpport' value='");
  reply += Settings.UDPPort;
  reply += F("'>");

  reply += F("<TR><TD>Enable Serial port:<TD>");
  if (Settings.UseSerial)
    reply += F("<input type=checkbox name='useserial' checked>");
  else
    reply += F("<input type=checkbox name='useserial'>");

  reply += F("<TR><TD>Serial log Level:<TD><input type='text' name='serialloglevel' value='");
  reply += Settings.SerialLogLevel;

  reply += F("'><TR><TD>Web log Level:<TD><input type='text' name='webloglevel' value='");
  reply += Settings.WebLogLevel;

  reply += F("'><TR><TD>SD Card log Level:<TD><input type='text' name='sdloglevel' value='");
  reply += Settings.SDLogLevel;

  reply += F("'><TR><TD>Baud Rate:<TD><input type='text' name='baudrate' value='");
  reply += Settings.BaudRate;
  reply += F("'>");

  reply += F("<TR><TD>WD I2C Address:<TD><input type='text' name='wdi2caddress' value='");
  reply += Settings.WDI2CAddress;
  reply += F("'>");

  reply += F("<TR><TD>Use SSDP:<TD>");
  if (Settings.UseSSDP)
    reply += F("<input type=checkbox name='usessdp' checked>");
  else
    reply += F("<input type=checkbox name='usessdp'>");

  reply += F("<TR><TD>Connection Failure Threshold:<TD><input type='text' name='cft' value='");
  reply += Settings.ConnectionFailuresThreshold;
  reply += F("'>");

  reply += F("<TR><TD>Rules:<TD>");
  if (Settings.UseRules)
    reply += F("<input type=checkbox name='userules' checked>");
  else
    reply += F("<input type=checkbox name='userules'>");

  reply += F("<TR><TH>Experimental Settings<TH>Value");

  reply += F("<TR><TD>I2C ClockStretchLimit:<TD><input type='text' name='wireclockstretchlimit' value='");
  reply += Settings.WireClockStretchLimit;
  reply += F("'>");

  reply += F("<TR><TD>Global Sync:<TD>");
  if (Settings.GlobalSync)
    reply += F("<input type=checkbox name='globalsync' checked>");
  else
    reply += F("<input type=checkbox name='globalsync'>");

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
    WebServer.sendContent(F("HTTP/1.1 302 \r\nLocation: /login\r\n"));
  }
  else
  {
    WebLoggedInTimer = 0;
  }

  return WebLoggedIn;
}


//********************************************************************************
// Web Interface download page
//********************************************************************************
void handle_download()
{
  if (!isLoggedIn()) return;

  fs::File dataFile = SPIFFS.open("config.dat", "r");
  if (!dataFile)
    return;

  WebServer.sendHeader("Content-Disposition", "attachment; filename=config.dat");
  WebServer.streamFile(dataFile, "application/octet-stream");
}


//********************************************************************************
// Web Interface upload page
//********************************************************************************
byte uploadResult = 0;
void handle_upload() {
  if (!isLoggedIn()) return;

  String reply = "";
  addHeader(true, reply);

  reply += F("<form enctype=\"multipart/form-data\" method=\"post\"><p>Upload settings file:<br><input type=\"file\" name=\"datafile\" size=\"40\"></p><div><input class=\"button-link\" type='submit' value='Upload'></div><input type='hidden' name='edit' value='1'></form>");
  addFooter(reply);
  WebServer.send(200, "text/html", reply);
  printWebString = "";
  printToWeb = false;
}


//********************************************************************************
// Web Interface upload page
//********************************************************************************
void handle_upload_post() {
  if (!isLoggedIn()) return;

  String reply = "";

  if (uploadResult == 1)
  {
    reply += F("Upload OK!<BR>You may need to reboot to apply all settings...");
    LoadSettings();
  }

  if (uploadResult == 2)
    reply += F("<font color=\"red\">Upload file invalid!</font>");

  if (uploadResult == 3)
    reply += F("<font color=\"red\">No filename!</font>");

  addHeader(true, reply);
  reply += F("Upload finished");
  addFooter(reply);
  WebServer.send(200, "text/html", reply);
  printWebString = "";
  printToWeb = false;
}


//********************************************************************************
// Web Interface upload handler
//********************************************************************************
fs::File uploadFile;
void handleFileUpload() {
  if (!isLoggedIn()) return;

  static boolean valid = false;
  String log = "";

  HTTPUpload& upload = WebServer.upload();

  if (upload.filename.c_str()[0] == 0)
  {
    uploadResult = 3;
    return;
  }

  if (upload.status == UPLOAD_FILE_START)
  {
    log = F("Upload: START, filename: ");
    log += upload.filename;
    addLog(LOG_LEVEL_INFO, log);
    valid = false;
    uploadResult = 0;
  }
  else if (upload.status == UPLOAD_FILE_WRITE)
  {
    // first data block, if this is the config file, check PID/Version
    if (upload.totalSize == 0)
    {
      if (strcasecmp(upload.filename.c_str(), "config.dat") == 0)
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
        // once we're safe, remove file and create empty one...
        SPIFFS.remove((char *)upload.filename.c_str());
        uploadFile = SPIFFS.open(upload.filename.c_str(), "w");
        flashCount();
      }
    }
    if (uploadFile) uploadFile.write(upload.buf, upload.currentSize);
    log = F("Upload: WRITE, Bytes: ");
    log += upload.currentSize;
    addLog(LOG_LEVEL_INFO, log);
  }
  else if (upload.status == UPLOAD_FILE_END)
  {
    if (uploadFile) uploadFile.close();
    log = F("Upload: END, Size: ");
    log += upload.totalSize;
    addLog(LOG_LEVEL_INFO, log);
  }

  if (valid)
    uploadResult = 1;
  else
    uploadResult = 2;

}


//********************************************************************************
// Web Interface server web file from SPIFFS
//********************************************************************************
bool loadFromFS(boolean spiffs, String path) {
  if (!isLoggedIn()) return false;

  String dataType = F("text/plain");
  if (path.endsWith("/")) path += F("index.htm");

  if (path.endsWith(".src")) path = path.substring(0, path.lastIndexOf("."));
  else if (path.endsWith(".htm")) dataType = F("text/html");
  else if (path.endsWith(".css")) dataType = F("text/css");
  else if (path.endsWith(".js")) dataType = F("application/javascript");
  else if (path.endsWith(".png")) dataType = F("image/png");
  else if (path.endsWith(".gif")) dataType = F("image/gif");
  else if (path.endsWith(".jpg")) dataType = F("image/jpeg");
  else if (path.endsWith(".ico")) dataType = F("image/x-icon");
  else if (path.endsWith(".txt")) dataType = F("application/octet-stream");
  else if (path.endsWith(".dat")) dataType = F("application/octet-stream");

  path = path.substring(1);
  if (spiffs)
  {
    fs::File dataFile = SPIFFS.open(path.c_str(), "r");
    if (!dataFile)
      return false;
    if (path.endsWith(".dat"))
      WebServer.sendHeader("Content-Disposition", "attachment;");
    WebServer.streamFile(dataFile, dataType);
    dataFile.close();
  }
  else
  {
    File dataFile = SD.open(path.c_str());
    if (!dataFile)
      return false;
    if (path.endsWith(".DAT"))
      WebServer.sendHeader("Content-Disposition", "attachment;");
    WebServer.streamFile(dataFile, dataType);
    dataFile.close();
  }
  return true;
}


//********************************************************************************
// Web Interface file list
//********************************************************************************
void handle_filelist() {

  String fdelete = WebServer.arg(F("delete"));

  if (fdelete.length() > 0)
  {
    SPIFFS.remove(fdelete);
    flashCount();
  }

  String reply = "";
  addHeader(true, reply);
  reply += F("<table cellpadding='4' border='1' frame='box' rules='all'><TH><TH>Filename:<TH>Size");

  fs::Dir dir = SPIFFS.openDir("");
  while (dir.next())
  {
    reply += F("<TR><TD>");
    if (dir.fileName() != "config.dat" && dir.fileName() != "security.dat" && dir.fileName() != "notification.dat")
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
    fs::File f = dir.openFile("r");
    reply += F("<TD>");
    reply += f.size();
  }
  reply += F("</table></form>");
  reply += F("<BR><a class=\"button-link\" href=\"/upload\">Upload</a>");
  addFooter(reply);
  WebServer.send(200, "text/html", reply);
}


//********************************************************************************
// Web Interface SD card file list
//********************************************************************************
void handle_SDfilelist() {

  String fdelete = WebServer.arg(F("delete"));

  if (fdelete.length() > 0)
  {
    SD.remove((char*)fdelete.c_str());
  }

  String reply = "";
  addHeader(true, reply);
  reply += F("<table cellpadding='4' border='1' frame='box' rules='all'><TH><TH>Filename:<TH>Size");

  File root = SD.open("/");
  root.rewindDirectory();
  File entry = root.openNextFile();
  while (entry)
  {
    if (!entry.isDirectory())
    {
      reply += F("<TR><TD>");
      if (entry.name() != "config.dat" && entry.name() != "security.dat")
      {
        reply += F("<a class=\"button-link\" href=\"SDfilelist?delete=");
        reply += entry.name();
        reply += F("\">Del</a>");
      }
      reply += F("<TD><a href=\"");
      reply += entry.name();
      reply += F("\">");
      reply += entry.name();
      reply += F("</a>");
      reply += F("<TD>");
      reply += entry.size();
    }
    entry.close();
    entry = root.openNextFile();
  }
  //entry.close();
  root.close();
  reply += F("</table></form>");
  //reply += F("<BR><a class=\"button-link\" href=\"/upload\">Upload</a>");
  addFooter(reply);
  WebServer.send(200, "text/html", reply);
}


//********************************************************************************
// Web Interface handle other requests
//********************************************************************************
void handleNotFound() {

  if (wifiSetup)
  {
    WebServer.send(200, "text/html", "<meta HTTP-EQUIV='REFRESH' content='0; url=/setup'>");
    return;
  }

  if (!isLoggedIn()) return;
  if (loadFromFS(true, WebServer.uri())) return;
  if (loadFromFS(false, WebServer.uri())) return;
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
// Web Interface Setup Wizard
//********************************************************************************
void handle_setup() {

  String reply = "";
  addHeader(false, reply);

  if (WiFi.status() == WL_CONNECTED)
  {
    SaveSettings();
    IPAddress ip = WiFi.localIP();
    char host[20];
    sprintf_P(host, PSTR("%u.%u.%u.%u"), ip[0], ip[1], ip[2], ip[3]);
    reply += F("<BR>ESP is connected and using IP Address: ");
    reply += host;
    reply += F("<BR><BR>Connect your laptop / tablet / phone back to your main Wifi network and ");
    reply += F("<a class=\"button-menu\" href='http://");
    reply += host;
    reply += F("/config'>Proceed to main config</a>");
    addFooter(reply);
    WebServer.send(200, "text/html", reply);
    wifiSetup = false;
    WifiAPMode(false);
    return;
  }

  static byte status = 0;
  static int n = 0;
  static byte refreshCount = 0;
  String ssid = WebServer.arg(F("ssid"));
  String other = WebServer.arg(F("other"));
  String password = WebServer.arg(F("pass"));

  if (other.length() != 0)
  {
    ssid = other;
  }

  // if ssid config not set and params are both provided
  if (status == 0 && ssid.length() != 0 && password.length() != 0 && strcasecmp(SecuritySettings.WifiSSID, "ssid") == 0)
  {
    strncpy(SecuritySettings.WifiKey, password.c_str(), sizeof(SecuritySettings.WifiKey));
    strncpy(SecuritySettings.WifiSSID, ssid.c_str(), sizeof(SecuritySettings.WifiSSID));
    wifiSetupConnect = true;
    status = 1;
    refreshCount = 0;
  }

  reply += F("<h1>Wifi Setup wizard</h1><BR>");
  reply += F("<form name='frmselect' method='post'>");

  if (status == 0)  // first step, scan and show access points within reach...
  {
    if (n == 0)
      n = WiFi.scanNetworks();

    if (n == 0)
      reply += F("No Access Points found");
    else
    {
      for (int i = 0; i < n; ++i)
      {
        reply += F("<input type='radio' name='ssid' value='");
        reply += WiFi.SSID(i);
        reply += F("'");
        if (WiFi.SSID(i) == ssid)
          reply += F(" checked ");
        reply += F(">");
        reply += WiFi.SSID(i);
        reply += F("</input><br>");
      }
    }

    reply += F("<input type='radio' name='ssid' id='other_ssid' value='other' >other SSID:</input>");
    reply += F("<input type ='text' name='other' value='");
    reply += other;
    reply += F("'><br><br>");
    reply += F("Password: <input type ='text' name='pass' value='");
    reply += password;
    reply += F("'><br>");

    reply += F("<input type='submit' value='Connect'>");
  }

  if (status == 1)  // connecting stage...
  {
    if (refreshCount > 0)
    {
      status = 0;
      strncpy(SecuritySettings.WifiSSID, "ssid", sizeof(SecuritySettings.WifiSSID));
      SecuritySettings.WifiKey[0] = 0;
      reply += F("<a class=\"button-menu\" href=\"setup\">Back to Setup</a>");
    }
    else
    {
      int wait = 20;
      if (refreshCount != 0)
        wait = 3;
      reply += F("Please wait for <h1 id=\"countdown\">20..</h1>");
      reply += F("<script type=\"text/JavaScript\">");
      reply += F("function timedRefresh(timeoutPeriod) {");
      reply += F("   var timer = setInterval(function() {");
      reply += F("   if (timeoutPeriod > 0) {");
      reply += F("       timeoutPeriod -= 1;");
      reply += F("       document.getElementById(\"countdown\").innerHTML = timeoutPeriod + \"..\" + \"<br />\";");
      reply += F("   } else {");
      reply += F("       clearInterval(timer);");
      reply += F("            window.location.href = window.location.href;");
      reply += F("       };");
      reply += F("   }, 1000);");
      reply += F("};");
      reply += F("timedRefresh(");
      reply += wait;
      reply += F(");");
      reply += F("</script>");
      reply += F("seconds while trying to connect");
    }
    refreshCount++;
  }

  reply += F("</form>");
  addFooter(reply);
  WebServer.send(200, "text/html", reply);
  delay(10);
}


//********************************************************************************
// Web Interface rules page
//********************************************************************************
void handle_rules() {
  if (!isLoggedIn()) return;
  static byte currentSet = 1;

  String set = WebServer.arg(F("set"));
  byte rulesSet = 1;
  if (set.length() > 0)
  {
    rulesSet = set.toInt();
  }

  String fileName = F("rules");
  fileName += rulesSet;
  fileName += F(".txt");

  String reply = "";
  checkRAM(8);
  addHeader(true, reply);

  if (WebServer.args() > 0)
  {
    if (currentSet == rulesSet) // only save when the dropbox was not used to change set
    {
      String rules = WebServer.arg(F("rules"));
      if (rules.length() > RULES_MAX_SIZE)
        reply += F("<span style=\"color:red\">Data was not saved, exceeds web editor limit!</span>");
      else
      {

        if (RTC.flashDayCounter > MAX_FLASHWRITES_PER_DAY)
        {
          String log = F("FS   : Daily flash write rate exceeded!");
          addLog(LOG_LEVEL_ERROR, log);
          reply += F("<span style=\"color:red\">Error saving to flash!</span>");
        }
        else
        {
          fs::File f = SPIFFS.open(fileName, "w");
          if (f)
          {
            f.print(rules);
            f.close();
            flashCount();
          }
        }
      }
    }
    else // changed set, check if file exists and create new
    {
      if (!SPIFFS.exists(fileName))
      {
        fs::File f = SPIFFS.open(fileName, "w");
        f.close();
      }
    }
  }

  if (rulesSet != currentSet)
    currentSet = rulesSet;

  reply += F("<form name = 'frmselect' method = 'post'><table><TH>Rules");

  byte choice = rulesSet;
  String options[RULESETS_MAX];
  int optionValues[RULESETS_MAX];
  for (byte x = 0; x < RULESETS_MAX; x++)
  {
    options[x] = F("Rules Set ");
    options[x] += x + 1;
    optionValues[x] = x + 1;
  }
  reply += F("<TR><TD>Edit: <select name = 'set' LANGUAGE = javascript onchange = \"return dept_onchange(frmselect)\" >");
  for (byte x = 0; x < RULESETS_MAX; x++)
  {
    reply += F("<option value='");
    reply += optionValues[x];
    reply += "'";
    if (choice == optionValues[x])
      reply += F(" selected");
    reply += ">";
    reply += options[x];
    reply += F("</option>");
  }
  reply += F("</select>");

  // load form data from flash

  fs::File f = SPIFFS.open(fileName, "r+");
  if (f)
  {
    if (f.size() > RULES_MAX_SIZE)
      reply += F("<span style=\"color:red\">Filesize exceeds web editor limit!</span>");
    else
    {
      reply += F("<TR><TD><textarea name='rules' rows='15' cols='80' wrap='off'>");
      while (f.available())
        reply += (char)f.read();
      reply += F("</textarea>");
    }
  }

  reply += F("<TR><TD>Current size: ");
  reply += f.size();
  reply += F(" characters (Max ");
  reply += RULES_MAX_SIZE;
  reply += F(")");

  reply += F("<TR><TD><input class=\"button-link\" type='submit' value='Submit'>");
  reply += F("</table></form>");
  addFooter(reply);
  WebServer.send(200, "text/html", reply);
}


//********************************************************************************
// Web Interface root page
//********************************************************************************
void handle_sysinfo() {
  if (!isLoggedIn()) return;

  int freeMem = ESP.getFreeHeap();
  String reply = "";
  addHeader(true, reply);

  IPAddress ip = WiFi.localIP();
  IPAddress gw = WiFi.gatewayIP();

  reply += printWebString;
  reply += F("<form>");
  reply += F("<table><TH>System Info<TH>");

  reply += F("<TR><TD>Unit:<TD>");
  reply += Settings.Unit;

  if (Settings.UseNTP)
  {

    reply += F("<TR><TD>Local Time:<TD>");
    reply += year();
    reply += F("-");
    if (month() < 10)
      reply += "0";
    reply += month();
    reply += F("-");
    if (day() < 10)
    	reply += F("0");
    reply += day();
    reply += F(" ");
    reply += hour();
    reply += F(":");
    if (minute() < 10)
      reply += F("0");
    reply += minute();
  }

  reply += F("<TR><TD>Uptime:<TD>");
  char strUpTime[40];
  int minutes = wdcounter / 2;
  int days = minutes / 1440;
  minutes = minutes % 1440;
  int hrs = minutes / 60;
  minutes = minutes % 60;
  sprintf_P(strUpTime, PSTR("%d days %d hours %d minutes"), days, hrs, minutes);
  reply += strUpTime;

  reply += F("<TR><TD>Load:<TD>");
  if (wdcounter > 0)
  {
    reply += 100 - (100 * loopCounterLast / loopCounterMax);
    reply += F("% (LC=");
    reply += int(loopCounterLast / 30);
    reply += F(")");
  }

  reply += F("<TR><TD>Free Mem:<TD>");
  reply += freeMem;
  reply += F(" (");
  reply += lowestRAM;
  reply += F(")");

  if (WiFi.status() == WL_CONNECTED)
  {
    reply += F("<TR><TD>Wifi RSSI:<TD>");
    reply += WiFi.RSSI();
    reply += F(" dB");
    reply += F("<TR><TD>Wifi Type:<TD>");
    byte PHYmode = wifi_get_phy_mode();
    switch(PHYmode)
    {
      case 1:
        reply += F("802.11B");
        break;
      case 2:
        reply += F("802.11G");
        break;
      case 3:
        reply += F("802.11N");
        break;
    }
  }

  char str[20];
  sprintf_P(str, PSTR("%u.%u.%u.%u"), ip[0], ip[1], ip[2], ip[3]);
  reply += F("<TR><TD>IP:<TD>");
  reply += str;

  sprintf_P(str, PSTR("%u.%u.%u.%u"), gw[0], gw[1], gw[2], gw[3]);
  reply += F("<TR><TD>GW:<TD>");
  reply += str;

  reply += F("<TR><TD>Build:<TD>");
  reply += BUILD;
  reply += F(" ");
  reply += F(BUILD_NOTES);

  reply += F("<TR><TD>GIT version:<TD>");
  reply += BUILD_GIT;

  reply += F("<TR><TD>Plugin sets:<TD>");

  #ifdef PLUGIN_BUILD_NORMAL
    reply += F("[Normal] ");
  #endif

  #ifdef PLUGIN_BUILD_TESTING
    reply += F("[Testing] ");
  #endif

  #ifdef PLUGIN_BUILD_DEV
    reply += F("[Development] ");
  #endif

  reply += F("<TR><TD>Core Version:<TD>");
  reply += ESP.getCoreVersion();

  reply += F("<TR><TD>Flash Size:<TD>");
  reply += ESP.getFlashChipRealSize() / 1024; //ESP.getFlashChipSize();
  reply += F(" kB");

  reply += F("<TR><TD>Flash Writes (daily/boot):<TD>");
  reply += RTC.flashDayCounter;
  reply += F(" / ");
  reply += RTC.flashCounter;

  reply += F("<TR><TD>Sketch Size/Free:<TD>");
  reply += ESP.getSketchSize() / 1024;
  reply += F(" kB / ");
  reply += ESP.getFreeSketchSpace() / 1024;
  reply += F(" kB");

  reply += F("<TR><TD>Devices:<TD>");
  reply += deviceCount + 1;

  reply += F("<TR><TD>Boot cause:<TD>");
  switch (lastBootCause)
  {
    case BOOT_CAUSE_MANUAL_REBOOT:
      reply += F("Manual reboot");
      break;
    case BOOT_CAUSE_DEEP_SLEEP: //nobody should ever see this, since it should sleep again right away.
      reply += F("Deep sleep");
      break;
    case BOOT_CAUSE_COLD_BOOT:
      reply += F("Cold boot");
      break;
    case BOOT_CAUSE_EXT_WD:
      reply += F("External Watchdog");
      break;
  }

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

  reply += F("<TR><TD>Flash Chip ID:<TD>");
  reply += ESP.getFlashChipId();

  reply += F("</table></form>");
  addFooter(reply);
  WebServer.send(200, "text/html", reply);
}


//********************************************************************************
// URNEncode char string to string object
//********************************************************************************
String URLEncode(const char* msg)
{
  const char *hex = "0123456789abcdef";
  String encodedMsg = "";

  while (*msg != '\0') {
    if ( ('a' <= *msg && *msg <= 'z')
         || ('A' <= *msg && *msg <= 'Z')
         || ('0' <= *msg && *msg <= '9') ) {
      encodedMsg += *msg;
    } else {
      encodedMsg += '%';
      encodedMsg += hex[*msg >> 4];
      encodedMsg += hex[*msg & 15];
    }
    msg++;
  }
  return encodedMsg;
}
