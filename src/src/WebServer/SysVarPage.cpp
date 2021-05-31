#include "../WebServer/SysVarPage.h"

#include "../WebServer/WebServer.h"
#include "../WebServer/AccessControl.h"
#include "../WebServer/Markup.h"
#include "../WebServer/Markup_Forms.h"
#include "../WebServer/HTML_wrappers.h"

#include "../Globals/RuntimeData.h"

#include "../Helpers/StringConverter.h"
#include "../Helpers/SystemVariables.h"

#ifdef WEBSERVER_SYSVARS


// ********************************************************************************
// Web Interface sysvars showing all system vars and their value.
// ********************************************************************************
void addSysVar_enum_html(SystemVariables::Enum enumval) {
  addSysVar_html(SystemVariables::toString(enumval));
}


void handle_sysvars() {
  #ifndef BUILD_NO_RAM_TRACKER
  checkRAM(F("handle_sysvars"));
  #endif

  if (!isLoggedIn()) { return; }
  TXBuffer.startStream();
  sendHeadandTail_stdtemplate();

  html_BR();
  addHtml(F("<p>This page may load slow.<BR>Do not load too often, since it may affect performance of the node.</p>"));
  html_BR();

  // the table header
  html_table_class_normal();
  html_TR();
  html_table_header(F("System Variables"));
  html_table_header(F("Normal"));
  html_table_header(F("URL encoded"), F("ESPEasy_System_Variables"), 0);

  addTableSeparator(F("Constants"), 3, 3);
  addSysVar_enum_html(SystemVariables::CR);
  addSysVar_enum_html(SystemVariables::LF);
  addSysVar_enum_html(SystemVariables::SPACE);
  addSysVar_enum_html(SystemVariables::S_CR);
  addSysVar_enum_html(SystemVariables::S_LF);

  addTableSeparator(F("Network"), 3, 3);
  addSysVar_enum_html(SystemVariables::MAC);
#if defined(ESP8266)
  addSysVar_enum_html(SystemVariables::MAC_INT);
#endif // if defined(ESP8266)
  addSysVar_enum_html(SystemVariables::IP);
  addSysVar_enum_html(SystemVariables::IP4);
  addSysVar_enum_html(SystemVariables::SUBNET);
  addSysVar_enum_html(SystemVariables::GATEWAY);
  addSysVar_enum_html(SystemVariables::DNS);
  addSysVar_enum_html(SystemVariables::RSSI);
  addSysVar_enum_html(SystemVariables::SSID);
  addSysVar_enum_html(SystemVariables::BSSID);
  addSysVar_enum_html(SystemVariables::WI_CH);

#ifdef HAS_ETHERNET
  addTableSeparator(F("Ethernet"), 3, 3);
  addSysVar_enum_html(SystemVariables::ETHWIFIMODE);
  addSysVar_enum_html(SystemVariables::ETHCONNECTED);
  addSysVar_enum_html(SystemVariables::ETHDUPLEX);
  addSysVar_enum_html(SystemVariables::ETHSPEED);
  addSysVar_enum_html(SystemVariables::ETHSTATE);
  addSysVar_enum_html(SystemVariables::ETHSPEEDSTATE);
 #endif

  addTableSeparator(F("System"), 3, 3);
  addSysVar_enum_html(SystemVariables::UNIT_sysvar);
  addSysVar_enum_html(SystemVariables::SYSLOAD);
  addSysVar_enum_html(SystemVariables::SYSHEAP);
  addSysVar_enum_html(SystemVariables::SYSSTACK);
  addSysVar_enum_html(SystemVariables::SYSNAME);
  addSysVar_enum_html(SystemVariables::BOOT_CAUSE);
#if FEATURE_ADC_VCC
  addSysVar_enum_html(SystemVariables::VCC);
#endif // if FEATURE_ADC_VCC

  addTableSeparator(F("System status"), 3, 3);

  addSysVar_enum_html(SystemVariables::ISWIFI);
  addSysVar_enum_html(SystemVariables::ISNTP);
  addSysVar_enum_html(SystemVariables::ISMQTT);
#ifdef USES_P037
  addSysVar_enum_html(SystemVariables::ISMQTTIMP);
#endif // USES_P037

  addTableSeparator(F("Time"), 3, 3);
  addSysVar_enum_html(SystemVariables::LCLTIME);
  addSysVar_enum_html(SystemVariables::LCLTIME_AM);
  addSysVar_enum_html(SystemVariables::SYSTM_HM);
  addSysVar_enum_html(SystemVariables::SYSTM_HM_AM);
  addSysVar_enum_html(SystemVariables::SYSTIME);
  addSysVar_enum_html(SystemVariables::SYSTIME_AM);
  addSysVar_enum_html(SystemVariables::SYSBUILD_DATE);
  addSysVar_enum_html(SystemVariables::SYSBUILD_TIME);
  addSysVar_enum_html(SystemVariables::SYSBUILD_FILENAME);
  addSysVar_enum_html(SystemVariables::SYSBUILD_DESCR);
  addSysVar_enum_html(SystemVariables::SYSBUILD_GIT);
  
  addTableSeparator(F("System"), 3, 3);
  addSysVar_html(F("%sysyear%  // %sysyear_0%"));
  addSysVar_html(F("%sysyears%"));
  addSysVar_html(F("%sysmonth% // %sysmonth_0%"));
  addSysVar_html(F("%sysday%   // %sysday_0%"));
  addSysVar_html(F("%syshour%  // %syshour_0%"));
  addSysVar_html(F("%sysmin%   // %sysmin_0%"));
  addSysVar_html(F("%syssec%   // %syssec_0%"));
  addSysVar_enum_html(SystemVariables::SYSSEC_D);
  addSysVar_enum_html(SystemVariables::SYSWEEKDAY);
  addSysVar_enum_html(SystemVariables::SYSWEEKDAY_S);
  addTableSeparator(F("System"), 3, 3);
  addSysVar_enum_html(SystemVariables::UPTIME);
  addSysVar_enum_html(SystemVariables::UPTIME_MS);
  addSysVar_enum_html(SystemVariables::UNIXTIME);
  addSysVar_enum_html(SystemVariables::UNIXDAY);
  addSysVar_enum_html(SystemVariables::UNIXDAY_SEC);
  addSysVar_html(F("%sunset%"));
  addSysVar_html(F("%sunset-1h%"));
  addSysVar_html(F("%sunrise%"));
  addSysVar_html(F("%sunrise+10m%"));
  addSysVar_html(F("%s_sunset%"));
  addSysVar_html(F("%s_sunrise%"));
  addSysVar_html(F("%m_sunset%"));
  addSysVar_html(F("%m_sunrise%"));

  addTableSeparator(F("Custom Variables"), 3, 3);

  bool customVariablesAdded = false;
  for (auto it = customFloatVar.begin(); it != customFloatVar.end(); ++it) {
    addSysVar_html("%v" + String(it->first) + '%');
    customVariablesAdded = true;
  }
  if (!customVariablesAdded) {
    html_TR_TD();
    addHtml(F("No variables set"));
    html_TD();
    html_TD();
  }
#ifndef BUILD_NO_SPECIAL_CHARACTERS_STRINGCONVERTER
  addTableSeparator(F("Special Characters"), 3, 2);
  addTableSeparator(F("Degree"),             3, 3);
  addSysVar_html(F("{D}"));
  addSysVar_html(F("&deg;"));

  addTableSeparator(F("Angle quotes"), 3, 3);
  addSysVar_html(F("{<<}"));
  addSysVar_html(F("&laquo;"));
  addFormSeparator(3);
  addSysVar_html(F("{>>}"));
  addSysVar_html(F("&raquo;"));
  addTableSeparator(F("Greek letter Mu"), 3, 3);
  addSysVar_html(F("{u}"));
  addSysVar_html(F("&micro;"));
  addTableSeparator(F("Currency"), 3, 3);
  addSysVar_html(F("{E}"));
  addSysVar_html(F("&euro;"));
  addFormSeparator(3);
  addSysVar_html(F("{Y}"));
  addSysVar_html(F("&yen;"));
  addFormSeparator(3);
  addSysVar_html(F("{P}"));
  addSysVar_html(F("&pound;"));
  addFormSeparator(3);
  addSysVar_html(F("{c}"));
  addSysVar_html(F("&cent;"));

  addTableSeparator(F("Math symbols"), 3, 3);
  addSysVar_html(F("{^1}"));
  addSysVar_html(F("&sup1;"));
  addFormSeparator(3);
  addSysVar_html(F("{^2}"));
  addSysVar_html(F("&sup2;"));
  addFormSeparator(3);
  addSysVar_html(F("{^3}"));
  addSysVar_html(F("&sup3;"));
  addFormSeparator(3);
  addSysVar_html(F("{1_4}"));
  addSysVar_html(F("&frac14;"));
  addFormSeparator(3);
  addSysVar_html(F("{1_2}"));
  addSysVar_html(F("&frac12;"));
  addFormSeparator(3);
  addSysVar_html(F("{3_4}"));
  addSysVar_html(F("&frac34;"));
  addFormSeparator(3);
  addSysVar_html(F("{+-}"));
  addSysVar_html(F("&plusmn;"));
  addFormSeparator(3);
  addSysVar_html(F("{x}"));
  addSysVar_html(F("&times;"));
  addFormSeparator(3);
  addSysVar_html(F("{..}"));
  addSysVar_html(F("&divide;"));
#endif
  addTableSeparator(F("Standard Conversions"), 3, 2);

  addSysVar_html(F("Wind Dir.:    %c_w_dir%(123.4)"));
  addSysVar_html(F("{D}C to {D}F: %c_c2f%(20.4)"));
  addSysVar_html(F("m/s to Bft:   %c_ms2Bft%(5.1)"));
  addSysVar_html(F("Dew point(T,H): %c_dew_th%(18.6,67)"));
  addSysVar_html(F("Altitude(air,sea): %c_alt_pres_sea%(850,1000)"));
  addSysVar_html(F("PressureElevation(air,alt): %c_sea_pres_alt%(850,1350.03)"));
  addFormSeparator(3);
  addSysVar_html(F("cm to imperial: %c_cm2imp%(190)"));
  addSysVar_html(F("mm to imperial: %c_mm2imp%(1900)"));
  addFormSeparator(3);
  addSysVar_html(F("Mins to days: %c_m2day%(1900)"));
  addSysVar_html(F("Mins to dh:   %c_m2dh%(1900)"));
  addSysVar_html(F("Mins to dhm:  %c_m2dhm%(1900)"));
  addSysVar_html(F("Secs to dhms: %c_s2dhms%(100000)"));
  addFormSeparator(3);
  addSysVar_html(F("To HEX: %c_2hex%(100000)"));
  addFormSeparator(3);
  addSysVar_html(F("Unit to IP: %c_u2ip%(%unit%, 2)"));

  html_end_table();
  html_end_form();
  sendHeadandTail_stdtemplate(true);
  TXBuffer.endStream();
}

void addSysVar_html_parsed(String input, bool URLencoded) {
  // Make deepcopy for replacement, so parameter is a copy, not a const reference
  parseSystemVariables(input, URLencoded); 
  parseStandardConversions(input, URLencoded);
  addHtml(input);
}

void addSysVar_html(const __FlashStringHelper * input) {
  addSysVar_html(String(input));
}

void addSysVar_html(const String& input) {
  html_TR_TD();
  {
    addHtml(F("<pre>")); // Make monospaced (<tt> tag?)
    addHtml(F("<xmp>")); // Make sure HTML code is escaped. Tag depricated??
    addHtml(input);
    addHtml(F("</xmp>"));
    addHtml(F("</pre>"));
  }
  html_TD();
  addSysVar_html_parsed(input, false); // Not URL encoded
  html_TD();
  addSysVar_html_parsed(input, true); // URL encoded
  delay(0);
}

#endif // WEBSERVER_SYSVARS
