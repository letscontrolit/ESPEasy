#include "rn2xx3_helper.h"


bool rn2xx3_helper::valid_hex_char(char ch)
{
  return (ch >= '0' && ch <= '9') || (ch >= 'A' && ch <= 'F') || (ch >= 'a' && ch <= 'f');
}

bool rn2xx3_helper::valid_char(char ch)
{
  switch (ch)
  {
    case '\n':
    case '\r':
    case ' ':
      return true;
  }
  return ch > 32 && ch < 127;
}

bool rn2xx3_helper::isHexStr(const String& str)
{
  const size_t strlength = str.length();

  for (size_t i = 0; i < strlength; ++i) {
    const char ch = str[i];

    if (!rn2xx3_helper::valid_hex_char(ch))
    {
      return false;
    }
  }
  return true;
}

bool rn2xx3_helper::isHexStr_of_length(const String& str, size_t length)
{
  if (str.length() != length) { return false; }
  return isHexStr(str);
}

String rn2xx3_helper::base16decode(const String& input_c)
{
  if (!isHexStr(input_c)) { return ""; }
  String input(input_c); // Make a deep copy to be able to do trim()
  input.trim();
  const size_t inputLength  = input.length();
  const size_t outputLength = inputLength / 2;
  String output;
  output.reserve(outputLength);

  for (size_t i = 0; i < outputLength; ++i)
  {
    char toDo[3];
    toDo[0] = input[i * 2];
    toDo[1] = input[i * 2 + 1];
    toDo[2] = '\0';
    unsigned long out = strtoul(toDo, 0, 16);

    if (out <= 0xFF)
    {
      output += char(out & 0xFF);
    }
  }
  return output;
}

String rn2xx3_helper::base16encode(const String& input_c)
{
  String input(input_c); // Make a deep copy to be able to do trim()

  input.trim();
  const size_t inputLength = input.length();
  String output;
  output.reserve(inputLength * 2);

  for (size_t i = 0; i < inputLength; ++i)
  {
    if (input[i] == '\0') { break; }

    char buffer[3];
    sprintf_P(buffer, PSTR("%02x"), static_cast<int>(input[i]));
    output += buffer[0];
    output += buffer[1];
  }
  return output;
}

String rn2xx3_helper::base16encode(const byte *data, uint8_t size)
{
  String dataToTx;

  dataToTx.reserve(size * 2);
  char buffer[3];

  for (unsigned i = 0; i < size; i++)
  {
    sprintf_P(buffer, PSTR("%02X"), data[i]);
    dataToTx += buffer[0];
    dataToTx += buffer[1];
  }
  return dataToTx;
}
