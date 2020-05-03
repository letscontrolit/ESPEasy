#ifndef HELPERS_COMPILETIMEDEFINES_H
#define HELPERS_COMPILETIMEDEFINES_H

#include <Arduino.h>

String get_binary_filename();
String get_build_time();
String get_build_date();
bool   official_build();
String get_build_platform();
String get_git_head();


#endif // HELPERS_COMPILETIMEDEFINES_H
