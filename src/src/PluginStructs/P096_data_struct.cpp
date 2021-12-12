#include "../PluginStructs/P096_data_struct.h"

#ifdef USES_P096


P096_data_struct::P096_data_struct(int width, int height, int8_t DC, int8_t RST, int8_t CS, int8_t BUSY)
 : eInkScreen(width, height, DC, RST, CS, BUSY),
   plugin_096_sequence_in_progress(false)
{
  eInkScreen.begin();
  eInkScreen.clearBuffer();
}


//Print some text
//param [in] string : The text to display
//param [in] X : The left position (X)
//param [in] Y : The top position (Y)
//param [in] textSize : The text size (default 1)
//param [in] color : The fore color (default ILI9341_WHITE)
//param [in] bkcolor : The background color (default ILI9341_BLACK)
void P096_data_struct::printText(const char *string, int X, int Y, unsigned int textSize, unsigned short color, unsigned short bkcolor)
{
  eInkScreen.clearBuffer();
  eInkScreen.clearDisplay();
  eInkScreen.setCursor(X, Y);
  eInkScreen.setTextColor(color, bkcolor);
  eInkScreen.setTextSize(textSize);
  String fixString = string;
  FixText(fixString);
  eInkScreen.println(fixString);
  eInkScreen.display();
}

//Parse color string to color
//param [in] colorString : The color string (white, red, ...)
//return : color (default EPD_WHITE)
unsigned short P096_data_struct::ParseColor(const String & colorString)
{
  //copy to local var and ensure lowercase
  //this optimise the next equlaity checks
  String s = colorString;
  s.toLowerCase();

  if (s.equals(F("black")))
    return EPD_BLACK;
  if (s.equals(F("white")))
    return EPD_WHITE;
  if (s.equals(F("inverse")))
    return EPD_INVERSE;
  if (s.equals(F("red")))
    return EPD_RED;
  if (s.equals(F("dark")))
    return EPD_DARK;
  if (s.equals(F("light")))
    return EPD_LIGHT;
  return EPD_WHITE;
}

//Fix text with handling special characters (degrees and main monetary symbols)
//This is specific case for current AdafruitGfx standard fontused for eink screen
//param [in/out] s : The string to fix
void P096_data_struct::FixText(String & s)
{
  const char degree[3] = {0xc2, 0xb0, 0};  // Unicode degree symbol
  const char degree_eink[2] = {0xf7, 0};  // eink degree symbol
  s.replace(degree, degree_eink);
  s.replace(F("{D}"), degree_eink);
  s.replace(F("&deg;"), degree_eink);
  
  const char euro[4]  = { 0xe2, 0x82, 0xac, 0 }; // Unicode euro symbol
  const char euro_eink[2] = {0xED, 0};  // eink degree symbol
  s.replace(euro, euro_eink);
  s.replace(F("{E}"), euro_eink);
  s.replace(F("&euro;"), euro_eink);

  const char pound[3] = { 0xc2, 0xa3, 0 };       // Unicode pound symbol
  const char pound_eink[2] = {0x9C, 0};  // eink pound symbol
  s.replace(pound, pound_eink);
  s.replace(F("{P}"), pound_eink);
  s.replace(F("&pound;"), pound_eink);

  const char yen[3]   = { 0xc2, 0xa5, 0 };       // Unicode yen symbol
  const char yen_eink[2] = {0x9D, 0};  // eink yen symbol
  s.replace(yen, yen_eink);
  s.replace(F("{Y}"), yen_eink);
  s.replace(F("&yen;"), yen_eink);

  const char cent[3]   = { 0xc2, 0xa2, 0 };       // Unicode yen symbol
  const char cent_eink[2] = {0x9B, 0};  // eink cent symbol
  s.replace(cent, cent_eink);
  s.replace(F("{c}"), cent_eink);
  s.replace(F("&cent;"), cent_eink);

}

//Split a string by delimiter
//param [in] s : The input string
//param [in] c : The delimiter
//param [out] op : The resulting string array
//param [in] limit : The maximum strings to find
//return : The string count
int P096_data_struct::StringSplit(const String &s, char c, String op[], int limit)
{
  int count = 0;
  char * pch;
  String d = String(c);
  pch = strtok ((char*)(s.c_str()),d.c_str());
  while (pch != NULL && count < limit)
  {
    op[count] = String(pch);
    count++;
    pch = strtok (NULL, ",");
  }  
  return count;
}


#endif // ifdef USES_P096
