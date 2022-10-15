// Copyright 2019-2021 - David Conran (@crankyoldgit)

/// @file IRtext.cpp
/// @warning If you add or remove an entry in this file, you should run:
///   '../tools/generate_irtext_h.sh' to rebuild the `IRtext.h` file.

#include "IRtext.h"
#ifndef UNIT_TEST
#include <Arduino.h>
#endif  // UNIT_TEST
#include "IRremoteESP8266.h"
#include "i18n.h"

#ifndef PROGMEM
#define PROGMEM  // Pretend we have the PROGMEM macro even if we really don't.
#endif

#ifndef FPSTR
#define FPSTR(X) X  // Also pretend we have flash-string helper class cast.
#endif

#define IRTEXT_CONST_BLOB_NAME(NAME)\
    NAME ## Blob

#define IRTEXT_CONST_BLOB_DECL(NAME)\
    const char IRTEXT_CONST_BLOB_NAME(NAME) [] PROGMEM

#define IRTEXT_CONST_BLOB_PTR(NAME)\
    IRTEXT_CONST_PTR(NAME) {\
        IRTEXT_CONST_PTR_CAST(IRTEXT_CONST_BLOB_NAME(NAME)) }

#define IRTEXT_CONST_STRING(NAME, VALUE)\
    static IRTEXT_CONST_BLOB_DECL(NAME) { VALUE };\
    IRTEXT_CONST_PTR(NAME) PROGMEM {\
        IRTEXT_CONST_PTR_CAST(&(IRTEXT_CONST_BLOB_NAME(NAME))[0]) }

// Common
IRTEXT_CONST_STRING(kUnknownStr, D_STR_UNKNOWN);  ///< "Unknown"
IRTEXT_CONST_STRING(kProtocolStr, D_STR_PROTOCOL);  ///< "Protocol"
IRTEXT_CONST_STRING(kPowerStr, D_STR_POWER);  ///< "Power"
IRTEXT_CONST_STRING(kOnStr, D_STR_ON);  ///< "On"
IRTEXT_CONST_STRING(kOffStr, D_STR_OFF);  ///< "Off"
IRTEXT_CONST_STRING(k1Str, D_STR_1);  ///< "1"
IRTEXT_CONST_STRING(k0Str, D_STR_0);  ///< "0"
IRTEXT_CONST_STRING(kModeStr, D_STR_MODE);  ///< "Mode"
IRTEXT_CONST_STRING(kToggleStr, D_STR_TOGGLE);  ///< "Toggle"
IRTEXT_CONST_STRING(kTurboStr, D_STR_TURBO);  ///< "Turbo"
IRTEXT_CONST_STRING(kSuperStr, D_STR_SUPER);  ///< "Super"
IRTEXT_CONST_STRING(kSleepStr, D_STR_SLEEP);  ///< "Sleep"
IRTEXT_CONST_STRING(kLightStr, D_STR_LIGHT);  ///< "Light"
IRTEXT_CONST_STRING(kPowerfulStr, D_STR_POWERFUL);  ///< "Powerful"
IRTEXT_CONST_STRING(kQuietStr, D_STR_QUIET);  ///< "Quiet"
IRTEXT_CONST_STRING(kEconoStr, D_STR_ECONO);  ///< "Econo"
IRTEXT_CONST_STRING(kSwingStr, D_STR_SWING);  ///< "Swing"
IRTEXT_CONST_STRING(kSwingHStr, D_STR_SWINGH);  ///< "SwingH"
IRTEXT_CONST_STRING(kSwingVStr, D_STR_SWINGV);  ///< "SwingV"
IRTEXT_CONST_STRING(kBeepStr, D_STR_BEEP);  ///< "Beep"
IRTEXT_CONST_STRING(kZoneFollowStr, D_STR_ZONEFOLLOW);  ///< "Zone Follow"
IRTEXT_CONST_STRING(kFixedStr, D_STR_FIXED);  ///< "Fixed"
IRTEXT_CONST_STRING(kMouldStr, D_STR_MOULD);  ///< "Mould"
IRTEXT_CONST_STRING(kCleanStr, D_STR_CLEAN);  ///< "Clean"
IRTEXT_CONST_STRING(kPurifyStr, D_STR_PURIFY);  ///< "Purify"
IRTEXT_CONST_STRING(kTimerStr, D_STR_TIMER);  ///< "Timer"
IRTEXT_CONST_STRING(kOnTimerStr, D_STR_ONTIMER);  ///< "On Timer"
IRTEXT_CONST_STRING(kOffTimerStr, D_STR_OFFTIMER);  ///< "Off Timer"
IRTEXT_CONST_STRING(kTimerModeStr, D_STR_TIMERMODE);  ///< "Timer Mode"
IRTEXT_CONST_STRING(kClockStr, D_STR_CLOCK);  ///< "Clock"
IRTEXT_CONST_STRING(kCommandStr, D_STR_COMMAND);  ///< "Command"
IRTEXT_CONST_STRING(kXFanStr, D_STR_XFAN);  ///< "XFan"
IRTEXT_CONST_STRING(kHealthStr, D_STR_HEALTH);  ///< "Health"
IRTEXT_CONST_STRING(kModelStr, D_STR_MODEL);  ///< "Model"
IRTEXT_CONST_STRING(kTempStr, D_STR_TEMP);  ///< "Temp"
IRTEXT_CONST_STRING(kIFeelStr, D_STR_IFEEL);  ///< "IFeel"
IRTEXT_CONST_STRING(kHumidStr, D_STR_HUMID);  ///< "Humid"
IRTEXT_CONST_STRING(kSaveStr, D_STR_SAVE);  ///< "Save"
IRTEXT_CONST_STRING(kEyeStr, D_STR_EYE);  ///< "Eye"
IRTEXT_CONST_STRING(kFollowStr, D_STR_FOLLOW);  ///< "Follow"
IRTEXT_CONST_STRING(kIonStr, D_STR_ION);  ///< "Ion"
IRTEXT_CONST_STRING(kFreshStr, D_STR_FRESH);  ///< "Fresh"
IRTEXT_CONST_STRING(kHoldStr, D_STR_HOLD);  ///< "Hold"
IRTEXT_CONST_STRING(kButtonStr, D_STR_BUTTON);  ///< "Button"
IRTEXT_CONST_STRING(k8CHeatStr, D_STR_8C_HEAT);  ///< "8C Heat"
IRTEXT_CONST_STRING(k10CHeatStr, D_STR_10C_HEAT);  ///< "10C Heat"
IRTEXT_CONST_STRING(kISeeStr, D_STR_ISEE);  ///< "ISee"
IRTEXT_CONST_STRING(kAbsenseDetectStr, D_STR_ABSENSEDETECT);
                                                   ///< "AbsenseDetect"
IRTEXT_CONST_STRING(kDirectIndirectModeStr, D_STR_DIRECTINDIRECTMODE);
                                                   ///< "Direct/Indirect mode"
IRTEXT_CONST_STRING(kDirectStr, D_STR_DIRECT);  ///< "Direct"
IRTEXT_CONST_STRING(kIndirectStr, D_STR_INDIRECT);  ///< "Indirect"

IRTEXT_CONST_STRING(kNightStr, D_STR_NIGHT);  ///< "Night"
IRTEXT_CONST_STRING(kSilentStr, D_STR_SILENT);  ///< "Silent"
IRTEXT_CONST_STRING(kFilterStr, D_STR_FILTER);  ///< "Filter"
IRTEXT_CONST_STRING(k3DStr, D_STR_3D);  ///< "3D"
IRTEXT_CONST_STRING(kCelsiusStr, D_STR_CELSIUS);  ///< "Celsius"
IRTEXT_CONST_STRING(kCelsiusFahrenheitStr, D_STR_CELSIUS_FAHRENHEIT);  ///<
///< "Celsius/Fahrenheit"
IRTEXT_CONST_STRING(kTempUpStr, D_STR_TEMPUP);  ///< "Temp Up"
IRTEXT_CONST_STRING(kTempDownStr, D_STR_TEMPDOWN);  ///< "Temp Down"
IRTEXT_CONST_STRING(kStartStr, D_STR_START);  ///< "Start"
IRTEXT_CONST_STRING(kStopStr, D_STR_STOP);  ///< "Stop"
IRTEXT_CONST_STRING(kMoveStr, D_STR_MOVE);  ///< "Move"
IRTEXT_CONST_STRING(kSetStr, D_STR_SET);  ///< "Set"
IRTEXT_CONST_STRING(kCancelStr, D_STR_CANCEL);  ///< "Cancel"
IRTEXT_CONST_STRING(kUpStr, D_STR_UP);  ///< "Up"
IRTEXT_CONST_STRING(kDownStr, D_STR_DOWN);  ///< "Down"
IRTEXT_CONST_STRING(kChangeStr, D_STR_CHANGE);  ///< "Change"
IRTEXT_CONST_STRING(kComfortStr, D_STR_COMFORT);  ///< "Comfort"
IRTEXT_CONST_STRING(kSensorStr, D_STR_SENSOR);  ///< "Sensor"
IRTEXT_CONST_STRING(kWeeklyTimerStr, D_STR_WEEKLYTIMER);  ///< "WeeklyTimer"
IRTEXT_CONST_STRING(kWifiStr, D_STR_WIFI);  ///< "Wifi"
IRTEXT_CONST_STRING(kLastStr, D_STR_LAST);  ///< "Last"
IRTEXT_CONST_STRING(kFastStr, D_STR_FAST);  ///< "Fast"
IRTEXT_CONST_STRING(kSlowStr, D_STR_SLOW);  ///< "Slow"
IRTEXT_CONST_STRING(kAirFlowStr, D_STR_AIRFLOW);  ///< "Air Flow"
IRTEXT_CONST_STRING(kStepStr, D_STR_STEP);  ///< "Step"
IRTEXT_CONST_STRING(kNAStr, D_STR_NA);  ///< "N/A"
IRTEXT_CONST_STRING(kInsideStr, D_STR_INSIDE);  ///< "Inside"
IRTEXT_CONST_STRING(kOutsideStr, D_STR_OUTSIDE);  ///< "Outside"
IRTEXT_CONST_STRING(kLoudStr, D_STR_LOUD);  ///< "Loud"
IRTEXT_CONST_STRING(kLowerStr, D_STR_LOWER);  ///< "Lower"
IRTEXT_CONST_STRING(kUpperStr, D_STR_UPPER);  ///< "Upper"
IRTEXT_CONST_STRING(kBreezeStr, D_STR_BREEZE);  ///< "Breeze"
IRTEXT_CONST_STRING(kCirculateStr, D_STR_CIRCULATE);  ///< "Circulate"
IRTEXT_CONST_STRING(kCeilingStr, D_STR_CEILING);  ///< "Ceiling"
IRTEXT_CONST_STRING(kWallStr, D_STR_WALL);  ///< "Wall"
IRTEXT_CONST_STRING(kRoomStr, D_STR_ROOM);  ///< "Room"
IRTEXT_CONST_STRING(k6thSenseStr, D_STR_6THSENSE);  ///< "6th Sense"
IRTEXT_CONST_STRING(kTypeStr, D_STR_TYPE);  ///< "Type"
IRTEXT_CONST_STRING(kSpecialStr, D_STR_SPECIAL);  ///< "Special"
IRTEXT_CONST_STRING(kIdStr, D_STR_ID);  ///< "Id" / Device Identifier
IRTEXT_CONST_STRING(kVaneStr, D_STR_VANE);  ///< "Vane"
IRTEXT_CONST_STRING(kLockStr, D_STR_LOCK);  ///< "Lock"

IRTEXT_CONST_STRING(kAutoStr, D_STR_AUTO);  ///< "Auto"
IRTEXT_CONST_STRING(kAutomaticStr, D_STR_AUTOMATIC);  ///< "Automatic"
IRTEXT_CONST_STRING(kManualStr, D_STR_MANUAL);  ///< "Manual"
IRTEXT_CONST_STRING(kCoolStr, D_STR_COOL);  ///< "Cool"
IRTEXT_CONST_STRING(kCoolingStr, D_STR_COOLING);  ///< "Cooling"
IRTEXT_CONST_STRING(kHeatStr, D_STR_HEAT);  ///< "Heat"
IRTEXT_CONST_STRING(kHeatingStr, D_STR_HEATING);  ///< "Heating"
IRTEXT_CONST_STRING(kDryStr, D_STR_DRY);  ///< "Dry"
IRTEXT_CONST_STRING(kDryingStr, D_STR_DRYING);  ///< "Drying"
IRTEXT_CONST_STRING(kDehumidifyStr, D_STR_DEHUMIDIFY);  ///< "Dehumidify"
IRTEXT_CONST_STRING(kFanStr, D_STR_FAN);  ///< "Fan"
// The following Fans strings with "only" are required to help with
// HomeAssistant & Google Home Climate integration. For compatibility only.
// Ref: https://www.home-assistant.io/integrations/google_assistant/#climate-operation-modes
IRTEXT_CONST_STRING(kFanOnlyStr, D_STR_FANONLY);  ///< "fan-only"
IRTEXT_CONST_STRING(kFan_OnlyStr, D_STR_FAN_ONLY);  ///< "fan_only" (HA/legacy)
IRTEXT_CONST_STRING(kFanOnlyWithSpaceStr, D_STR_FANSPACEONLY);  ///< "Fan Only"
IRTEXT_CONST_STRING(kFanOnlyNoSpaceStr, D_STR_FANONLYNOSPACE);  ///< "FanOnly"

IRTEXT_CONST_STRING(kRecycleStr, D_STR_RECYCLE);  ///< "Recycle"

IRTEXT_CONST_STRING(kMaxStr, D_STR_MAX);  ///< "Max"
IRTEXT_CONST_STRING(kMaximumStr, D_STR_MAXIMUM);  ///< "Maximum"
IRTEXT_CONST_STRING(kMinStr, D_STR_MIN);  ///< "Min"
IRTEXT_CONST_STRING(kMinimumStr, D_STR_MINIMUM);  ///< "Minimum"
IRTEXT_CONST_STRING(kMedStr, D_STR_MED);  ///< "Med"
IRTEXT_CONST_STRING(kMediumStr, D_STR_MEDIUM);  ///< "Medium"

IRTEXT_CONST_STRING(kHighestStr, D_STR_HIGHEST);  ///< "Highest"
IRTEXT_CONST_STRING(kHighStr, D_STR_HIGH);  ///< "High"
IRTEXT_CONST_STRING(kHiStr, D_STR_HI);  ///< "Hi"
IRTEXT_CONST_STRING(kMidStr, D_STR_MID);  ///< "Mid"
IRTEXT_CONST_STRING(kMiddleStr, D_STR_MIDDLE);  ///< "Middle"
IRTEXT_CONST_STRING(kLowStr, D_STR_LOW);  ///< "Low"
IRTEXT_CONST_STRING(kLoStr, D_STR_LO);  ///< "Lo"
IRTEXT_CONST_STRING(kLowestStr, D_STR_LOWEST);  ///< "Lowest"
IRTEXT_CONST_STRING(kMaxRightStr, D_STR_MAXRIGHT);  ///< "Max Right"
IRTEXT_CONST_STRING(kMaxRightNoSpaceStr, D_STR_MAXRIGHT_NOSPACE);  ///<
    ///< "MaxRight"
IRTEXT_CONST_STRING(kRightMaxStr, D_STR_RIGHTMAX);  ///< "Right Max"
IRTEXT_CONST_STRING(kRightMaxNoSpaceStr, D_STR_RIGHTMAX_NOSPACE);  ///<
    ///< "RightMax"
IRTEXT_CONST_STRING(kRightStr, D_STR_RIGHT);  ///< "Right"
IRTEXT_CONST_STRING(kLeftStr, D_STR_LEFT);  ///< "Left"
IRTEXT_CONST_STRING(kMaxLeftStr, D_STR_MAXLEFT);  ///< "Max Left"
IRTEXT_CONST_STRING(kMaxLeftNoSpaceStr, D_STR_MAXLEFT_NOSPACE);  ///< "MaxLeft"
IRTEXT_CONST_STRING(kLeftMaxStr, D_STR_LEFTMAX);  ///< "Left Max"
IRTEXT_CONST_STRING(kLeftMaxNoSpaceStr, D_STR_LEFTMAX_NOSPACE);  ///< "LeftMax"
IRTEXT_CONST_STRING(kWideStr, D_STR_WIDE);  ///< "Wide"
IRTEXT_CONST_STRING(kCentreStr, D_STR_CENTRE);  ///< "Centre"
IRTEXT_CONST_STRING(kTopStr, D_STR_TOP);  ///< "Top"
IRTEXT_CONST_STRING(kBottomStr, D_STR_BOTTOM);  ///< "Bottom"

// Compound words/phrases/descriptions from pre-defined words.
IRTEXT_CONST_STRING(kEconoToggleStr, D_STR_ECONOTOGGLE);  ///< "Econo Toggle"
IRTEXT_CONST_STRING(kEyeAutoStr, D_STR_EYEAUTO);  ///< "Eye Auto"
IRTEXT_CONST_STRING(kLightToggleStr, D_STR_LIGHTTOGGLE);  ///< "Light Toggle"
///< "Outside Quiet"
IRTEXT_CONST_STRING(kOutsideQuietStr, D_STR_OUTSIDEQUIET);
IRTEXT_CONST_STRING(kPowerToggleStr, D_STR_POWERTOGGLE);  ///< "Power Toggle"
IRTEXT_CONST_STRING(kPowerButtonStr, D_STR_POWERBUTTON);  ///< "Power Button"
IRTEXT_CONST_STRING(kPreviousPowerStr, D_STR_PREVIOUSPOWER);  ///<
///< "Previous Power"
IRTEXT_CONST_STRING(kDisplayTempStr, D_STR_DISPLAYTEMP);  ///< "Display Temp"
IRTEXT_CONST_STRING(kSensorTempStr, D_STR_SENSORTEMP);  ///< "Sensor Temp"
IRTEXT_CONST_STRING(kSleepTimerStr, D_STR_SLEEP_TIMER);  ///< "Sleep Timer"
IRTEXT_CONST_STRING(kSwingVModeStr, D_STR_SWINGVMODE);  ///< "Swing(V) Mode"
IRTEXT_CONST_STRING(kSwingVToggleStr, D_STR_SWINGVTOGGLE);  ///<
///< "Swing(V) Toggle"
IRTEXT_CONST_STRING(kTurboToggleStr, D_STR_TURBOTOGGLE);  ///< "Turbo Toggle"

// Separators & Punctuation
const char kTimeSep = D_CHR_TIME_SEP;  ///< ':'
IRTEXT_CONST_STRING(kSpaceLBraceStr, D_STR_SPACELBRACE);  ///< " ("
IRTEXT_CONST_STRING(kCommaSpaceStr, D_STR_COMMASPACE);  ///< ", "
IRTEXT_CONST_STRING(kColonSpaceStr, D_STR_COLONSPACE);  ///< ": "
IRTEXT_CONST_STRING(kDashStr, D_STR_DASH);  ///< "-"

// IRutils
//  - Time
IRTEXT_CONST_STRING(kDayStr, D_STR_DAY);  ///< "Day"
IRTEXT_CONST_STRING(kDaysStr, D_STR_DAYS);  ///< "Days"
IRTEXT_CONST_STRING(kHourStr, D_STR_HOUR);  ///< "Hour"
IRTEXT_CONST_STRING(kHoursStr, D_STR_HOURS);  ///< "Hours"
IRTEXT_CONST_STRING(kMinuteStr, D_STR_MINUTE);  ///< "Minute"
IRTEXT_CONST_STRING(kMinutesStr, D_STR_MINUTES);  ///< "Minutes"
IRTEXT_CONST_STRING(kSecondStr, D_STR_SECOND);  ///< "Second"
IRTEXT_CONST_STRING(kSecondsStr, D_STR_SECONDS);  ///< "Seconds"
IRTEXT_CONST_STRING(kNowStr, D_STR_NOW);  ///< "Now"
IRTEXT_CONST_STRING(kThreeLetterDayOfWeekStr, D_STR_THREELETTERDAYS);  ///<
///< "SunMonTueWedThuFriSat"
IRTEXT_CONST_STRING(kYesStr, D_STR_YES);  ///< "Yes"
IRTEXT_CONST_STRING(kNoStr, D_STR_NO);  ///< "No"
IRTEXT_CONST_STRING(kTrueStr, D_STR_TRUE);  ///< "True"
IRTEXT_CONST_STRING(kFalseStr, D_STR_FALSE);  ///< "False"

IRTEXT_CONST_STRING(kRepeatStr, D_STR_REPEAT);  ///< "Repeat"
IRTEXT_CONST_STRING(kCodeStr, D_STR_CODE);  ///< "Code"
IRTEXT_CONST_STRING(kBitsStr, D_STR_BITS);  ///< "Bits"

// Model Names
IRTEXT_CONST_STRING(kYaw1fStr, D_STR_YAW1F);  ///< "YAW1F"
IRTEXT_CONST_STRING(kYbofbStr, D_STR_YBOFB);  ///< "YBOFB"
IRTEXT_CONST_STRING(kYx1fsfStr, D_STR_YX1FSF);  ///< "YX1FSF"
IRTEXT_CONST_STRING(kV9014557AStr, D_STR_V9014557_A);  ///< "V9014557-A"
IRTEXT_CONST_STRING(kV9014557BStr, D_STR_V9014557_B);  ///< "V9014557-B"
IRTEXT_CONST_STRING(kRlt0541htaaStr, D_STR_RLT0541HTA_A);  ///< "R-LT0541-HTA-A"
IRTEXT_CONST_STRING(kRlt0541htabStr, D_STR_RLT0541HTA_B);  ///< "R-LT0541-HTA-B"
IRTEXT_CONST_STRING(kArrah2eStr, D_STR_ARRAH2E);  ///< "ARRAH2E"
IRTEXT_CONST_STRING(kArdb1Str, D_STR_ARDB1);  ///< "ARDB1"
IRTEXT_CONST_STRING(kArreb1eStr, D_STR_ARREB1E);  ///< "ARREB1E"
IRTEXT_CONST_STRING(kArjw2Str, D_STR_ARJW2);  ///< "ARJW2"
IRTEXT_CONST_STRING(kArry4Str, D_STR_ARRY4);  ///< "ARRY4"
IRTEXT_CONST_STRING(kArrew4eStr, D_STR_ARREW4E);  ///< "ARREW4E"
IRTEXT_CONST_STRING(kGe6711ar2853mStr, D_STR_GE6711AR2853M);  ///<
    ///< "GE6711AR2853M"
IRTEXT_CONST_STRING(kAkb75215403Str, D_STR_AKB75215403);  ///< "AKB75215403"
IRTEXT_CONST_STRING(kAkb74955603Str, D_STR_AKB74955603);  ///< "AKB74955603"
IRTEXT_CONST_STRING(kAkb73757604Str, D_STR_AKB73757604);  ///< "AKB73757604"
IRTEXT_CONST_STRING(kLg6711a20083vStr, D_STR_LG6711A20083V);  ///<
    ///< "LG6711A20083V"
IRTEXT_CONST_STRING(kKkg9ac1Str, D_STR_KKG9AC1);  ///< "KKG9AC1"
IRTEXT_CONST_STRING(kKkg29ac1Str, D_STR_KKG29AC1);  ///< "KKG29AC1"
IRTEXT_CONST_STRING(kLkeStr, D_STR_LKE);  ///< "LKE"
IRTEXT_CONST_STRING(kNkeStr, D_STR_NKE);  ///< "NKE"
IRTEXT_CONST_STRING(kDkeStr, D_STR_DKE);  ///< "DKE"
IRTEXT_CONST_STRING(kPkrStr, D_STR_PKR);  ///< "PKR"
IRTEXT_CONST_STRING(kJkeStr, D_STR_JKE);  ///< "JKE"
IRTEXT_CONST_STRING(kCkpStr, D_STR_CKP);  ///< "CKP"
IRTEXT_CONST_STRING(kRkrStr, D_STR_RKR);  ///< "RKR"
IRTEXT_CONST_STRING(kPanasonicLkeStr, D_STR_PANASONICLKE);  ///< "PANASONICLKE"
IRTEXT_CONST_STRING(kPanasonicNkeStr, D_STR_PANASONICNKE);  ///< "PANASONICNKE"
IRTEXT_CONST_STRING(kPanasonicDkeStr, D_STR_PANASONICDKE);  ///< "PANASONICDKE"
IRTEXT_CONST_STRING(kPanasonicPkrStr, D_STR_PANASONICPKR);  ///< "PANASONICPKR"
IRTEXT_CONST_STRING(kPanasonicJkeStr, D_STR_PANASONICJKE);  ///< "PANASONICJKE"
IRTEXT_CONST_STRING(kPanasonicCkpStr, D_STR_PANASONICCKP);  ///< "PANASONICCKP"
IRTEXT_CONST_STRING(kPanasonicRkrStr, D_STR_PANASONICRKR);  ///< "PANASONICRKR"
IRTEXT_CONST_STRING(kA907Str, D_STR_A907);  ///< "A907"
IRTEXT_CONST_STRING(kA705Str, D_STR_A705);  ///< "A705"
IRTEXT_CONST_STRING(kA903Str, D_STR_A903);  ///< "A903"
IRTEXT_CONST_STRING(kTac09chsdStr, D_STR_TAC09CHSD);  ///< "TAC09CHSD"
IRTEXT_CONST_STRING(kGz055be1Str, D_STR_GZ055BE1);  ///< "GZ055BE1"
IRTEXT_CONST_STRING(k122lzfStr, D_STR_122LZF);  ///< "122LZF"
IRTEXT_CONST_STRING(kDg11j13aStr, D_STR_DG11J13A);  ///< "DG11J13A"
IRTEXT_CONST_STRING(kDg11j104Str, D_STR_DG11J104);  ///< "DG11J104"
IRTEXT_CONST_STRING(kDg11j191Str, D_STR_DG11J191);  ///< "DG11J191"

#define D_STR_UNSUPPORTED "?"  // Unsupported protocols will be showing as
                               // a question mark, check for length > 1
                               // to show only currently included protocols
// Protocol Names
// Needs to be in decode_type_t order.
IRTEXT_CONST_BLOB_DECL(kAllProtocolNamesStr) {
    D_STR_UNUSED "\x0"
    #if DECODE_RC5 || SEND_RC5
    D_STR_RC5 "\x0"
    #else
    D_STR_UNSUPPORTED "\x0"
    #endif  // if DECODE_RC5 || SEND_RC5
    #if DECODE_RC6 || SEND_RC6
    D_STR_RC6 "\x0"
    #else
    D_STR_UNSUPPORTED "\x0"
    #endif  // if DECODE_RC6 || SEND_RC6
    #if DECODE_NEC || SEND_NEC
    D_STR_NEC "\x0"
    #else
    D_STR_UNSUPPORTED "\x0"
    #endif  // if DECODE_NEC || SEND_NEC
    #if DECODE_SONY || SEND_SONY
    D_STR_SONY "\x0"
    #else
    D_STR_UNSUPPORTED "\x0"
    #endif  // if DECODE_SONY || SEND_SONY
    #if DECODE_PANASONIC || SEND_PANASONIC
    D_STR_PANASONIC "\x0"
    #else
    D_STR_UNSUPPORTED "\x0"
    #endif  // if DECODE_PANASONIC || SEND_PANASONIC
    #if DECODE_JVC || SEND_JVC
    D_STR_JVC "\x0"
    #else
    D_STR_UNSUPPORTED "\x0"
    #endif  // if DECODE_JVC || SEND_JVC
    #if DECODE_SAMSUNG || SEND_SAMSUNG
    D_STR_SAMSUNG "\x0"
    #else
    D_STR_UNSUPPORTED "\x0"
    #endif  // if DECODE_SAMSUNG || SEND_SAMSUNG
    #if DECODE_WHYNTER || SEND_WHYNTER
    D_STR_WHYNTER "\x0"
    #else
    D_STR_UNSUPPORTED "\x0"
    #endif  // if DECODE_WHYNTER || SEND_WHYNTER
    #if DECODE_AIWA_RC_T501 || SEND_AIWA_RC_T501
    D_STR_AIWA_RC_T501 "\x0"
    #else
    D_STR_UNSUPPORTED "\x0"
    #endif  // if DECODE_AIWA_RC_T501 || SEND_AIWA_RC_T501
    #if DECODE_LG || SEND_LG
    D_STR_LG "\x0"
    #else
    D_STR_UNSUPPORTED "\x0"
    #endif  // if DECODE_LG || SEND_LG
    #if DECODE_SANYO || SEND_SANYO
    D_STR_SANYO "\x0"
    #else
    D_STR_UNSUPPORTED "\x0"
    #endif  // if DECODE_SANYO || SEND_SANYO
    #if DECODE_MITSUBISHI || SEND_MITSUBISHI
    D_STR_MITSUBISHI "\x0"
    #else
    D_STR_UNSUPPORTED "\x0"
    #endif  // if DECODE_MITSUBISHI || SEND_MITSUBISHI
    #if DECODE_DISH || SEND_DISH
    D_STR_DISH "\x0"
    #else
    D_STR_UNSUPPORTED "\x0"
    #endif  // if DECODE_DISH || SEND_DISH
    #if DECODE_SHARP || SEND_SHARP
    D_STR_SHARP "\x0"
    #else
    D_STR_UNSUPPORTED "\x0"
    #endif  // if DECODE_SHARP || SEND_SHARP
    #if DECODE_COOLIX || SEND_COOLIX
    D_STR_COOLIX "\x0"
    #else
    D_STR_UNSUPPORTED "\x0"
    #endif  // if DECODE_COOLIX || SEND_COOLIX
    #if DECODE_DAIKIN || SEND_DAIKIN
    D_STR_DAIKIN "\x0"
    #else
    D_STR_UNSUPPORTED "\x0"
    #endif  // if DECODE_DAIKIN || SEND_DAIKIN
    #if DECODE_DENON || SEND_DENON
    D_STR_DENON "\x0"
    #else
    D_STR_UNSUPPORTED "\x0"
    #endif  // if DECODE_DENON || SEND_DENON
    #if DECODE_KELVINATOR || SEND_KELVINATOR
    D_STR_KELVINATOR "\x0"
    #else
    D_STR_UNSUPPORTED "\x0"
    #endif  // if DECODE_KELVINATOR || SEND_KELVINATOR
    #if SEND_SHERWOOD  // SEND-ONLY
    D_STR_SHERWOOD "\x0"
    #else
    D_STR_UNSUPPORTED "\x0"
    #endif  // if SEND_SHERWOOD
    #if DECODE_MITSUBISHI_AC || SEND_MITSUBISHI_AC
    D_STR_MITSUBISHI_AC "\x0"
    #else
    D_STR_UNSUPPORTED "\x0"
    #endif  // if DECODE_MITSUBISHI_AC || SEND_MITSUBISHI_AC
    #if DECODE_RCMM || SEND_RCMM
    D_STR_RCMM "\x0"
    #else
    D_STR_UNSUPPORTED "\x0"
    #endif  // if DECODE_RCMM || SEND_RCMM
    #if DECODE_SANYO || SEND_SANYO
    D_STR_SANYO_LC7461 "\x0"
    #else
    D_STR_UNSUPPORTED "\x0"
    #endif  // if DECODE_SANYO || SEND_SANYO
    #if DECODE_RC5 || SEND_RC5
    D_STR_RC5X "\x0"
    #else
    D_STR_UNSUPPORTED "\x0"
    #endif  // if DECODE_RC5 || SEND_RC5
    #if DECODE_GREE || SEND_GREE
    D_STR_GREE "\x0"
    #else
    D_STR_UNSUPPORTED "\x0"
    #endif  // if DECODE_GREE || SEND_GREE
    #if SEND_PRONTO  // SEND-ONLY
    D_STR_PRONTO "\x0"
    #else
    D_STR_UNSUPPORTED "\x0"
    #endif  // if SEND_PRONTO
    #if DECODE_NEC || SEND_NEC
    D_STR_NEC_LIKE "\x0"
    #else
    D_STR_UNSUPPORTED "\x0"
    #endif  // if DECODE_NEC || SEND_NEC
    #if DECODE_ARGO || SEND_ARGO
    D_STR_ARGO "\x0"
    #else
    D_STR_UNSUPPORTED "\x0"
    #endif  // if DECODE_ARGO || SEND_ARGO
    #if DECODE_TROTEC || SEND_TROTEC
    D_STR_TROTEC "\x0"
    #else
    D_STR_UNSUPPORTED "\x0"
    #endif  // if DECODE_TROTEC || SEND_TROTEC
    #if DECODE_NIKAI || SEND_NIKAI
    D_STR_NIKAI "\x0"
    #else
    D_STR_UNSUPPORTED "\x0"
    #endif  // if DECODE_NIKAI || SEND_NIKAI
    #if SEND_RAW  // SEND-ONLY
    D_STR_RAW "\x0"
    #else
    D_STR_UNSUPPORTED "\x0"
    #endif  // if SEND_RAW
    #if SEND_GLOBALCACHE  // SEND-ONLY
    D_STR_GLOBALCACHE "\x0"
    #else
    D_STR_UNSUPPORTED "\x0"
    #endif  // if SEND_GLOBALCACHE
    #if DECODE_TOSHIBA_AC || SEND_TOSHIBA_AC
    D_STR_TOSHIBA_AC "\x0"
    #else
    D_STR_UNSUPPORTED "\x0"
    #endif  // if DECODE_TOSHIBA_AC || SEND_TOSHIBA_AC
    #if DECODE_FUJITSU_AC || SEND_FUJITSU_AC
    D_STR_FUJITSU_AC "\x0"
    #else
    D_STR_UNSUPPORTED "\x0"
    #endif  // if DECODE_FUJITSU_AC || SEND_FUJITSU_AC
    #if DECODE_MIDEA || SEND_MIDEA
    D_STR_MIDEA "\x0"
    #else
    D_STR_UNSUPPORTED "\x0"
    #endif  // if DECODE_MIDEA || SEND_MIDEA
    #if DECODE_MAGIQUEST || SEND_MAGIQUEST
    D_STR_MAGIQUEST "\x0"
    #else
    D_STR_UNSUPPORTED "\x0"
    #endif  // if DECODE_MAGIQUEST || SEND_MAGIQUEST
    #if DECODE_LASERTAG || SEND_LASERTAG
    D_STR_LASERTAG "\x0"
    #else
    D_STR_UNSUPPORTED "\x0"
    #endif  // if DECODE_LASERTAG || SEND_LASERTAG
    #if DECODE_CARRIER_AC || SEND_CARRIER_AC
    D_STR_CARRIER_AC "\x0"
    #else
    D_STR_UNSUPPORTED "\x0"
    #endif  // if DECODE_CARRIER_AC || SEND_CARRIER_AC
    #if DECODE_HAIER_AC || SEND_HAIER_AC
    D_STR_HAIER_AC "\x0"
    #else
    D_STR_UNSUPPORTED "\x0"
    #endif  // if DECODE_HAIER_AC || SEND_HAIER_AC
    #if DECODE_MITSUBISHI2 || SEND_MITSUBISHI2
    D_STR_MITSUBISHI2 "\x0"
    #else
    D_STR_UNSUPPORTED "\x0"
    #endif  // if DECODE_MITSUBISHI2 || SEND_MITSUBISHI2
    #if DECODE_HITACHI_AC || SEND_HITACHI_AC
    D_STR_HITACHI_AC "\x0"
    #else
    D_STR_UNSUPPORTED "\x0"
    #endif  // if DECODE_HITACHI_AC || SEND_HITACHI_AC
    #if DECODE_HITACHI_AC1 || SEND_HITACHI_AC1
    D_STR_HITACHI_AC1 "\x0"
    #else
    D_STR_UNSUPPORTED "\x0"
    #endif  // if DECODE_HITACHI_AC1 || SEND_HITACHI_AC1
    #if DECODE_HITACHI_AC2 || SEND_HITACHI_AC2
    D_STR_HITACHI_AC2 "\x0"
    #else
    D_STR_UNSUPPORTED "\x0"
    #endif  // if DECODE_HITACHI_AC2 || SEND_HITACHI_AC2
    #if DECODE_GICABLE || SEND_GICABLE
    D_STR_GICABLE "\x0"
    #else
    D_STR_UNSUPPORTED "\x0"
    #endif  // if DECODE_GICABLE || SEND_GICABLE
    #if DECODE_HAIER_AC_YRW02 || SEND_HAIER_AC_YRW02
    D_STR_HAIER_AC_YRW02 "\x0"
    #else
    D_STR_UNSUPPORTED "\x0"
    #endif  // if DECODE_HAIER_AC_YRW02 || SEND_HAIER_AC_YRW02
    #if DECODE_WHIRLPOOL_AC || SEND_WHIRLPOOL_AC
    D_STR_WHIRLPOOL_AC "\x0"
    #else
    D_STR_UNSUPPORTED "\x0"
    #endif  // if DECODE_WHIRLPOOL_AC || SEND_WHIRLPOOL_AC
    #if DECODE_SAMSUNG_AC || SEND_SAMSUNG_AC
    D_STR_SAMSUNG_AC "\x0"
    #else
    D_STR_UNSUPPORTED "\x0"
    #endif  // if DECODE_SAMSUNG_AC || SEND_SAMSUNG_AC
    #if DECODE_LUTRON || SEND_LUTRON
    D_STR_LUTRON "\x0"
    #else
    D_STR_UNSUPPORTED "\x0"
    #endif  // if DECODE_LUTRON || SEND_LUTRON
    #if DECODE_ELECTRA_AC || SEND_ELECTRA_AC
    D_STR_ELECTRA_AC "\x0"
    #else
    D_STR_UNSUPPORTED "\x0"
    #endif  // if DECODE_ELECTRA_AC || SEND_ELECTRA_AC
    #if DECODE_PANASONIC_AC || SEND_PANASONIC_AC
    D_STR_PANASONIC_AC "\x0"
    #else
    D_STR_UNSUPPORTED "\x0"
    #endif  // if DECODE_PANASONIC_AC || SEND_PANASONIC_AC
    #if DECODE_PIONEER || SEND_PIONEER
    D_STR_PIONEER "\x0"
    #else
    D_STR_UNSUPPORTED "\x0"
    #endif  // if DECODE_PIONEER || SEND_PIONEER
    #if DECODE_LG || SEND_LG
    D_STR_LG2 "\x0"
    #else
    D_STR_UNSUPPORTED "\x0"
    #endif  // if DECODE_LG || SEND_LG
    #if DECODE_MWM || SEND_MWM
    D_STR_MWM "\x0"
    #else
    D_STR_UNSUPPORTED "\x0"
    #endif  // if DECODE_MWM || SEND_MWM
    #if DECODE_DAIKIN2 || SEND_DAIKIN2
    D_STR_DAIKIN2 "\x0"
    #else
    D_STR_UNSUPPORTED "\x0"
    #endif  // if DECODE_DAIKIN2 || SEND_DAIKIN2
    #if DECODE_VESTEL_AC || SEND_VESTEL_AC
    D_STR_VESTEL_AC "\x0"
    #else
    D_STR_UNSUPPORTED "\x0"
    #endif  // if DECODE_VESTEL_AC || SEND_VESTEL_AC
    #if DECODE_TECO || SEND_TECO
    D_STR_TECO "\x0"
    #else
    D_STR_UNSUPPORTED "\x0"
    #endif  // if DECODE_TECO || SEND_TECO
    #if DECODE_SAMSUNG36 || SEND_SAMSUNG36
    D_STR_SAMSUNG36 "\x0"
    #else
    D_STR_UNSUPPORTED "\x0"
    #endif  // if DECODE_SAMSUNG36 || SEND_SAMSUNG36
    #if DECODE_TCL112AC || SEND_TCL112AC
    D_STR_TCL112AC "\x0"
    #else
    D_STR_UNSUPPORTED "\x0"
    #endif  // if DECODE_TCL112AC || SEND_TCL112AC
    #if DECODE_LEGOPF || SEND_LEGO
    D_STR_LEGOPF "\x0"
    #else
    D_STR_UNSUPPORTED "\x0"
    #endif  // if DECODE_LEGOPF || SEND_LEGO
    #if DECODE_MITSUBISHIHEAVY || SEND_MITSUBISHIHEAVY  // Exception
    D_STR_MITSUBISHI_HEAVY_88 "\x0"
    #else
    D_STR_UNSUPPORTED "\x0"
    #endif  // if DECODE_MITSUBISHIHEAVY || SEND_MITSUBISHIHEAVY
    #if DECODE_MITSUBISHIHEAVY || SEND_MITSUBISHIHEAVY  // Exception
    D_STR_MITSUBISHI_HEAVY_152 "\x0"
    #else
    D_STR_UNSUPPORTED "\x0"
    #endif  // if DECODE_MITSUBISHIHEAVY || SEND_MITSUBISHIHEAVY
    #if DECODE_DAIKIN216 || SEND_DAIKIN216
    D_STR_DAIKIN216 "\x0"
    #else
    D_STR_UNSUPPORTED "\x0"
    #endif  // if DECODE_DAIKIN216 || SEND_DAIKIN216
    #if DECODE_SHARP_AC || SEND_SHARP_AC
    D_STR_SHARP_AC "\x0"
    #else
    D_STR_UNSUPPORTED "\x0"
    #endif  // if DECODE_SHARP_AC || SEND_SHARP_AC
    #if DECODE_GOODWEATHER || SEND_GOODWEATHER
    D_STR_GOODWEATHER "\x0"
    #else
    D_STR_UNSUPPORTED "\x0"
    #endif  // if DECODE_GOODWEATHER || SEND_GOODWEATHER
    #if DECODE_INAX || SEND_INAX
    D_STR_INAX "\x0"
    #else
    D_STR_UNSUPPORTED "\x0"
    #endif  // if DECODE_INAX || SEND_INAX
    #if DECODE_DAIKIN160 || SEND_DAIKIN160
    D_STR_DAIKIN160 "\x0"
    #else
    D_STR_UNSUPPORTED "\x0"
    #endif  // if DECODE_DAIKIN160 || SEND_DAIKIN160
    #if DECODE_NEOCLIMA || SEND_NEOCLIMA
    D_STR_NEOCLIMA "\x0"
    #else
    D_STR_UNSUPPORTED "\x0"
    #endif  // if DECODE_NEOCLIMA || SEND_NEOCLIMA
    #if DECODE_DAIKIN176 || SEND_DAIKIN176
    D_STR_DAIKIN176 "\x0"
    #else
    D_STR_UNSUPPORTED "\x0"
    #endif  // if DECODE_DAIKIN176 || SEND_DAIKIN176
    #if DECODE_DAIKIN128 || SEND_DAIKIN128
    D_STR_DAIKIN128 "\x0"
    #else
    D_STR_UNSUPPORTED "\x0"
    #endif  // if DECODE_DAIKIN128 || SEND_DAIKIN128
    #if DECODE_AMCOR || SEND_AMCOR
    D_STR_AMCOR "\x0"
    #else
    D_STR_UNSUPPORTED "\x0"
    #endif  // if DECODE_AMCOR || SEND_AMCOR
    #if DECODE_DAIKIN152 || SEND_DAIKIN152
    D_STR_DAIKIN152 "\x0"
    #else
    D_STR_UNSUPPORTED "\x0"
    #endif  // if DECODE_DAIKIN152 || SEND_DAIKIN152
    #if DECODE_MITSUBISHI136 || SEND_MITSUBISHI136
    D_STR_MITSUBISHI136 "\x0"
    #else
    D_STR_UNSUPPORTED "\x0"
    #endif  // if DECODE_MITSUBISHI136 || SEND_MITSUBISHI136
    #if DECODE_MITSUBISHI112 || SEND_MITSUBISHI112
    D_STR_MITSUBISHI112 "\x0"
    #else
    D_STR_UNSUPPORTED "\x0"
    #endif  // if DECODE_MITSUBISHI112 || SEND_MITSUBISHI112
    #if DECODE_HITACHI_AC424 || SEND_HITACHI_AC424
    D_STR_HITACHI_AC424 "\x0"
    #else
    D_STR_UNSUPPORTED "\x0"
    #endif  // if DECODE_HITACHI_AC424 || SEND_HITACHI_AC424
    #if SEND_SONY  // Exception
    D_STR_SONY_38K "\x0"
    #else
    D_STR_UNSUPPORTED "\x0"
    #endif  // if SEND_SONY
    #if DECODE_EPSON || SEND_EPSON
    D_STR_EPSON "\x0"
    #else
    D_STR_UNSUPPORTED "\x0"
    #endif  // if DECODE_EPSON || SEND_EPSON
    #if DECODE_SYMPHONY || SEND_SYMPHONY
    D_STR_SYMPHONY "\x0"
    #else
    D_STR_UNSUPPORTED "\x0"
    #endif  // if DECODE_SYMPHONY || SEND_SYMPHONY
    #if DECODE_HITACHI_AC3 || SEND_HITACHI_AC3
    D_STR_HITACHI_AC3 "\x0"
    #else
    D_STR_UNSUPPORTED "\x0"
    #endif  // if DECODE_HITACHI_AC3 || SEND_HITACHI_AC3
    #if DECODE_DAIKIN64 || SEND_DAIKIN64
    D_STR_DAIKIN64 "\x0"
    #else
    D_STR_UNSUPPORTED "\x0"
    #endif  // if DECODE_DAIKIN64 || SEND_DAIKIN64
    #if DECODE_AIRWELL || SEND_AIRWELL
    D_STR_AIRWELL "\x0"
    #else
    D_STR_UNSUPPORTED "\x0"
    #endif  // if DECODE_AIRWELL || SEND_AIRWELL
    #if DECODE_DELONGHI_AC || SEND_DELONGHI_AC
    D_STR_DELONGHI_AC "\x0"
    #else
    D_STR_UNSUPPORTED "\x0"
    #endif  // if DECODE_DELONGHI_AC || SEND_DELONGHI_AC
    #if DECODE_DOSHISHA || SEND_DOSHISHA
    D_STR_DOSHISHA "\x0"
    #else
    D_STR_UNSUPPORTED "\x0"
    #endif  // if DECODE_DOSHISHA || SEND_DOSHISHA
    #if DECODE_MULTIBRACKETS || SEND_MULTIBRACKETS
    D_STR_MULTIBRACKETS "\x0"
    #else
    D_STR_UNSUPPORTED "\x0"
    #endif  // if DECODE_MULTIBRACKETS || SEND_MULTIBRACKETS
    #if DECODE_CARRIER_AC40 || SEND_CARRIER_AC40
    D_STR_CARRIER_AC40 "\x0"
    #else
    D_STR_UNSUPPORTED "\x0"
    #endif  // if DECODE_CARRIER_AC40 || SEND_CARRIER_AC40
    #if DECODE_CARRIER_AC64 || SEND_CARRIER_AC64
    D_STR_CARRIER_AC64 "\x0"
    #else
    D_STR_UNSUPPORTED "\x0"
    #endif  // if DECODE_CARRIER_AC64 || SEND_CARRIER_AC64
    #if DECODE_HITACHI_AC344 || SEND_HITACHI_AC344
    D_STR_HITACHI_AC344 "\x0"
    #else
    D_STR_UNSUPPORTED "\x0"
    #endif  // if DECODE_HITACHI_AC344 || SEND_HITACHI_AC344
    #if DECODE_CORONA_AC || SEND_CORONA_AC
    D_STR_CORONA_AC "\x0"
    #else
    D_STR_UNSUPPORTED "\x0"
    #endif  // if DECODE_CORONA_AC || SEND_CORONA_AC
    #if DECODE_MIDEA24 || SEND_MIDEA24
    D_STR_MIDEA24 "\x0"
    #else
    D_STR_UNSUPPORTED "\x0"
    #endif  // if DECODE_MIDEA24 || SEND_MIDEA24
    #if DECODE_ZEPEAL || SEND_ZEPEAL
    D_STR_ZEPEAL "\x0"
    #else
    D_STR_UNSUPPORTED "\x0"
    #endif  // if DECODE_ZEPEAL || SEND_ZEPEAL
    #if DECODE_SANYO_AC || SEND_SANYO_AC
    D_STR_SANYO_AC "\x0"
    #else
    D_STR_UNSUPPORTED "\x0"
    #endif  // if DECODE_SANYO_AC || SEND_SANYO_AC
    #if DECODE_VOLTAS || SEND_VOLTAS
    D_STR_VOLTAS "\x0"
    #else
    D_STR_UNSUPPORTED "\x0"
    #endif  // if DECODE_VOLTAS || SEND_VOLTAS
    #if DECODE_METZ || SEND_METZ
    D_STR_METZ "\x0"
    #else
    D_STR_UNSUPPORTED "\x0"
    #endif  // if DECODE_METZ || SEND_METZ
    #if DECODE_TRANSCOLD || SEND_TRANSCOLD
    D_STR_TRANSCOLD "\x0"
    #else
    D_STR_UNSUPPORTED "\x0"
    #endif  // if DECODE_TRANSCOLD || SEND_TRANSCOLD
    #if DECODE_TECHNIBEL_AC || SEND_TECHNIBEL_AC
    D_STR_TECHNIBEL_AC "\x0"
    #else
    D_STR_UNSUPPORTED "\x0"
    #endif  // if DECODE_TECHNIBEL_AC || SEND_TECHNIBEL_AC
    #if DECODE_MIRAGE || SEND_MIRAGE
    D_STR_MIRAGE "\x0"
    #else
    D_STR_UNSUPPORTED "\x0"
    #endif  // if DECODE_MIRAGE || SEND_MIRAGE
    #if DECODE_ELITESCREENS || SEND_ELITESCREENS
    D_STR_ELITESCREENS "\x0"
    #else
    D_STR_UNSUPPORTED "\x0"
    #endif  // if DECODE_ELITESCREENS || SEND_ELITESCREENS
    #if DECODE_PANASONIC_AC32 || SEND_PANASONIC_AC32
    D_STR_PANASONIC_AC32 "\x0"
    #else
    D_STR_UNSUPPORTED "\x0"
    #endif  // if DECODE_PANASONIC_AC32 || SEND_PANASONIC_AC32
    #if DECODE_MILESTAG2 || SEND_MILESTAG2
    D_STR_MILESTAG2 "\x0"
    #else
    D_STR_UNSUPPORTED "\x0"
    #endif  // if DECODE_MILESTAG2 || SEND_MILESTAG2
    #if DECODE_ECOCLIM || SEND_ECOCLIM
    D_STR_ECOCLIM "\x0"
    #else
    D_STR_UNSUPPORTED "\x0"
    #endif  // if DECODE_ECOCLIM || SEND_ECOCLIM
    #if DECODE_XMP || SEND_XMP
    D_STR_XMP "\x0"
    #else
    D_STR_UNSUPPORTED "\x0"
    #endif  // if DECODE_XMP || SEND_XMP
    #if DECODE_TRUMA || SEND_TRUMA
    D_STR_TRUMA "\x0"
    #else
    D_STR_UNSUPPORTED "\x0"
    #endif  // if DECODE_TRUMA || SEND_TRUMA
    #if DECODE_HAIER_AC176 || SEND_HAIER_AC176
    D_STR_HAIER_AC176 "\x0"
    #else
    D_STR_UNSUPPORTED "\x0"
    #endif  // if DECODE_HAIER_AC176 || SEND_HAIER_AC176
    #if DECODE_TEKNOPOINT || SEND_TEKNOPOINT
    D_STR_TEKNOPOINT "\x0"
    #else
    D_STR_UNSUPPORTED "\x0"
    #endif  // if DECODE_TEKNOPOINT || SEND_TEKNOPOINT
    #if DECODE_KELON || SEND_KELON
    D_STR_KELON "\x0"
    #else
    D_STR_UNSUPPORTED "\x0"
    #endif  // if DECODE_KELON || SEND_KELON
    #if DECODE_TROTEC_3550 || SEND_TROTEC_3550
    D_STR_TROTEC_3550 "\x0"
    #else
    D_STR_UNSUPPORTED "\x0"
    #endif  // if DECODE_TROTEC_3550 || SEND_TROTEC_3550
    #if DECODE_SANYO_AC88 || SEND_SANYO_AC88
    D_STR_SANYO_AC88 "\x0"
    #else
    D_STR_UNSUPPORTED "\x0"
    #endif  // if DECODE_SANYO_AC88 || SEND_SANYO_AC88
    #if DECODE_BOSE || SEND_BOSE
    D_STR_BOSE "\x0"
    #else
    D_STR_UNSUPPORTED "\x0"
    #endif  // if DECODE_BOSE || SEND_BOSE
    #if DECODE_ARRIS || SEND_ARRIS
    D_STR_ARRIS "\x0"
    #else
    D_STR_UNSUPPORTED "\x0"
    #endif  // if DECODE_ARRIS || SEND_ARRIS
    #if DECODE_RHOSS || SEND_RHOSS
    D_STR_RHOSS "\x0"
    #else
    D_STR_UNSUPPORTED "\x0"
    #endif  // if DECODE_RHOSS || SEND_RHOSS
    #if DECODE_AIRTON || SEND_AIRTON
    D_STR_AIRTON "\x0"
    #else
    D_STR_UNSUPPORTED "\x0"
    #endif  // if DECODE_AIRTON || SEND_AIRTON
    #if DECODE_COOLIX48 || SEND_COOLIX48
    D_STR_COOLIX48 "\x0"
    #else
    D_STR_UNSUPPORTED "\x0"
    #endif  // if DECODE_COOLIX48 || SEND_COOLIX48
    #if DECODE_HITACHI_AC264 || SEND_HITACHI_AC264
    D_STR_HITACHI_AC264 "\x0"
    #else
    D_STR_UNSUPPORTED "\x0"
    #endif  // if DECODE_HITACHI_AC264 || SEND_HITACHI_AC264
    #if DECODE_KELON168 || SEND_KELON168
    D_STR_KELON168 "\x0"
    #else
    D_STR_UNSUPPORTED "\x0"
    #endif  // if DECODE_KELON168 || SEND_KELON168
    #if DECODE_HITACHI_AC296 || SEND_HITACHI_AC296
    D_STR_HITACHI_AC296 "\x0"
    #else
    D_STR_UNSUPPORTED "\x0"
    #endif  // if DECODE_HITACHI_AC296 || SEND_HITACHI_AC296
    #if DECODE_DAIKIN200 || SEND_DAIKIN200
    D_STR_DAIKIN200 "\x0"
    #else
    D_STR_UNSUPPORTED "\x0"
    #endif  // if DECODE_DAIKIN200 || SEND_DAIKIN200
    #if DECODE_HAIER_AC160 || SEND_HAIER_AC160
    D_STR_HAIER_AC160 "\x0"
    #else
    D_STR_UNSUPPORTED "\x0"
    #endif  // if DECODE_HAIER_AC160 || SEND_HAIER_AC160
    #if DECODE_CARRIER_AC128 || SEND_CARRIER_AC128
    D_STR_CARRIER_AC128 "\x0"
    #else
    D_STR_UNSUPPORTED "\x0"
    #endif  // if DECODE_CARRIER_AC128 || SEND_CARRIER_AC128
    #if DECODE_TOTO || SEND_TOTO
    D_STR_TOTO "\x0"
    #else
    D_STR_UNSUPPORTED "\x0"
    #endif  // if DECODE_TOTO || SEND_TOTO
    #if DECODE_CLIMABUTLER || SEND_CLIMABUTLER
    D_STR_CLIMABUTLER "\x0"
    #else
    D_STR_UNSUPPORTED "\x0"
    #endif  // if DECODE_CLIMABUTLER || SEND_CLIMABUTLER
    #if DECODE_TCL96AC || SEND_TCL96AC
    D_STR_TCL96AC "\x0"
    #else
    D_STR_UNSUPPORTED "\x0"
    #endif  // if DECODE_TCL96AC || SEND_TCL96AC
    #if DECODE_BOSCH144 || SEND_BOSCH144
    D_STR_BOSCH144 "\x0"
    #else
    D_STR_UNSUPPORTED "\x0"
    #endif  // if DECODE_BOSCH144 || SEND_BOSCH144
    #if DECODE_SANYO_AC152 || SEND_SANYO_AC152
    D_STR_SANYO_AC152 "\x0"
    #else
    D_STR_UNSUPPORTED "\x0"
    #endif  // if DECODE_SANYO_AC152 || SEND_SANYO_AC152
    #if DECODE_DAIKIN312 || SEND_DAIKIN312
    D_STR_DAIKIN312 "\x0"
    #else
    D_STR_UNSUPPORTED "\x0"
    #endif  // if DECODE_DAIKIN312 || SEND_DAIKIN312
    ///< New protocol strings should be added just above this line.
    "\x0"  ///< This string requires double null termination.
};

IRTEXT_CONST_BLOB_PTR(kAllProtocolNamesStr);
