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
  WebServer.begin();
}

void addMenu(String& str)
{
  // Inline style definitions
  str += F("<style>");
  str += F("* {font-family:sans-serif; font-size:12pt;}");
  str += F("h1 {font-size:16pt; border:1px solid #333; color:#ffffff; background:#27f;}");
  str += F(".button-link {padding:5px 10px; background:#5bf; color:#fff; border-radius:4px; border:solid 1px #258; text-decoration:none}");
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
  str += F("<a class=\"button-link\" href=\"devices\">Devices</a>");
  str += F("<a class=\"button-link\" href=\"hardware\">Hardware</a>");
#ifdef ESP_CONNEXIO
  str += F("<a class=\"button-link\" href=\"eventlist\">Eventlist</a>");
#endif
  str += F("<a class=\"button-link\" href=\"log\">Log</a>");
  str += F("<a class=\"button-link\" href=\"tools\">Tools</a><BR><BR>");
}

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
  webrequest.toCharArray(command, 79);

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

  char tmpstring[26];

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
    name.toCharArray(tmpstring, 25);
    strcpy(Settings.Name, tmpstring);
    password.toCharArray(tmpstring, 25);
    strcpy(Settings.Password, tmpstring);
    ssid.toCharArray(tmpstring, 25);
    strcpy(Settings.WifiSSID, tmpstring);
    key.toCharArray(tmpstring, 25);
    strcpy(Settings.WifiKey, tmpstring);
    controllerip.toCharArray(tmpstring, 25);
    str2ip(tmpstring, Settings.Controller_IP);
    Settings.ControllerPort = controllerport.toInt();
    controlleruser.toCharArray(tmpstring, 25);
    strcpy(Settings.ControllerUser, tmpstring);
    controllerpassword.toCharArray(tmpstring, 25);
    strcpy(Settings.ControllerPassword, tmpstring);
    Settings.Protocol = protocol.toInt();
    Settings.IP_Octet = ip.toInt();
    espip.toCharArray(tmpstring, 25);
    str2ip(tmpstring, Settings.IP);
    espgateway.toCharArray(tmpstring, 25);
    str2ip(tmpstring, Settings.Gateway);
    espsubnet.toCharArray(tmpstring, 25);
    str2ip(tmpstring, Settings.Subnet);
    Settings.Unit = unit.toInt();
    apkey.toCharArray(tmpstring, 25);
    strcpy(Settings.WifiAPKey, tmpstring);
    syslogip.toCharArray(tmpstring, 25);
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
  reply += F("'><TR><TD>WPA Key:<TD><input type='text' name='key' value='");
  reply += Settings.WifiKey;

  reply += F("'><TR><TD>WPA AP Mode Key:<TD><input type='text' name='apkey' value='");
  reply += Settings.WifiAPKey;

  reply += F("'><TR><TD>Unit nr:<TD><input type='text' name='unit' value='");
  reply += Settings.Unit;

  reply += F("'><TR><TD>Protocol:");
  byte choice = Settings.Protocol;
  String options[5];
  options[0] = F("");
  options[1] = F("Domoticz HTTP");
  options[2] = F("Domoticz MQTT");
  options[3] = F("Nodo Telnet");
  options[4] = F("ThingsSpeak HTTP");
  reply += F("<TD><select name='protocol'>");
  for (byte x = 0; x < 5; x++)
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
    
  if (Settings.Protocol == 3 or Settings.Protocol == 4)
    {
      reply += F("'><TR><TD>Controller Password:<TD><input type='text' name='controllerpassword' value='");
      reply += Settings.ControllerPassword;
    }

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

  if (Settings.SerialLogLevel >= LOG_LEVEL_DEBUG)
    Serial.println(F("HTTP : Webdevices"));

  String boardtype = WebServer.arg("boardtype");
  String sensordelay = WebServer.arg("delay");
  String messagedelay = WebServer.arg("messagedelay");
  String dallas = WebServer.arg("dallas");
  String dht = WebServer.arg("dht");
  String dhttype = WebServer.arg("dhttype");
  String bmp = WebServer.arg("bmp");
  String lux = WebServer.arg("lux");
  String rfid = WebServer.arg("rfid");
  String analog = WebServer.arg("analog");
  String pulse1 = WebServer.arg("pulse1");
  String switch1 = WebServer.arg("switch1");

  if (sensordelay.toInt() != 0)
  {
    Settings.Delay = sensordelay.toInt();
    Settings.MessageDelay = messagedelay.toInt();
    Settings.Dallas = dallas.toInt();
    Settings.DHT = dht.toInt();
    Settings.DHTType = dhttype.toInt();
    Settings.BMP = bmp.toInt();
    Settings.LUX = lux.toInt();
    Settings.RFID = rfid.toInt();
    Settings.Analog = analog.toInt();
    Settings.Pulse1 = pulse1.toInt();
    Settings.Switch1 = switch1.toInt();
#ifdef ESP_CONNEXIO

    struct NodoEventStruct TempEvent;
    ClearEvent(&TempEvent);
    byte x = 1;
    while (Eventlist_Write(x++, &TempEvent, &TempEvent)) delay(1);

    char cmd[80];
    sprintf_P(cmd, PSTR("eventlistwrite; boot %u; TimerSet 1,%u"), Settings.Unit, Settings.Delay);
    ExecuteLine(cmd, VALUE_SOURCE_SERIAL);
    sprintf_P(cmd, PSTR("eventlistwrite; Timer 1; TimerSet 1,%u"), Settings.Delay);
    ExecuteLine(cmd, VALUE_SOURCE_SERIAL);
    if (Settings.Dallas != 0)
    {
      eventAddTimer((char*)"DallasRead 1,1");
      eventAddVarSend(1, 1, Settings.Dallas);
      if (Settings.MessageDelay != 0)
        eventAddDelay();
    }
    if (Settings.DHT != 0)
    {
      eventAddTimer((char*)"DHTRead 2,2");
      eventAddVarSend(2, 2, Settings.DHT);
      if (Settings.MessageDelay != 0)
        eventAddDelay();
    }
    if (Settings.BMP != 0)
    {
      eventAddTimer((char*)"BMP085Read 4");
      eventAddVarSend(4, 3, Settings.BMP);
      if (Settings.MessageDelay != 0)
        eventAddDelay();
    }
    if (Settings.LUX != 0)
    {
      eventAddTimer((char*)"LuxRead 6");
      eventAddVarSend(6, 1, Settings.LUX);
      if (Settings.MessageDelay != 0)
        eventAddDelay();
    }
    if (Settings.Analog != 0)
    {
      eventAddTimer((char*)"VariableWiredAnalog 7");
      eventAddVarSend(7, 1, Settings.Analog);
    }
#endif
    Save_Settings();
  }

  String reply = "";
  addMenu(reply);
  reply += F("<form  method='post'><table bgcolor='#ddeeff'><tr bgcolor='#55bbff'><td>Device Settings<td>IDX/Variable<TD>Connect to<TR><TD>");
  reply += F("<TR><TD>Delay:<TD><input type='text' name='delay' value='");
  reply += Settings.Delay;
  reply += F("'><TR><TD>Message Delay (ms):<TD><input type='text' name='messagedelay' value='");
  reply += Settings.MessageDelay;
  reply += F("'><TR><TD>Dallas:<TD><input type='text' name='dallas' value='");
  reply += Settings.Dallas;
  reply += F("'><TD>Output 1<TR><TD>DHT:<TD><input type='text' name='dht' value='");
  reply += Settings.DHT;
  reply += F("'><TD>Output 2<TR><TD>DHT Type:<TD><input type='text' name='dhttype' value='");
  reply += Settings.DHTType;
  reply += F("'><TR><TD>BMP:<TD><input type='text' name='bmp' value='");
  reply += Settings.BMP;
  reply += F("'><TD>I2C<TR><TD>LUX:<TD><input type='text' name='lux' value='");
  reply += Settings.LUX;
  reply += F("'><TD>I2C<TR><TD>RFID:<TD><input type='text' name='rfid' value='");
  reply += Settings.RFID;
  reply += F("'><TD>Input 1+2<TR><TD>Analog:<TD><input type='text' name='analog' value='");
  reply += Settings.Analog;
  reply += F("'><TD>ADC<TR><TD>Pulse:<TD><input type='text' name='pulse1' value='");
  reply += Settings.Pulse1;
  reply += F("'><TD>Input 1<TR><TD>Switch:<TD><input type='text' name='switch1' value='");
  reply += Settings.Switch1;
  reply += F("'><TD>Input 1<TR><TD><TD><input class=\"button-link\" type='submit' value='Submit'>");
  reply += F("</table></form>");
  addFooter(reply);
  WebServer.send(200, "text/html", reply);
}

#ifdef ESP_CONNEXIO
void eventAddVarSend(byte var, byte sensortype, int idx)
{
  char cmd[80];
  char strProtocol[5];
  if (Settings.Protocol == 1)
    strcpy(strProtocol, "HTTP");
  if (Settings.Protocol == 2)
    strcpy(strProtocol, "MQTT");
  if (Settings.Protocol == 3)
    strcpy(strProtocol, "TELNET");
  if (Settings.Protocol == 4)
    strcpy(strProtocol, "TSPK");
  sprintf_P(cmd, PSTR("eventlistwrite; Timer 1; VariableSend %u,%s,%u,%u"), var, strProtocol, sensortype, idx);
  ExecuteLine(cmd, VALUE_SOURCE_SERIAL);
}

void eventAddTimer(char* event)
{
  char cmd[80];
  sprintf_P(cmd, PSTR("eventlistwrite; Timer 1; %s"), event);
  ExecuteLine(cmd, VALUE_SOURCE_SERIAL);
}

void eventAddDelay()
{
  char cmd[80];
  sprintf_P(cmd, PSTR("eventlistwrite; Timer 1; Delay %u"), Settings.MessageDelay);
  ExecuteLine(cmd, VALUE_SOURCE_SERIAL);
}
#endif


//********************************************************************************
// Web Interface hardware page
//********************************************************************************
void handle_hardware() {
  if (!isLoggedIn()) return;

  if (Settings.SerialLogLevel >= LOG_LEVEL_DEBUG)
    Serial.println(F("HTTP : Hardware"));

  String boardtype = WebServer.arg("boardtype");
  String pin_i2c_sda = WebServer.arg("pini2csda");
  String pin_i2c_scl = WebServer.arg("pini2cscl");
  String pin_wired_in_1 = WebServer.arg("pinwiredin1");
  String pin_wired_in_2 = WebServer.arg("pinwiredin2");
  String pin_wired_out_1 = WebServer.arg("pinwiredout1");
  String pin_wired_out_2 = WebServer.arg("pinwiredout2");

  if (boardtype.length() != 0)
  {
    Settings.BoardType = boardtype.toInt();
    switch (Settings.BoardType)
    {
      case 0:
        Settings.Pin_i2c_sda     = 0;
        Settings.Pin_i2c_scl     = 2;
        Settings.Pin_wired_in_1  = 4;
        Settings.Pin_wired_in_2  = 5;
        Settings.Pin_wired_out_1 = 12;
        Settings.Pin_wired_out_2 = 13;
        break;
      case 1:
        Settings.Pin_i2c_sda     = 0;
        Settings.Pin_i2c_scl     = 2;
        Settings.Pin_wired_in_1  = -1;
        Settings.Pin_wired_in_2  = -1;
        Settings.Pin_wired_out_1 = -1;
        Settings.Pin_wired_out_2 = -1;
        break;
      case 2:
        Settings.Pin_i2c_sda     = -1;
        Settings.Pin_i2c_scl     = -1;
        Settings.Pin_wired_in_1  = 0;
        Settings.Pin_wired_in_2  = -1;
        Settings.Pin_wired_out_1 = 2;
        Settings.Pin_wired_out_2 = -1;
        break;
      case 3:
        Settings.Pin_i2c_sda     = -1;
        Settings.Pin_i2c_scl     = -1;
        Settings.Pin_wired_in_1  = 0;
        Settings.Pin_wired_in_2  = 2;
        Settings.Pin_wired_out_1 = -1;
        Settings.Pin_wired_out_2 = -1;
        break;
      case 4:
        Settings.Pin_i2c_sda     = -1;
        Settings.Pin_i2c_scl     = -1;
        Settings.Pin_wired_in_1  = -1;
        Settings.Pin_wired_in_2  = -1;
        Settings.Pin_wired_out_1 = 0;
        Settings.Pin_wired_out_2 = 2;
        break;
      case 5:
        if (pin_i2c_sda.length() != 0)
        {
        Settings.Pin_i2c_sda     = pin_i2c_sda.toInt();
        Settings.Pin_i2c_scl     = pin_i2c_scl.toInt();
        Settings.Pin_wired_in_1  = pin_wired_in_1.toInt();
        Settings.Pin_wired_in_2  = pin_wired_in_2.toInt();
        Settings.Pin_wired_out_1 = pin_wired_out_1.toInt();
        Settings.Pin_wired_out_2 = pin_wired_out_2.toInt();
        break;
        }
    }
    Save_Settings();
  }

  String reply = "";
  addMenu(reply);

  reply += F("<form  method='post'><table bgcolor='#ddeeff'><tr bgcolor='#55bbff'><td>Hardware Settings<td><TR><TD>");

  reply += "<TR><TD>Board Type:<TD>";
  byte choice = Settings.BoardType;
  String options[6];
  options[0] = F("ESP-07/12");
  options[1] = F("ESP-01 I2C");
  options[2] = F("ESP-01 In/Out");
  options[3] = F("ESP-01 2 x In");
  options[4] = F("ESP-01 2 x Out");
  options[5] = F("Custom");
  reply += F("<select name='boardtype'>");
  for (byte x = 0; x < 6; x++)
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

  if (choice == 5) // custom config
  {
    reply += F("<TR><TD>SDA:<TD>");
    addSelect(reply,"pini2csda",Settings.Pin_i2c_sda);
    reply += F("<TR><TD>SCL:<TD>");
    addSelect(reply,"pini2cscl",Settings.Pin_i2c_scl);
    reply += F("<TR><TD>Input 1:<TD>");
    addSelect(reply,"pinwiredin1",Settings.Pin_wired_in_1);
    reply += F("<TR><TD>Input 2:<TD>");
    addSelect(reply,"pinwiredin2",Settings.Pin_wired_in_2);
    reply += F("<TR><TD>Output 1:<TD>");
    addSelect(reply,"pinwiredout1",Settings.Pin_wired_out_1);
    reply += F("<TR><TD>Output 2:<TD>");
    addSelect(reply,"pinwiredout2",Settings.Pin_wired_out_2);
  }

  reply += F("<TR><TD><TD><input class=\"button-link\" type='submit' value='Submit'><TR><TD>");

  switch (Settings.BoardType)
  {
    case 0:
    case 5:
      reply += F("<TR><TD>SDA:<TD>");
      reply += Settings.Pin_i2c_sda;
      reply += F("<TR><TD>SCL:<TD>");
      reply += Settings.Pin_i2c_scl;
      reply += F("<TR><TD>Input 1:<TD>");
      reply += Settings.Pin_wired_in_1;
      reply += F("<TR><TD>Input 2:<TD>");
      reply += Settings.Pin_wired_in_2;
      reply += F("<TR><TD>Output 1:<TD>");
      reply += Settings.Pin_wired_out_1;
      reply += F("<TR><TD>Output 2:<TD>");
      reply += Settings.Pin_wired_out_2;
      break;
    case 1:
      reply += F("<TR><TD>SDA:<TD>");
      reply += Settings.Pin_i2c_sda;
      reply += F("<TR><TD>SCL:<TD>");
      reply += Settings.Pin_i2c_scl;
      break;
    case 2:
      reply += F("<TR><TD>Input 1:<TD>");
      reply += Settings.Pin_wired_in_1;
      reply += F("<TR><TD>Output 1:<TD>");
      reply += Settings.Pin_wired_out_1;
      break;
    case 3:
      reply += F("<TR><TD>Input 1:<TD>");
      reply += Settings.Pin_wired_in_1;
      reply += F("<TR><TD>Input 2:<TD>");
      reply += Settings.Pin_wired_in_2;
      break;
    case 4:
      reply += F("<TR><TD>Output 1:<TD>");
      reply += Settings.Pin_wired_out_1;
      reply += F("<TR><TD>Output 2:<TD>");
      reply += Settings.Pin_wired_out_2;
      break;
  }

  reply += F("</table></form>");
  addFooter(reply);
  WebServer.send(200, "text/html", reply);
}

void addSelect(String& str, String name,  int choice)
{
  String options[7];
  options[0] = F(" ");
  options[1] = F("GPIO-0");
  options[2] = F("GPIO-2");
  options[3] = F("GPIO-4");
  options[4] = F("GPIO-5");
  options[5] = F("GPIO-12");
  options[6] = F("GPIO-13");
  int optionValues[7];
  optionValues[0] = -1;
  optionValues[1] = 0;
  optionValues[2] = 2;
  optionValues[3] = 4;
  optionValues[4] = 5;
  optionValues[5] = 12;
  optionValues[6] = 13;

  str += F("<select name='");
  str += name;
  str += "'>";
  for (byte x = 0; x < 7; x++)
  {
    str += F("<option value='");
    str += optionValues[x];
    str += "'";
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
  idx.toCharArray(c_idx, 9);
  char c_svalue[40];
  c_svalue[0] = 0;
  svalue.toCharArray(c_svalue, 39);

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
    while (Eventlist_Write(x++, &TempEvent, &TempEvent)) delay(1);

    String eventlist = WebServer.arg("eventlist");
    eventlist.replace("%0D%0A", "\n");
    int NewLineIndex = eventlist.indexOf('\n');
    byte limit = 0;
    byte messagecode = 0;
    while ((NewLineIndex > 0) && (limit < EventlistMax))
    {
      limit++;
      String line = eventlist.substring(0, NewLineIndex);
      line.replace("%3B", ";");
      line.replace("%2C", ",");
      line.replace("+", " ");
      //int SemiColonIndex = line.indexOf(';');
      //line = line.substring(SemiColonIndex+1);
      String strCommand = F("eventlistwrite;");
      strCommand += line;
      if (Settings.SerialLogLevel >= LOG_LEVEL_DEBUG)
        Serial.println(strCommand);
      strCommand.toCharArray(TempString, 79);
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
  webrequest.toCharArray(command, 79);

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
  reply += F("<table bgcolor='#ddeeff'><tr bgcolor='#55bbff'><td>I2C Addresses in use<TR><TD>");

  byte error, address;
  int nDevices;
  nDevices = 0;
  for(address = 1; address < 127; address++ ) 
    {
      Wire.beginTransmission(address);
      error = Wire.endTransmission();
      if (error == 0)
        {
          reply += "0x";
          reply += String(address,HEX);
          reply += "<BR>";
          nDevices++;
        }
      else if (error==4) 
        {
          reply += F("Unknow error at address 0x");
          reply += String(address,HEX);
        }    
    }

  if (nDevices == 0)
    reply += F("No I2C devices found");

  reply += F("</table>");
  addFooter(reply);
  WebServer.send(200, "text/html", reply);
  free(TempString);
}

//********************************************************************************
// Web Interface I2C scanner
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
  webrequest.toCharArray(command, 79);

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

