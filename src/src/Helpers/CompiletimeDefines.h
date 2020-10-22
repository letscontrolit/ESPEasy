#ifndef HELPERS_COMPILETIMEDEFINES_H
#define HELPERS_COMPILETIMEDEFINES_H

#include <Arduino.h>

//#include "../../ESPEasy_common.h"

String get_binary_filename();
String get_build_time();
String get_build_date();
String get_build_origin();
String get_build_platform();
String get_git_head();


#endif // HELPERS_COMPILETIMEDEFINES_H
