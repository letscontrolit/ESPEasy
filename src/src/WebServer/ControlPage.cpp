#include "../WebServer/ControlPage.h"


#ifdef WEBSERVER_CONTROL


# include "../WebServer/HTML_wrappers.h"
# include "../WebServer/WebServer.h"
# include "../Helpers/WebServer_commandHelper.h"

# include "../../ESPEasy-Globals.h"


// ********************************************************************************
// Web Interface control page (no password!)
// ********************************************************************************
void handle_control() {
  # ifndef BUILD_NO_RAM_TRACKER
  checkRAM(F("handle_control"));
  # endif // ifndef BUILD_NO_RAM_TRACKER

  String webrequest            = webArg(F("cmd"));
  HandledWebCommand_result res = handle_command_from_web(EventValueSource::Enum::VALUE_SOURCE_HTTP, webrequest);

  switch (res) {
    case HandledWebCommand_result::IP_not_allowed:
    case HandledWebCommand_result::NoCommand:
      return;
    case HandledWebCommand_result::CommandHandled:
    case HandledWebCommand_result::Unknown_or_restricted_command:
      break;
  }

  if (printToWebJSON) { // it may be set in PLUGIN_WRITE (SendStatus)
    TXBuffer.startJsonStream();
    addHtml(printWebString);
  } else {
    TXBuffer.startStream();
    addEncodedHtml(printWebString);
  }

  TXBuffer.endStream();

  printWebString = "";
  printToWeb     = false;
  printToWebJSON = false;
}

#endif // ifdef WEBSERVER_CONTROL
