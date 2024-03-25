#include "../PluginStructs/P094_Filter.h"

#ifdef USES_P094


# include "../DataStructs/mBusPacket.h"

# include "../Globals/ESPEasy_time.h"
# include "../Globals/TimeZone.h"
# include "../Helpers/ESPEasy_Storage.h"
# include "../Helpers/StringConverter.h"


// *INDENT-OFF*
# define P094_FILTER_WEBARG_LABEL(x)         getPluginCustomArgName((x * 10) + 10)
# define P094_FILTER_WEBARG_MANUFACTURER(x)  getPluginCustomArgName((x * 10) + 11)
# define P094_FILTER_WEBARG_METERTYPE(x)     getPluginCustomArgName((x * 10) + 12)
# define P094_FILTER_WEBARG_SERIAL(x)        getPluginCustomArgName((x * 10) + 13)
# define P094_FILTER_WEBARG_FILTER_WINDOW(x) getPluginCustomArgName((x * 10) + 14)
// *INDENT-ON*

const char P094_Filter_Window_names[] PROGMEM = "none|all|1m|5m|15m|1h|day|month|once";

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

P094_filter::P094_filter() {
  _filter._manufacturer = mBus_packet_wildcard_manufacturer;
  _filter._meterType    = mBus_packet_wildcard_metertype;
  _filter._serialNr     = mBus_packet_wildcard_serial;
  _filter._filterWindow = static_cast<int>(P094_Filter_Window::None);
}

void P094_filter::fromString(String str)
{
  // Set everything to wildcards
  _filter._manufacturer = mBus_packet_wildcard_manufacturer;
  _filter._meterType    = mBus_packet_wildcard_metertype;
  _filter._serialNr     = mBus_packet_wildcard_serial;
  _filter._filterWindow = static_cast<int>(P094_Filter_Window::None);

  const int semicolonPos = str.indexOf(';');

  if (semicolonPos != -1) {
    _filter._filterWindow = static_cast<int>(get_FilterWindow(str.substring(semicolonPos + 1)));
    str                   = str.substring(0, semicolonPos);
  }

  for (size_t i = 0; i < 3; ++i) {
    String tmp;

    if (GetArgv(str.c_str(), tmp, (i + 1), '.')) {
      if (!(tmp.isEmpty() || tmp.startsWith(F("*")))) {
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
            int32_t metertype = mBus_packet_wildcard_metertype;

            if (validIntFromString(tmp, metertype)) {
              _filter._meterType = metertype;
            }
            break;
          }
          case 2: // Serial
          {
            int32_t serial = mBus_packet_wildcard_serial;

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
  String res;

  res += getManufacturer();
  res += '.';

  res += getMeterType();
  res += '.';

  res += getSerial();
  res += ';';

  res += Filter_WindowToString(getFilterWindow());

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
  return getBinarySize();
}

bool P094_filter::isValid() const
{
  if ((_filter._manufacturer == 0) &&
      (_filter._meterType == 0) &&
      (_filter._serialNr == 0) &&
      (getFilterWindow() == P094_Filter_Window::None)) {
    return false;
  }
  return
    !isWildcardManufacturer() ||
    !isWildcardMeterType() ||
    !isWildcardSerial() ||
    getFilterWindow() != P094_Filter_Window::None;
}

bool P094_filter::operator<(const P094_filter& rhs) const
{
  if (isValid() != rhs.isValid()) {
    return isValid();
  }
/*
  // Disable sorting, only sort by having valid filters at top.
  if (isWildcardManufacturer() != rhs.isWildcardManufacturer()) {
    return rhs.isWildcardManufacturer();
  }

  if (isWildcardMeterType() != rhs.isWildcardMeterType()) {
    return rhs.isWildcardMeterType();
  }

  if (isWildcardSerial() != rhs.isWildcardSerial()) {
    return rhs.isWildcardSerial();
  }

  if (!isWildcardManufacturer() && (_filter._manufacturer != rhs._filter._manufacturer)) {
    return _filter._manufacturer < rhs._filter._manufacturer;
  }

  if (!isWildcardMeterType() && (_filter._meterType != rhs._filter._meterType)) {
    return _filter._meterType < rhs._filter._meterType;
  }

  if (!isWildcardSerial() && (_filter._serialNr != rhs._filter._serialNr)) {
    return _filter._serialNr < rhs._filter._serialNr;
  }
*/
  return false;
}

bool P094_filter::operator==(const P094_filter& rhs) const
{
  return equals(*this, rhs);
}

bool P094_filter::operator!=(const P094_filter& rhs) const
{
  return !equals(*this, rhs);
}

bool P094_filter::equals(const P094_filter& lhs, const P094_filter& rhs)
{
  if (!lhs.isValid() && !rhs.isValid()) { return true; }

  return lhs.toString() == rhs.toString();
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

unsigned long P094_filter::computeUnixTimeExpiration() const
{
  // Match the interval window.
  const P094_Filter_Window filterWindow = getFilterWindow();

  if ((filterWindow == P094_Filter_Window::None) ||
      (filterWindow == P094_Filter_Window::Once)) {
    // Return date infinitely far in the future
    return 0xFFFFFFFF;
  }

  if (filterWindow == P094_Filter_Window::All) {
    // Return timestamp in the past
    return 0;
  }

  // Using UnixTime
  const unsigned long currentTime = node_time.getUnixTime();
  unsigned long window_max        = currentTime;

  if ((filterWindow == P094_Filter_Window::One_hour) ||
      (filterWindow == P094_Filter_Window::Day) ||
      (filterWindow == P094_Filter_Window::Month))
  {
    // Create time struct in local time.
    struct tm tm_max;
    breakTime(time_zone.toLocal(currentTime), tm_max);
    tm_max.tm_sec = 59;
    tm_max.tm_min = 59;

    if (filterWindow == P094_Filter_Window::Day) {
      // Using local time, thus incl. timezone and DST.
      if (tm_max.tm_hour < 23) {
        // Either:
        // - between 00:00 and 12:00 => Max: 11:59:59
        // - between 12:00 and 23:00 => Max: 22:59:59

        tm_max.tm_hour = (tm_max.tm_hour < 12) ? 11 : 22;
      } else {
        // between 23:00 and 00:00 => Max: 23:59:59
        tm_max.tm_hour = 23;
      }
    } else if (filterWindow == P094_Filter_Window::Month) {
      // First set minute to midnight of today => Max: 23:59:59
      tm_max.tm_hour = 23;

      if (tm_max.tm_mday < 15) {
        // - between 1st of month 00:00:00 and 15th of month 00:00:00
        tm_max.tm_mday = 14;
      } else {
        // Check if this is the last day of the month.
        // Add 24h to the time and see if it is still the same month.
        const uint8_t maxMonthDay = getMonthDays(tm_max);

        if (tm_max.tm_mday < maxMonthDay) {
          // - between 15th of month 00:00:00 and last of month 00:00:00
          // So we must subtract one day.
          tm_max.tm_mday = maxMonthDay - 1;
        } else {
          // - between last of month 00:00:00 and 1st of next month 00:00:00
          // Thus do not change the date as it is already at the last day of the month
        }
      }
    }

    // Convert from local time.
    window_max = time_zone.fromLocal(makeTime(tm_max));
  } else {
    switch (filterWindow) {
      case P094_Filter_Window::One_minute:
        window_max = currentTime - (currentTime % (1 * 60)) + (1 * 60 - 1);
        break;
      case P094_Filter_Window::Five_minutes:
        window_max = currentTime - (currentTime % (5 * 60)) + (5 * 60 - 1);
        break;
      case P094_Filter_Window::Fifteen_minutes:
        window_max = currentTime - (currentTime % (15 * 60)) + (15 * 60 - 1);
        break;

      default:
        break;
    }
  }
  return window_max;
}

void P094_filter::WebformLoad(uint8_t filterIndex) const
{
  addRowLabel_tr_id(
    concat(F("Filter "), static_cast<int>(filterIndex + 1)),
    P094_FILTER_WEBARG_LABEL(filterIndex));

  // Manufacturer
  addTextBox(
    P094_FILTER_WEBARG_MANUFACTURER(filterIndex),
    getManufacturer(),
    3, false, false, EMPTY_STRING, F("widenumber")
# if FEATURE_TOOLTIPS
    , F("Manufacturer")
# endif // if FEATURE_TOOLTIPS
    );

  // Meter Type
  addTextBox(
    P094_FILTER_WEBARG_METERTYPE(filterIndex),
    getMeterType(),
    4, false, false, EMPTY_STRING, F("widenumber")
# if FEATURE_TOOLTIPS
    , F("Meter Type (HEX)")
# endif // if FEATURE_TOOLTIPS
    );

  // Serial nr
  addTextBox(
    P094_FILTER_WEBARG_SERIAL(filterIndex),
    getSerial(),
    10, false, false, EMPTY_STRING, F("widenumber")
# if FEATURE_TOOLTIPS
    , F("Serial (HEX)")
# endif // if FEATURE_TOOLTIPS
    );

  {
    // Filter Window
    const int optionValues[] = {
      static_cast<int>(P094_Filter_Window::All),
      static_cast<int>(P094_Filter_Window::One_minute),
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

    for (size_t i = 0; i < nrOptions; ++i) {
      const P094_Filter_Window filterWindow = static_cast<P094_Filter_Window>(optionValues[i]);
      options[i] = Filter_WindowToString(filterWindow);
    }
    addSelector(P094_FILTER_WEBARG_FILTER_WINDOW(filterIndex),
                nrOptions,
                options,
                optionValues,
                nullptr,
                _filter._filterWindow,
                false,
                true,
                F("widenumber")
# if FEATURE_TOOLTIPS
                , F("Filter Window")
# endif // if FEATURE_TOOLTIPS
                );
  }
}

String P094_WebformSave_GetWebArg(const String& id) {
  String webarg_str = webArg(id);

  if (webarg_str.isEmpty()) {
    webarg_str = '*';
  }
  return webarg_str;
}

bool P094_filter::WebformSave(uint8_t filterIndex)
{
  String filterString;

  // Manufacturer
  filterString += P094_WebformSave_GetWebArg(P094_FILTER_WEBARG_MANUFACTURER(filterIndex));
  filterString += '.';

  // Meter Type
  filterString += P094_WebformSave_GetWebArg(P094_FILTER_WEBARG_METERTYPE(filterIndex));
  filterString += '.';

  // Serial nr
  filterString += P094_WebformSave_GetWebArg(P094_FILTER_WEBARG_SERIAL(filterIndex));

  fromString(filterString);

  // Filter Window
  _filter._filterWindow = getFormItemInt(
    P094_FILTER_WEBARG_FILTER_WINDOW(filterIndex),
    0);

  return isValid();
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

String P094_filter::getMeterType() const
{
  String metertype;

  if (isWildcardMeterType()) {
    metertype = '*';
  } else {
    metertype = formatToHex_no_prefix(_filter._meterType, 2);
  }
  return metertype;
}

String P094_filter::getSerial() const
{
  String serial;

  if (isWildcardSerial()) {
    serial = '*';
  } else {
    serial = formatToHex_no_prefix(_filter._serialNr, 8);
  }
  return serial;
}

P094_Filter_Window P094_filter::getFilterWindow() const
{
  return static_cast<P094_Filter_Window>(_filter._filterWindow);
}


#endif // ifdef USES_P094