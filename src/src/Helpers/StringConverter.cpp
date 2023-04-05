#include "../Helpers/StringConverter.h"


#include "../../_Plugin_Helper.h"

#include "../DataStructs/ESPEasy_EventStruct.h"

#include "../ESPEasyCore/ESPEasy_Log.h"

#include "../Globals/Cache.h"
#include "../Globals/CRCValues.h"
#include "../Globals/Device.h"
#include "../Globals/ESPEasyWiFiEvent.h"
#include "../Globals/ESPEasy_time.h"
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

String concat(const __FlashStringHelper * str, const String &val) {
  String res(str);
  res.concat(val);
  return res;
}

String concat(const __FlashStringHelper * str, const __FlashStringHelper *val) {
  return concat(str, String(val));
}

bool equals(const String& str, const __FlashStringHelper * f_str) {
  return str.equals(String(f_str));
}

bool equals(const String& str, const char& c) {
  return str.equals(String(c));
}

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

String ll2String(int64_t value, uint8_t  base) {
  if (value < 0) {
    String res;
    res = '-';
    res += ull2String(value * -1ll, base);
    return res;
  } else {
    return ull2String(value, base);
  }
}


/********************************************************************************************\
   Check if valid float and convert string to float.
 \*********************************************************************************************/
bool string2float(const String& string, float& floatvalue) {
  return validFloatFromString(string, floatvalue);
}

/********************************************************************************************\
   Convert a char string to IP uint8_t array
 \*********************************************************************************************/
bool isIP(const String& string) {
  IPAddress tmpip;
  return (tmpip.fromString(string));
}

bool str2ip(const String& string, uint8_t *IP) {
  return str2ip(string.c_str(), IP);
}

bool str2ip(const char *string, uint8_t *IP)
{
  IPAddress tmpip; // Default constructor => set to 0.0.0.0

  if ((*string == 0) || tmpip.fromString(string)) {
    // Eiher empty string or a valid IP addres, so copy value.
    for (uint8_t i = 0; i < 4; ++i) {
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


/********************************************************************************************\
   Handling HEX strings
 \*********************************************************************************************/

// Convert max. 8 hex decimals to unsigned long
unsigned long hexToUL(const String& input_c, size_t nrHexDecimals) {
  const unsigned long long resULL = hexToULL(input_c, nrHexDecimals);
  return static_cast<unsigned long>(resULL & 0xFFFFFFFFull);
}

unsigned long hexToUL(const String& input_c) {
  return hexToUL(input_c, input_c.length());
}

unsigned long hexToUL(const String& input_c, size_t startpos, size_t nrHexDecimals) {
  return hexToUL(input_c.substring(startpos, startpos + nrHexDecimals), nrHexDecimals);
}

// Convert max. 16 hex decimals to unsigned long long (aka uint64_t)
unsigned long long hexToULL(const String& input_c, size_t nrHexDecimals) {
  size_t nr_decimals = nrHexDecimals;

  if (nr_decimals > 16) {
    nr_decimals = 16;
  }
  const size_t inputLength = input_c.length();

  if (nr_decimals > inputLength) {
    nr_decimals = inputLength;
  } else if (input_c.startsWith(F("0x"))) { // strtoull handles that prefix nicely
    nr_decimals += 2;
  }
  return strtoull(input_c.substring(0, nr_decimals).c_str(), 0, 16);
}

unsigned long long hexToULL(const String& input_c) {
  return hexToULL(input_c, input_c.length());
}

unsigned long long hexToULL(const String& input_c, size_t startpos, size_t nrHexDecimals) {
  return hexToULL(input_c.substring(startpos, startpos + nrHexDecimals), nrHexDecimals);
}

void appendHexChar(uint8_t data, String& string)
{
  const char *hex_chars = "0123456789abcdef";
  string += hex_chars[(data >> 4) & 0xF];
  string += hex_chars[(data) & 0xF];
}

String formatToHex_array(const uint8_t* data, size_t size)
{
  String res;
  res.reserve(2 * size);
  for (size_t i = 0; i < size; ++i) {
    appendHexChar(data[i], res);
  }
  return res;
}

String formatToHex(unsigned long value, 
                   const __FlashStringHelper * prefix,
                   unsigned int minimal_hex_digits) {
  String result = prefix;
  String hex(value, HEX);

  hex.toUpperCase();
  if (hex.length() < minimal_hex_digits) {
    const size_t leading_zeros = minimal_hex_digits - hex.length();
    for (size_t i = 0; i < leading_zeros; ++i) {
      result += '0';
    }
  }
  result += hex;
  return result;
}

String formatToHex(unsigned long value,
                   const __FlashStringHelper * prefix) {
  return formatToHex(value, prefix, 0);
}

String formatToHex(unsigned long value, unsigned int minimal_hex_digits) {
  return formatToHex(value, F("0x"), minimal_hex_digits);
}

String formatToHex_no_prefix(unsigned long value, unsigned int minimal_hex_digits) {
  return formatToHex(value, F(""), minimal_hex_digits);
}

String formatHumanReadable(unsigned long value, unsigned long factor) {
  String result = formatHumanReadable(value, factor, 2);

  result.replace(F(".00"), EMPTY_STRING);
  return result;
}

String formatHumanReadable(unsigned long value, unsigned long factor, int NrDecimals) {
  float floatValue(value);
  uint8_t  steps = 0;

  while (value >= factor) {
    value /= factor;
    ++steps;
    floatValue /= float(factor);
  }
  String result = toString(floatValue, NrDecimals);

  switch (steps) {
    case 0: break;
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

  result += F(" (");

  if (factor > 1) {
    result += formatHumanReadable(value, factor);
  } else {
    result += value;
  }
  result += ')';
  return result;
}

const __FlashStringHelper * boolToString(bool value) {
  return value ? F("true") : F("false");
}

/*********************************************************************************************\
   Typical string replace functions.
\*********************************************************************************************/
void removeExtraNewLine(String& line) {
  while (line.endsWith(F("\r\n\r\n"))) {
    line.remove(line.length() - 2);
  }
}

void removeChar(String& line, char character) {
  line.replace(String(character), EMPTY_STRING);
}

void addNewLine(String& line) {
  line += F("\r\n");
}

size_t UTF8_charLength(uint8_t firstByte) {
  if (firstByte <= 0x7f) {
    return 1;
  }
  // First Byte  Second Byte Third Byte  Fourth Byte
  // [0x00,0x7F]         
  // [0xC2,0xDF] [0x80,0xBF]     
  // 0xE0        [0xA0,0xBF] [0x80,0xBF] 
  // [0xE1,0xEC] [0x80,0xBF] [0x80,0xBF] 
  // 0xED        [0x80,0x9F] [0x80,0xBF] 
  // [0xEE,0xEF] [0x80,0xBF] [0x80,0xBF] 
  // 0xF0        [0x90,0xBF] [0x80,0xBF] [0x80,0xBF]
  // [0xF1,0xF3] [0x80,0xBF] [0x80,0xBF] [0x80,0xBF]
  // 0xF4        [0x80,0x8F] [0x80,0xBF] [0x80,0xBF]
  // See: https://lemire.me/blog/2018/05/09/how-quickly-can-you-check-that-a-string-is-valid-unicode-utf-8/

  size_t charLength = 2;
  if (firstByte > 0xEF) {
    charLength = 4;
  } else if (firstByte > 0xDF) {
    charLength = 3;
  }
  return charLength;
}

void replaceUnicodeByChar(String& line, char replChar) {
  size_t pos = 0;
  while (pos < line.length()) {
    const size_t charLength = UTF8_charLength((uint8_t)line[pos]);

    if (charLength > 1) {
      // Is unicode char in UTF-8 format
      // Need to find how many characters we need to replace.
      const size_t charsLeft = line.length() - pos;
      if (charsLeft >= charLength) {
        line.replace(line.substring(pos, pos + charLength), String(replChar));
      }
    }
    ++pos;
  }
}

/*********************************************************************************************\
   Format a value to the set number of decimals
\*********************************************************************************************/
String doFormatUserVar(struct EventStruct *event, uint8_t rel_index, bool mustCheck, bool& isvalid) {
  if (event == nullptr) return EMPTY_STRING;
  isvalid = true;

  const deviceIndex_t DeviceIndex = getDeviceIndex_from_TaskIndex(event->TaskIndex);

  if (!validDeviceIndex(DeviceIndex)) {
    isvalid = false;
    return EMPTY_STRING;
  }

  {
    // First try to format using the plugin specific formatting.
    String result;
    EventStruct tempEvent;
    tempEvent.deep_copy(event);
    tempEvent.idx = rel_index;
    PluginCall(PLUGIN_FORMAT_USERVAR, &tempEvent, result);
    if (result.length() > 0) {
      return result;
    }
  }


  const uint8_t   valueCount = getValueCountForTask(event->TaskIndex);
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
      addLogMove(LOG_LEVEL_ERROR, log);
    }
    #endif // ifndef BUILD_NO_DEBUG
    return EMPTY_STRING;
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
      addLogMove(LOG_LEVEL_DEBUG, log);
    }
#endif // ifndef BUILD_NO_DEBUG
    f = 0;
  }

  uint8_t nrDecimals = 0;
  if (Device[DeviceIndex].configurableDecimals()) {
    nrDecimals = Cache.getTaskDeviceValueDecimals(event->TaskIndex, rel_index);
  }

  String result = toString(f, nrDecimals);
  result.trim();
  return result;
}

String formatUserVarNoCheck(taskIndex_t TaskIndex, uint8_t rel_index) {
  bool isvalid;

  // FIXME TD-er: calls to this function cannot handle Sensor_VType::SENSOR_TYPE_STRING
  struct EventStruct TempEvent(TaskIndex);

  return doFormatUserVar(&TempEvent, rel_index, false, isvalid);
}

String formatUserVar(taskIndex_t TaskIndex, uint8_t rel_index, bool& isvalid) {
  // FIXME TD-er: calls to this function cannot handle Sensor_VType::SENSOR_TYPE_STRING
  struct EventStruct TempEvent(TaskIndex);

  return doFormatUserVar(&TempEvent, rel_index, true, isvalid);
}

String formatUserVarNoCheck(struct EventStruct *event, uint8_t rel_index)
{
  bool isvalid;

  return doFormatUserVar(event, rel_index, false, isvalid);
}

String formatUserVar(struct EventStruct *event, uint8_t rel_index, bool& isvalid)
{
  return doFormatUserVar(event, rel_index, true, isvalid);
}

String get_formatted_Controller_number(cpluginID_t cpluginID) {
  if (!validCPluginID(cpluginID)) {
    return F("C---");
  }
  String result;
  result += 'C';

  if (cpluginID < 100) { result += '0'; }

  if (cpluginID < 10) { result += '0'; }
  result += cpluginID;
  return result;
}

/*********************************************************************************************\
   Wrap a string with given pre- and postfix string.
\*********************************************************************************************/
String wrap_braces(const String& string) {
  return wrap_String(string, '(', ')');
}

String wrap_String(const String& string, char wrap) {
  String result;
  result.reserve(string.length() + 2);
  result += wrap;
  result += string;
  result += wrap;
  return result;
}

String wrap_String(const String& string, char char1, char char2) {
  String result;
  result.reserve(string.length() + 2);
  result += char1;
  result += string;
  result += char2;
  return result;
}

String wrapIfContains(const String& value, char contains, char wrap) {
  if (value.indexOf(contains) != -1) {
    return wrap_String(value, wrap, wrap);
  }
  return value;
}

String wrapWithQuotes(const String& text) {
  if (isWrappedWithQuotes(text)) {
    return text;
  }
  // Try to find unused quote char and wrap
  char quotechar = '_';
  if (!findUnusedQuoteChar(text, quotechar)) {
    if (loglevelActiveFor(LOG_LEVEL_ERROR)) {
      String log = F("No unused quote to wrap: _");
      log += text;
      log += '_';
      addLogMove(LOG_LEVEL_ERROR, log);
    }
  }
  return wrap_String(text, quotechar);
}

String wrapWithQuotesIfContainsParameterSeparatorChar(const String& text) {
  if (isWrappedWithQuotes(text)) {
    return text;
  }
  if (stringContainsSeparatorChar(text)) {
    return wrapWithQuotes(text);
  }
  return text;
}

/*********************************************************************************************\
   Format an object value pair for use in JSON.
\*********************************************************************************************/
String to_json_object_value(const __FlashStringHelper * object,
                            const __FlashStringHelper * value,
                            bool wrapInQuotes) 
{
  return to_json_object_value(String(object), String(value), wrapInQuotes);
}


String to_json_object_value(const __FlashStringHelper * object,
                            const String& value,
                            bool wrapInQuotes) 
{
  return to_json_object_value(String(object), value, wrapInQuotes);
}

String to_json_object_value(const __FlashStringHelper * object,
                            String&& value,
                            bool wrapInQuotes) 
{
  return to_json_object_value(String(object), value, wrapInQuotes);
}

String to_json_object_value(const String& object, const String& value, bool wrapInQuotes) {
  String result;
  result.reserve(object.length() + value.length() + 6);
  result = wrap_String(object, '"');
  result += ':';
  result += to_json_value(value, wrapInQuotes);
  return result;
}

String to_json_value(const String& value, bool wrapInQuotes) {
  if (value.isEmpty()) {
    // Empty string
    return F("\"\"");
  }
  if (wrapInQuotes || mustConsiderAsJSONString(value)) {
    // Is not a numerical value, or BIN/HEX notation, thus wrap with quotes

    // First we check for not allowed special characters.
    const size_t val_length = value.length();
    for (size_t i = 0; i < val_length; ++i) {
      switch (value[i]) {
        case '\n':
        case '\r':
        case '\t':
        case '\\':
        case '\b':
        case '\f':
        case '"':
        {
          // Special characters not allowed in JSON:
          //  \b  Backspace (ascii code 08)
          //  \f  Form feed (ascii code 0C)
          //  \n  New line
          //  \r  Carriage return
          //  \t  Tab
          //  \"  Double quote
          //  \\  Backslash character
          // Must replace characters, so make a deepcopy
          String tmpValue(value);
          tmpValue.replace('\n', '^');
          tmpValue.replace('\r', '^');
          tmpValue.replace('\t', ' ');
          tmpValue.replace('\\', '^');
          tmpValue.replace('\b', '^');
          tmpValue.replace('\f', '^');
          tmpValue.replace('"',  '\'');
          return wrap_String(tmpValue, '"');
        }
      }
    }
    return wrap_String(value, '"');
  } 
  // It is a numerical
  return value;
}


/*********************************************************************************************\
   Strip wrapping chars (e.g. quotes)
\*********************************************************************************************/
String stripWrappingChar(const String& text, char wrappingChar) {
  const unsigned int length = text.length();

  if ((length >= 2) && stringWrappedWithChar(text, wrappingChar)) {
    return text.substring(1, length - 1);
  }
  return text;
}

bool stringWrappedWithChar(const String& text, char wrappingChar) {
  const unsigned int length = text.length();

  if (length < 2) { return false; }
  return (text.charAt(0) == wrappingChar) && 
         (text.charAt(length - 1) == wrappingChar);
}

bool isQuoteChar(char c) {
  return c == '\'' || c == '"' || c == '`';
}

bool findUnusedQuoteChar(const String& text, char& quotechar) {
  quotechar = '_';
  if (text.indexOf('\'') == -1) quotechar = '\'';
  else if (text.indexOf('"') == -1) quotechar = '"';
  else if (text.indexOf('`') == -1) quotechar = '`';
  
  return isQuoteChar(quotechar);
}

bool isParameterSeparatorChar(char c) {
  return c == ',' || c == ' ';
}

bool stringContainsSeparatorChar(const String& text) {
  return text.indexOf(',') != -1 || text.indexOf(' ') != -1;
}

bool isWrappedWithQuotes(const String& text) {
  if (text.length() < 2) {
    return false;
  }
  const char quoteChar = text[0];
  return isQuoteChar(quoteChar) && stringWrappedWithChar(text, quoteChar);
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

bool safe_strncpy(char         *dest,
                  const __FlashStringHelper * source,
                  size_t        max_size) 
{
  return safe_strncpy(dest, String(source), max_size);
}

bool safe_strncpy(char *dest, const String& source, size_t max_size) {
  return safe_strncpy(dest, source.c_str(), max_size);
}

bool safe_strncpy(char *dest, const char *source, size_t max_size) {
  if (max_size < 1) { return false; }

  if (dest == nullptr) { return false; }

  if (source == nullptr) { return false; }
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
String parseString(const char * string, uint8_t indexFind, char separator, bool trimResult) {
  return parseString(String(string), indexFind, separator, trimResult);
}

String parseString(const String& string, uint8_t indexFind, char separator, bool trimResult) {
  String result = parseStringKeepCase(string, indexFind, separator, trimResult);

  result.toLowerCase();
  return result;
}

String parseStringKeepCaseNoTrim(const String& string, uint8_t indexFind, char separator) {
  return parseStringKeepCase(string, indexFind, separator, false);
}

String parseStringKeepCase(const String& string, uint8_t indexFind, char separator, bool trimResult) {
  String result;

  if (!GetArgv(string.c_str(), result, indexFind, separator)) {
    return EMPTY_STRING;
  }
  if (trimResult) {
    result.trim();
  }
  return stripQuotes(result);
}

String parseStringToEnd(const String& string, uint8_t indexFind, char separator, bool trimResult) {
  String result = parseStringToEndKeepCase(string, indexFind, separator, trimResult);

  result.toLowerCase();
  return result;
}

String parseStringToEndKeepCaseNoTrim(const String& string, uint8_t indexFind, char separator) {
  return parseStringToEndKeepCase(string, indexFind, separator, false);
}

String parseStringToEndKeepCase(const String& string, uint8_t indexFind, char separator, bool trimResult) {
  // Loop over the arguments to find the first and last pos of the arguments.
  int  pos_begin = string.length();
  int  pos_end = pos_begin;
  int  tmppos_begin, tmppos_end = -1;
  uint8_t nextArgument = indexFind;
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

  if (!hasArgument || (pos_begin < 0) || (pos_begin == pos_end)) {
    return EMPTY_STRING;
  }
  String result = string.substring(pos_begin, pos_end);

  if (trimResult) {
    result.trim();
  }
  return stripQuotes(result);
}

String tolerantParseStringKeepCase(const char * string,
                                   uint8_t      indexFind,
                                   char         separator,
                                   bool         trimResult)
{
  return tolerantParseStringKeepCase(String(string), indexFind, separator, trimResult);
}


String tolerantParseStringKeepCase(const String& string, uint8_t indexFind, char separator, bool trimResult)
{
  if (Settings.TolerantLastArgParse()) {
    return parseStringToEndKeepCase(string, indexFind, separator, trimResult);
  }
  return parseStringKeepCase(string, indexFind, separator, trimResult);
}

/*****************************************************************************
 * handles: 0xXX,text,0xXX," more text ",0xXX starting from index 2 (1-based)
 ****************************************************************************/
String parseHexTextString(const String& argument, int index) {
  String result;

  // Ignore these characters when used as hex-byte separators (0x01ab 23-cd:45 -> 0x01,0xab,0x23,0xcd,0x45)
  const String skipChars = F(" -:,.;");

  result.reserve(argument.length()); // longer than needed, most likely
  int i      = index;
  String arg = parseStringKeepCase(argument, i, ',', false);

  while (!arg.isEmpty()) {
    if ((arg.startsWith(F("0x")) || arg.startsWith(F("0X")))) {
      size_t j = 2;

      while (j < arg.length()) {
        int hex = -1;

        if (validIntFromString(concat(F("0x"), arg.substring(j, j + 2)), hex) && (hex > 0) && (hex < 256)) {
          result += char(hex);
        }
        j += 2;
        int c = skipChars.indexOf(arg.substring(j, j + 1));

        while (j < arg.length() && c > -1) {
          j++;
          c = skipChars.indexOf(arg.substring(j, j + 1));
        }
      }
    } else {
      result += arg;
    }
    i++;
    arg = parseStringKeepCase(argument, i, ',', false);
  }

  return result;
}

/*****************************************************************************
 * handles: 0xXX,text,0xXX," more text ",0xXX starting from index 2 (1-based)
 ****************************************************************************/
std::vector<uint8_t> parseHexTextData(const String& argument, int index) {
  std::vector<uint8_t> result;

  // Ignore these characters when used as hex-byte separators (0x01ab 23-cd:45 -> 0x01,0xab,0x23,0xcd,0x45)
  const String skipChars = F(" -:,.;");

  result.reserve(argument.length()); // longer than needed, most likely
  int i      = index;
  String arg = parseStringKeepCase(argument, i, ',', false);

  while (!arg.isEmpty()) {
    if ((arg.startsWith(F("0x")) || arg.startsWith(F("0X")))) {
      size_t j = 2;

      while (j < arg.length()) {
        int hex = -1;

        if (validIntFromString(concat(F("0x"), arg.substring(j, j + 2)), hex) && (hex > -1) && (hex < 256)) {
          result.push_back(char(hex));
        }
        j += 2;
        int c = skipChars.indexOf(arg.substring(j, j + 1));

        while (j < arg.length() && c > -1) {
          j++;
          c = skipChars.indexOf(arg.substring(j, j + 1));
        }
      }
    } else {
      for (size_t s = 0; s < arg.length(); s++) {
        result.push_back(arg[s]);
      }
    }
    i++;
    arg = parseStringKeepCase(argument, i, ',', false);
  }

  return result;
}

/*********************************************************************************************\
   GetTextIndexed: Get text from large PROGMEM stored string
   Items are separated by a '|'
   Code (c) Tasmota:
   https://github.com/arendst/Tasmota/blob/293ae8064d753e6d38488b46d21cdc52a4a6e637/tasmota/tasmota_support/support.ino#L937
\*********************************************************************************************/
char* GetTextIndexed(char* destination, size_t destination_size, uint32_t index, const char* haystack)
{
  // Returns empty string if not found
  // Returns text of found
  char* write = destination;
  const char* read = haystack;

  index++;
  while (index--) {
    size_t size = destination_size -1;
    write = destination;
    char ch = '.';
    while ((ch != '\0') && (ch != '|')) {
      ch = pgm_read_byte(read++);
      if (size && (ch != '|'))  {
        *write++ = ch;
        size--;
      }
    }
    if (0 == ch) {
      if (index) {
        write = destination;
      }
      break;
    }
  }
  *write = '\0';
  return destination;
}

/*********************************************************************************************\
   GetCommandCode: Find string in large PROGMEM stored string
   Items are separated by a '|'
   Code (c) Tasmota:
   https://github.com/arendst/Tasmota/blob/293ae8064d753e6d38488b46d21cdc52a4a6e637/tasmota/tasmota_support/support.ino#L967
\*********************************************************************************************/
int GetCommandCode(char* destination, size_t destination_size, const char* needle, const char* haystack)
{
  // Returns -1 of not found
  // Returns index and command if found
  int result = -1;
  const char* read = haystack;
  char* write = destination;

  while (true) {
    result++;
    size_t size = destination_size -1;
    write = destination;
    char ch = '.';
    while ((ch != '\0') && (ch != '|')) {
      ch = pgm_read_byte(read++);
      if (size && (ch != '|'))  {
        *write++ = ch;
        size--;
      }
    }
    *write = '\0';
    if (!strcasecmp(needle, destination)) {
      break;
    }
    if (0 == ch) {
      result = -1;
      break;
    }
  }
  return result;
}



// escapes special characters in strings for use in html-forms
bool htmlEscapeChar(char c, String& esc)
{
  const __FlashStringHelper * escaped = F("");
  switch (c)
  {
    case '&':  escaped = F("&amp;");  break;
    case '\"': escaped = F("&quot;"); break;
    case '\'': escaped = F("&#039;"); break;
    case '<':  escaped = F("&lt;");   break;
    case '>':  escaped = F("&gt;");   break;
    case '/':  escaped = F("&#047;"); break;
    default:
      return false;
  }

  esc = String(escaped);  
  return true;
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
    if (isAlphaNumeric(html[i]))
    {
      escaped += html[i];
    }
    else
    {
      char s[4] = {0};
      sprintf_P(s, PSTR("%03d"), static_cast<int>(html[i]));
      escaped += '&';
      escaped += '#';
      escaped += s;
      escaped += ';';
    }
  }
  html = escaped;
}

// ********************************************************************************
// URNEncode char string to string object
// ********************************************************************************
String URLEncode(const String& msg)
{
  String encodedMsg;

  const size_t msg_length = msg.length();

  encodedMsg.reserve(msg_length);

  for (size_t i = 0; i < msg_length; ++i) {
    const char ch = msg[i];
    if (isAlphaNumeric(ch)
        || ('-' == ch) || ('_' == ch)
        || ('.' == ch) || ('~' == ch)) {
      encodedMsg += ch;
    } else {
      encodedMsg += '%';
      appendHexChar(ch, encodedMsg);
    }
  }
  return encodedMsg;
}

void repl(const __FlashStringHelper * key,
            const String& val,
            String      & s,
            bool       useURLencode)
{
  repl(String(key), val, s, useURLencode);
}

void repl(const __FlashStringHelper * key,
          const char* val,
          String      & s,
          bool       useURLencode)
{
  repl(String(key), String(val), s, useURLencode);
}

void repl(const __FlashStringHelper * key1,
           const __FlashStringHelper * key2,
           const char* val,
           String      & s,
           bool       useURLencode)
{
  repl(key1, val, s, useURLencode);
  repl(key2, val, s, useURLencode);
}


void repl(const String& key, const String& val, String& s, bool useURLencode)
{
  if (useURLencode) {
    // URLEncode does take resources, so check first if needed.
    if (s.indexOf(key) == -1) { return; }
    s.replace(key, URLEncode(val));
  } else {
    s.replace(key, val);
  }
}

void parseSpecialCharacters(String& s, bool useURLencode)
{
  const bool no_accolades   = s.indexOf('{') == -1 || s.indexOf('}') == -1;
  const bool no_html_entity = s.indexOf('&') == -1 || s.indexOf(';') == -1;

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
  // Degree symbol is often used on displays, so still support that one.
#ifndef BUILD_NO_SPECIAL_CHARACTERS_STRINGCONVERTER
  {
    // Angle quotes
    const char laquo[3] = { 0xc2, 0xab, 0 }; // Unicode left angle quotes symbol
    const char raquo[3] = { 0xc2, 0xbb, 0 }; // Unicode right angle quotes symbol
    repl(F("{<<}"), F("&laquo;"), laquo, s, useURLencode);
    repl(F("{>>}"), F("&raquo;"), raquo, s, useURLencode);
  }
  {
    // Greek letter Mu
    const char mu[3] = { 0xc2, 0xb5, 0 }; // Unicode greek letter mu
    repl(F("{u}"), F("&micro;"), mu, s, useURLencode);
  }
  {
    // Currency
    const char euro[4]  = { 0xe2, 0x82, 0xac, 0 }; // Unicode euro symbol
    const char yen[3]   = { 0xc2, 0xa5, 0 };       // Unicode yen symbol
    const char pound[3] = { 0xc2, 0xa3, 0 };       // Unicode pound symbol
    const char cent[3]  = { 0xc2, 0xa2, 0 };       // Unicode cent symbol
    repl(F("{E}"), F("&euro;"),  euro,  s, useURLencode);
    repl(F("{Y}"), F("&yen;"),   yen,   s, useURLencode);
    repl(F("{P}"), F("&pound;"), pound, s, useURLencode);
    repl(F("{c}"), F("&cent;"),  cent,  s, useURLencode);
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
    repl(F("{^1}"),  F("&sup1;"),   sup1,   s, useURLencode);
    repl(F("{^2}"),  F("&sup2;"),   sup2,   s, useURLencode);
    repl(F("{^3}"),  F("&sup3;"),   sup3,   s, useURLencode);
    repl(F("{1_4}"), F("&frac14;"), frac14, s, useURLencode);
    repl(F("{1_2}"), F("&frac12;"), frac12, s, useURLencode);
    repl(F("{3_4}"), F("&frac34;"), frac34, s, useURLencode);
    repl(F("{+-}"),  F("&plusmn;"), plusmn, s, useURLencode);
    repl(F("{x}"),   F("&times;"),  times,  s, useURLencode);
    repl(F("{..}"),  F("&divide;"), divide, s, useURLencode);
  }
#endif // ifndef BUILD_NO_SPECIAL_CHARACTERS_STRINGCONVERTER
}


/********************************************************************************************\
   replace other system variables like %sysname%, %systime%, %ip%
 \*********************************************************************************************/
void parseControllerVariables(String& s, struct EventStruct *event, bool useURLencode) {
  s = parseTemplate(s, useURLencode);
  parseEventVariables(s, event, useURLencode);
}

void parseSingleControllerVariable(String            & s,
                                   struct EventStruct *event,
                                   uint8_t                taskValueIndex,
                                   bool             useURLencode) {
  if (validTaskIndex(event->TaskIndex)) {
    repl(F("%valname%"), getTaskValueName(event->TaskIndex, taskValueIndex), s, useURLencode);
  } else {
    repl(F("%valname%"), EMPTY_STRING, s, useURLencode);
  }
}

// FIXME TD-er: These macros really increase build size.
// Simple macro to create the replacement string only when needed.
#define SMART_REPL(T, S) \
  if (s.indexOf(T) != -1) { repl((T), (S), s, useURLencode); }
void parseSystemVariables(String& s, bool useURLencode)
{
  parseSpecialCharacters(s, useURLencode);

  SystemVariables::parseSystemVariables(s, useURLencode);
}

void parseEventVariables(String& s, struct EventStruct *event, bool useURLencode)
{
  if (s.indexOf('%') == -1) {
    return;
  }
  repl(F("%id%"), String(event->idx), s, useURLencode);

  if (validTaskIndex(event->TaskIndex)) {
    if (s.indexOf(F("%val")) != -1) {
      if (event->getSensorType() == Sensor_VType::SENSOR_TYPE_LONG) {
        SMART_REPL(F("%val1%"), String(UserVar.getSensorTypeLong(event->TaskIndex)))
      } else {
        for (uint8_t i = 0; i < getValueCountForTask(event->TaskIndex); ++i) {
          String valstr = F("%val");
          valstr += (i + 1);
          valstr += '%';
          SMART_REPL(valstr, formatUserVarNoCheck(event, i));
        }
      }
    }
  }

  if (s.indexOf(F("%tskname%")) != -1) {
    if (validTaskIndex(event->TaskIndex)) {
      repl(F("%tskname%"), getTaskDeviceName(event->TaskIndex), s, useURLencode);
    } else {
      repl(F("%tskname%"), EMPTY_STRING, s, useURLencode);
    }
  }

  const bool vname_found = s.indexOf(F("%vname")) != -1;

  if (vname_found) {
    for (uint8_t i = 0; i < 4; ++i) {
      String vname = F("%vname");
      vname += (i + 1);
      vname += '%';

      if (validTaskIndex(event->TaskIndex)) {
        repl(vname, getTaskValueName(event->TaskIndex, i), s, useURLencode);
      } else {
        repl(vname, EMPTY_STRING, s, useURLencode);
      }
    }
  }
}

#undef SMART_REPL

bool getConvertArgument(const __FlashStringHelper * marker, const String& s, float& argument, int& startIndex, int& endIndex) {
  String argumentString;

  if (getConvertArgumentString(marker, s, argumentString, startIndex, endIndex)) {
    return validFloatFromString(argumentString, argument);
  }
  return false;
}

bool getConvertArgument2(const __FlashStringHelper * marker, const String& s, float& arg1, float& arg2, int& startIndex, int& endIndex) {
  String argumentString;

  if (getConvertArgumentString(marker, s, argumentString, startIndex, endIndex)) {
    const int pos_comma = argumentString.indexOf(',');

    if (pos_comma == -1) { return false; }

    if (validFloatFromString(argumentString.substring(0, pos_comma), arg1)) {
      return validFloatFromString(argumentString.substring(pos_comma + 1), arg2);
    }
  }
  return false;
}

bool getConvertArgumentString(const __FlashStringHelper * marker, const String& s, String& argumentString, int& startIndex, int& endIndex) {
  return getConvertArgumentString(String(marker), s, argumentString, startIndex, endIndex);
}

bool getConvertArgumentString(const String& marker,
                              const String& s,
                              String      & argumentString,
                              int         & startIndex,
                              int         & endIndex) {


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

  if (argumentString.isEmpty()) { return false; }
  ++endIndex; // Must also strip ')' from the original string.
  return true;
}


// FIXME TD-er: These macros really increase build size
struct ConvertArgumentData {
  ConvertArgumentData(String& s, bool useURLencode) 
    : str(s), arg1(0.0f), arg2(0.0f), startIndex(0), endIndex(0),
      URLencode(useURLencode) {}

  ConvertArgumentData() = delete;

  String& str;
  float arg1, arg2;
  int   startIndex;
  int   endIndex;
  bool  URLencode;
};

void repl(ConvertArgumentData& data, const String& repl_str) {
  repl(data.str.substring(data.startIndex, data.endIndex), repl_str, data.str, data.URLencode);
}

bool getConvertArgument(const __FlashStringHelper * marker, ConvertArgumentData& data) {
  return getConvertArgument(marker, data.str, data.arg1, data.startIndex, data.endIndex);
}

bool getConvertArgument2(const __FlashStringHelper * marker, ConvertArgumentData& data) {
  return getConvertArgument2(marker, data.str, data.arg1, data.arg2, data.startIndex, data.endIndex);
}

// Parse conversions marked with "%conv_marker%(float)"
// Must be called last, since all sensor values must be converted, processed, etc.
void parseStandardConversions(String& s, bool useURLencode) {
  if (s.indexOf(F("%c_")) == -1) {
    return; // Nothing to replace
  }

  ConvertArgumentData data(s, useURLencode);

  // These replacements should be done in a while loop per marker,
  // since they also replace the numerical parameter.
  // The marker may occur more than once per string, but with different parameters.
  #define SMART_CONV(T, FUN) \
  while (getConvertArgument((T), data)) { repl(data, (FUN)); }
  SMART_CONV(F("%c_w_dir%"),  getBearing(data.arg1))
  SMART_CONV(F("%c_c2f%"),    toString(CelsiusToFahrenheit(data.arg1), 2))
  SMART_CONV(F("%c_ms2Bft%"), String(m_secToBeaufort(data.arg1)))
  SMART_CONV(F("%c_cm2imp%"), centimeterToImperialLength(data.arg1))
  SMART_CONV(F("%c_mm2imp%"), millimeterToImperialLength(data.arg1))
  SMART_CONV(F("%c_m2day%"),  toString(minutesToDay(data.arg1), 2))
  SMART_CONV(F("%c_m2dh%"),   minutesToDayHour(data.arg1))
  SMART_CONV(F("%c_m2dhm%"),  minutesToDayHourMinute(data.arg1))
  SMART_CONV(F("%c_m2hcm%"),  minutesToHourColonMinute(data.arg1))
  SMART_CONV(F("%c_s2dhms%"), secondsToDayHourMinuteSecond(data.arg1))
  SMART_CONV(F("%c_2hex%"),   formatToHex_no_prefix(data.arg1))
  #undef SMART_CONV

  // Conversions with 2 parameters
  #define SMART_CONV(T, FUN) \
  while (getConvertArgument2((T), data)) { repl(data, (FUN)); }
  SMART_CONV(F("%c_dew_th%"), toString(compute_dew_point_temp(data.arg1, data.arg2), 2))
  #if FEATURE_ESPEASY_P2P
  SMART_CONV(F("%c_u2ip%"),   formatUnitToIPAddress(data.arg1, data.arg2))
  #endif
  SMART_CONV(F("%c_alt_pres_sea%"), toString(altitudeFromPressure(data.arg1, data.arg2), 2))
  SMART_CONV(F("%c_sea_pres_alt%"), toString(pressureElevation(data.arg1, data.arg2), 2))
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

  argvString = String();

  if (!hasArgument) { return false; }

  if ((pos_begin >= 0) && (pos_end >= 0) && (pos_end > pos_begin)) {
    argvString.reserve(pos_end - pos_begin);

    for (int i = pos_begin; i < pos_end; ++i) {
      argvString += string[i];
    }
    argvString.trim();
    argvString = stripQuotes(argvString);
  }
  return true;
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
    char c, d, e; // c = current char, d,e = next char (if available)
    c = string[string_pos];
    d = 0;
    e = 0;

    if ((string_pos + 1) < string_len) {
      d = string[string_pos + 1];
    }
    if ((string_pos + 2) < string_len) {
      e = string[string_pos + 2];
    }

    if  (!parenthesis && (((c == ' ') && (d == ' ')) || 
                          ((c == separator) && (d == ' ')))) {
      // Consider multiple consequitive spaces as one.
    }
    else if  (!parenthesis && ((d == ' ') && (e == separator))) {
      // Skip the space.      
    }
    else
    {
      // Found the start of the new argument.
      if (pos_begin == -1 && !parenthesis && !((c == separator) || isParameterSeparatorChar(c))) {
        pos_begin = string_pos;
        pos_end   = string_pos;
      }
      if (pos_end != -1) {
        ++pos_end;
      }

      // Check if we're in a set of parenthesis (any quote char or [])
      if (!parenthesis && (isQuoteChar(c) || (c == '['))) {
        parenthesis          = true;
        matching_parenthesis = c;

        if (c == '[') {
          matching_parenthesis = ']';
        }
      } else if (parenthesis && (c == matching_parenthesis)) {
        parenthesis = false;
      }

      if (!parenthesis && (isParameterSeparatorChar(d) || (d == separator) || (d == 0))) // end of word
      {
        argc_pos++;
        if (argc_pos == argc)
        {
          return true;
        }
        // new Argument separator found
        pos_begin = -1;
        pos_end   = -1;
      }
    }
    string_pos++;
  }
  return false;
}
