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

void handle_rules_backup();

void handle_rules_delete();

bool handle_rules_edit(const String& originalUri);

bool handle_rules_edit(String originalUri, bool isAddNew);

void Rule_showRuleTextArea(const String& fileName);

bool Rule_Download(const String& path);

void Goto_Rules_Root();

bool EnumerateFileAndDirectory(String          & rootPath
                               , int             skip
                               , HandlerFileInfo handler);


#endif

#endif 
