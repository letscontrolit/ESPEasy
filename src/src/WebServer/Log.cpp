#include "../WebServer/Log.h"

#include "../WebServer/WebServer.h"
#include "../WebServer/404.h"
#include "../WebServer/HTML_wrappers.h"
#include "../WebServer/JSON.h"
#include "../WebServer/Markup.h"
#include "../WebServer/Markup_Buttons.h"

#include "../DataStructs/LogStruct.h"

#include "../Globals/Logging.h"
#include "../Globals/Settings.h"
#include "../Helpers/ESPEasy_time_calc.h"
#include "../Static/WebStaticData.h"

// ********************************************************************************
// Web Interface log page
// ********************************************************************************
void handle_log() {
  if (!isLoggedIn()) { return; }
  navMenuIndex = MENU_INDEX_TOOLS;

  TXBuffer.startStream();
  sendHeadandTail_stdtemplate(_HEAD);
  html_table_class_normal();

  #ifdef WEBSERVER_LOG
  addHtml(F("<TR><TH id=\"headline\" align=\"left\">Log"));
  addCopyButton(F("copyText"), "", F("Copy log to clipboard"));
  addHtml(F("</TR></table><div  id='current_loglevel' style='font-weight: bold;'>Logging: </div><div class='logviewer' id='copyText_1'></div>"));
  addHtml(F("Autoscroll: "));
  addCheckBox(F("autoscroll"), true);
  addHtml(F("<BR></body>"));

  serve_JS(JSfiles_e::FetchAndParseLog);

  #else // ifdef WEBSERVER_LOG
  addHtml(F("Not included in build"));
  #endif // ifdef WEBSERVER_LOG
  sendHeadandTail_stdtemplate(_TAIL);
  TXBuffer.endStream();
}

// ********************************************************************************
// Web Interface JSON log page
// ********************************************************************************
void handle_log_JSON() {
  if (!isLoggedIn()) { return; }
  #ifdef WEBSERVER_LOG
  TXBuffer.startJsonStream();
  String webrequest = web_server.arg(F("view"));
  addHtml(F("{\"Log\": {"));

  if (webrequest == F("legend")) {
    addHtml(F("\"Legend\": ["));

    for (byte i = 0; i < LOG_LEVEL_NRELEMENTS; ++i) {
      if (i != 0) {
        addHtml(",");
      }
      addHtml('{');
      int loglevel;
      stream_next_json_object_value(F("label"), getLogLevelDisplayStringFromIndex(i, loglevel));
      stream_last_json_object_value(F("loglevel"), String(loglevel));
    }
    addHtml(F("],\n"));
  }
  addHtml(F("\"Entries\": ["));
  bool logLinesAvailable       = true;
  int  nrEntries               = 0;
  unsigned long firstTimeStamp = 0;
  unsigned long lastTimeStamp  = 0;

  while (logLinesAvailable) {
    String reply = Logging.get_logjson_formatted(logLinesAvailable, lastTimeStamp);

    if (reply.length() > 0) {
      addHtml(reply);

      if (nrEntries == 0) {
        firstTimeStamp = lastTimeStamp;
      }
      ++nrEntries;
    }

    // Do we need to do something here and maybe limit number of lines at once?
  }
  addHtml(F("],\n"));
  long logTimeSpan       = timeDiff(firstTimeStamp, lastTimeStamp);
  long refreshSuggestion = 1000;
  long newOptimum        = 1000;

  if ((nrEntries > 2) && (logTimeSpan > 1)) {
    // May need to lower the TTL for refresh when time needed
    // to fill half the log is lower than current TTL
    newOptimum = logTimeSpan * (LOG_STRUCT_MESSAGE_LINES / 2);
    newOptimum = newOptimum / (nrEntries - 1);
  }

  if (newOptimum < refreshSuggestion) { refreshSuggestion = newOptimum; }

  if (refreshSuggestion < 100) {
    // Reload times no lower than 100 msec.
    refreshSuggestion = 100;
  }
  stream_next_json_object_value(F("TTL"),                 String(refreshSuggestion));
  stream_next_json_object_value(F("timeHalfBuffer"),      String(newOptimum));
  stream_next_json_object_value(F("nrEntries"),           String(nrEntries));
  stream_next_json_object_value(F("SettingsWebLogLevel"), String(Settings.WebLogLevel));
  stream_last_json_object_value(F("logTimeSpan"), String(logTimeSpan));
  addHtml(F("}\n"));
  TXBuffer.endStream();
  updateLogLevelCache();

  #else // ifdef WEBSERVER_LOG
  handleNotFound();
  #endif // ifdef WEBSERVER_LOG
}
