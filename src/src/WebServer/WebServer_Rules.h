#ifndef WEBSERVER_RULES_H
#define WEBSERVER_RULES_H

#include <functional>
typedef struct s_fileinfo
{
  String Name;
  int    Size        = 0;
  bool   isDirectory = false;
} fileInfo;

typedef std::function<bool (fileInfo)> HandlerFileInfo;


#endif // WEBSERVER_RULES_H
