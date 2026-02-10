#include "../Helpers/SyslogWriter.h"

#if FEATURE_SYSLOG


#include "../../ESPEasy/net/ESPEasyNetwork.h"
#include "../../ESPEasy/net/Globals/NetworkState.h"
#include "../Globals/ESPEasy_time.h"
#include "../Globals/Settings.h"
#include "../Helpers/ESPEasy_time_calc.h"
#include "../Helpers/Networking.h"
#include "../Helpers/StringConverter.h"

#define MAX_LENGTH_SYSLOG_MESSAGE  1000

bool SyslogWriter::process()
{
  if ((Settings.SyslogLevel == 0) || (getNrMessages() == 0)) {
    return false;
  }

  bool somethingWritten{};

  if ((Settings.Syslog_IP[0] != 0) && ESPEasy::net::NetworkConnected())
  {
    WiFiUDP syslogUDP;

    if (!beginWiFiUDP_randomPort(syslogUDP)) {
      return false;
    }

    const IPAddress syslogIP(Settings.Syslog_IP[0], Settings.Syslog_IP[1], Settings.Syslog_IP[2], Settings.Syslog_IP[3]);
    const uint32_t  start = millis();
    bool done             = false;

    // Only try to process syslog messages for a limited amount of time to keep ESPEasy responsive
    while (!done && timePassedSince(start) < 200) {
      FeedSW_watchdog();

      if (syslogUDP.beginPacket(syslogIP, Settings.SyslogPort) == 0) {
        // Could not create socket
        return somethingWritten;
      }

      if (write_single_item(syslogUDP, MAX_LENGTH_SYSLOG_MESSAGE) != 0) {
        somethingWritten = true;
      } else { done = true; }

      syslogUDP.endPacket();
      FeedSW_watchdog();
      delay(0);
    }
  }
  return somethingWritten;
}

void SyslogWriter::prepare_prefix()
{
  unsigned int prio = Settings.SyslogFacility * 8;

  if (_loglevel == LOG_LEVEL_ERROR) {
    prio += 3; // syslog error
  }
  else if (_loglevel == LOG_LEVEL_INFO) {
    prio += 5; // syslog notice
  }
  else {
    prio += 7;
  }

  // An RFC3164 compliant message must be formated like:
  //   "<PRIO>[TimeStamp ]Hostname TaskName: Message"

  // See: https://www.rfc-editor.org/rfc/rfc3164

  /*
     The TIMESTAMP field is the local time and is in the format of
     "Mmm dd hh:mm:ss" (without the quote marks) where:

       Mmm is the English language abbreviation for the month of the
       year with the first character in uppercase and the other two
       characters in lowercase.  The following are the only acceptable
       values:

       Jan, Feb, Mar, Apr, May, Jun, Jul, Aug, Sep, Oct, Nov, Dec

       dd is the day of the month.  If the day of the month is less
       than 10, then it MUST be represented as a space and then the
       number.  For example, the 7th day of August would be
       represented as "Aug  7", with two spaces between the "g" and
       the "7".



     Lonvick                      Informational                     [Page 10]

     RFC 3164                The BSD syslog Protocol              August 2001


       hh:mm:ss is the local time.  The hour (hh) is represented in a
       24-hour format.  Valid entries are between 00 and 23,
       inclusive.  The minute (mm) and second (ss) entries are between
       00 and 59 inclusive.

     A single space character MUST follow the TIMESTAMP field.
   */

  // Using Settings.Name as the Hostname (Hostname must NOT contain space)

  String formattedTimestamp;

  if (statusNTPInitialized) {
    // Calculate the local time retroactively from the log timestamps
    struct tm ts;
    uint32_t  unix_time_frac{};
    const uint32_t local_timestamp = node_time.systemMicros_to_Localtime(
      node_time.internalTimestamp_to_systemMicros(_timestamp),
      unix_time_frac);
    breakTime(local_timestamp, ts);

    // A single space character MUST follow the TIMESTAMP field.
    formattedTimestamp = strformat(
      F("%s %2d %02d:%02d:%02d "),
      ESPEasy_time::month_str(ts.tm_mon).c_str(),
      ts.tm_mday,
      ts.tm_hour, ts.tm_min, ts.tm_sec);
  }

  String hostname(ESPEasy::net::NetworkCreateRFCCompliantHostname(true));
  hostname.replace(' ', '_');
  hostname.trim();

  _prefix = strformat(
    F("<%d>%s%s EspEasy: "),
    prio,
    formattedTimestamp.c_str(),
    hostname.c_str());
}
#endif