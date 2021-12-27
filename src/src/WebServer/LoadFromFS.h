#ifndef WEBSERVER_WEBSERVER_LOADFROMFS_H
#define WEBSERVER_WEBSERVER_LOADFROMFS_H

#include "../WebServer/common.h"


bool loadFromFS(String path);


// Send the content of a file directly to the webserver, like addHtml()
// Return is nr bytes streamed.
size_t streamFromFS(String path, bool htmlEscape = false);

#endif