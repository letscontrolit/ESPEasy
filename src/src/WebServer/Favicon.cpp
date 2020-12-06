
#include "../WebServer/Favicon.h"

#include "../WebServer/WebServer.h"
#include "../WebServer/404.h"

#include "../Globals/RamTracker.h"


#include "../Static/WebStaticData.h"

void handle_favicon() {
  #ifdef WEBSERVER_FAVICON
  #ifndef BUILD_NO_RAM_TRACKER
  checkRAM(F("handle_favicon"));
  #endif
  web_server.send_P(200, PSTR("image/x-icon"), favicon_8b_ico, favicon_8b_ico_len);
  #else // ifdef WEBSERVER_FAVICON
  handleNotFound();
  #endif // ifdef WEBSERVER_FAVICON
}
