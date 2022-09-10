#ifndef HELPERS_STRINGGENERATOR_PLUGIN_H
#define HELPERS_STRINGGENERATOR_PLUGIN_H

#include <Arduino.h>


// Generate string with appending nr (when > 0)
// When NOT used as displaystring, it will be converted to lower case.
String Plugin_valuename(
    const __FlashStringHelper * name_prefix, 
    uint8_t value_nr, 
    bool displayString);


#endif