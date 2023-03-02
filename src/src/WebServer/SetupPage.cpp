#include "../WebServer/SetupPage.h"


#ifdef WEBSERVER_SETUP

# include "../WebServer/ESPEasy_WebServer.h"
# include "../WebServer/AccessControl.h"
# include "../WebServer/HTML_wrappers.h"
# include "../WebServer/Markup.h"
# include "../WebServer/Markup_Buttons.h"
# include "../WebServer/Markup_Forms.h"
# include "../WebServer/SysInfoPage.h"

# include "../ESPEasyCore/ESPEasyNetwork.h"
# include "../ESPEasyCore/ESPEasyWifi.h"

# include "../Globals/ESPEasyWiFiEvent.h"
# include "../Globals/NetworkState.h"
# include "../Globals/RTC.h"
# include "../Globals/Settings.h"
# include "../Globals/SecuritySettings.h"
# include "../Globals/WiFi_AP_Candidates.h"

# include "../Helpers/Misc.h"
# include "../Helpers/Networking.h"
# include "../Helpers/ESPEasy_Storage.h"
# include "../Helpers/StringConverter.h"



#ifndef SETUP_PAGE_SHOW_CONFIG_BUTTON
  #define SETUP_PAGE_SHOW_CONFIG_BUTTON true
#endif



// ********************************************************************************
// Web Interface Setup Wizard
// ********************************************************************************

# define HANDLE_SETUP_SCAN_STAGE       0
# define HANDLE_SETUP_CONNECTING_STAGE 1

void handle_setup() {
  # ifndef BUILD_NO_RAM_TRACKER
  checkRAM(F("handle_setup"));
  # endif // ifndef BUILD_NO_RAM_TRACKER

  // Do not check client IP range allowed.
  TXBuffer.startStream();

  const bool connected = NetworkConnected();


//  if (connected) {
    navMenuIndex = MENU_INDEX_SETUP;
    sendHeadandTail_stdtemplate(_HEAD);
/*  } else {
    sendHeadandTail(F("TmplAP"));
  }
  */

  const bool clearButtonPressed = hasArg(F("performclearcredentials"));
  const bool clearWiFiCredentials = 
    isFormItemChecked(F("clearcredentials")) && clearButtonPressed;

  {
    if (clearWiFiCredentials) {
      SecuritySettings.clearWiFiCredentials();
      addHtmlError(SaveSecuritySettings());

      html_add_form();
      html_table_class_normal();

      addFormHeader(F("WiFi credentials cleared, reboot now"));
      html_end_table();
    } else {    
  //    if (active_network_medium == NetworkMedium_t::WIFI)
  //    {
        static uint8_t status       = HANDLE_SETUP_SCAN_STAGE;
        static uint8_t refreshCount = 0;

        String ssid              = webArg(F("ssid"));
        String other             = webArg(F("other"));
        String password;
        bool passwordGiven = getFormPassword(F("pass"), password);
        if (passwordGiven) {
          passwordGiven = !password.isEmpty();
        }
        const bool emptyPassAllowed = isFormItemChecked(F("emptypass"));
        const bool performRescan = hasArg(F("performrescan"));
        if (performRescan) {
          WiFiEventData.lastScanMoment.clear();
          WifiScan(false);
        }

        if (!other.isEmpty())
        {
          ssid = other;
        }

        if (!performRescan) {
          // if ssid config not set and params are both provided
          if ((status == HANDLE_SETUP_SCAN_STAGE) && (!ssid.isEmpty()) /*&& strcasecmp(SecuritySettings.WifiSSID, "ssid") == 0 */)
          {
            if (clearButtonPressed) {
              addHtmlError(F("Warning: Need to confirm to clear WiFi credentials"));
            } else if (!passwordGiven && !emptyPassAllowed) {
              addHtmlError(F("No password entered"));
            } else {
              safe_strncpy(SecuritySettings.WifiKey,  password.c_str(), sizeof(SecuritySettings.WifiKey));
              safe_strncpy(SecuritySettings.WifiSSID, ssid.c_str(),     sizeof(SecuritySettings.WifiSSID));
              // Hidden SSID
              Settings.IncludeHiddenSSID(isFormItemChecked(F("hiddenssid")));
              addHtmlError(SaveSettings());
              WiFiEventData.wifiSetupConnect         = true;
              WiFiEventData.wifiConnectAttemptNeeded = true;
              WiFi_AP_Candidates.force_reload(); // Force reload of the credentials and found APs from the last scan

              if (loglevelActiveFor(LOG_LEVEL_INFO)) {
                String reconnectlog = F("WIFI : Credentials Changed, retry connection. SSID: ");
                reconnectlog += ssid;
                addLogMove(LOG_LEVEL_INFO, reconnectlog);
              }
              status       = HANDLE_SETUP_CONNECTING_STAGE;
              refreshCount = 0;
              AttemptWiFiConnect();
            }
          }
        }
        html_BR();
        wrap_html_tag(F("h1"), connected ? F("Connected to a network") : F("Wifi Setup wizard"));
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
  /*
      } else {
        html_add_form();
        addFormHeader(F("Ethernet Setup Complete"));

      }
  */

      html_table_class_normal();
      html_TR();
#if defined(WEBSERVER_SYSINFO) && !defined(WEBSERVER_SYSINFO_MINIMAL)
      handle_sysinfo_NetworkServices();
#endif
      if (connected) {

        //addFormHeader(F("Current network configuration"));

#ifdef WEBSERVER_SYSINFO
        handle_sysinfo_Network();
#endif

        addFormSeparator(2);

        html_TR_TD();
        html_TD();
        
        #if SETUP_PAGE_SHOW_CONFIG_BUTTON
        if (!clientIPinSubnet()) {
          String host = formatIP(NetworkLocalIP());
          String url  = F("http://");
          url += host;
          url += F("/config");
          addButton(url, host);
        }
        #endif

        WiFiEventData.wifiSetup = false;
      } 
      html_end_table();

      html_BR();
      html_BR();
      html_BR();
      html_BR();
      html_BR();
      html_BR();
      html_BR();

      html_table_class_normal();

      addFormHeader(F("Advanced WiFi settings"));

      addFormCheckBox(F("Include Hidden SSID"), F("hiddenssid"), Settings.IncludeHiddenSSID());
      addFormNote(F("Must be checked to connect to a hidden SSID"));

      html_BR();
      html_BR();

      addFormHeader(F("Clear WiFi credentials"));
      addFormCheckBox(F("Confirm clear"), F("clearcredentials"), false);

      html_TR_TD();
      html_TD();
      addSubmitButton(F("Clear and Reboot"), F("performclearcredentials"), F("red"));
      html_end_table();
    }

    html_end_form();
  }
//  if (connected) {
    sendHeadandTail_stdtemplate(_TAIL);
/*  } else {
    sendHeadandTail(F("TmplAP"), true);
  }
*/
  TXBuffer.endStream();
  delay(10);
  if (clearWiFiCredentials) {
    reboot(ESPEasy_Scheduler::IntendedRebootReason_e::RestoreSettings);
  }
}

void handle_setup_scan_and_show(const String& ssid, const String& other, const String& password) {
  int8_t scanCompleteStatus = WiFi_AP_Candidates.scanComplete();
  const bool needsRescan = scanCompleteStatus <= 0 || WiFiScanAllowed();
  if (needsRescan) {
    WiFiMode_t cur_wifimode = WiFi.getMode();
    WifiScan(false);
    scanCompleteStatus = WiFi_AP_Candidates.scanComplete();
    setWifiMode(cur_wifimode);
  }


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

    for (auto it = WiFi_AP_Candidates.scanned_begin(); it != WiFi_AP_Candidates.scanned_end(); ++it)
    {
      html_TR_TD();
      const String id = it->toString("");
      addHtml(F("<label "));
      addHtmlAttribute(F("class"), F("container2"));
      addHtmlAttribute(F("for"), id);
      addHtml('>');

      addHtml(F("<input type='radio' name='ssid' value='"));
      if (it->isHidden) {
        addHtml(F("#Hidden#' disabled"));
      } else {
        String escapeBuffer = it->ssid;
        htmlStrongEscape(escapeBuffer);
        addHtml(escapeBuffer);
        addHtml('\'');
      }

      addHtmlAttribute(F("id"), id);

      {
        if (it->bssid_match(RTC.lastBSSID)) {
          if (!WiFi_AP_Candidates.SettingsIndexMatchCustomCredentials(RTC.lastWiFiSettingsIndex)) {
            addHtml(F(" checked "));  
          }
        }
      }

      addHtml(F("><span class='dotmark'></span>"));
      addHtml(F("</label>"));

      html_TD();
      addHtml(F("<label "));
      addHtmlAttribute(F("for"), id);
      addHtml('>');
      addHtml(it->toString(F("<BR>")));
      addHtml(F("</label>"));

      html_TD();
      addHtml(F("<label "));
      addHtmlAttribute(F("for"), id);
      addHtml('>');
      getWiFi_RSSI_icon(it->rssi, 45);
      addHtml(F("</label>"));
    }
    html_end_table();
  }

  html_BR();

  addSubmitButton(F("Rescan"), F("performrescan"));

  html_BR();

  html_table_class_normal();
  html_TR_TD();

  addHtml(F("<label "));
  addHtmlAttribute(F("class"), F("container2"));
  addHtml('>');
  addHtml(F("other SSID:"));
  addHtml(F("<input "));
  addHtmlAttribute(F("type"),  F("radio"));
  addHtmlAttribute(F("name"),  F("ssid"));
  addHtmlAttribute(F("id"),    F("other_ssid"));
  addHtmlAttribute(F("value"), F("other"));
  addHtml('>');
  addHtml(F("<span class='dotmark'></span></label>"));

  html_TD();

  addHtml(F("<label "));
  addHtmlAttribute(F("for"),    F("other_ssid"));
  addHtml('>');

  addHtml(F("<input "));
  addHtmlAttribute(F("class"), F("wide"));
  addHtmlAttribute(F("type"),  F("text"));
  addHtmlAttribute(F("name"),  F("other"));
  addHtmlAttribute(F("value"), other);
  addHtml('>');
  addHtml(F("</label>"));


  html_TR();

  html_BR();
  html_BR();

  addFormSeparator(2);

  html_BR();

  addFormPasswordBox(F("Password"), F("pass"), password, 63);
  addFormCheckBox(F("Allow Empty Password"), F("emptypass"), false);
  
/*
  if (SecuritySettings.hasWiFiCredentials(SecurityStruct::WiFiCredentialsSlot::first)) {
    addFormCheckBox(F("Clear Stored SSID1"), F("clearssid1"), false);
    addFormNote(String(F("Current: ")) + getValue(LabelType::WIFI_STORED_SSID1));
  }
  if (SecuritySettings.hasWiFiCredentials(SecurityStruct::WiFiCredentialsSlot::second)) {
    addFormCheckBox(F("Clear Stored SSID2"), F("clearssid2"), false);
    addFormNote(String(F("Current: ")) + getValue(LabelType::WIFI_STORED_SSID2));
  }
  */

  html_TR_TD();
  html_TD();
  html_BR();
  addSubmitButton(F("Connect"), EMPTY_STRING);

  html_end_table();
}

bool handle_setup_connectingStage(uint8_t refreshCount) {
  if (refreshCount > 0)
  {
    //      safe_strncpy(SecuritySettings.WifiSSID, "ssid", sizeof(SecuritySettings.WifiSSID));
    //      SecuritySettings.WifiKey[0] = 0;
    addButton(F("/setup"), F("Back to Setup"));
    html_TR_TD();
    html_BR();
    WiFiEventData.wifiSetupConnect = false;
    return false;
  }
  int wait = WIFI_RECONNECT_WAIT / 1000;

  if (refreshCount != 0) {
    wait = 3;
  }
  addHtml(F("Please wait for <h1 id='countdown'>20..</h1>" 
            "<script>"
            "function timedRefresh(tp) {"
            "var t=setInterval(function(){"
            "if(tp>0){"
            "tp-=1;"
            "document.getElementById('countdown').innerHTML=tp+'..'+'<br/>';"
            "}else{"
            "clearInterval(t);"
            "window.location.href=window.location.href;"
            "};"
            "},1000);"
            "};"
            "timedRefresh("));
  addHtmlInt(wait);
  addHtml(')', ';');
  html_add_script_end();
  addHtml(F("seconds while trying to connect"));
  return true;
}

#endif // ifdef WEBSERVER_SETUP
