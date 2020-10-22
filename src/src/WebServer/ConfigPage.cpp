#include "../WebServer/ConfigPage.h"

#ifdef WEBSERVER_CONFIG

#include "../WebServer/WebServer.h"
#include "../WebServer/HTML_wrappers.h"
#include "../WebServer/AccessControl.h"
#include "../WebServer/Markup.h"
#include "../WebServer/Markup_Buttons.h"
#include "../WebServer/Markup_Forms.h"

#include "../ESPEasyCore/Controller.h"

#include "../Globals/MQTT.h"
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
  checkRAM(F("handle_config"));

  if (!isLoggedIn()) { return; }

  navMenuIndex = MENU_INDEX_CONFIG;
  TXBuffer.startStream();
  sendHeadandTail_stdtemplate(_HEAD);

  if (web_server.args() != 0)
  {
    String name = web_server.arg(F("name"));
    name.trim();

    // String password = web_server.arg(F("password"));
    String iprangelow  = web_server.arg(F("iprangelow"));
    String iprangehigh = web_server.arg(F("iprangehigh"));

    Settings.Delay              = getFormItemInt(F("delay"), Settings.Delay);
    Settings.deepSleep_wakeTime = getFormItemInt(F("awaketime"), Settings.deepSleep_wakeTime);
    String espip      = web_server.arg(F("espip"));
    String espgateway = web_server.arg(F("espgateway"));
    String espsubnet  = web_server.arg(F("espsubnet"));
    String espdns     = web_server.arg(F("espdns"));
#ifdef HAS_ETHERNET
    String espethip      = web_server.arg(F("espethip"));
    String espethgateway = web_server.arg(F("espethgateway"));
    String espethsubnet  = web_server.arg(F("espethsubnet"));
    String espethdns     = web_server.arg(F("espethdns"));
#endif
    Settings.Unit = getFormItemInt(F("unit"), Settings.Unit);

    // String apkey = web_server.arg(F("apkey"));
    String ssid = web_server.arg(F("ssid"));

    if (strcmp(Settings.Name, name.c_str()) != 0) {
      addLog(LOG_LEVEL_INFO, F("Unit Name changed."));

      if (CPluginCall(CPlugin::Function::CPLUGIN_GOT_INVALID, 0)) { // inform controllers that the old name will be invalid from now on.
#ifdef USES_MQTT
        MQTTDisconnect();                                           // disconnect form MQTT Server if invalid message was sent succesfull.
#endif // USES_MQTT
      }
#ifdef USES_MQTT
      MQTTclient_should_reconnect = true;
#endif // USES_MQTT
    }

    // Unit name
    safe_strncpy(Settings.Name, name.c_str(), sizeof(Settings.Name));
    Settings.appendUnitToHostname(isFormItemChecked(F("appendunittohostname")));

    // Password
    copyFormPassword(F("password"), SecuritySettings.Password, sizeof(SecuritySettings.Password));

    // SSID 1
    safe_strncpy(SecuritySettings.WifiSSID, ssid.c_str(), sizeof(SecuritySettings.WifiSSID));
    copyFormPassword(F("key"), SecuritySettings.WifiKey, sizeof(SecuritySettings.WifiKey));

    // SSID 2
    strncpy_webserver_arg(SecuritySettings.WifiSSID2, F("ssid2"));
    copyFormPassword(F("key2"),  SecuritySettings.WifiKey2,  sizeof(SecuritySettings.WifiKey2));

    // Access point password.
    copyFormPassword(F("apkey"), SecuritySettings.WifiAPKey, sizeof(SecuritySettings.WifiAPKey));


    // TD-er Read access control from form.
    SecuritySettings.IPblockLevel = getFormItemInt(F("ipblocklevel"));

    switch (SecuritySettings.IPblockLevel) {
      case LOCAL_SUBNET_ALLOWED:
      {
        IPAddress low, high;
        getSubnetRange(low, high);

        for (byte i = 0; i < 4; ++i) {
          SecuritySettings.AllowedIPrangeLow[i]  = low[i];
          SecuritySettings.AllowedIPrangeHigh[i] = high[i];
        }
        break;
      }
      case ONLY_IP_RANGE_ALLOWED:
      case ALL_ALLOWED:

        // iprangelow.toCharArray(tmpString, 26);
        str2ip(iprangelow,  SecuritySettings.AllowedIPrangeLow);

        // iprangehigh.toCharArray(tmpString, 26);
        str2ip(iprangehigh, SecuritySettings.AllowedIPrangeHigh);
        break;
    }

    Settings.deepSleepOnFail = isFormItemChecked(F("deepsleeponfail"));
    str2ip(espip,      Settings.IP);
    str2ip(espgateway, Settings.Gateway);
    str2ip(espsubnet,  Settings.Subnet);
    str2ip(espdns,     Settings.DNS);
#ifdef HAS_ETHERNET
    str2ip(espethip,      Settings.ETH_IP);
    str2ip(espethgateway, Settings.ETH_Gateway);
    str2ip(espethsubnet,  Settings.ETH_Subnet);
    str2ip(espethdns,     Settings.ETH_DNS);
#endif
    addHtmlError(SaveSettings());
  }

  html_add_form();
  html_table_class_normal();

  addFormHeader(F("Main Settings"));

  Settings.Name[25]             = 0;
  SecuritySettings.Password[25] = 0;
  addFormTextBox(F("Unit Name"), F("name"), Settings.Name, 25);
  addFormNumericBox(F("Unit Number"), F("unit"), Settings.Unit, 0, UNIT_NUMBER_MAX);
  addFormCheckBox(F("Append Unit Number to hostname"), F("appendunittohostname"), Settings.appendUnitToHostname());
  addFormPasswordBox(F("Admin Password"), F("password"), SecuritySettings.Password, 25);

  addFormSubHeader(F("Wifi Settings"));

  addFormTextBox(getLabel(LabelType::SSID), F("ssid"), SecuritySettings.WifiSSID, 31);
  addFormPasswordBox(F("WPA Key"), F("key"), SecuritySettings.WifiKey, 63);
  addFormTextBox(F("Fallback SSID"), F("ssid2"), SecuritySettings.WifiSSID2, 31);
  addFormPasswordBox(F("Fallback WPA Key"), F("key2"), SecuritySettings.WifiKey2, 63);
  addFormSeparator(2);
  addFormPasswordBox(F("WPA AP Mode Key"), F("apkey"), SecuritySettings.WifiAPKey, 63);

  // TD-er add IP access box F("ipblocklevel")
  addFormSubHeader(F("Client IP filtering"));
  {
    IPAddress low, high;
    getIPallowedRange(low, high);
    byte iplow[4];
    byte iphigh[4];

    for (byte i = 0; i < 4; ++i) {
      iplow[i]  = low[i];
      iphigh[i] = high[i];
    }
    addFormIPaccessControlSelect(F("Client IP block level"), F("ipblocklevel"), SecuritySettings.IPblockLevel);
    addFormIPBox(F("Access IP lower range"), F("iprangelow"),  iplow);
    addFormIPBox(F("Access IP upper range"), F("iprangehigh"), iphigh);
  }

  addFormSubHeader(F("WiFi IP Settings"));

  addFormIPBox(F("ESP WiFi IP"),         F("espip"),      Settings.IP);
  addFormIPBox(F("ESP WiFi Gateway"),    F("espgateway"), Settings.Gateway);
  addFormIPBox(F("ESP WiFi Subnetmask"), F("espsubnet"),  Settings.Subnet);
  addFormIPBox(F("ESP WiFi DNS"),        F("espdns"),     Settings.DNS);
  addFormNote(F("Leave empty for DHCP"));

#ifdef HAS_ETHERNET
  addFormSubHeader(F("Ethernet IP Settings"));

  addFormIPBox(F("ESP Ethernet IP"),         F("espethip"),      Settings.ETH_IP);
  addFormIPBox(F("ESP Ethernet Gateway"),    F("espethgateway"), Settings.ETH_Gateway);
  addFormIPBox(F("ESP Ethernet Subnetmask"), F("espethsubnet"),  Settings.ETH_Subnet);
  addFormIPBox(F("ESP Ethernet DNS"),        F("espethdns"),     Settings.ETH_DNS);
  addFormNote(F("Leave empty for DHCP"));
#endif


  addFormSubHeader(F("Sleep Mode"));

  addFormNumericBox(F("Sleep awake time"), F("awaketime"), Settings.deepSleep_wakeTime, 0, 255);
  addUnit(F("sec"));
  addHelpButton(F("SleepMode"));
  addFormNote(F("0 = Sleep Disabled, else time awake from sleep"));

  int dsmax = getDeepSleepMax();
  addFormNumericBox(F("Sleep time"), F("delay"), Settings.Delay, 0, dsmax); // limited by hardware
  {
    String maxSleeptimeUnit = F("sec (max: ");
    maxSleeptimeUnit += String(dsmax);
    maxSleeptimeUnit += ')';
    addUnit(maxSleeptimeUnit);
  }

  addFormCheckBox(F("Sleep on connection failure"), F("deepsleeponfail"), Settings.deepSleepOnFail);

  addFormSeparator(2);

  html_TR_TD();
  html_TD();
  addSubmitButton();
  html_end_table();
  html_end_form();

  sendHeadandTail_stdtemplate(_TAIL);
  TXBuffer.endStream();
}

#endif // ifdef WEBSERVER_CONFIG
