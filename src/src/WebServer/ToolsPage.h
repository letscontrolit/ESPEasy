#ifndef WEBSERVER_WEBSERVER_TOOLSPAGE_H
#define WEBSERVER_WEBSERVER_TOOLSPAGE_H

#include "../WebServer/common.h"


#ifdef WEBSERVER_TOOLS

# include "../Commands/InternalCommands.h"

// ********************************************************************************
// Web Interface Tools page
// ********************************************************************************
void handle_tools();

// ********************************************************************************
// Web Interface debug page
// ********************************************************************************
void addWideButtonPlusDescription(const String& url,
                                  const String& buttonText,
                                  const String& description);

#endif // ifdef WEBSERVER_TOOLS

#endif // ifndef WEBSERVER_WEBSERVER_TOOLSPAGE_H
