#ifndef WEBSERVER_WEBSERVER_PINSTATES_H
#define WEBSERVER_WEBSERVER_PINSTATES_H

#include "../WebServer/common.h"


#ifdef WEBSERVER_NEW_UI


// ********************************************************************************
// Web Interface pin state list
// ********************************************************************************
void handle_pinstates_json();

#endif // WEBSERVER_NEW_UI

#ifdef WEBSERVER_PINSTATES

void handle_pinstates();

#endif // ifdef WEBSERVER_PINSTATES


#endif