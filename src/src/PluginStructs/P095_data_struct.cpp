#include "../PluginStructs/P095_data_struct.h"

#ifdef USES_P095

/****************************************************************************
 * ILI9xxx_type_toString: Display-value for the device selected
 ***************************************************************************/
const __FlashStringHelper* ILI9xxx_type_toString(ILI9xxx_type_e device) {
  switch (device) {
    case ILI9xxx_type_e::ILI9341_240x320: return F("ILI9341 240 x 320px");
    case ILI9xxx_type_e::ILI9342_240x320: return F("ILI9342 240 x 320px (M5Stack)");
    case ILI9xxx_type_e::ILI9481_320x480: return F("ILI9481 320 x 480px");
    case ILI9xxx_type_e::ILI9481_CPT29_320x480: return F("ILI9481 320 x 480px (CPT29)");
    case ILI9xxx_type_e::ILI9481_PVI35_320x480: return F("ILI9481 320 x 480px (PVI35)");
    case ILI9xxx_type_e::ILI9481_AUO317_320x480: return F("ILI9481 320 x 480px (AUO317)");
    case ILI9xxx_type_e::ILI9481_CMO35_320x480: return F("ILI9481 320 x 480px (CMO35)");
    case ILI9xxx_type_e::ILI9481_RGB_320x480: return F("ILI9481 320 x 480px (RGB)");
    case ILI9xxx_type_e::ILI9486_320x480: return F("ILI9486 320 x 480px");
    case ILI9xxx_type_e::ILI9488_320x480: return F("ILI9488 320 x 480px");
    case ILI9xxx_type_e::ILI9xxx_MAX: break;
  }
  return F("Unsupported type!");
}

/****************************************************************************
 * ILI9xxx_type_toResolution: X and Y resolution for the selected type
 ***************************************************************************/
void ILI9xxx_type_toResolution(ILI9xxx_type_e device, uint16_t& x, uint16_t& y) {
  switch (device) {
    case ILI9xxx_type_e::ILI9341_240x320:
    case ILI9xxx_type_e::ILI9342_240x320:
      x = 240;
      y = 320;
      break;
    case ILI9xxx_type_e::ILI9481_320x480:
    case ILI9xxx_type_e::ILI9481_CPT29_320x480:
    case ILI9xxx_type_e::ILI9481_PVI35_320x480:
    case ILI9xxx_type_e::ILI9481_AUO317_320x480:
    case ILI9xxx_type_e::ILI9481_CMO35_320x480:
    case ILI9xxx_type_e::ILI9481_RGB_320x480:
    case ILI9xxx_type_e::ILI9486_320x480:
    case ILI9xxx_type_e::ILI9488_320x480:
      x = 320;
      y = 480;
      break;
    case ILI9xxx_type_e::ILI9xxx_MAX:
      break;
  }
}

/****************************************************************************
 * Constructor
 ***************************************************************************/
P095_data_struct::P095_data_struct(ILI9xxx_type_e displayType,
                                   int8_t _CS, int8_t _DC, int8_t _RST)
  : _displayType(displayType) // , tft(_CS,  _DC, _RST)
{
  _xpix = 240;
  _ypix = 320;
  ILI9xxx_type_toResolution(_displayType, _xpix, _ypix);

  tft = new (std::nothrow) Adafruit_ILI9341(_CS, _DC, _RST, static_cast<uint8_t>(_displayType), _xpix, _ypix);

  if (nullptr != tft) {
    tft->begin();
  }
}

/****************************************************************************
 * Destructor
 ***************************************************************************/
P095_data_struct::~P095_data_struct() {
  if (nullptr != tft) {
    delete tft;
    tft = nullptr;
  }
}

// Print some text
// param [in] string : The text to display
// param [in] X : The left position (X)
// param [in] Y : The top position (Y)
// param [in] textSize : The text size (default 1)
// param [in] color : The fore color (default ILI9341_WHITE)
// param [in] bkcolor : The background color (default ILI9341_BLACK)
void P095_data_struct::printText(const String& string, int X, int Y, unsigned int textSize, unsigned short color, unsigned short bkcolor)
{
  tft->setCursor(X, Y);
  tft->setTextColor(color, bkcolor);
  tft->setTextSize(textSize);
  tft->println(string);
}

// Parse color string to ILI9341 color
// param [in] s : The color string (white, red, ...)
// return : color (default ILI9341_WHITE)
unsigned short P095_data_struct::ParseColor(String& s)
{
  if (s.equalsIgnoreCase(F("BLACK"))) {
    return ILI9341_BLACK;
  }

  if (s.equalsIgnoreCase(F("NAVY"))) {
    return ILI9341_NAVY;
  }

  if (s.equalsIgnoreCase(F("DARKGREEN"))) {
    return ILI9341_DARKGREEN;
  }

  if (s.equalsIgnoreCase(F("DARKCYAN"))) {
    return ILI9341_DARKCYAN;
  }

  if (s.equalsIgnoreCase(F("MAROON"))) {
    return ILI9341_MAROON;
  }

  if (s.equalsIgnoreCase(F("PURPLE"))) {
    return ILI9341_PURPLE;
  }

  if (s.equalsIgnoreCase(F("OLIVE"))) {
    return ILI9341_OLIVE;
  }

  if (s.equalsIgnoreCase(F("LIGHTGREY"))) {
    return ILI9341_LIGHTGREY;
  }

  if (s.equalsIgnoreCase(F("DARKGREY"))) {
    return ILI9341_DARKGREY;
  }

  if (s.equalsIgnoreCase(F("BLUE"))) {
    return ILI9341_BLUE;
  }

  if (s.equalsIgnoreCase(F("GREEN"))) {
    return ILI9341_GREEN;
  }

  if (s.equalsIgnoreCase(F("CYAN"))) {
    return ILI9341_CYAN;
  }

  if (s.equalsIgnoreCase(F("RED"))) {
    return ILI9341_RED;
  }

  if (s.equalsIgnoreCase(F("MAGENTA"))) {
    return ILI9341_MAGENTA;
  }

  if (s.equalsIgnoreCase(F("YELLOW"))) {
    return ILI9341_YELLOW;
  }

  if (s.equalsIgnoreCase(F("WHITE"))) {
    return ILI9341_WHITE;
  }

  if (s.equalsIgnoreCase(F("ORANGE"))) {
    return ILI9341_ORANGE;
  }

  if (s.equalsIgnoreCase(F("GREENYELLOW"))) {
    return ILI9341_GREENYELLOW;
  }

  if (s.equalsIgnoreCase(F("PINK"))) {
    return ILI9341_PINK;
  }

  if ((s.length() == 7) && (s[0] == '#'))
  {
    // convrt to long value in base16, then split up into r, g, b values
    long long number = strtoll(&s[1], NULL, 16);

    // long long r = number >> 16;
    // long long g = number >> 8 & 0xFF;
    // long long b = number & 0xFF;
    // convert to color565 (used by adafruit lib)
    return tft->color565(number >> 16, number >> 8 & 0xFF, number & 0xFF);
  }
  return ILI9341_WHITE; // fallback value
}

// Split a string by delimiter
// param [in] s : The input string
// param [in] c : The delimiter
// param [out] op : The resulting string array
// param [in] limit : The maximum strings to find
// return : The string count
int P095_data_struct::StringSplit(String& s, char c, String op[], int limit)
{
  int    count = 0;
  char  *pch;
  String d = String(c);

  pch = strtok((char *)(s.c_str()), d.c_str());

  while (pch != NULL && count < limit)
  {
    op[count] = String(pch);
    count++;
    pch = strtok(NULL, ",");
  }
  return count;
}

#endif // ifdef USES_P095
