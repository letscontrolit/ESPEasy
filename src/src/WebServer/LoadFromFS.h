#ifndef WEBSERVER_WEBSERVER_LOADFROMFS_H
#define WEBSERVER_WEBSERVER_LOADFROMFS_H

#include "../WebServer/common.h"


bool loadFromFS(String path);


// Send the content of a file directly to the webserver, like addHtml()
bool streamFromFS(String path);

#endif