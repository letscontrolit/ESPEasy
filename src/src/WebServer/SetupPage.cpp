#include "../WebServer/SetupPage.h"


#ifdef WEBSERVER_SETUP

#include "../WebServer/WebServer.h"
#include "../WebServer/AccessControl.h"
#include "../WebServer/HTML_wrappers.h"
#include "../WebServer/Markup.h"
#include "../WebServer/Markup_Buttons.h"
#include "../WebServer/Markup_Forms.h"
#include "../WebServer/SysInfoPage.h"

#include "../ESPEasyCore/ESPEasyNetwork.h"
#include "../ESPEasyCore/ESPEasyWifi.h"


#include "../Globals/ESPEasyWiFiEvent.h"
#include "../Globals/SecuritySettings.h"

#include "../Helpers/Networking.h"
#include "../Helpers/ESPEasy_Storage.h"
#include "../Helpers/StringConverter.h"


// ********************************************************************************
// Web Interface Setup Wizard
// ********************************************************************************

#define HANDLE_SETUP_SCAN_STAGE       0
#define HANDLE_SETUP_CONNECTING_STAGE 1

void handle_setup() {
  checkRAM(F("handle_setup"));

  // Do not check client IP range allowed.
  TXBuffer.startStream();

  if (!NetworkConnected())
  {
    sendHeadandTail(F("TmplAP"));
    static byte status       = 0;
    static byte refreshCount = 0;
    String ssid              = web_server.arg(F("ssid"));
    String other             = web_server.arg(F("other"));
    String password          = web_server.arg(F("pass"));

    if (other.length() != 0)
    {
      ssid = other;
    }

    // if ssid config not set and params are both provided
    if ((status == 0) && (ssid.length() != 0) /*&& strcasecmp(SecuritySettings.WifiSSID, "ssid") == 0 */)
    {
      safe_strncpy(SecuritySettings.WifiKey,  password.c_str(), sizeof(SecuritySettings.WifiKey));
      safe_strncpy(SecuritySettings.WifiSSID, ssid.c_str(),     sizeof(SecuritySettings.WifiSSID));
      WiFiEventData.wifiSetupConnect         = true;
      WiFiEventData.wifiConnectAttemptNeeded = true;

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
  if (WiFi.scanComplete() <= 0) {
    WiFiMode_t cur_wifimode = WiFi.getMode();
    WifiScan(false, false);
    setWifiMode(cur_wifimode);
  }

  const int8_t scanCompleteStatus = WiFi.scanComplete();

  if (scanCompleteStatus <= 0) {
    addHtml(F("No Access Points found"));
  }
  else
  {
    html_table_class_multirow();
    html_TR();
    html_table_header(F("Pick"), 50);
    html_table_header(F("Network info"));
    html_table_header(F("RSSI"), 50);

    for (int i = 0; i < scanCompleteStatus; ++i)
    {
      html_TR_TD();
      addHtml(F("<label class='container2'>"));
      addHtml(F("<input type='radio' name='ssid' value='"));
      {
        String escapeBuffer = WiFi.SSID(i);
        htmlStrongEscape(escapeBuffer);
        addHtml(escapeBuffer);
      }
      addHtml("'");

      if (WiFi.SSID(i) == ssid) {
        addHtml(F(" checked "));
      }
      addHtml(F("><span class='dotmark'></span></label><TD>"));
      int32_t rssi = 0;
      addHtml(formatScanResult(i, "<BR>", rssi));
      html_TD();
      getWiFi_RSSI_icon(rssi, 45);
    }
    html_end_table();
  }

  addHtml(F(
            "<BR><label class='container2'>other SSID:<input type='radio' name='ssid' id='other_ssid' value='other' ><span class='dotmark'></span></label>"));
  addHtml(F("<input class='wide' type ='text' name='other' value='"));
  addHtml(other);
  addHtml(F("'><BR><BR>"));

  addFormSeparator(2);

  addHtml(F("<BR>Password:<BR><input class='wide' type ='text' name='pass' value='"));
  addHtml(password);
  addHtml(F("'><BR><BR>"));

  addSubmitButton(F("Connect"), "");
}

bool handle_setup_connectingStage(byte refreshCount) {
  if (refreshCount > 0)
  {
    //      safe_strncpy(SecuritySettings.WifiSSID, "ssid", sizeof(SecuritySettings.WifiSSID));
    //      SecuritySettings.WifiKey[0] = 0;
    addButton(F("/setup"), F("Back to Setup"));
    html_BR();
    WiFiEventData.wifiSetupConnect = false;
    return false;
  }
  int wait = WIFI_RECONNECT_WAIT / 1000;

  if (refreshCount != 0) {
    wait = 3;
  }
  addHtml(F("Please wait for <h1 id='countdown'>20..</h1>"));
  addHtml(F("<script type='text/JavaScript'>"));
  addHtml(F("function timedRefresh(timeoutPeriod) {"));
  addHtml(F("var timer = setInterval(function() {"));
  addHtml(F("if (timeoutPeriod > 0) {"));
  addHtml(F("timeoutPeriod -= 1;"));
  addHtml(F("document.getElementById('countdown').innerHTML = timeoutPeriod + '..' + '<br />';"));
  addHtml(F("} else {"));
  addHtml(F("clearInterval(timer);"));
  addHtml(F("window.location.href = window.location.href;"));
  addHtml(F("};"));
  addHtml(F("}, 1000);"));
  addHtml(F("};"));
  addHtml(F("timedRefresh("));
  addHtml(String(wait));
  addHtml(F(");"));
  html_add_script_end();
  addHtml(F("seconds while trying to connect"));
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
    String host = formatIP(NetworkLocalIP());
    String url  = F("http://");
    url += host;
    url += F("/config");
    addButton(url, host);
  }
  html_end_table();
  html_end_form();

  WiFiEventData.wifiSetup = false;
  sendHeadandTail_stdtemplate(_TAIL);
}

#endif // ifdef WEBSERVER_SETUP
