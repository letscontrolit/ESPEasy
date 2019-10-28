

#ifdef WEBSERVER_NEW_UI

// ********************************************************************************
// Web Interface Wifi scanner
// ********************************************************************************
void handle_wifiscanner_json() {
  checkRAM(F("handle_wifiscanner"));

  if (!isLoggedIn()) { return; }
  navMenuIndex = MENU_INDEX_TOOLS;
  TXBuffer.startJsonStream();
  TXBuffer += "[{";
  bool firstentry = true;
  int  n          = WiFi.scanNetworks(false, true);

  for (int i = 0; i < n; ++i)
  {
    if (firstentry) { firstentry = false; }
    else { TXBuffer += ",{"; }

    stream_next_json_object_value(getLabel(LabelType::SSID),      WiFi.SSID(i));
    stream_next_json_object_value(getLabel(LabelType::BSSID),     WiFi.BSSIDstr(i));
    stream_next_json_object_value(getLabel(LabelType::CHANNEL),   String(WiFi.channel(i)));
    stream_next_json_object_value(getLabel(LabelType::WIFI_RSSI), String(WiFi.RSSI(i)));
    String authType;

    switch (WiFi.encryptionType(i)) {
    # ifdef ESP32
      case WIFI_AUTH_OPEN: authType            = F("open"); break;
      case WIFI_AUTH_WEP:  authType            = F("WEP"); break;
      case WIFI_AUTH_WPA_PSK: authType         = F("WPA/PSK"); break;
      case WIFI_AUTH_WPA2_PSK: authType        = F("WPA2/PSK"); break;
      case WIFI_AUTH_WPA_WPA2_PSK: authType    = F("WPA/WPA2/PSK"); break;
      case WIFI_AUTH_WPA2_ENTERPRISE: authType = F("WPA2 Enterprise"); break;
    # else // ifdef ESP32
      case ENC_TYPE_WEP:  authType = F("WEP"); break;
      case ENC_TYPE_TKIP: authType = F("WPA/PSK"); break;
      case ENC_TYPE_CCMP: authType = F("WPA2/PSK"); break;
      case ENC_TYPE_NONE: authType = F("open"); break;
      case ENC_TYPE_AUTO: authType = F("WPA/WPA2/PSK"); break;
    # endif // ifdef ESP32
      default:
        break;
    }

    if (authType.length() > 0) {
      stream_last_json_object_value(F("auth"), authType);
    }
  }
  TXBuffer += "]";
  TXBuffer.endStream();
}

#endif // WEBSERVER_NEW_UI

void handle_wifiscanner() {
  checkRAM(F("handle_wifiscanner"));

  if (!isLoggedIn()) { return; }
  navMenuIndex = MENU_INDEX_TOOLS;
  TXBuffer.startStream();
  sendHeadandTail_stdtemplate(_HEAD);
  html_table_class_multirow();
  html_TR();
  html_table_header(getLabel(LabelType::SSID));
  html_table_header(getLabel(LabelType::BSSID));
  html_table_header("info");

  int n = WiFi.scanNetworks(false, true);

  if (n == 0) {
    TXBuffer += F("No Access Points found");
  }
  else
  {
    for (int i = 0; i < n; ++i)
    {
      html_TR_TD();
      TXBuffer += formatScanResult(i, "<TD>");
    }
  }

  html_end_table();
  sendHeadandTail_stdtemplate(_TAIL);
  TXBuffer.endStream();
}
