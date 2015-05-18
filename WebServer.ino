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

  if (strcasecmp(command, "wifidisconnect") != 0)
  {
    ExecuteCommand(command);
    String reply = "<body><form>Welcome to ESP Easy";
    reply += "<BR><a href='/config'>Config</a>";
    reply += "<BR><a href='/devices'>Devices</a>";
    reply += "<BR><a href='/hardware'>Hardware</a>";
    reply += "<BR><a href='/?cmd=reboot'>Reboot</a>";
    reply += "<BR><a href='/?cmd=wifidisconnect'>Disconnect</a>";
    reply += "<BR><a href='/?cmd=wificonnect'>Connect</a>";

    reply += "<BR><BR>System Info";

    IPAddress ip = WiFi.localIP();
    IPAddress gw = WiFi.gatewayIP();

    char str[20];
    sprintf(str, "%u.%u.%u.%u", ip[0], ip[1], ip[2], ip[3]);
    reply += "<BR><BR>IP : ";
    reply += str;

    sprintf(str, "%u.%u.%u.%u", gw[0], gw[1], gw[2], gw[3]);
    reply += "<BR>GW : ";
    reply += str;

    reply += "<BR>Build : ";
    reply += BUILD;

    reply += "<BR>Unit : ";
    reply += Settings.Unit;

    reply += "</form></body>";
    server.send(200, "text/html", reply);
    delay(100);
  }
  else
  {
    // have to disconnect from within the main loop
    // because the webconnection is still active at this point
    // disconnect would result into a crash/reboot...
    Serial.println("WIFI : Disconnecting...");
    cmd_disconnect = true;
  }
}

//********************************************************************************
// Web Interface config page
//********************************************************************************
void handle_config() {
  char tmpstring[26];

  Serial.println("HTTP : Webconfig : ");

  String ssid = server.arg("ssid");
  String key = server.arg("key");
  String controllerip = server.arg("controllerip");
  String controllerport = server.arg("controllerport");
  String ip = server.arg("ip");
  String unit = server.arg("unit");
  String apkey = server.arg("apkey");
  String syslogip = server.arg("syslogip");

  if (ssid[0] != 0)
  {
    ssid.toCharArray(tmpstring, 25);
    strcpy(Settings.WifiSSID, tmpstring);
    key.toCharArray(tmpstring, 25);
    strcpy(Settings.WifiKey, tmpstring);
    controllerip.toCharArray(tmpstring, 25);
    str2ip(tmpstring, Settings.Controller_IP);
    Settings.ControllerPort = controllerport.toInt();
    Settings.IP_Octet = ip.toInt();
    Settings.Unit = unit.toInt();
    apkey.toCharArray(tmpstring, 25);
    strcpy(Settings.WifiAPKey, tmpstring);
    syslogip.toCharArray(tmpstring, 25);
    str2ip(tmpstring, Settings.Syslog_IP);
    Save_Settings();
    LoadSettings();
  }

  String reply = "<body>";
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
  reply += "'><BR>Fixed IP Octet: (Optional)<BR><input type='text' name='ip' value='";
  reply += Settings.IP_Octet;

  reply += "'><BR>Unit nr:<BR><input type='text' name='unit' value='";
  reply += Settings.Unit;

  reply += "'><BR>WPA AP Mode Key:<BR><input type='text' name='apkey' value='";
  reply += Settings.WifiAPKey;

  reply += "'><BR>Syslog IP:<BR><input type='text' name='syslogip' value='";
  str[0]=0;
  sprintf(str, "%u.%u.%u.%u", Settings.Syslog_IP[0], Settings.Syslog_IP[1], Settings.Syslog_IP[2], Settings.Syslog_IP[3]);
  reply += str;

  reply += "'><BR><input type='submit' value='Submit'>";
  reply += "</form></body>";
  server.send(200, "text/html", reply);
  delay(1000);

}

//********************************************************************************
// Web Interface device page
//********************************************************************************
void handle_devices() {

  Serial.println("HTTP : Webdevices : ");

  String boardtype = server.arg("boardtype");
  String sensordelay = server.arg("delay");
  String dallas = server.arg("dallas");
  String dht = server.arg("dht");
  String dhttype = server.arg("dhttype");
  String bmp = server.arg("bmp");
  String lux = server.arg("lux");
  String rfid = server.arg("rfid");
  String analog = server.arg("analog");
  String pulse1 = server.arg("pulse1");

  if (sensordelay.toInt() != 0)
  {
    Settings.Delay = sensordelay.toInt();
    Settings.Dallas = dallas.toInt();
    Settings.DHT = dht.toInt();
    Settings.DHTType = dhttype.toInt();
    Settings.BMP = bmp.toInt();
    Settings.LUX = lux.toInt();
    Settings.RFID = rfid.toInt();
    Settings.Analog = analog.toInt();
    Settings.Pulse1 = pulse1.toInt();
    Save_Settings();
  }

  String reply = "<body><form>";
  reply += "Delay:<BR><input type='text' name='delay' value='";
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
  reply += "'><BR>Pulse:<BR><input type='text' name='pulse1' value='";
  reply += Settings.Pulse1;

  reply += "'><BR><input type='submit' value='Submit'>";
  reply += "</form></body>";
  server.send(200, "text/html", reply);
  delay(100);
}

//********************************************************************************
// Web Interface hardware page
//********************************************************************************
void handle_hardware() {

  Serial.println("HTTP : Hardware : ");

  String boardtype = server.arg("boardtype");

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

      }
    Save_Settings();
  }

  String reply = "<body><form>";

  reply +="Board Type:";
  byte choice = Settings.BoardType;
  String options[5];
  options[0]="ESP-07/12";
  options[1]="ESP-01 I2C";
  options[2]="ESP-01 In/Out";
  options[3]="ESP-01 2 x In";
  options[4]="ESP-01 2 x Out";
  reply +="<BR><select name='boardtype'>";
  for (byte x=0; x<5;x++)
    {
    reply +="<option value='";
    reply +=x;
    reply +="'";
    if (choice==x)
      reply +=" selected";
    reply +=">";
    reply +=options[x];
    reply+="</option>";
    }
  reply +="</select>";



    switch (Settings.BoardType)
      {
        case 0:
          reply +="<BR>SDA: ";
          reply += Settings.Pin_i2c_sda;
          reply +="<BR>SCL: ";
          reply += Settings.Pin_i2c_scl;
          reply +="<BR>Input 1: ";
          reply += Settings.Pin_wired_in_1;
          reply +="<BR>Input 2: ";
          reply += Settings.Pin_wired_in_2;
          reply +="<BR>Output 1: ";
          reply += Settings.Pin_wired_out_1;
          reply +="<BR>Output 2: ";
          reply += Settings.Pin_wired_out_2;
          break;
        case 1:
          reply +="<BR>SDA: ";
          reply += Settings.Pin_i2c_sda;
          reply +="<BR>SCL: ";
          reply += Settings.Pin_i2c_scl;
          break;
        case 2:
          reply +="<BR>Input 1: ";
          reply += Settings.Pin_wired_in_1;
          reply +="<BR>Output 1: ";
          reply += Settings.Pin_wired_out_1;
          break;
        case 3:
          reply +="<BR>Input 1: ";
          reply += Settings.Pin_wired_in_1;
          reply +="<BR>Input 2: ";
          reply += Settings.Pin_wired_in_2;
          break;
        case 4:
          reply +="<BR>Output 1: ";
          reply += Settings.Pin_wired_out_1;
          reply +="<BR>Output 2: ";
          reply += Settings.Pin_wired_out_2;
          break;
      }
  
  reply += "<BR><input type='submit' value='Submit'>";
  reply += "</form></body>";
  server.send(200, "text/html", reply);
  delay(100);
}

// Nodo proof of concept. send json query as nodo event on I2C to mega
// Compatible with Nodo 3.8 only, tested on R818
// set used variables to global on the Mega.

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
  Serial.print("HTTP : Web json : idx: ");
  String idx = server.arg("idx");
  String svalue = server.arg("svalue");
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

  server.send(200, "text/html", "OK");
  delay(100);

}

unsigned long float2ul(float f)
{
  unsigned long ul;
  memcpy(&ul, &f, 4);
  return ul;
}

