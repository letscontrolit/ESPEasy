#ifndef _CUSTOM_IR_H
#define _CUSTOM_IR_H

/**
 * Custom_IR-sample.h
 * Usage:
 * - Copy this file to CustomIR.h (CustomIR.h is excluded from Git, so don't try to commit that)
 * - Change the supported list of IR devices by commenting/uncommenting options that DISABLE a device when UNcommented
 */


// Special plugins needing IR library
// #define USES_P016   // IR
// #define P016_SEND_IR_TO_CONTROLLER false //IF true then the JSON replay solution is transmited back to the condroller.
// #define P016_FEATURE_COMMAND_HANDLING 0 // By default set to 1 to have the command table, that can be dsabled here
// #define USES_P035   // IRTX
// #define P016_P035_Extended_AC // The following define is needed for extended decoding of A/C Messages and or using standardised 
                                 //common arguments for controlling all deeply supported A/C units
// #define P016_P035_USE_RAW_RAW2 //Use the RAW and RAW2 encodings, disabling it saves 3.7Kb
// #define USES_P088   // Heatpump IR

// *** This file must be updated adding new supported devices when updating IRreceive8266 library ***

// Set flags to enable (1) or disable (0) the DECODE_ and/or SEND_ feature for a specific IR device
// To limit ESPEasy build-size you can disable DECODE_ or SEND_ flags for devices not needed

// SEND-ONLY protocols:
// #define SEND_GLOBALCACHE 0 // Is used by many sending protocols, so should probably be left to default
// #define SEND_PRONTO 0
// #define SEND_RAW 0 // SEND_RAW support should be left to default unless explicitly disabled via P016_P035_USE_RAW_RAW2
// #define SEND_SHERWOOD 0
// UNUSED (exception) // #define SEND_SONY_38K 0 // is enabled/disabled via SEND_SONY

// Standard: Use defaults for up to library version 2.8.2
// Change as desired after copying CustomIR-sample.h to CustomIR.h

// #define DECODE_RC5 0
// #define SEND_RC5 0
// #define DECODE_RC6 0
// #define SEND_RC6 0
// #define DECODE_NEC 0
// #define SEND_NEC 0
// #define DECODE_SONY 0
// #define SEND_SONY 0
// #define DECODE_PANASONIC 0
// #define SEND_PANASONIC 0
// #define DECODE_JVC 0
// #define SEND_JVC 0
// #define DECODE_SAMSUNG 0
// #define SEND_SAMSUNG 0
// #define DECODE_WHYNTER 0
// #define SEND_WHYNTER 0
// #define DECODE_AIWA_RC_T501 0
// #define SEND_AIWA_RC_T501 0
// #define DECODE_LG 0
// #define SEND_LG 0
// #define DECODE_SANYO 0
// #define SEND_SANYO 0
// #define DECODE_MITSUBISHI 0
// #define SEND_MITSUBISHI 0
// #define DECODE_DISH 0
// #define SEND_DISH 0
// #define DECODE_SHARP 0
// #define SEND_SHARP 0
// #define DECODE_COOLIX 0
// #define SEND_COOLIX 0
// #define DECODE_DAIKIN 0
// #define SEND_DAIKIN 0
// #define DECODE_DENON 0
// #define SEND_DENON 0
// #define DECODE_KELVINATOR 0
// #define SEND_KELVINATOR 0
// #define DECODE_MITSUBISHI_AC 0
// #define SEND_MITSUBISHI_AC 0
// #define DECODE_RCMM 0
// #define SEND_RCMM 0
// #define DECODE_SANYO_LC7461 0
// #define SEND_SANYO_LC7461 0
// #define DECODE_RC5X 0
// #define SEND_RC5X 0
// #define DECODE_GREE 0
// #define SEND_GREE 0
// #define DECODE_NEC_LIKE 0
// #define SEND_NEC_LIKE 0
// #define DECODE_ARGO 0
// #define SEND_ARGO 0
// #define DECODE_TROTEC 0
// #define SEND_TROTEC 0
// #define DECODE_NIKAI 0
// #define SEND_NIKAI 0
// #define DECODE_TOSHIBA_AC 0
// #define SEND_TOSHIBA_AC 0
// #define DECODE_FUJITSU_AC 0
// #define SEND_FUJITSU_AC 0
// #define DECODE_MIDEA 0
// #define SEND_MIDEA 0
// #define DECODE_MAGIQUEST 0
// #define SEND_MAGIQUEST 0
// #define DECODE_LASERTAG 0
// #define SEND_LASERTAG 0
// #define DECODE_CARRIER_AC 0
// #define SEND_CARRIER_AC 0
// #define DECODE_HAIER_AC 0
// #define SEND_HAIER_AC 0
// #define DECODE_MITSUBISHI2 0
// #define SEND_MITSUBISHI2 0
// #define DECODE_HITACHI_AC 0
// #define SEND_HITACHI_AC 0
// #define DECODE_HITACHI_AC1 0
// #define SEND_HITACHI_AC1 0
// #define DECODE_HITACHI_AC2 0
// #define SEND_HITACHI_AC2 0
// #define DECODE_GICABLE 0
// #define SEND_GICABLE 0
// #define DECODE_HAIER_AC_YRW02 0
// #define SEND_HAIER_AC_YRW02 0
// #define DECODE_WHIRLPOOL_AC 0
// #define SEND_WHIRLPOOL_AC 0
// #define DECODE_SAMSUNG_AC 0
// #define SEND_SAMSUNG_AC 0
// #define DECODE_LUTRON 0
// #define SEND_LUTRON 0
// #define DECODE_ELECTRA_AC 0
// #define SEND_ELECTRA_AC 0
// #define DECODE_PANASONIC_AC 0
// #define SEND_PANASONIC_AC 0
// #define DECODE_PIONEER 0
// #define SEND_PIONEER 0
// #define DECODE_LG2 0
// #define SEND_LG2 0
// #define DECODE_MWM 0
// #define SEND_MWM 0
// #define DECODE_DAIKIN2 0
// #define SEND_DAIKIN2 0
// #define DECODE_VESTEL_AC 0
// #define SEND_VESTEL_AC 0
// #define DECODE_TECO 0
// #define SEND_TECO 0
// #define DECODE_SAMSUNG36 0
// #define SEND_SAMSUNG36 0
// #define DECODE_TCL112AC 0
// #define SEND_TCL112AC 0
// #define DECODE_LEGOPF 0
// #define SEND_LEGOPF 0
// #define DECODE_MITSUBISHI_HEAVY_88 0
// #define SEND_MITSUBISHI_HEAVY_88 0
// #define DECODE_MITSUBISHI_HEAVY_152 0
// #define SEND_MITSUBISHI_HEAVY_152 0
// #define DECODE_DAIKIN216 0
// #define SEND_DAIKIN216 0
// #define DECODE_SHARP_AC 0
// #define SEND_SHARP_AC 0
// #define DECODE_GOODWEATHER 0
// #define SEND_GOODWEATHER 0
// #define DECODE_INAX 0
// #define SEND_INAX 0
// #define DECODE_DAIKIN160 0
// #define SEND_DAIKIN160 0
// #define DECODE_NEOCLIMA 0
// #define SEND_NEOCLIMA 0
// #define DECODE_DAIKIN176 0
// #define SEND_DAIKIN176 0
// #define DECODE_DAIKIN128 0
// #define SEND_DAIKIN128 0
// #define DECODE_AMCOR 0
// #define SEND_AMCOR 0
// #define DECODE_DAIKIN152 0
// #define SEND_DAIKIN152 0
// #define DECODE_MITSUBISHI136 0
// #define SEND_MITSUBISHI136 0
// #define DECODE_MITSUBISHI112 0
// #define SEND_MITSUBISHI112 0
// #define DECODE_HITACHI_AC424 0
// #define SEND_HITACHI_AC424 0
// #define DECODE_EPSON 0
// #define SEND_EPSON 0
// #define DECODE_SYMPHONY 0
// #define SEND_SYMPHONY 0
// #define DECODE_HITACHI_AC3 0
// #define SEND_HITACHI_AC3 0
// #define DECODE_DAIKIN64 0
// #define SEND_DAIKIN64 0
// #define DECODE_AIRWELL 0
// #define SEND_AIRWELL 0
// #define DECODE_DELONGHI_AC 0
// #define SEND_DELONGHI_AC 0
// #define DECODE_DOSHISHA 0
// #define SEND_DOSHISHA 0
// #define DECODE_MULTIBRACKETS 0
// #define SEND_MULTIBRACKETS 0
// #define DECODE_CARRIER_AC40 0
// #define SEND_CARRIER_AC40 0
// #define DECODE_CARRIER_AC64 0
// #define SEND_CARRIER_AC64 0
// #define DECODE_HITACHI_AC344 0
// #define SEND_HITACHI_AC344 0
// #define DECODE_CORONA_AC 0
// #define SEND_CORONA_AC 0
// #define DECODE_MIDEA24 0
// #define SEND_MIDEA24 0
// #define DECODE_ZEPEAL 0
// #define SEND_ZEPEAL 0
// #define DECODE_SANYO_AC 0
// #define SEND_SANYO_AC 0
// #define DECODE_VOLTAS 0
// #define SEND_VOLTAS 0
// #define DECODE_METZ 0
// #define SEND_METZ 0
// #define DECODE_TRANSCOLD 0
// #define SEND_TRANSCOLD 0
// #define DECODE_TECHNIBEL_AC 0
// #define SEND_TECHNIBEL_AC 0
// #define DECODE_MIRAGE 0
// #define SEND_MIRAGE 0
// #define DECODE_ELITESCREENS 0
// #define SEND_ELITESCREENS 0
// #define DECODE_PANASONIC_AC32 0
// #define SEND_PANASONIC_AC32 0
// #define DECODE_MILESTAG2 0
// #define SEND_MILESTAG2 0
// #define DECODE_ECOCLIM 0
// #define SEND_ECOCLIM 0
// #define DECODE_XMP 0
// #define SEND_XMP 0
// #define DECODE_TRUMA 0
// #define SEND_TRUMA 0
// #define DECODE_HAIER_AC176 0
// #define SEND_HAIER_AC176 0
// #define DECODE_TEKNOPOINT 0
// #define SEND_TEKNOPOINT 0
// #define DECODE_KELON 0
// #define SEND_KELON 0
// #define DECODE_TROTEC_3550 0
// #define SEND_TROTEC_3550 0
// #define DECODE_SANYO_AC88 0
// #define SEND_SANYO_AC88 0
// #define DECODE_BOSE 0
// #define SEND_BOSE 0
// #define DECODE_ARRIS 0
// #define SEND_ARRIS 0
// #define DECODE_RHOSS 0
// #define SEND_RHOSS 0
// #define DECODE_AIRTON 0
// #define SEND_AIRTON 0
// #define DECODE_COOLIX48 0
// #define SEND_COOLIX48 0

// ATTENTION: DISABLE new devices from library version 2.8.2 and newer by default to save ESPEasy .bin size
// When extending this list: Add both the DECODE_ and SEND_ defines per new device!

// Version 2.8.2 added devices:
#define DECODE_HITACHI_AC264 0
#define SEND_HITACHI_AC264 0
#define DECODE_KELON168 0
#define SEND_KELON168 0
#define DECODE_HITACHI_AC296 0
#define SEND_HITACHI_AC296 0
#define DECODE_DAIKIN200 0
#define SEND_DAIKIN200 0
#define DECODE_HAIER_AC160 0
#define SEND_HAIER_AC160 0
#define DECODE_CARRIER_AC128 0
#define SEND_CARRIER_AC128 0
#define DECODE_TOTO 0
#define SEND_TOTO 0
#define DECODE_CLIMABUTLER 0
#define SEND_CLIMABUTLER 0
#define DECODE_TCL96AC 0
#define SEND_TCL96AC 0
#define DECODE_BOSCH144 0
#define SEND_BOSCH144 0
#define DECODE_SANYO_AC152 0
#define SEND_SANYO_AC152 0
#define DECODE_DAIKIN312 0
#define SEND_DAIKIN312 0

// Version 2.x.y added devices:

#endif // ifndef _CUSTOM_IR_H
