
// ********************************************************************************
// Web Interface Setup Wizard
// ********************************************************************************

#define HANDLE_SETUP_SCAN_STAGE       0
#define HANDLE_SETUP_CONNECTING_STAGE 1

void handle_setup() {
  checkRAM(F("handle_setup"));

  // Do not check client IP range allowed.
  TXBuffer.startStream();

  if (!WiFiConnected())
  {
    sendHeadandTail(F("TmplAP"));
    static byte status       = 0;
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
      wifiSetupConnect         = true;
      wifiConnectAttemptNeeded = true;

      if (loglevelActiveFor(LOG_LEVEL_INFO)) {
        String reconnectlog = F("WIFI : Credentials Changed, retry connection. SSID: ");
        reconnectlog += ssid;
        addLog(LOG_LEVEL_INFO, reconnectlog);
      }
      status       = HANDLE_SETUP_CONNECTING_STAGE;
      refreshCount = 0;
    }
    html_BR();
    wrap_html_tag(F("h1"), F("Wifi Setup wizard"));
    html_add_form();

    switch (status) {
      case HANDLE_SETUP_SCAN_STAGE:
      {
        // first step, scan and show access points within reach...
        handle_setup_scan_and_show(ssid, other, password);
        break;
      }
      case  HANDLE_SETUP_CONNECTING_STAGE:
      {
        if (!handle_setup_connectingStage(refreshCount)) {
          status = HANDLE_SETUP_SCAN_STAGE;
        }
        ++refreshCount;
        break;
      }
    }
    html_end_form();
    sendHeadandTail(F("TmplAP"), true);
  } else {
    // Connect Success
    handle_setup_finish();
  }

  TXBuffer.endStream();
  delay(10);
}

void handle_setup_scan_and_show(const String& ssid, const String& other, const String& password) {
  static int n            = 0;
  WiFiMode_t cur_wifimode = WiFi.getMode();

  if (n == 0) {
    addLog(LOG_LEVEL_INFO, F("Start scan for WiFi APs"));
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
    html_table_header(F("RSSI"), 50);

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
      int32_t rssi = 0;
      TXBuffer +=  formatScanResult(i, "<BR>", rssi);
      html_TD();
      getWiFi_RSSI_icon(rssi, 45);
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

bool handle_setup_connectingStage(byte& refreshCount) {
  if (refreshCount > 0)
  {
    //      safe_strncpy(SecuritySettings.WifiSSID, "ssid", sizeof(SecuritySettings.WifiSSID));
    //      SecuritySettings.WifiKey[0] = 0;
    addButton(F("/setup"), F("Back to Setup"));
    html_BR();
    wifiSetupConnect = false;
    return false;
  }
  int wait = WIFI_RECONNECT_WAIT / 1000;

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
  return true;
}

void handle_setup_finish() {
  navMenuIndex = MENU_INDEX_TOOLS;
  sendHeadandTail_stdtemplate(_HEAD);
  addHtmlError(SaveSettings());
  html_add_form();
  html_table_class_normal();
  html_TR();

  addFormHeader(F("WiFi Setup Complete"));

  handle_sysinfo_Network();

  addFormSeparator(2);

  html_TR_TD();
  html_TD();

  if (!clientIPinSubnet()) {
    String host = formatIP(WiFi.localIP());
    String url  = F("http://");
    url += host;
    url += F("/config");
    addButton(url, host);
  }
  html_end_table();
  html_end_form();

  wifiSetup = false;
  sendHeadandTail_stdtemplate(_TAIL);
}
