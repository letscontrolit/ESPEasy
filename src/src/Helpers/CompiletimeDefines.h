#ifndef HELPERS_COMPILETIMEDEFINES_H
#define HELPERS_COMPILETIMEDEFINES_H

#include <Arduino.h>

String get_build_filename();
String get_build_time();
String get_build_date();
bool official_build();


#endif // HELPERS_COMPILETIMEDEFINES_H