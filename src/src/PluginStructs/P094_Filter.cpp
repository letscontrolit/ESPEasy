#include "../PluginStructs/P094_Filter.h"

#ifdef USES_P094


# include "../DataStructs/mBusPacket.h"

# include "../Globals/ESPEasy_time.h"
# include "../Globals/TimeZone.h"
# include "../Helpers/ESPEasy_Storage.h"
# include "../Helpers/StringConverter.h"


// *INDENT-OFF*
# define P094_FILTER_WEBARG_LABEL(x)         getPluginCustomArgName((x * 10) + 0)
# define P094_FILTER_WEBARG_MANUFACTURER(x)  getPluginCustomArgName((x * 10) + 1)
# define P094_FILTER_WEBARG_METERTYPE(x)     getPluginCustomArgName((x * 10) + 2)
# define P094_FILTER_WEBARG_SERIAL(x)        getPluginCustomArgName((x * 10) + 3)
# define P094_FILTER_WEBARG_FILTER_WINDOW(x) getPluginCustomArgName((x * 10) + 4)
// *INDENT-ON*

const char P094_Filter_Window_names[] PROGMEM = "all|5m|15m|1h|day|month|once|none";

P094_Filter_Window get_FilterWindow(const String& str)
{
  char tmp[10]{};
  const int command_i = GetCommandCode(tmp, sizeof(tmp), str.c_str(), P094_Filter_Window_names);

  if (command_i == -1) {
    // No match found
    return P094_Filter_Window::None;
  }
  return static_cast<P094_Filter_Window>(command_i);
}

String Filter_WindowToString(P094_Filter_Window filterWindow)
{
  char   tmp[10]{};
  String res(GetTextIndexed(tmp, sizeof(tmp), static_cast<uint32_t>(filterWindow), P094_Filter_Window_names));

  return res;
}

void P094_filter::fromString(const String& str)
{
  // Set everything to wildcards
  _filter._encodedValue = 0;
  _filter._filterWindow = static_cast<int>(P094_Filter_Window::All);

  const int semicolonPos = str.indexOf(';');

  if (semicolonPos != -1) {
    _filter._filterWindow = static_cast<int>(get_FilterWindow(str.substring(semicolonPos + 1)));
  }

  for (size_t i = 0; i < 3; ++i) {
    String tmp;

    if (GetArgv(str.c_str(), tmp, (i + 1), '.')) {
      if (!(tmp.isEmpty() || equals(tmp, '*'))) {
        if (i != 0) {
          // Make sure the numerical values are parsed as HEX
          if (!tmp.startsWith(F("0x")) && !tmp.startsWith(F("0X"))) {
            tmp = concat(F("0x"), tmp);
          }
        }

        switch (i) {
          case 0: // Manufacturer
            _filter._manufacturer = mBusPacket_header_t::encodeManufacturerID(tmp);
            break;
          case 1: // Meter type
          {
            int metertype = 0;

            if (validIntFromString(tmp, metertype)) {
              _filter._meterType = metertype;
            }
            break;
          }
          case 2: // Serial
          {
            int serial = 0;

            if (validIntFromString(tmp, serial)) {
              _filter._serialNr = serial;
            }
            break;
          }
        }
      }
    }
  }
}

String P094_filter::toString() const
{
  const bool includeHexPrefix = false;
  String     res;

  res += getManufacturer();
  res += '.';

  res += getMeterType(includeHexPrefix);
  res += '.';

  res += getSerial(includeHexPrefix);
  res += ';';

  res += Filter_WindowToString(static_cast<P094_Filter_Window>(_filter._filterWindow));

  return res;
}

const uint8_t * P094_filter::toBinary(size_t& size) const
{
  size = getBinarySize();
  return (uint8_t *)this;
}

size_t P094_filter::fromBinary(const uint8_t *data)
{
  memcpy(this, data, getBinarySize());
  _lastSeenUnixTime = 0;
  return getBinarySize();
}

size_t P094_filter::getBinarySize()
{
  // Only store the filter
  constexpr size_t P094_filter_size = sizeof(_filter);

  return P094_filter_size;
}

bool P094_filter::matches(const mBusPacket_header_t& other) const
{
  if (!isWildcardManufacturer()) {
    if (_filter._manufacturer != other._manufacturer) { return false; }
  }

  if (!isWildcardMeterType()) {
    if (_filter._meterType != other._meterType) { return false; }
  }

  if (!isWildcardSerial()) {
    if (_filter._serialNr != other._serialNr) { return false; }
  }

  return true;
}

bool P094_filter::shouldPass()
{
  // Match the interval window.
  const P094_Filter_Window filterWindow = static_cast<P094_Filter_Window>(_filter._filterWindow);

  if (filterWindow == P094_Filter_Window::None) { return false; }

  if (filterWindow == P094_Filter_Window::All) { return true; }

  // Using UnixTime
  const unsigned long currentTime = node_time.getUnixTime();
  unsigned long window_min        = currentTime;

  if ((filterWindow == P094_Filter_Window::One_hour) ||
      (filterWindow == P094_Filter_Window::Day) ||
      (filterWindow == P094_Filter_Window::Month))
  {
    // Create time struct in local time.
    struct tm tmp;
    breakTime(time_zone.toLocal(currentTime), tmp);
    tmp.tm_sec = 0;
    tmp.tm_min = 0;

    if (filterWindow == P094_Filter_Window::Day) {
      // Using local time, thus incl. timezone and DST.
      if (tmp.tm_hour < 23) {
        // Either:
        // - between 00:00 and 12:00
        // - between 12:00 and 23:00
        tmp.tm_hour = (tmp.tm_hour < 12) ? 0 : 12;
      } else {
        // between 23:00 and 00:00
        tmp.tm_hour = 23;
      }
    } else if (filterWindow == P094_Filter_Window::Month) {
      // First set min to midnight of today:
      tmp.tm_hour = 0;

      if (tmp.tm_mday < 15) {
        // - between 1st of month 00:00:00 and 15th of month 00:00:00
        tmp.tm_mday = 1;
      } else {
        // Check if this is the last day of the month.
        // Add 24h to the time and see if it is still the same month.
        struct tm tm_next_day;
        breakTime(node_time.now() + (24 * 60 * 60), tm_next_day);

        if (tm_next_day.tm_mon == tmp.tm_mon) {
          // - between 15th of month 00:00:00 and last of month 00:00:00
          tmp.tm_mday = 15;
        } else {
          // - between last of month 00:00:00 and 1st of next month 00:00:00
          // Thus do not change the date.
        }
      }
    }

    // Convert from local time.
    window_min = time_zone.fromLocal(makeTime(tmp));
  } else {
    switch (filterWindow) {
      case P094_Filter_Window::Once:

        if (_lastSeenUnixTime != 0) {
          return false;
        }
        break;
      case P094_Filter_Window::Five_minutes:
        window_min = currentTime - (currentTime % (5 * 60));
        break;
      case P094_Filter_Window::Fifteen_minutes:
        window_min = currentTime - (currentTime % (15 * 60));
        break;
      case P094_Filter_Window::One_hour:
        window_min = currentTime - (currentTime % (60 * 60));
        break;

      default:
        return false;
    }
  }


  if (_lastSeenUnixTime > window_min) {
    return false;
  }

  _lastSeenUnixTime = currentTime;
  return true;
}

void P094_filter::WebformLoad(uint8_t filterIndex) const
{
  const bool includeHexPrefix = true;

  addRowLabel_tr_id(
    concat(F("Filter "), static_cast<int>(filterIndex)),
    P094_FILTER_WEBARG_LABEL(filterIndex));
  {
    // Manufacturer
    addTextBox(
      P094_FILTER_WEBARG_MANUFACTURER(filterIndex),
      getManufacturer(),
      3, false, false, EMPTY_STRING, F(""));
  }

  {
    // Meter Type
    addTextBox(
      P094_FILTER_WEBARG_METERTYPE(filterIndex),
      getMeterType(includeHexPrefix),
      4, false, false, EMPTY_STRING, F(""));
  }

  {
    // Serial nr
    addTextBox(
      P094_FILTER_WEBARG_SERIAL(filterIndex),
      getSerial(includeHexPrefix),
      10, false, false, EMPTY_STRING, F(""));
  }

  {
    // Filter Window
    const int optionValues[] = {
      static_cast<int>(P094_Filter_Window::All),
      static_cast<int>(P094_Filter_Window::Five_minutes),
      static_cast<int>(P094_Filter_Window::Fifteen_minutes),
      static_cast<int>(P094_Filter_Window::One_hour),
      static_cast<int>(P094_Filter_Window::Day),
      static_cast<int>(P094_Filter_Window::Month),
      static_cast<int>(P094_Filter_Window::Once),
      static_cast<int>(P094_Filter_Window::None)
    };

    constexpr size_t nrOptions = sizeof(optionValues) / sizeof(optionValues[0]);

    String options[nrOptions];

    for (int i = 0; i < nrOptions; ++i) {
      const P094_Filter_Window filterWindow = static_cast<P094_Filter_Window>(optionValues[i]);
      options[i] = Filter_WindowToString(filterWindow);
    }
    addFormSelector(F("Filter Window"),
                    P094_FILTER_WEBARG_FILTER_WINDOW(filterIndex),
                    nrOptions,
                    options,
                    optionValues,
                    _filter._filterWindow,
                    false);
  }
}

bool P094_filter::WebformSave(uint8_t filterIndex)
{
  {
    // Manufacturer
    const String manufacturer_str = webArg(P094_FILTER_WEBARG_MANUFACTURER(filterIndex));

    if (equals(manufacturer_str, '*')) {
      _filter._manufacturer = 0;
    }
    else {
      _filter._manufacturer = mBusPacket_header_t::encodeManufacturerID(manufacturer_str);
    }
  }

  {
    // Meter Type
    _filter._meterType = getFormItemInt(P094_FILTER_WEBARG_METERTYPE(filterIndex), 0);
  }

  {
    // Serial nr
    const String serial_str = webArg(P094_FILTER_WEBARG_SERIAL(filterIndex));

    if (equals(serial_str, '*')) {
      _filter._serialNr = 0;
    }
    else {
      _filter._serialNr = 0;
      uint32_t serial{};

      if (validUIntFromString(serial_str, serial)) {
        _filter._serialNr = serial;
      }
    }
  }

  {
    // Filter Window
    _filter._filterWindow = getFormItemInt(
      P094_FILTER_WEBARG_METERTYPE(filterIndex),
      0);
  }

  return _filter._manufacturer != 0 ||
         _filter._meterType != 0    ||
         _filter._serialNr != 0;
}

String P094_filter::getManufacturer() const
{
  String manufacturer;

  if (isWildcardManufacturer()) {
    manufacturer = '*';
  }  else {
    manufacturer = mBusPacket_header_t::decodeManufacturerID(_filter._manufacturer);
  }
  return manufacturer;
}

String P094_filter::getMeterType(bool includeHexPrefix) const
{
  String metertype;

  if (isWildcardMeterType()) {
    metertype = '*';
  } else {
    metertype = includeHexPrefix
      ? formatToHex(_filter._meterType, 2)
      : formatToHex_no_prefix(_filter._meterType, 2);
  }
  return metertype;
}

String P094_filter::getSerial(bool includeHexPrefix) const
{
  String serial;

  if (isWildcardSerial()) {
    serial = '*';
  } else {
    serial = includeHexPrefix
      ? formatToHex(_filter._serialNr, 8)
      : formatToHex_no_prefix(_filter._serialNr, 8);
  }
  return serial;
}

#endif