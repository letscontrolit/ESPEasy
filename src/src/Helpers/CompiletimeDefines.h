#ifndef HELPERS_COMPILETIMEDEFINES_H
#define HELPERS_COMPILETIMEDEFINES_H

#include <Arduino.h>

//#include "../../ESPEasy_common.h"

const __FlashStringHelper * get_binary_filename();
const __FlashStringHelper * get_build_time();
const __FlashStringHelper * get_build_date();
const __FlashStringHelper * get_build_origin();
const __FlashStringHelper * get_build_platform();
const __FlashStringHelper * get_git_head();


#endif // HELPERS_COMPILETIMEDEFINES_H
