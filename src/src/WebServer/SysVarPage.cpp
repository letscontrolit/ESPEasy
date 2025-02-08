#include "../WebServer/SysVarPage.h"


#ifdef WEBSERVER_SYSVARS

# include "../WebServer/ESPEasy_WebServer.h"
# include "../WebServer/AccessControl.h"
# include "../WebServer/Markup.h"
# include "../WebServer/Markup_Forms.h"
# include "../WebServer/HTML_wrappers.h"

# include "../Globals/RuntimeData.h"

# include "../Helpers/StringConverter.h"
# include "../Helpers/SystemVariables.h"


// ********************************************************************************
// Web Interface sysvars showing all system vars and their value.
// ********************************************************************************
void addSysVar_enum_html(SystemVariables::Enum enumval) {
  addSysVar_html(SystemVariables::toString(enumval), false);
}

void addSysVar_enum_html(const SystemVariables::Enum enumval[], size_t nrElements) {
  for (size_t i = 0; i < nrElements; ++i) {
    addSysVar_enum_html(enumval[i]);
  }
}

void handle_sysvars() {
  # ifndef BUILD_NO_RAM_TRACKER
  checkRAM(F("handle_sysvars"));
  # endif // ifndef BUILD_NO_RAM_TRACKER

  if (!isLoggedIn()) { return; }
  TXBuffer.startStream();
  sendHeadandTail_stdtemplate(_HEAD);

  html_BR();
  addHtml(F("<p>This page may load slow.<BR>Do not load too often, since it may affect performance of the node.</p>"));
  html_BR();

  // the table header
  html_table_class_normal();
  html_TR();
  html_table_header(F("System Variables"));
  html_table_header(F("Normal"));
  html_table_header(F("URL encoded"), F("ESPEasy_System_Variables"), 0);

  addTableSeparator(F("Custom Variables"), 3, 3);

  if (customFloatVar.empty()) {
    html_TR_TD();
    addHtml(F("No variables set"));
    html_TD();
    html_TD();
  } else {
    for (auto it = customFloatVar.begin(); it != customFloatVar.end(); ++it) {
      addSysVar_html(strformat(F("%%v%u%%"), it->first), false);
    }
  }

  addTableSeparator(F("Constants"), 3, 3);
  {
    const SystemVariables::Enum vars[] = {
      SystemVariables::CR,
      SystemVariables::LF,
      SystemVariables::SPACE,
      SystemVariables::S_CR,
      SystemVariables::S_LF
    };
    addSysVar_enum_html(vars, NR_ELEMENTS(vars));
  }

  addTableSeparator(F("Network"), 3, 3);
  {
    const SystemVariables::Enum vars[] = {
      SystemVariables::MAC,
      SystemVariables::MAC_INT,
      SystemVariables::IP,
      SystemVariables::IP4,
      SystemVariables::SUBNET,
      SystemVariables::GATEWAY,
      SystemVariables::DNS,
      SystemVariables::DNS_1,
      SystemVariables::DNS_2,
#if FEATURE_USE_IPV6
      SystemVariables::IP6_LOCAL,
#endif
      SystemVariables::RSSI,
      SystemVariables::SSID,
      SystemVariables::BSSID,
      SystemVariables::WI_CH
    };
    addSysVar_enum_html(vars, NR_ELEMENTS(vars));
  }
  # if FEATURE_ETHERNET
  addTableSeparator(F("Ethernet"), 3, 3);
  {
    const SystemVariables::Enum vars[] = {
      SystemVariables::ETHWIFIMODE,
      SystemVariables::ETHCONNECTED,
      SystemVariables::ETHDUPLEX,
      SystemVariables::ETHSPEED,
      SystemVariables::ETHSTATE,
      SystemVariables::ETHSPEEDSTATE,
    };
    addSysVar_enum_html(vars, NR_ELEMENTS(vars));
  }
  # endif // if FEATURE_ETHERNET

  addTableSeparator(F("System"), 3, 3);
  {
    const SystemVariables::Enum vars[] = {
      SystemVariables::UNIT_sysvar,
  # if FEATURE_ZEROFILLED_UNITNUMBER
      SystemVariables::UNIT_0_sysvar,
  # endif // FEATURE_ZEROFILLED_UNITNUMBER
      SystemVariables::SYSLOAD,
      SystemVariables::SYSHEAP,
      SystemVariables::SYSSTACK,
      SystemVariables::SYSNAME,
# if FEATURE_ADC_VCC
      SystemVariables::VCC,
# endif   // if FEATURE_ADC_VCC
  # if FEATURE_INTERNAL_TEMPERATURE
      SystemVariables::INTERNAL_TEMPERATURE,
  # endif // if FEATURE_INTERNAL_TEMPERATURE
      SystemVariables::BOOT_CAUSE,
      SystemVariables::ISLIMITED_BUILD,
      SystemVariables::ISVAR_DOUBLE,
    };
    addSysVar_enum_html(vars, NR_ELEMENTS(vars));
  }

  addTableSeparator(F("Services Status"), 3, 3);
  {
    const SystemVariables::Enum vars[] = {
      SystemVariables::ISWIFI,
      SystemVariables::ISNTP,
# if FEATURE_MQTT
      SystemVariables::ISMQTT,
# endif // if FEATURE_MQTT
# ifdef USES_P037
      SystemVariables::ISMQTTIMP,
# endif // USES_P037
    };
    addSysVar_enum_html(vars, NR_ELEMENTS(vars));
  }

  addTableSeparator(F("Time"), 3, 3);
  {
    const SystemVariables::Enum vars[] = {
      SystemVariables::LCLTIME,
      SystemVariables::LCLTIME_AM,
      SystemVariables::SYSTM_HM,
      SystemVariables::SYSTM_HM_0,
      SystemVariables::SYSTM_HM_SP,
      SystemVariables::SYSTM_HM_AM,
      SystemVariables::SYSTM_HM_AM_0,
      SystemVariables::SYSTM_HM_AM_SP,
      SystemVariables::SYSTIME,
      SystemVariables::SYSTIME_AM,
      SystemVariables::SYSTIME_AM_0,
      SystemVariables::SYSTIME_AM_SP,
      SystemVariables::SYSBUILD_DATE,
      SystemVariables::SYSBUILD_TIME,
      SystemVariables::SYSBUILD_FILENAME,
      SystemVariables::SYSBUILD_DESCR,
      SystemVariables::SYSBUILD_GIT
    };
    addSysVar_enum_html(vars, NR_ELEMENTS(vars));
  }


  addTableSeparator(F("System Time"), 3, 3);
  {
    const SystemVariables::Enum vars[] = {
      SystemVariables::UPTIME,
      SystemVariables::UPTIME_MS,
      SystemVariables::UNIXTIME,
      SystemVariables::UNIXDAY,
      SystemVariables::UNIXDAY_SEC,
    };
    addSysVar_enum_html(vars, NR_ELEMENTS(vars));
  }
  {
    const __FlashStringHelper *vars[] = {
      F("%sysyear%  // %sysyear_0%"),
      F("%sysyears%"),
      F("%sysmonth% // %sysmonth_0%")
    };

    for (unsigned int i = 0; i < NR_ELEMENTS(vars); ++i) {
      addSysVar_html(vars[i]);
    }
  }
  addSysVar_enum_html(SystemVariables::SYSMONTH_S);
  {
    const __FlashStringHelper *vars[] = {
      F("%sysday%   // %sysday_0%"),
      F("%syshour%  // %syshour_0%"),
      F("%sysmin%   // %sysmin_0%"),
      F("%syssec%   // %syssec_0%")
    };

    for (unsigned int i = 0; i < NR_ELEMENTS(vars); ++i) {
      addSysVar_html(vars[i]);
    }
  }

  {
    const SystemVariables::Enum vars[] = {
      SystemVariables::SYSSEC_D,
      SystemVariables::SYSWEEKDAY,
      SystemVariables::SYSWEEKDAY_S,
      SystemVariables::SYSTZOFFSET,
    };
    addSysVar_enum_html(vars, NR_ELEMENTS(vars));
  }

  addTableSeparator(F("Sunrise/Sunset"), 3, 3);
  {
    const __FlashStringHelper *vars[] = {
      F("%sunset%"),
      F("%sunset-1h%"),
      F("%sunrise%"),
      F("%sunrise+10m%"),
      F("%s_sunset%"),
      F("%s_sunrise%"),
      F("%m_sunset%"),
      F("%m_sunrise%")
    };

    for (unsigned int i = 0; i < NR_ELEMENTS(vars); ++i) {
      addSysVar_html(vars[i]);
    }
  }

  addTableSeparator(F("ESP Board"), 3, 3);
  {
    const SystemVariables::Enum vars[] = {
      SystemVariables::ESP_CHIP_ID,
      SystemVariables::ESP_CHIP_FREQ,
      SystemVariables::ESP_CHIP_MODEL,
      SystemVariables::ESP_CHIP_REVISION,
      SystemVariables::ESP_CHIP_CORES,
      SystemVariables::BOARD_NAME,
    };
    addSysVar_enum_html(vars, NR_ELEMENTS(vars));
  }

  addTableSeparator(F("Storage"), 3, 3);
  {
    const SystemVariables::Enum vars[] = {
      SystemVariables::FLASH_FREQ,
      SystemVariables::FLASH_SIZE,
      SystemVariables::FLASH_CHIP_VENDOR,
      SystemVariables::FLASH_CHIP_MODEL,
      SystemVariables::FS_SIZE,
      SystemVariables::FS_FREE,
    };
    addSysVar_enum_html(vars, NR_ELEMENTS(vars));
  }

# ifndef BUILD_NO_SPECIAL_CHARACTERS_STRINGCONVERTER
  {
    addTableSeparator(F("Special Characters"), 3, 2);
    const __FlashStringHelper *MathSymbols[] = {
      // addTableSeparator(F("Degree"),             3, 3);
      F("{D}"),
      F("&deg;"),

      // addTableSeparator(F("Angle quotes"), 3, 3);
      F("{<<}"),
      F("&laquo;"),

      // addFormSeparator(3);
      F("{>>}"),
      F("&raquo;"),

      // addTableSeparator(F("Greek letter Mu"), 3, 3);
      F("{u}"),
      F("&micro;"),

      // addTableSeparator(F("Currency"), 3, 3);
      F("{E}"),
      F("&euro;"),

      // addFormSeparator(3);
      F("{Y}"),
      F("&yen;"),

      // addFormSeparator(3);
      F("{P}"),
      F("&pound;"),

      // addFormSeparator(3);
      F("{c}"),
      F("&cent;"),

      // addTableSeparator(F("Math symbols"), 3, 3);
      F("{^1}"),
      F("&sup1;"),

      // addFormSeparator(3);
      F("{^2}"),
      F("&sup2;"),

      // addFormSeparator(3);
      F("{^3}"),
      F("&sup3;"),

      // addFormSeparator(3);
      F("{1_4}"),
      F("&frac14;"),

      // addFormSeparator(3);
      F("{1_2}"),
      F("&frac12;"),

      // addFormSeparator(3);
      F("{3_4}"),
      F("&frac34;"),

      // addFormSeparator(3);
      F("{+-}"),
      F("&plusmn;"),

      // addFormSeparator(3);
      F("{x}"),
      F("&times;"),

      // addFormSeparator(3);
      F("{..}"),
      F("&divide;"),
    };

    constexpr unsigned int nrGroups = NR_ELEMENTS(MathSymbols) >> 1;

    for (unsigned int i = 0; i < nrGroups; ++i) {
      switch (i) {
        case 0: addTableSeparator(F("Degree"),         3, 3); break;
        case 1: addTableSeparator(F("Angle quotes"),   3, 3); break;
        case 3: addTableSeparator(F("Greek letter Mu"), 3, 3); break;
        case 4: addTableSeparator(F("Currency"),       3, 3); break;
        case 8: addTableSeparator(F("Math symbols"),   3, 3); break;
        default: addFormSeparator(3); break;
      }
      addSysVar_html_specialChar(MathSymbols[2 * i]);
      addSysVar_html_specialChar(MathSymbols[(2 * i) + 1]);
    }
  }
# endif // ifndef BUILD_NO_SPECIAL_CHARACTERS_STRINGCONVERTER
  {
    addTableSeparator(F("Standard Conversions"), 3, 2);

    const __FlashStringHelper *StdConversions[] = {
      F("Wind Dir.:    %c_w_dir%(123.4)"),
      F("{D}C to {D}F: %c_c2f%(20.4)"),
      F("m/s to Bft:   %c_ms2Bft%(5.1)"),
      F("Dew point(T,H): %c_dew_th%(18.6,67)"),
      F("Altitude(air,sea): %c_alt_pres_sea%(850,1000)"),
      F("PressureElevation(air,alt): %c_sea_pres_alt%(850,1350.03)"),

      // addFormSeparator(3,
      F("cm to imperial: %c_cm2imp%(190)"),
      F("mm to imperial: %c_mm2imp%(1900)"),

      // addFormSeparator(3,
      F("Mins to days: %c_m2day%(1900)"),
      F("Mins to dh:   %c_m2dh%(1900)"),
      F("Mins to dhm:  %c_m2dhm%(1900)"),
      F("Mins to hcm:  %c_m2hcm%(482)"),
      F("Secs to dhms: %c_s2dhms%(100000)"),

      // addFormSeparator(3,
      F("To HEX: %c_2hex%(100000)"),

      #if FEATURE_ESPEASY_P2P
      // addFormSeparator(3,
      F("Unit to IP: %c_u2ip%(%unit%, 2)"),
      F("Unit to Name: %c_uname%(%unit%)"),
      F("Unit to Age: %c_uage%(%unit%)"),
      F("Unit to Build: %c_ubuild%(%unit%)"),
      F("Unit to Build-string: %c_ubuildstr%(%unit%)"),
      F("Unit to Load: %c_uload%(%unit%)"),
      F("Unit to ESP-type: %c_utype%(%unit%)"),
      F("Unit to ESP-type-string: %c_utypestr%(%unit%)"),
      #endif // if FEATURE_ESPEASY_P2P
    };

    for (unsigned int i = 0; i < NR_ELEMENTS(StdConversions); ++i) {
      if ((i == 6) ||
          (i == 8) ||
          (i == 13)
          #if FEATURE_ESPEASY_P2P
          || (i == 14)
          #endif // if FEATURE_ESPEASY_P2P
          ) {
        addFormSeparator(3);
      }
      addSysVar_html(StdConversions[i]);
    }
  }
  html_end_table();
  html_end_form();
  sendHeadandTail_stdtemplate(_TAIL);
  TXBuffer.endStream();
}

void addSysVar_html(const __FlashStringHelper *input) {
  addSysVar_html(String(input), false);
}

void addSysVar_html_specialChar(const __FlashStringHelper *input) {
  addSysVar_html(String(input), true);
}

void addSysVar_html(const String& input) {
  addSysVar_html(input, false);
}

void addSysVar_html(String input, bool isSpecialChar) {
  // Make deepcopy for replacement, so parameter is a copy, not a const reference

  html_TR_TD();
  addHtml(F("<pre>")); // Make monospaced (<tt> tag?)
  addHtml(input);
  addHtml(F("</pre>"));

  if (isSpecialChar) {
    parseSpecialCharacters(input, false);
    html_TD();
    addHtml(input);
    html_TD();
    addHtml(URLEncode(input));
  } else {
    const String orig_input(input);

    for (int i = 0; i < 2; ++i) {
      const bool URLencoded = i == 1;

      if (URLencoded) {
        input = orig_input;
      }

      parseSystemVariables(input, URLencoded);
      parseStandardConversions(input, URLencoded);

      html_TD();
      addHtml(input);
    }
  }
  delay(0);
}

#endif // WEBSERVER_SYSVARS
