
#ifdef WEBSERVER_SYSVARS

// ********************************************************************************
// Web Interface sysvars showing all system vars and their value.
// ********************************************************************************
void handle_sysvars() {
  checkRAM(F("handle_sysvars"));

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
  addSysVar_html(F("%CR%"));
  addSysVar_html(F("%LF%"));
  addSysVar_html(F("%SP%"));
  addSysVar_html(F("%R%"));
  addSysVar_html(F("%N%"));

  addTableSeparator(F("Network"), 3, 3);
  addSysVar_html(F("%mac%"));
#if defined(ESP8266)
  addSysVar_html(F("%mac_int%"));
#endif // if defined(ESP8266)
  addSysVar_html(F("%ip4%"));
  addSysVar_html(F("%ip%"));
  addSysVar_html(F("%rssi%"));
  addSysVar_html(F("%ssid%"));
  addSysVar_html(F("%bssid%"));
  addSysVar_html(F("%wi_ch%"));

  addTableSeparator(F("System"), 3, 3);
  addSysVar_html(F("%unit%"));
  addSysVar_html(F("%sysload%"));
  addSysVar_html(F("%sysheap%"));
  addSysVar_html(F("%sysstack%"));
  addSysVar_html(F("%sysname%"));
#if FEATURE_ADC_VCC
  addSysVar_html(F("%vcc%"));
#endif // if FEATURE_ADC_VCC

  addTableSeparator(F("System status"), 3, 3);

  addSysVar_html(F("%iswifi%"));
  addSysVar_html(F("%isntp%"));
  addSysVar_html(F("%ismqtt%"));
#ifdef USES_P037
  addSysVar_html(F("%ismqttimp%"));
#endif // USES_P037

  addTableSeparator(F("Time"), 3, 3);
  addSysVar_html(F("%lcltime%"));
  addSysVar_html(F("%lcltime_am%"));
  addSysVar_html(F("%systm_hm%"));
  addSysVar_html(F("%systm_hm_am%"));
  addSysVar_html(F("%systime%"));
  addSysVar_html(F("%systime_am%"));
  addSysVar_html(F("%sysbuild_date%"));
  addSysVar_html(F("%sysbuild_time%"));
  addTableSeparator(F("System"), 3, 3);
  addSysVar_html(F("%sysyear%  // %sysyear_0%"));
  addSysVar_html(F("%sysyears%"));
  addSysVar_html(F("%sysmonth% // %sysmonth_0%"));
  addSysVar_html(F("%sysday%   // %sysday_0%"));
  addSysVar_html(F("%syshour%  // %syshour_0%"));
  addSysVar_html(F("%sysmin%   // %sysmin_0%"));
  addSysVar_html(F("%syssec%   // %syssec_0%"));
  addSysVar_html(F("%syssec_d%"));
  addSysVar_html(F("%sysweekday%"));
  addSysVar_html(F("%sysweekday_s%"));
  addTableSeparator(F("System"), 3, 3);
  addSysVar_html(F("%uptime%"));
  addSysVar_html(F("%unixtime%"));
  addSysVar_html(F("%unixday%"));
  addSysVar_html(F("%unixday_sec%"));
  addSysVar_html(F("%sunset%"));
  addSysVar_html(F("%sunset-1h%"));
  addSysVar_html(F("%sunrise%"));
  addSysVar_html(F("%sunrise+10m%"));

  addTableSeparator(F("Custom Variables"), 3, 3);

  for (byte i = 0; i < CUSTOM_VARS_MAX; ++i) {
    addSysVar_html("%v" + toString(i + 1, 0) + '%');
  }

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

  addTableSeparator(F("Standard Conversions"), 3, 2);

  addSysVar_html(F("Wind Dir.:    %c_w_dir%(123.4)"));
  addSysVar_html(F("{D}C to {D}F: %c_c2f%(20.4)"));
  addSysVar_html(F("m/s to Bft:   %c_ms2Bft%(5.1)"));
  addSysVar_html(F("Dew point(T,H): %c_dew_th%(18.6,67)"));
  addFormSeparator(3);
  addSysVar_html(F("cm to imperial: %c_cm2imp%(190)"));
  addSysVar_html(F("mm to imperial: %c_mm2imp%(1900)"));
  addFormSeparator(3);
  addSysVar_html(F("Mins to days: %c_m2day%(1900)"));
  addSysVar_html(F("Mins to dh:   %c_m2dh%(1900)"));
  addSysVar_html(F("Mins to dhm:  %c_m2dhm%(1900)"));
  addSysVar_html(F("Secs to dhms: %c_s2dhms%(100000)"));

  html_end_table();
  html_end_form();
  sendHeadandTail_stdtemplate(true);
  TXBuffer.endStream();
}

void addSysVar_html(const String& input) {
  html_TR_TD();
  {
    String html;
    html.reserve(24 + input.length());
    html += F("<pre>"); // Make monospaced (<tt> tag?)
    html += F("<xmp>"); // Make sure HTML code is escaped. Tag depricated??
    html += input;
    html += F("</xmp>");
    html += F("</pre>");
    addHtml(html);
  }
  html_TD();
  String replacement(input);                // Make deepcopy for replacement
  parseSystemVariables(replacement, false); // Not URL encoded
  parseStandardConversions(replacement, false);
  addHtml(replacement);
  html_TD();
  replacement = input;
  parseSystemVariables(replacement, true); // URL encoded
  parseStandardConversions(replacement, true);
  addHtml(replacement);
  delay(0);
}

#endif // WEBSERVER_SYSVARS
