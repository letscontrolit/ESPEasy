#include "../WebServer/Log.h"

#include "../WebServer/ESPEasy_WebServer.h"
#include "../WebServer/404.h"
#include "../WebServer/HTML_wrappers.h"
#include "../WebServer/JSON.h"
#include "../WebServer/Markup.h"
#include "../WebServer/Markup_Buttons.h"
#include "../WebServer/Markup_Forms.h"

#include "../DataStructs/LogBuffer.h"
#include "../DataStructs/TimingStats.h"

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
  #ifdef WEBSERVER_GITHUB_COPY
  addCopyButton(F("copyText"), EMPTY_STRING, F("Copy log to clipboard"));
  #endif
  addHtml(F(
            "</TR></table><div  id='current_loglevel' style='font-weight: bold;'>Logging: </div><div class='logviewer' id='copyText_1'></div>"));
  addHtml(F("Autoscroll: "));
  addCheckBox(F("autoscroll"), true);
  addFormTextBox(F("Filter"), F("logfilter"), "", 30);
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
  START_TIMER;
  TXBuffer.startJsonStream();
  {
    KeyValueWriter_JSON top(true);
    {
      String webrequest = webArg(F("view"));

      auto mainWriter = top.createChild(F("Log"));

      if (mainWriter) {

        if (equals(webrequest, F("legend"))) {

          auto legendWriter = mainWriter->createChildArray(F("Legend"));

          if (legendWriter) {
            for (uint8_t i = LOG_LEVEL_ERROR; i < LOG_LEVEL_NRELEMENTS(); ++i) {
              auto loglevelWriter = legendWriter->createChild();

              if (loglevelWriter) {
                int loglevel;
                loglevelWriter->write({ F("label"), getLogLevelDisplayStringFromIndex(i, loglevel) });
                loglevelWriter->write({ F("loglevel"), loglevel });
              }
            }
          }
        }
        uint32_t firstTimeStamp = 0;
        uint32_t lastTimeStamp  = 0;
        int nrEntries           = 0;

        bool logLinesAvailable = true;
        {
          auto entriesWriter = mainWriter->createChildArray(F("Entries"));

          if (entriesWriter) {
            uint32_t startTime = millis();

            while (logLinesAvailable && timePassedSince(startTime) < 200) {
              String  message;
              uint8_t loglevel;

              if (Logging.getNext(LOG_TO_WEBLOG, lastTimeStamp, message, loglevel)) {
                auto logWriter = entriesWriter->createChild();

                if (logWriter) {
                  logWriter->write({ F("timestamp"), format_msec_duration(lastTimeStamp) });
                  logWriter->write({ F("text"),      std::move(message) });
                  logWriter->write({ F("level"), loglevel });

                  if (nrEntries == 0) {
                    firstTimeStamp = lastTimeStamp;
                  }
                  ++nrEntries;
                }

                // Do we need to do something here and maybe limit number of lines at once?
              } else { logLinesAvailable = false; }
            }
          }
        }
        const uint32_t nrEntriesLeft = Logging.getNrMessages(LOG_TO_WEBLOG);
        int32_t logTimeSpan       = timeDiff(firstTimeStamp, lastTimeStamp);
        int32_t refreshSuggestion = (nrEntriesLeft > 0) ? 200 : 1000;
        int32_t newOptimum        = 1000;


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
        mainWriter->write({ F("TTL"),                 refreshSuggestion });
        mainWriter->write({ F("timeHalfBuffer"),      newOptimum });
        mainWriter->write({ F("nrEntries"),           nrEntries });
        mainWriter->write({ F("SettingsWebLogLevel"), Settings.WebLogLevel });
        mainWriter->write({ F("logTimeSpan"),         logTimeSpan });
      }
    }
  }
  TXBuffer.endStream();
  STOP_TIMER(HANDLE_SERVING_WEBPAGE_JSON);
  updateLogLevelCache();

  #else // ifdef WEBSERVER_LOG
  handleNotFound();
  #endif // ifdef WEBSERVER_LOG
}
