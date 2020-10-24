#ifndef WEBSERVER_WEBSERVER_FACTORYRESETPAGE_H
#define WEBSERVER_WEBSERVER_FACTORYRESETPAGE_H

#include "../WebServer/common.h"


#ifdef WEBSERVER_FACTORY_RESET

#include "../Globals/ResetFactoryDefaultPref.h"

// ********************************************************************************
// Web Interface Factory Reset
// ********************************************************************************
void handle_factoryreset();

// ********************************************************************************
// Create pre-defined config selector
// ********************************************************************************
void addPreDefinedConfigSelector();

#ifdef WEBSERVER_NEW_UI
void handle_factoryreset_json();

#endif // WEBSERVER_NEW_UI

#endif // ifdef WEBSERVER_FACTORY_RESET


#endif