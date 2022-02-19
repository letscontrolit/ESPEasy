#include "../PluginStructs/P133_data_struct.h"

#ifdef USES_P133


P133_data_struct::P133_data_struct(int8_t _CS, int8_t _DC, int8_t _RST, int16_t _W, int16_t _H)
  : tft(_W, _H) 
  {
    tft.begin();
  }

//Print some text
//param [in] string : The text to display
//param [in] X : The left position (X)
//param [in] Y : The top position (Y)
//param [in] textSize : The text size (default 1)
//param [in] color : The fore color (default TFT_WHITE)
//param [in] bkcolor : The background color (default TFT_BLACK)
void P133_data_struct::printText(const String& string, int X, int Y, unsigned int textSize, unsigned short color, unsigned short bkcolor)
{
  tft.setTextColor(color, bkcolor);
  tft.setTextSize(textSize);
  tft.drawString(string, X, Y);
}


//Parse color string to TFT_eSPI color
//param [in] s : The color string (white, red, ...)
//return : color (default TFT_WHITE)
unsigned short P133_data_struct::ParseColor(String & s)
{
  if (s.equalsIgnoreCase(F("BLACK")))
    return TFT_BLACK;
  if (s.equalsIgnoreCase(F("NAVY")))
    return TFT_NAVY;
  if (s.equalsIgnoreCase(F("DARKGREEN")))
    return TFT_DARKGREEN;
  if (s.equalsIgnoreCase(F("DARKCYAN")))
    return TFT_DARKCYAN;
  if (s.equalsIgnoreCase(F("MAROON")))
    return TFT_MAROON;
  if (s.equalsIgnoreCase(F("PURPLE")))
    return TFT_PURPLE;
  if (s.equalsIgnoreCase(F("OLIVE")))
    return TFT_OLIVE;
  if (s.equalsIgnoreCase(F("LIGHTGREY")))
    return TFT_LIGHTGREY;
  if (s.equalsIgnoreCase(F("DARKGREY")))
    return TFT_DARKGREY;
  if (s.equalsIgnoreCase(F("BLUE")))
    return TFT_BLUE;
  if (s.equalsIgnoreCase(F("GREEN")))
    return TFT_GREEN;
  if (s.equalsIgnoreCase(F("CYAN")))
    return TFT_CYAN;
  if (s.equalsIgnoreCase(F("RED")))
    return TFT_RED;
  if (s.equalsIgnoreCase(F("MAGENTA")))
    return TFT_MAGENTA;
  if (s.equalsIgnoreCase(F("YELLOW")))
    return TFT_YELLOW;
  if (s.equalsIgnoreCase(F("WHITE")))
    return TFT_WHITE;
  if (s.equalsIgnoreCase(F("ORANGE")))
    return TFT_ORANGE;
  if (s.equalsIgnoreCase(F("GREENYELLOW")))
    return TFT_GREENYELLOW;
  if (s.equalsIgnoreCase(F("PINK")))
    return TFT_PINK;

  if(s.length() == 7 && s[0] == '#')
  {
    // convrt to long value in base16, then split up into r, g, b values
    long long number = strtoll( &s[1], nullptr, 16);
    //long long r = number >> 16;
    //long long g = number >> 8 & 0xFF;
    //long long b = number & 0xFF;
    //convert to color565 (used by adafruit lib)
    return tft.color565(number >> 16, number >> 8 & 0xFF, number & 0xFF);
  }
  return TFT_WHITE; //fallback value
}

//Split a string by delimiter
//param [in] s : The input string
//param [in] c : The delimiter
//param [out] op : The resulting string array
//param [in] limit : The maximum strings to find
//return : The string count
int P133_data_struct::StringSplit(String &s, char c, String op[], int limit)
{
  int count = 0;
  char * pch;
  String d = String(c);
  pch = strtok ((char*)(s.c_str()),d.c_str());
  while (pch != nullptr && count < limit)
  {
    op[count] = String(pch);
    count++;
    pch = strtok (nullptr, ",");
  }
  return count;
}

#endif // ifdef USES_P133
