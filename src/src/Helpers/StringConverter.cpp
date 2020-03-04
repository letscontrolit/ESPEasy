#include "StringConverter.h"




// ********************************************************************************
// URNEncode char string to string object
// ********************************************************************************
String URLEncode(const char *msg)
{
  const char *hex   = "0123456789abcdef";
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
    if (s.indexOf(key) == -1) return;
    s.replace(key, URLEncode(val.c_str()));
  } else {
    s.replace(key, val);
  }
}