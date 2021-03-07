#include "StringConverter.h"


#include "../../_Plugin_Helper.h"

#include "../DataStructs/ESPEasy_EventStruct.h"

#include "../ESPEasyCore/ESPEasy_Log.h"

#include "../Globals/CRCValues.h"
#include "../Globals/Device.h"
#include "../Globals/ESPEasyWiFiEvent.h"
#include "../Globals/ESPEasy_time.h"
#include "../Globals/ExtraTaskSettings.h"
#include "../Globals/MQTT.h"
#include "../Globals/Plugins.h"
#include "../Globals/Settings.h"

#include "../Helpers/Convert.h"
#include "../Helpers/ESPEasy_Storage.h"
#include "../Helpers/Misc.h"
#include "../Helpers/Networking.h"
#include "../Helpers/Numerical.h"
#include "../Helpers/StringParser.h"
#include "../Helpers/SystemVariables.h"
#include "../Helpers/_Plugin_SensorTypeHelper.h"

// -V::569

/********************************************************************************************\
   Convert a char string to integer
 \*********************************************************************************************/

// FIXME: change original code so it uses String and String.toInt()
unsigned long str2int(const char *string)
{
  unsigned int temp = 0;

  validUIntFromString(string, temp);

  return static_cast<unsigned long>(temp);
}

String ull2String(uint64_t value, uint8_t base) {
  String res;

  if (value == 0) {
    res = '0';
    return res;
  }

  while (value > 0) {
    res   += String(static_cast<uint32_t>(value % base), base);
    value /= base;
  }

  int endpos   = res.length() - 1;
  int beginpos = 0;

  while (endpos > beginpos) {
    const char c = res[beginpos];
    res[beginpos] = res[endpos];
    res[endpos]   = c;
    ++beginpos;
    --endpos;
  }

  return res;
}

/********************************************************************************************\
   Check if valid float and convert string to float.
 \*********************************************************************************************/
bool string2float(const String& string, float& floatvalue) {
  return validFloatFromString(string, floatvalue);
}

/********************************************************************************************\
   Convert a char string to IP byte array
 \*********************************************************************************************/
boolean str2ip(const String& string, byte *IP) {
  return str2ip(string.c_str(), IP);
}

boolean str2ip(const char *string, byte *IP)
{
  IPAddress tmpip; // Default constructor => set to 0.0.0.0

  if ((*string == 0) || tmpip.fromString(string)) {
    // Eiher empty string or a valid IP addres, so copy value.
    for (byte i = 0; i < 4; ++i) {
      IP[i] = tmpip[i];
    }
    return true;
  }
  return false;
}

String formatIP(const IPAddress& ip) {
#if defined(ARDUINO_ESP8266_RELEASE_2_3_0)
  IPAddress tmp(ip);
  return tmp.toString();
#else // if defined(ARDUINO_ESP8266_RELEASE_2_3_0)
  return ip.toString();
#endif // if defined(ARDUINO_ESP8266_RELEASE_2_3_0)
}

void formatMAC(const uint8_t *mac, char (& strMAC)[20]) {
  sprintf_P(strMAC, PSTR("%02X:%02X:%02X:%02X:%02X:%02X"), mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
  ZERO_TERMINATE(strMAC);
}

String formatMAC(const uint8_t *mac) {
  char str[20] = { 0 };

  formatMAC(mac, str);
  return String(str);
}

/********************************************************************************************\
   Handling HEX strings
 \*********************************************************************************************/

// Convert max. 8 hex decimals to unsigned long
unsigned long hexToUL(const String& input_c, size_t nrHexDecimals) {
  size_t nr_decimals = nrHexDecimals;

  if (nr_decimals > 8) {
    nr_decimals = 8;
  }
  size_t inputLength = input_c.length();

  if (nr_decimals > inputLength) {
    nr_decimals = inputLength;
  }
  String tmp = input_c.substring(0, nr_decimals);

  return strtoul(tmp.c_str(), 0, 16);
}

unsigned long hexToUL(const String& input_c) {
  return hexToUL(input_c, input_c.length());
}

unsigned long hexToUL(const String& input_c, size_t startpos, size_t nrHexDecimals) {
  return hexToUL(input_c.substring(startpos, startpos + nrHexDecimals), nrHexDecimals);
}

String formatToHex(unsigned long value, const String& prefix) {
  String result = prefix;
  String hex(value, HEX);

  hex.toUpperCase();
  result += hex;
  return result;
}

String formatToHex(unsigned long value) {
  return formatToHex(value, F("0x"));
}

String formatHumanReadable(unsigned long value, unsigned long factor) {
  String result = formatHumanReadable(value, factor, 2);

  result.replace(F(".00"), "");
  return result;
}

String formatHumanReadable(unsigned long value, unsigned long factor, int NrDecimals) {
  float floatValue(value);
  byte  steps = 0;

  while (value >= factor) {
    value /= factor;
    ++steps;
    floatValue /= float(factor);
  }
  String result = toString(floatValue, NrDecimals);

  switch (steps) {
    case 0: return String(value);
    case 1: result += 'k'; break;
    case 2: result += 'M'; break;
    case 3: result += 'G'; break;
    case 4: result += 'T'; break;
    default:
      result += '*';
      result += factor;
      result += '^';
      result += steps;
      break;
  }
  return result;
}

String formatToHex_decimal(unsigned long value) {
  return formatToHex_decimal(value, 1);
}

String formatToHex_decimal(unsigned long value, unsigned long factor) {
  String result = formatToHex(value);

  result += " (";

  if (factor > 1) {
    result += formatHumanReadable(value, factor);
  } else {
    result += value;
  }
  result += ')';
  return result;
}

String boolToString(bool value) {
  return value ? F("true") : F("false");
}

/*********************************************************************************************\
   Typical string replace functions.
\*********************************************************************************************/
void removeExtraNewLine(String& line) {
  while (line.endsWith("\r\n\r\n")) {
    line.remove(line.length() - 2);
  }
}

void addNewLine(String& line) {
  line += "\r\n";
}

/*********************************************************************************************\
   Format a value to the set number of decimals
\*********************************************************************************************/
String doFormatUserVar(struct EventStruct *event, byte rel_index, bool mustCheck, bool& isvalid) {
  isvalid = true;

  const deviceIndex_t DeviceIndex = getDeviceIndex_from_TaskIndex(event->TaskIndex);

  if (!validDeviceIndex(DeviceIndex)) {
    isvalid = false;
    return "0";
  }

  {
    // First try to format using the plugin specific formatting.
    String result;
    EventStruct tempEvent(*event);
    tempEvent.idx = rel_index;
    PluginCall(PLUGIN_FORMAT_USERVAR, &tempEvent, result);
    if (result.length() > 0) {
      return result;
    }
  }


  const byte   valueCount = getValueCountForTask(event->TaskIndex);
  Sensor_VType sensorType = event->getSensorType();

  if (valueCount <= rel_index) {
    isvalid = false;

    #ifndef BUILD_NO_DEBUG

    if (loglevelActiveFor(LOG_LEVEL_ERROR)) {
      String log = F("No sensor value for TaskIndex: ");
      log += event->TaskIndex + 1;
      log += F(" varnumber: ");
      log += rel_index + 1;
      log += F(" type: ");
      log += getSensorTypeLabel(sensorType);
      addLog(LOG_LEVEL_ERROR, log);
    }
    #endif // ifndef BUILD_NO_DEBUG
    return "";
  }

  switch (sensorType) {
    case Sensor_VType::SENSOR_TYPE_LONG:
      return String(UserVar.getSensorTypeLong(event->TaskIndex));
    case Sensor_VType::SENSOR_TYPE_STRING:
      return event->String2;

    default:
      break;
  }

  float f(UserVar[event->BaseVarIndex + rel_index]);

  if (mustCheck && !isValidFloat(f)) {
    isvalid = false;
#ifndef BUILD_NO_DEBUG

    if (loglevelActiveFor(LOG_LEVEL_DEBUG)) {
      String log = F("Invalid float value for TaskIndex: ");
      log += event->TaskIndex;
      log += F(" varnumber: ");
      log += rel_index;
      addLog(LOG_LEVEL_DEBUG, log);
    }
#endif // ifndef BUILD_NO_DEBUG
    f = 0;
  }
  LoadTaskSettings(event->TaskIndex);

  byte nrDecimals = ExtraTaskSettings.TaskDeviceValueDecimals[rel_index];

  if (!Device[DeviceIndex].configurableDecimals()) {
    nrDecimals = 0;
  }

  String result = toString(f, nrDecimals);
  result.trim();
  return result;
}

String formatUserVarNoCheck(taskIndex_t TaskIndex, byte rel_index) {
  bool isvalid;

  // FIXME TD-er: calls to this function cannot handle Sensor_VType::SENSOR_TYPE_STRING
  struct EventStruct TempEvent(TaskIndex);

  return doFormatUserVar(&TempEvent, rel_index, false, isvalid);
}

String formatUserVar(taskIndex_t TaskIndex, byte rel_index, bool& isvalid) {
  // FIXME TD-er: calls to this function cannot handle Sensor_VType::SENSOR_TYPE_STRING
  struct EventStruct TempEvent(TaskIndex);

  return doFormatUserVar(&TempEvent, rel_index, true, isvalid);
}

String formatUserVarNoCheck(struct EventStruct *event, byte rel_index)
{
  bool isvalid;

  return doFormatUserVar(event, rel_index, false, isvalid);
}

String formatUserVar(struct EventStruct *event, byte rel_index, bool& isvalid)
{
  return doFormatUserVar(event, rel_index, true, isvalid);
}

String get_formatted_Controller_number(cpluginID_t cpluginID) {
  if (!validCPluginID(cpluginID)) {
    return F("C---");
  }
  String result = F("C");

  if (cpluginID < 100) { result += '0'; }

  if (cpluginID < 10) { result += '0'; }
  result += cpluginID;
  return result;
}

/*********************************************************************************************\
   Wrap a string with given pre- and postfix string.
\*********************************************************************************************/
String wrap_String(const String& string, char wrap) {
  String result;
  result.reserve(string.length() + 2);
  result += wrap;
  result += string;
  result += wrap;
  return result;
}


void wrap_String(const String& string, const String& wrap, String& result) {
  result += wrap;
  result += string;
  result += wrap;
}

String wrapIfContains(const String& value, char contains, char wrap) {
  if (value.indexOf(contains) != -1) {
    String result(wrap);
    result += value;
    result += wrap;
    return result;
  }
  return value;
}

/*********************************************************************************************\
   Format an object value pair for use in JSON.
\*********************************************************************************************/
String to_json_object_value(const String& object, const String& value) {
  String result;

  result.reserve(object.length() + value.length() + 6);
  wrap_String(object, F("\""), result);
  result += F(":");

  if (value.length() == 0) {
    // Empty string
    result += F("\"\"");
    return result;
  }
  if (mustConsiderAsString(value)) {
    // Is not a numerical value, or BIN/HEX notation, thus wrap with quotes
    if ((value.indexOf('\n') != -1) || (value.indexOf('\r') != -1) || (value.indexOf('"') != -1)) {
      // Must replace characters, so make a deepcopy
      String tmpValue(value);
      tmpValue.replace('\n', '^');
      tmpValue.replace('\r', '^');
      tmpValue.replace('"',  '\'');
      wrap_String(tmpValue, F("\""), result);
    } else {
      wrap_String(value, F("\""), result);
    }
  } else {
    // It is a numerical
    result += value;
  }
  return result;
}

/*********************************************************************************************\
   Strip wrapping chars (e.g. quotes)
\*********************************************************************************************/
String stripWrappingChar(const String& text, char wrappingChar) {
  unsigned int length = text.length();

  if ((length >= 2) && stringWrappedWithChar(text, wrappingChar)) {
    return text.substring(1, length - 1);
  }
  return text;
}

bool stringWrappedWithChar(const String& text, char wrappingChar) {
  unsigned int length = text.length();

  if (length < 2) { return false; }

  if (text.charAt(0) != wrappingChar) { return false; }
  return text.charAt(length - 1) == wrappingChar;
}

bool isQuoteChar(char c) {
  return c == '\'' || c == '"' || c == '`';
}

bool isParameterSeparatorChar(char c) {
  return c == ',' || c == ' ';
}

String stripQuotes(const String& text) {
  if (text.length() >= 2) {
    char c = text.charAt(0);

    if (isQuoteChar(c)) {
      return stripWrappingChar(text, c);
    }
  }
  return text;
}

bool safe_strncpy(char *dest, const String& source, size_t max_size) {
  return safe_strncpy(dest, source.c_str(), max_size);
}

bool safe_strncpy(char *dest, const char *source, size_t max_size) {
  if (max_size < 1) { return false; }

  if (dest == NULL) { return false; }

  if (source == NULL) { return false; }
  bool result = true;

  memset(dest, 0, max_size);
  size_t str_length = strlen_P(source);

  if (str_length >= max_size) {
    str_length = max_size;
    result     = false;
  }
  strncpy_P(dest, source, str_length);
  dest[max_size - 1] = 0;
  return result;
}

// Convert a string to lower case and replace spaces with underscores.
String to_internal_string(const String& input, char replaceSpace) {
  String result = input;

  result.trim();
  result.toLowerCase();
  result.replace(' ', replaceSpace);
  return result;
}

/*********************************************************************************************\
   Parse a string and get the xth command or parameter
   IndexFind = 1 => command.
    // FIXME TD-er: parseString* should use index starting at 0.
\*********************************************************************************************/
String parseString(const String& string, byte indexFind, char separator) {
  String result = parseStringKeepCase(string, indexFind, separator);

  result.toLowerCase();
  return result;
}

String parseStringKeepCase(const String& string, byte indexFind, char separator) {
  String result;

  if (!GetArgv(string.c_str(), result, indexFind, separator)) {
    return "";
  }
  result.trim();
  return stripQuotes(result);
}

String parseStringToEnd(const String& string, byte indexFind, char separator) {
  String result = parseStringToEndKeepCase(string, indexFind, separator);

  result.toLowerCase();
  return result;
}

String parseStringToEndKeepCase(const String& string, byte indexFind, char separator) {
  // Loop over the arguments to find the first and last pos of the arguments.
  int  pos_begin = string.length();
  int  pos_end = pos_begin;
  int  tmppos_begin, tmppos_end = -1;
  byte nextArgument = indexFind;
  bool hasArgument  = false;

  while (GetArgvBeginEnd(string.c_str(), nextArgument, tmppos_begin, tmppos_end, separator))
  {
    hasArgument = true;

    if ((tmppos_begin < pos_begin) && (tmppos_begin >= 0)) {
      pos_begin = tmppos_begin;
    }

    if ((tmppos_end >= 0)) {
      pos_end = tmppos_end;
    }
    ++nextArgument;
  }

  if (!hasArgument || (pos_begin < 0)) {
    return "";
  }
  String result = string.substring(pos_begin, pos_end);

  result.trim();
  return stripQuotes(result);
}

String tolerantParseStringKeepCase(const String& string, byte indexFind, char separator)
{
  if (Settings.TolerantLastArgParse()) {
    return parseStringToEndKeepCase(string, indexFind, separator);
  }
  return parseStringKeepCase(string, indexFind, separator);
}

// escapes special characters in strings for use in html-forms
bool htmlEscapeChar(char c, String& escaped)
{
  switch (c)
  {
    case '&':  escaped = F("&amp;");  return true;
    case '\"': escaped = F("&quot;"); return true;
    case '\'': escaped = F("&#039;"); return true;
    case '<':  escaped = F("&lt;");   return true;
    case '>':  escaped = F("&gt;");   return true;
    case '/':  escaped = F("&#047;"); return true;
  }
  return false;
}

void htmlEscape(String& html, char c)
{
  String repl;

  if (htmlEscapeChar(c, repl)) {
    html.replace(String(c), repl);
  }
}

void htmlEscape(String& html)
{
  htmlEscape(html, '&');
  htmlEscape(html, '\"');
  htmlEscape(html, '\'');
  htmlEscape(html, '<');
  htmlEscape(html, '>');
  htmlEscape(html, '/');
}

void htmlStrongEscape(String& html)
{
  String escaped;

  escaped.reserve(html.length());

  for (unsigned i = 0; i < html.length(); ++i)
  {
    if (((html[i] >= 'a') && (html[i] <= 'z')) || ((html[i] >= 'A') && (html[i] <= 'Z')) || ((html[i] >= '0') && (html[i] <= '9')))
    {
      escaped += html[i];
    }
    else
    {
      char s[4];
      sprintf_P(s, PSTR("%03d"), static_cast<int>(html[i]));
      escaped += "&#";
      escaped += s;
      escaped += ";";
    }
  }
  html = escaped;
}

// ********************************************************************************
// URNEncode char string to string object
// ********************************************************************************
String URLEncode(const char *msg)
{
  const char *hex = "0123456789abcdef";
  String encodedMsg;

  encodedMsg.reserve(strlen(msg));

  while (*msg != '\0') {
    if ((('a' <= *msg) && (*msg <= 'z'))
        || (('A' <= *msg) && (*msg <= 'Z'))
        || (('0' <= *msg) && (*msg <= '9'))
        || ('-' == *msg) || ('_' == *msg)
        || ('.' == *msg) || ('~' == *msg)) {
      encodedMsg += *msg;
    } else {
      encodedMsg += '%';
      encodedMsg += hex[*msg >> 4];
      encodedMsg += hex[*msg & 15];
    }
    msg++;
  }
  return encodedMsg;
}

void repl(const String& key, const String& val, String& s, boolean useURLencode)
{
  if (useURLencode) {
    // URLEncode does take resources, so check first if needed.
    if (s.indexOf(key) == -1) { return; }
    s.replace(key, URLEncode(val.c_str()));
  } else {
    s.replace(key, val);
  }
}

#ifndef BUILD_NO_SPECIAL_CHARACTERS_STRINGCONVERTER
void parseSpecialCharacters(String& s, boolean useURLencode)
{
  bool no_accolades   = s.indexOf('{') == -1 || s.indexOf('}') == -1;
  bool no_html_entity = s.indexOf('&') == -1 || s.indexOf(';') == -1;

  if (no_accolades && no_html_entity) {
    return; // Nothing to replace
  }
  {
    // Degree
    const char degree[3]   = { 0xc2, 0xb0, 0 };       // Unicode degree symbol
    const char degreeC[4]  = { 0xe2, 0x84, 0x83, 0 }; // Unicode degreeC symbol
    const char degree_C[4] = { 0xc2, 0xb0, 'C', 0 };  // Unicode degree symbol + captial C
    repl(F("{D}"),   degree,   s, useURLencode);
    repl(F("&deg;"), degree,   s, useURLencode);
    repl(degreeC,    degree_C, s, useURLencode);
  }
  {
    // Angle quotes
    const char laquo[3] = { 0xc2, 0xab, 0 }; // Unicode left angle quotes symbol
    const char raquo[3] = { 0xc2, 0xbb, 0 }; // Unicode right angle quotes symbol
    repl(F("{<<}"),    laquo, s, useURLencode);
    repl(F("&laquo;"), laquo, s, useURLencode);
    repl(F("{>>}"),    raquo, s, useURLencode);
    repl(F("&raquo;"), raquo, s, useURLencode);
  }
  {
    // Greek letter Mu
    const char mu[3] = { 0xc2, 0xb5, 0 }; // Unicode greek letter mu
    repl(F("{u}"),     mu, s, useURLencode);
    repl(F("&micro;"), mu, s, useURLencode);
  }
  {
    // Currency
    const char euro[4]  = { 0xe2, 0x82, 0xac, 0 }; // Unicode euro symbol
    const char yen[3]   = { 0xc2, 0xa5, 0 };       // Unicode yen symbol
    const char pound[3] = { 0xc2, 0xa3, 0 };       // Unicode pound symbol
    const char cent[3]  = { 0xc2, 0xa2, 0 };       // Unicode cent symbol
    repl(F("{E}"),     euro,  s, useURLencode);
    repl(F("&euro;"),  euro,  s, useURLencode);
    repl(F("{Y}"),     yen,   s, useURLencode);
    repl(F("&yen;"),   yen,   s, useURLencode);
    repl(F("{P}"),     pound, s, useURLencode);
    repl(F("&pound;"), pound, s, useURLencode);
    repl(F("{c}"),     cent,  s, useURLencode);
    repl(F("&cent;"),  cent,  s, useURLencode);
  }
  {
    // Math symbols
    const char sup1[3]   = { 0xc2, 0xb9, 0 }; // Unicode sup1 symbol
    const char sup2[3]   = { 0xc2, 0xb2, 0 }; // Unicode sup2 symbol
    const char sup3[3]   = { 0xc2, 0xb3, 0 }; // Unicode sup3 symbol
    const char frac14[3] = { 0xc2, 0xbc, 0 }; // Unicode frac14 symbol
    const char frac12[3] = { 0xc2, 0xbd, 0 }; // Unicode frac12 symbol
    const char frac34[3] = { 0xc2, 0xbe, 0 }; // Unicode frac34 symbol
    const char plusmn[3] = { 0xc2, 0xb1, 0 }; // Unicode plusmn symbol
    const char times[3]  = { 0xc3, 0x97, 0 }; // Unicode times symbol
    const char divide[3] = { 0xc3, 0xb7, 0 }; // Unicode divide symbol
    repl(F("{^1}"),     sup1,   s, useURLencode);
    repl(F("&sup1;"),   sup1,   s, useURLencode);
    repl(F("{^2}"),     sup2,   s, useURLencode);
    repl(F("&sup2;"),   sup2,   s, useURLencode);
    repl(F("{^3}"),     sup3,   s, useURLencode);
    repl(F("&sup3;"),   sup3,   s, useURLencode);
    repl(F("{1_4}"),    frac14, s, useURLencode);
    repl(F("&frac14;"), frac14, s, useURLencode);
    repl(F("{1_2}"),    frac12, s, useURLencode);
    repl(F("&frac12;"), frac12, s, useURLencode);
    repl(F("{3_4}"),    frac34, s, useURLencode);
    repl(F("&frac34;"), frac34, s, useURLencode);
    repl(F("{+-}"),     plusmn, s, useURLencode);
    repl(F("&plusmn;"), plusmn, s, useURLencode);
    repl(F("{x}"),      times,  s, useURLencode);
    repl(F("&times;"),  times,  s, useURLencode);
    repl(F("{..}"),     divide, s, useURLencode);
    repl(F("&divide;"), divide, s, useURLencode);
  }
}

#endif // ifndef BUILD_NO_SPECIAL_CHARACTERS_STRINGCONVERTER

/********************************************************************************************\
   replace other system variables like %sysname%, %systime%, %ip%
 \*********************************************************************************************/
void parseControllerVariables(String& s, struct EventStruct *event, boolean useURLencode) {
  s = parseTemplate(s, useURLencode);
  parseEventVariables(s, event, useURLencode);
}

void parseSingleControllerVariable(String            & s,
                                   struct EventStruct *event,
                                   byte                taskValueIndex,
                                   boolean             useURLencode) {
  if (validTaskIndex(event->TaskIndex)) {
    LoadTaskSettings(event->TaskIndex);
    repl(F("%valname%"), ExtraTaskSettings.TaskDeviceValueNames[taskValueIndex], s, useURLencode);
  } else {
    repl(F("%valname%"), F(""), s, useURLencode);
  }
}

// FIXME TD-er: These macros really increase build size.
// Simple macro to create the replacement string only when needed.
#define SMART_REPL(T, S) \
  if (s.indexOf(T) != -1) { repl((T), (S), s, useURLencode); }
void parseSystemVariables(String& s, boolean useURLencode)
{
  #ifndef BUILD_NO_SPECIAL_CHARACTERS_STRINGCONVERTER
  parseSpecialCharacters(s, useURLencode);
  #endif // ifndef BUILD_NO_SPECIAL_CHARACTERS_STRINGCONVERTER

  SystemVariables::parseSystemVariables(s, useURLencode);
}

void parseEventVariables(String& s, struct EventStruct *event, boolean useURLencode)
{
  repl(F("%id%"), String(event->idx), s, useURLencode);

  if (validTaskIndex(event->TaskIndex)) {
    if (s.indexOf(F("%val")) != -1) {
      if (event->getSensorType() == Sensor_VType::SENSOR_TYPE_LONG) {
        SMART_REPL(F("%val1%"), String(UserVar.getSensorTypeLong(event->TaskIndex)))
      } else {
        for (byte i = 0; i < getValueCountForTask(event->TaskIndex); ++i) {
          String valstr = F("%val");
          valstr += (i + 1);
          valstr += '%';
          SMART_REPL(valstr, formatUserVarNoCheck(event, i));
        }
      }
    }
  }

  if (validTaskIndex(event->TaskIndex)) {
    // These replacements use ExtraTaskSettings, so make sure the correct TaskIndex is set in the event.
    LoadTaskSettings(event->TaskIndex);
    repl(F("%tskname%"), ExtraTaskSettings.TaskDeviceName, s, useURLencode);
  } else {
    repl(F("%tskname%"), F(""), s, useURLencode);
  }

  const bool vname_found = s.indexOf(F("%vname")) != -1;

  if (vname_found) {
    for (byte i = 0; i < 4; ++i) {
      String vname = F("%vname");
      vname += (i + 1);
      vname += '%';

      if (validTaskIndex(event->TaskIndex)) {
        repl(vname, ExtraTaskSettings.TaskDeviceValueNames[i], s, useURLencode);
      } else {
        repl(vname, F(""), s, useURLencode);
      }
    }
  }
}

#undef SMART_REPL

bool getConvertArgument(const String& marker, const String& s, float& argument, int& startIndex, int& endIndex) {
  String argumentString;

  if (getConvertArgumentString(marker, s, argumentString, startIndex, endIndex)) {
    return validFloatFromString(argumentString, argument);
  }
  return false;
}

bool getConvertArgument2(const String& marker, const String& s, float& arg1, float& arg2, int& startIndex, int& endIndex) {
  String argumentString;

  if (getConvertArgumentString(marker, s, argumentString, startIndex, endIndex)) {
    int pos_comma = argumentString.indexOf(',');

    if (pos_comma == -1) { return false; }

    if (validFloatFromString(argumentString.substring(0, pos_comma), arg1)) {
      return validFloatFromString(argumentString.substring(pos_comma + 1), arg2);
    }
  }
  return false;
}

bool getConvertArgumentString(const String& marker, const String& s, String& argumentString, int& startIndex, int& endIndex) {
  startIndex = s.indexOf(marker);

  if (startIndex == -1) { return false; }

  int startIndexArgument = startIndex + marker.length();

  if (s.charAt(startIndexArgument) != '(') {
    return false;
  }
  ++startIndexArgument;
  endIndex = s.indexOf(')', startIndexArgument);

  if (endIndex == -1) { return false; }

  argumentString = s.substring(startIndexArgument, endIndex);

  if (argumentString.length() == 0) { return false; }
  ++endIndex; // Must also strip ')' from the original string.
  return true;
}


// FIXME TD-er: These macros really increase build size

// Parse conversions marked with "%conv_marker%(float)"
// Must be called last, since all sensor values must be converted, processed, etc.
void parseStandardConversions(String& s, boolean useURLencode) {
  if (s.indexOf(F("%c_")) == -1) {
    return; // Nothing to replace
  }
  float arg1       = 0.0f;
  int   startIndex = 0;
  int   endIndex   = 0;

  // These replacements should be done in a while loop per marker,
  // since they also replace the numerical parameter.
  // The marker may occur more than once per string, but with different parameters.
  #define SMART_CONV(T, FUN) \
  while (getConvertArgument((T), s, arg1, startIndex, endIndex)) { repl(s.substring(startIndex, endIndex), (FUN), s, useURLencode); }
  SMART_CONV(F("%c_w_dir%"),  getBearing(arg1))
  SMART_CONV(F("%c_c2f%"),    toString(CelsiusToFahrenheit(arg1), 2))
  SMART_CONV(F("%c_ms2Bft%"), String(m_secToBeaufort(arg1)))
  SMART_CONV(F("%c_cm2imp%"), centimeterToImperialLength(arg1))
  SMART_CONV(F("%c_mm2imp%"), millimeterToImperialLength(arg1))
  SMART_CONV(F("%c_m2day%"),  toString(minutesToDay(arg1), 2))
  SMART_CONV(F("%c_m2dh%"),   minutesToDayHour(arg1))
  SMART_CONV(F("%c_m2dhm%"),  minutesToDayHourMinute(arg1))
  SMART_CONV(F("%c_s2dhms%"), secondsToDayHourMinuteSecond(arg1))
  SMART_CONV(F("%c_2hex%"),   formatToHex(arg1, F("")))
  #undef SMART_CONV

  // Conversions with 2 parameters
  #define SMART_CONV(T, FUN) \
  while (getConvertArgument2((T), s, arg1, arg2, startIndex, endIndex)) { repl(s.substring(startIndex, endIndex), (FUN), s, useURLencode); }
  float arg2 = 0.0f;
  SMART_CONV(F("%c_dew_th%"), toString(compute_dew_point_temp(arg1, arg2), 2))
  SMART_CONV(F("%c_u2ip%"),   formatUnitToIPAddress(arg1, arg2))
  #undef SMART_CONV
}

/********************************************************************************************\
   Find positional parameter in a char string
 \*********************************************************************************************/
bool HasArgv(const char *string, unsigned int argc) {
  String argvString;

  return GetArgv(string, argvString, argc);
}

bool GetArgv(const char *string, String& argvString, unsigned int argc, char separator) {
  int  pos_begin, pos_end;
  bool hasArgument = GetArgvBeginEnd(string, argc, pos_begin, pos_end, separator);

  argvString = "";

  if (!hasArgument) { return false; }

  if ((pos_begin >= 0) && (pos_end >= 0) && (pos_end > pos_begin)) {
    argvString.reserve(pos_end - pos_begin);

    for (int i = pos_begin; i < pos_end; ++i) {
      argvString += string[i];
    }
  }
  argvString.trim();
  argvString = stripQuotes(argvString);
  return argvString.length() > 0;
}

bool GetArgvBeginEnd(const char *string, const unsigned int argc, int& pos_begin, int& pos_end, char separator) {
  pos_begin = -1;
  pos_end   = -1;
  size_t string_len = strlen(string);
  unsigned int string_pos = 0, argc_pos = 0;
  bool parenthesis          = false;
  char matching_parenthesis = '"';

  while (string_pos < string_len)
  {
    char c, d; // c = current char, d = next char (if available)
    c = string[string_pos];
    d = 0;

    if ((string_pos + 1) < string_len) {
      d = string[string_pos + 1];
    }

    if       (!parenthesis && (c == ' ') && (d == ' ')) {}
    else if  (!parenthesis && (c == ' ') && (d == separator)) {}
    else if  (!parenthesis && (c == separator) && (d == ' ')) {}
    else if  (!parenthesis && (c == ' ') && (d >= 33) && (d <= 126)) {}
    else if  (!parenthesis && (c == separator) && (d >= 33) && (d <= 126)) {}
    else
    {
      if (!parenthesis && (isQuoteChar(c) || (c == '['))) {
        parenthesis          = true;
        matching_parenthesis = c;

        if (c == '[') {
          matching_parenthesis = ']';
        }
      } else if (parenthesis && (c == matching_parenthesis)) {
        parenthesis = false;
      }

      if (pos_begin == -1) {
        pos_begin = string_pos;
        pos_end   = string_pos;
      }
      ++pos_end;

      if (!parenthesis && (isParameterSeparatorChar(d) || (d == separator) || (d == 0))) // end of word
      {
        argc_pos++;

        if (argc_pos == argc)
        {
          return true;
        }
        pos_begin = -1;
        pos_end   = -1;
        string_pos++;
      }
    }
    string_pos++;
  }
  return false;
}
