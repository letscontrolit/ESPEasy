
// ********************************************************************************
// Web Interface Setup Wizard
// ********************************************************************************
void handle_setup() {
  checkRAM(F("handle_setup"));

  // Do not check client IP range allowed.
  TXBuffer.startStream();
  sendHeadandTail(F("TmplAP"));

  if (WiFiConnected())
  {
    addHtmlError(SaveSettings());
    String host = formatIP(WiFi.localIP());
    TXBuffer += F("<BR>ESP is connected and using IP Address: <BR><h1>");
    TXBuffer += host;
    TXBuffer += F("</h1><BR><BR>Connect your laptop / tablet / phone<BR>back to your main Wifi network and<BR><BR>");
    TXBuffer += F("<a class='button' href='http://");
    TXBuffer += host;
    TXBuffer += F("/config'>Proceed to main config</a><BR><BR>");

    sendHeadandTail(F("TmplAP"), true);
    TXBuffer.endStream();

    wifiSetup = false;

    // setWifiMode(WIFI_STA);  //this forces the iPhone to exit safari and this page was never displayed
    timerAPoff = millis() + 60000L; // switch the AP off in 1 minute
    return;
  }

  static byte status       = 0;
  static int  n            = 0;
  static byte refreshCount = 0;
  String ssid              = WebServer.arg(F("ssid"));
  String other             = WebServer.arg(F("other"));
  String password          = WebServer.arg(F("pass"));

  if (other.length() != 0)
  {
    ssid = other;
  }

  // if ssid config not set and params are both provided
  if ((status == 0) && (ssid.length() != 0) /*&& strcasecmp(SecuritySettings.WifiSSID, "ssid") == 0 */)
  {
    safe_strncpy(SecuritySettings.WifiKey,  password.c_str(), sizeof(SecuritySettings.WifiKey));
    safe_strncpy(SecuritySettings.WifiSSID, ssid.c_str(),     sizeof(SecuritySettings.WifiSSID));
    wifiSetupConnect = true;

    if (loglevelActiveFor(LOG_LEVEL_INFO)) {
      String reconnectlog = F("WIFI : Credentials Changed, retry connection. SSID: ");
      reconnectlog += ssid;
      addLog(LOG_LEVEL_INFO, reconnectlog);
    }
    status       = 1;
    refreshCount = 0;
  }

  TXBuffer += F("<BR><h1>Wifi Setup wizard</h1>");
  html_add_form();

  if (status == 0) // first step, scan and show access points within reach...
  {
    WiFiMode_t cur_wifimode = WiFi.getMode();

    if (n == 0) {
      n = WiFi.scanNetworks(false, true);
    }
    setWifiMode(cur_wifimode);

    if (n == 0) {
      TXBuffer += F("No Access Points found");
    }
    else
    {
      html_table_class_multirow();
      html_TR();
      html_table_header(F("Pick"), 50);
      html_table_header(F("Network info"));

      for (int i = 0; i < n; ++i)
      {
        html_TR_TD(); TXBuffer += F("<label class='container2'>");
        TXBuffer               += F("<input type='radio' name='ssid' value='");
        {
          String escapeBuffer = WiFi.SSID(i);
          htmlStrongEscape(escapeBuffer);
          TXBuffer += escapeBuffer;
        }
        TXBuffer += '\'';

        if (WiFi.SSID(i) == ssid) {
          TXBuffer += F(" checked ");
        }
        TXBuffer += F("><span class='dotmark'></span></label><TD>");
        TXBuffer += formatScanResult(i, "<BR>");
        TXBuffer += "";
      }
      html_end_table();
    }

    TXBuffer += F(
      "<BR><label class='container2'>other SSID:<input type='radio' name='ssid' id='other_ssid' value='other' ><span class='dotmark'></span></label>");
    TXBuffer += F("<input class='wide' type ='text' name='other' value='");
    TXBuffer += other;
    TXBuffer += F("'><BR><BR>");

    addFormSeparator(2);

    TXBuffer += F("<BR>Password:<BR><input class='wide' type ='text' name='pass' value='");
    TXBuffer += password;
    TXBuffer += F("'><BR><BR>");

    addSubmitButton(F("Connect"), "");
  }

  if (status == 1) // connecting stage...
  {
    if (refreshCount > 0)
    {
      status = 0;

      //      safe_strncpy(SecuritySettings.WifiSSID, "ssid", sizeof(SecuritySettings.WifiSSID));
      //      SecuritySettings.WifiKey[0] = 0;
      TXBuffer += F("<a class='button' href='setup'>Back to Setup</a><BR><BR>");
    }
    else
    {
      int wait = 20;

      if (refreshCount != 0) {
        wait = 3;
      }
      TXBuffer += F("Please wait for <h1 id='countdown'>20..</h1>");
      TXBuffer += F("<script type='text/JavaScript'>");
      TXBuffer += F("function timedRefresh(timeoutPeriod) {");
      TXBuffer += F("var timer = setInterval(function() {");
      TXBuffer += F("if (timeoutPeriod > 0) {");
      TXBuffer += F("timeoutPeriod -= 1;");
      TXBuffer += F("document.getElementById('countdown').innerHTML = timeoutPeriod + '..' + '<br />';");
      TXBuffer += F("} else {");
      TXBuffer += F("clearInterval(timer);");
      TXBuffer += F("window.location.href = window.location.href;");
      TXBuffer += F("};");
      TXBuffer += F("}, 1000);");
      TXBuffer += F("};");
      TXBuffer += F("timedRefresh(");
      TXBuffer += wait;
      TXBuffer += F(");");
      html_add_script_end();
      TXBuffer += F("seconds while trying to connect");
    }
    refreshCount++;
  }

  html_end_form();
  sendHeadandTail(F("TmplAP"), true);
  TXBuffer.endStream();
  delay(10);
}
