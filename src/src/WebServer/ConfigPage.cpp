#include "../WebServer/ConfigPage.h"

#ifdef WEBSERVER_CONFIG

#include "../WebServer/HTML_wrappers.h"
#include "../WebServer/AccessControl.h"
#include "../WebServer/Markup.h"
#include "../WebServer/Markup_Buttons.h"
#include "../WebServer/Markup_Forms.h"
#include "../WebServer/ESPEasy_WebServer.h"

#ifdef USES_ESPEASY_NOW
#include "../DataStructs/MAC_address.h"
#include "../DataStructs/NodeStruct.h"
#endif

#include "../ESPEasyCore/Controller.h"
#include "../../ESPEasy/net/ESPEasyNetwork.h"
#include "../../ESPEasy/net/Helpers/NWAccessControl.h"

#include "../Globals/MQTT.h"
#include "../Globals/Nodes.h"
#include "../Globals/SecuritySettings.h"
#include "../Globals/Settings.h"

#include "../Helpers/DeepSleep.h"
#include "../Helpers/ESPEasy_Storage.h"
#include "../Helpers/Networking.h"
#include "../Helpers/StringConverter.h"


// ********************************************************************************
// Web Interface config page
// ********************************************************************************
void handle_config() {
  #ifndef BUILD_NO_RAM_TRACKER
  checkRAM(F("handle_config"));
  #endif

  if (!isLoggedIn()) { return; }

  navMenuIndex = MENU_INDEX_CONFIG;
  TXBuffer.startStream();
  sendHeadandTail_stdtemplate(_HEAD);

  if (web_server.args() != 0)
  {
    String name = webArg(F("name"));
    name.trim();

    Settings.Delay              = getFormItemInt(F("delay"), Settings.Delay);
    Settings.deepSleep_wakeTime = getFormItemInt(F("awaketime"), Settings.deepSleep_wakeTime);
    Settings.Unit = getFormItemInt(F("unit"), Settings.Unit);

    if (strcmp(Settings.Name, name.c_str()) != 0) {
      #ifndef LIMIT_BUILD_SIZE
      addLog(LOG_LEVEL_INFO, F("Unit Name changed."));
      #endif

      if (CPluginCall(CPlugin::Function::CPLUGIN_GOT_INVALID, 0)) { // inform controllers that the old name will be invalid from now on.
#if FEATURE_MQTT
        MQTTDisconnect();                                           // disconnect form MQTT Server if invalid message was sent succesfull.
#endif // if FEATURE_MQTT
      }
#if FEATURE_MQTT
      MQTTclient_should_reconnect = true;
#endif // if FEATURE_MQTT
    }

    // Unit name
    safe_strncpy(Settings.Name, name.c_str(), sizeof(Settings.Name));
    Settings.appendUnitToHostname(isFormItemChecked(F("appendunittohostname")));

    // Password
    copyFormPassword(F("password"), SecuritySettings.Password, sizeof(SecuritySettings.Password));

    // TD-er Read access control from form.
    SecuritySettings.IPblockLevel = getFormItemInt(F("ipblocklevel"));

    if (SecuritySettings.IPblockLevel == ONLY_IP_RANGE_ALLOWED) {
        webArg2ip(F("iprangelow"),  SecuritySettings.AllowedIPrangeLow);
        webArg2ip(F("iprangehigh"), SecuritySettings.AllowedIPrangeHigh);
    } else {
      for (size_t i = 0; i < 4; ++i) {
        SecuritySettings.AllowedIPrangeLow[i] = 0;
        SecuritySettings.AllowedIPrangeHigh[i] = 255;
      }
    }

    #ifdef USES_ESPEASY_NOW
    for (int peer = 0; peer < ESPEASY_NOW_PEER_MAX; ++peer) {
      String peer_mac = webArg(concat(F("peer"), peer));
      if (peer_mac.length() == 0) {
        peer_mac = F("00:00:00:00:00:00");
      }
      MAC_address mac;
      if (mac.set(peer_mac.c_str())) {
        mac.get(SecuritySettings.EspEasyNowPeerMAC[peer]);
      }
      /*
      String log = F("MAC decoding ");
      log += peer_mac;
      log += F(" => ");
      log += mac.toString();
      addLog(LOG_LEVEL_INFO, log);
      */
    }
    #endif

    Settings.deepSleepOnFail = isFormItemChecked(F("deepsleeponfail"));
    #if FEATURE_ALTERNATIVE_CDN_URL
    set_CDN_url_custom(webArg(F("alturl")));
    #endif // if FEATURE_ALTERNATIVE_CDN_URL
    addHtmlError(SaveSettings());
  }

  html_add_form();
  html_table_class_normal();

  addFormHeader(F("Main Settings"));

  Settings.Name[25]             = 0;
  SecuritySettings.Password[25] = 0;
  addFormTextBox(F("Unit Name"), F("name"), Settings.Name, 25);
  addFormNote(concat(F("Hostname: "), ESPEasy::net::NetworkCreateRFCCompliantHostname()));
  addFormNumericBox(F("Unit Number"), F("unit"), Settings.Unit, 0, UNIT_NUMBER_MAX);
  addFormCheckBox(F("Append Unit Number to hostname"), F("appendunittohostname"), Settings.appendUnitToHostname());
  addFormPasswordBox(F("Admin Password"), F("password"), SecuritySettings.Password, 25);

  // TD-er add IP access box F("ipblocklevel")
  addFormSubHeader(F("Client IP filtering"));
  {
    addFormIPaccessControlSelect(F("Client IP block level"), F("ipblocklevel"), SecuritySettings.IPblockLevel);
    
    if (SecuritySettings.IPblockLevel == ONLY_IP_RANGE_ALLOWED) {
      addFormIPBox(F("Access IP lower range"), F("iprangelow"),  SecuritySettings.AllowedIPrangeLow);
      addFormIPBox(F("Access IP upper range"), F("iprangehigh"), SecuritySettings.AllowedIPrangeHigh);
    }
  }

#ifdef USES_ESPEASY_NOW
  addFormSubHeader(F("ESPEasy-NOW"));
  for (int peer = 0; peer < ESPEASY_NOW_PEER_MAX; ++peer) {
    addFormMACBox(concat(F("Peer "), peer + 1),
                  concat(F("peer"), peer), 
                  SecuritySettings.EspEasyNowPeerMAC[peer]);

    bool match_STA;
    const NodeStruct* nodeInfo = Nodes.getNodeByMac(SecuritySettings.EspEasyNowPeerMAC[peer], match_STA);
    if (nodeInfo != nullptr)
    {
      String summary = nodeInfo->getSummary();
      summary += match_STA ? F(" (STA)") : F(" (AP)");
      addFormNote(summary);
    }
    
  }
#endif

  addFormSubHeader(F("Sleep Mode"));

  addFormNumericBox(F("Sleep awake time"), F("awaketime"), Settings.deepSleep_wakeTime, 0, 255);
  addUnit(F("sec"));
  addHelpButton(F("SleepMode"));
  addFormNote(F("0 = Sleep Disabled, else time awake from sleep"));

  int dsmax = getDeepSleepMax();
  addFormNumericBox(F("Sleep time"), F("delay"), Settings.Delay, 0, dsmax); // limited by hardware
  {
    addUnit(concat(F("sec (max: "), dsmax) + ')');
  }

  addFormCheckBox(F("Sleep on connection failure"), F("deepsleeponfail"), Settings.deepSleepOnFail);

  addFormSeparator(2);

  #if FEATURE_ALTERNATIVE_CDN_URL
  addFormSubHeader(F("CDN (Content delivery network)"));

  addFormTextBox(F("Custom CDN URL"), F("alturl"), get_CDN_url_custom(), 255);
  addFormNote(concat(F("Leave empty for default CDN url: "), get_CDN_url_prefix()));

  addFormSeparator(2);
  #endif // if FEATURE_ALTERNATIVE_CDN_URL

  html_TR_TD();
  html_TD();
  addSubmitButton();
  html_end_table();
  html_end_form();

  sendHeadandTail_stdtemplate(_TAIL);
  TXBuffer.endStream();
}

#endif // ifdef WEBSERVER_CONFIG