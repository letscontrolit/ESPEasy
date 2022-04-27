#ifndef GLOBALS_SETTINGS_H
#define GLOBALS_SETTINGS_H

#include "../DataStructs/SettingsStruct.h"

// include the source file since it is a template class.
// Moved to a separate folder to allow concatenating all *.cpp files 
// in the DataStructs folder to make Windows builds work again.
#include "../DataStructs_templ/SettingsStruct.cpp"

extern SettingsStruct Settings;


#endif // GLOBALS_SETTINGS_H
