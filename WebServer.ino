//********************************************************************************
// Web Interface root page
//********************************************************************************
void handle_root() {
  Serial.print("HTTP : Webrequest : ");
  String webrequest = server.arg("cmd");
  webrequest.replace("%20", " ");
  Serial.println(webrequest);
  char command[80];
  command[0] = 0;
  webrequest.toCharArray(command, 79);
  ExecuteCommand(command);
  String reply="<body><form>Welcome to ESP Easy";
  reply += "<BR><a href='/config'>Config</a>";
  reply += "<BR><a href='/devices'>Devices</a>";
  reply += "<BR><a href='/?cmd=reboot'>Reboot</a>";
  reply += "</form></body>";
  server.send(200, "text/html", reply);
  delay(100);
}

//********************************************************************************
// Web Interface config page
//********************************************************************************
void handle_config() {
  char c_ssid[26];
  c_ssid[0] = 0;
  char c_key[26];
  c_key[0] = 0;
  char c_controllerip[26];
  c_controllerip[0] = 0;
  char c_controllerport[26];
  c_controllerport[0] = 0;

  Serial.println("HTTP : Webconfig : ");

  String ssid = server.arg("ssid");
  Serial.println(ssid);

  String key = server.arg("key");
  Serial.println(key);

  String controllerip = server.arg("controllerip");
  Serial.println(controllerip);

  String controllerport = server.arg("controllerport");
  Serial.println(controllerport);

  if (ssid[0] !=0)
    {
      ssid.toCharArray(c_ssid, 25);
      strcpy(Settings.WifiSSID,c_ssid);
      key.toCharArray(c_key, 25);
      strcpy(Settings.WifiKey,c_key);
      controllerip.toCharArray(c_controllerip, 25);
      str2ip(c_controllerip, Settings.Controller_IP);
      controllerport.toCharArray(c_controllerport, 25);
      Settings.ControllerPort = str2int(c_controllerport);
      Save_Settings();
    }
    
  String reply="<body>";
  reply += "<form>SSID:<BR><input type='text' name='ssid' value='";
  reply += Settings.WifiSSID;
  reply += "'><BR>WPA Key:<BR><input type='text' name='key' value='";
  reply += Settings.WifiKey;

  reply += "'><BR>Controller IP:<BR><input type='text' name='controllerip' value='";
  char str[20];
  sprintf(str, "%u.%u.%u.%u", Settings.Controller_IP[0], Settings.Controller_IP[1], Settings.Controller_IP[2], Settings.Controller_IP[3]);
  reply += str;

  reply += "'><BR>Controller Port:<BR><input type='text' name='controllerport' value='";
  reply += Settings.ControllerPort;

  reply += "'><BR><input type='submit' value='Submit'>";
  reply += "</form></body>";
  server.send(200, "text/html", reply);
  delay(100);
}

//********************************************************************************
// Web Interface device page
//********************************************************************************
void handle_devices() {
  char c_delay[10];
  c_delay[0] = 0;
  char c_dallas[10];
  c_dallas[0] = 0;
  char c_dht[10];
  c_dht[0] = 0;
  char c_dhttype[10];
  c_dhttype[0] = 0;
  char c_bmp[10];
  c_bmp[0] = 0;
  char c_lux[10];
  c_lux[0] = 0;
  char c_rfid[10];
  c_rfid[0] = 0;
  char c_analog[10];
  c_analog[0] = 0;

  Serial.println("HTTP : Webdevices : ");

  String sdelay = server.arg("delay");
  Serial.println(sdelay);
  String dallas = server.arg("dallas");
  Serial.println(dallas);
  String dht = server.arg("dht");
  Serial.println(dht);
  String dhttype = server.arg("dhttype");
  Serial.println(dhttype);

  String bmp = server.arg("bmp");
  Serial.println(bmp);
  String lux = server.arg("lux");
  Serial.println(lux);
  String rfid = server.arg("rfid");
  Serial.println(rfid);
  String analog = server.arg("analog");
  Serial.println(analog);

  if (sdelay[0] !=0)
    {
      sdelay.toCharArray(c_delay, 9);
      Settings.Delay = str2int(c_delay);
      dallas.toCharArray(c_dallas, 9);
      Settings.Dallas = str2int(c_dallas);
      dht.toCharArray(c_dht, 9);
      Settings.DHT = str2int(c_dht);
      dhttype.toCharArray(c_dhttype, 9);
      Settings.DHTType = str2int(c_dhttype);
      bmp.toCharArray(c_bmp, 9);
      Settings.BMP = str2int(c_bmp);
      lux.toCharArray(c_lux, 9);
      Settings.LUX = str2int(c_lux);
      rfid.toCharArray(c_rfid, 9);
      Settings.RFID = str2int(c_rfid);
      analog.toCharArray(c_analog, 9);
      Settings.Analog = str2int(c_analog);
      Save_Settings();
    }
    
  String reply="<body>";
  reply += "<form>Delay:<BR><input type='text' name='delay' value='";
  reply += Settings.Delay;
  reply += "'><BR>Dallas:<BR><input type='text' name='dallas' value='";
  reply += Settings.Dallas;
  reply += "'><BR>DHT:<BR><input type='text' name='dht' value='";
  reply += Settings.DHT;
  reply += "'><BR>DHT Type:<BR><input type='text' name='dhttype' value='";
  reply += Settings.DHTType;
  reply += "'><BR>BMP:<BR><input type='text' name='bmp' value='";
  reply += Settings.BMP;
  reply += "'><BR>LUX:<BR><input type='text' name='lux' value='";
  reply += Settings.LUX;
  reply += "'><BR>RFID:<BR><input type='text' name='rfid' value='";
  reply += Settings.RFID;
  reply += "'><BR>Analog:<BR><input type='text' name='analog' value='";
  reply += Settings.Analog;

  reply += "'><BR><input type='submit' value='Submit'>";
  reply += "</form></body>";
  server.send(200, "text/html", reply);
  delay(100);
}

