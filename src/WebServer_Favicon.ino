
#include "src/Static/WebStaticData.h"

void handle_favicon() {
  checkRAM(F("handle_favicon"));
  WebServer.send_P(200, PSTR("image/x-icon"), favicon_8b_ico, favicon_8b_ico_len);
}
