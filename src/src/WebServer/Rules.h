#ifndef WEBSERVER_WEBSERVER_RULES_H
#define WEBSERVER_WEBSERVER_RULES_H

#include "../WebServer/common.h"

#ifdef WEBSERVER_RULES

#include <functional>
typedef struct s_fileinfo
{
  String Name;
  int    Size        = 0;
  bool   isDirectory = false;
} fileInfo;

typedef std::function<bool (fileInfo)> HandlerFileInfo;


// ********************************************************************************
// Web Interface rules page
// ********************************************************************************
void handle_rules();

// ********************************************************************************
// Web Interface rules page  (NEW)
// ********************************************************************************
void handle_rules_new();

# ifdef WEBSERVER_NEW_RULES
void handle_rules_backup();
void handle_rules_delete();
#endif

bool handle_rules_edit(const String& originalUri);

void Goto_Rules_Root();


#endif

#endif 
