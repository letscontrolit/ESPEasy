#ifndef HELPERS_STRINGCONVERTER_H
#define HELPERS_STRINGCONVERTER_H

#include <Arduino.h>


String URLEncode(const char *msg);
void repl(const String& key, const String& val, String& s, boolean useURLencode);



#endif // HELPERS_STRINGCONVERTER_H