#include "ESPEasy_common.h"

#ifdef USES_N001

// #######################################################################################################
// ########################### Notification Plugin 001: Email ############################################
// #######################################################################################################

/** Changelog:
 * 2024-04-06 tonhuisman: Add support for (also) supplying a custom subject when sending an email notification via the notify command
 *                        Log reply from mailserver at DEBUG level
 *                        Code improvements
 * 2022-12-29 tonhuisman: Add Date: field to email header to reduce spam score, see https://github.com/letscontrolit/ESPEasy/issues/3865
 * 2022-12-29 tonhuisman: Start changelog
 */

# define NPLUGIN_001
# define NPLUGIN_ID_001         1
# define NPLUGIN_NAME_001       "Email (SMTP)"

# define NPLUGIN_001_TIMEOUT 5000

# include "src/DataStructs/ESPEasy_EventStruct.h"
# include "src/DataStructs/NotificationSettingsStruct.h"
# include "src/ESPEasyCore/ESPEasy_Log.h"
# include "src/ESPEasyCore/ESPEasy_backgroundtasks.h"
# include "src/Globals/NPlugins.h"
# include "src/Globals/Settings.h"
# include "src/Helpers/ESPEasy_Storage.h"
# include "src/Helpers/ESPEasy_time_calc.h"
# include "src/Helpers/Networking.h"
# include "src/Helpers/StringGenerator_System.h"
# include "src/Helpers/StringParser.h"
# include "src/Helpers/_CPlugin_Helper.h" // safeReadStringUntil
# include "src/Helpers/_NPlugin_init.h"

# include <base64.h>

// Forward declaration
bool NPlugin_001_send(const NotificationSettingsStruct& notificationsettings,
                      const String                    & aSub,
                      String                          & aMesg);
bool NPlugin_001_Auth(WiFiClient  & client,
                      const String& user,
                      const String& pass);
bool NPlugin_001_MTA(WiFiClient  & client,
                     const String& aStr,
                     uint16_t      aWaitForPattern);
bool getNextMailAddress(const String& data,
                        String      & address,
                        int           index);


// The message body is included in event->String1
bool NPlugin_001(NPlugin::Function function, struct EventStruct *event, String& string)
{
  bool success = false;

  switch (function) {
    case NPlugin::Function::NPLUGIN_PROTOCOL_ADD:
    {
      Notification[++notificationCount].Number      = NPLUGIN_ID_001;
      Notification[notificationCount].usesMessaging = true;
      Notification[notificationCount].usesGPIO      = 0;
      break;
    }

    case NPlugin::Function::NPLUGIN_GET_DEVICENAME:
    {
      string = F(NPLUGIN_NAME_001);
      break;
    }

    // Edwin: NPlugin::Function::NPLUGIN_WRITE seems to be not implemented/not used yet? Disabled because its confusing now.
    // case NPlugin::Function::NPLUGIN_WRITE:
    //   {
    //     String log;
    //     String command = parseString(string, 1);
    //
    //     if (command == F("email"))
    //     {
    //       MakeNotificationSettings(NotificationSettings);
    //       LoadNotificationSettings(event->NotificationIndex, (uint8_t*)&NotificationSettings, sizeof(NotificationSettingsStruct));
    //       NPlugin_001_send(NotificationSettings.Domain, NotificationSettings.Receiver, NotificationSettings.Sender,
    // NotificationSettings.Subject, NotificationSettings.Body, NotificationSettings.Server, NotificationSettings.Port);
    //       success = true;
    //     }
    //     break;
    //   }

    case NPlugin::Function::NPLUGIN_NOTIFY:
    {
      MakeNotificationSettings(NotificationSettings);
      LoadNotificationSettings(event->NotificationIndex, (uint8_t *)&NotificationSettings, sizeof(NotificationSettingsStruct));
      NotificationSettings.validate();
      String subject = NotificationSettings.Subject;
      String body    = NotificationSettings.Body;

      if (!event->String1.isEmpty()) {
        body = event->String1;
      }

      if (!event->String2.isEmpty()) {
        subject = event->String2;
      }
      subject = parseTemplate(subject);
      body    = parseTemplate(body);
      NPlugin_001_send(NotificationSettings, subject, body);
      success = true;
      break;
    }

    default:
      break;
  }
  return success;
}

bool NPlugin_001_send(const NotificationSettingsStruct& notificationsettings, const String& aSub, String& aMesg)
{
  //  String& aDomain , String aTo, String aFrom, String aSub, String aMesg, String aHost, int aPort)
  bool myStatus = false;

  // Use WiFiClient class to create TCP connections
  WiFiClient client;

  # ifdef MUSTFIX_CLIENT_TIMEOUT_IN_SECONDS

  // See: https://github.com/espressif/arduino-esp32/pull/6676
  client.setTimeout((CONTROLLER_CLIENTTIMEOUT_MAX + 500) / 1000); // in seconds!!!!
  Client *pClient = &client;
  pClient->setTimeout(CONTROLLER_CLIENTTIMEOUT_MAX);
  # else // ifdef MUSTFIX_CLIENT_TIMEOUT_IN_SECONDS
  client.setTimeout(CONTROLLER_CLIENTTIMEOUT_MAX); // in msec as it should be!
  # endif // ifdef MUSTFIX_CLIENT_TIMEOUT_IN_SECONDS

  # ifndef BUILD_NO_DEBUG

  if (loglevelActiveFor(LOG_LEVEL_DEBUG)) {
    addLog(LOG_LEVEL_DEBUG, strformat(
             F("EMAIL: Connecting to %s:%d"),
             notificationsettings.Server,
             notificationsettings.Port));
  }
  # endif // ifndef BUILD_NO_DEBUG

  if (!connectClient(client, notificationsettings.Server, notificationsettings.Port, CONTROLLER_CLIENTTIMEOUT_DFLT)) {
    if (loglevelActiveFor(LOG_LEVEL_ERROR)) {
      addLog(LOG_LEVEL_ERROR, strformat(
               F("EMAIL: Error connecting to %s:%d"),
               notificationsettings.Server,
               notificationsettings.Port));
    }
    myStatus = false;
  } else {
    String mailheader = F(
      "From: $nodename <$emailfrom>\r\n"
      "To: $ato\r\n"
      "Subject: $subject\r\n"
      "Reply-To: $nodename <$emailfrom>\r\n"
      "Date: $date\r\n"
      "MIME-VERSION: 1.0\r\n"
      "Content-type: text/html; charset=UTF-8\r\n"
      "X-Mailer: EspEasy v$espeasyversion\r\n\r\n"
      );

    String email_address(notificationsettings.Sender);
    int    pos_less   = email_address.indexOf('<');
    String senderName = Settings.getHostname();

    if (pos_less > -1) {
      senderName = email_address.substring(0, pos_less);
      removeChar(senderName,    '"'); // Remove quotes
      email_address = email_address.substring(pos_less + 1);
      removeChar(email_address, '<');
      removeChar(email_address, '>');
      email_address.trim();
      senderName.trim();
    }

    mailheader.replace(F("$nodename"),       senderName);
    mailheader.replace(F("$emailfrom"),      email_address);
    mailheader.replace(F("$ato"),            notificationsettings.Receiver);
    mailheader.replace(F("$subject"),        aSub);
    String dateFmtHdr = F("%sysweekday_s%, %sysday_0% %sysmonth_s% %sysyear% %systime% %systzoffset%");
    mailheader.replace(F("$date"),           parseTemplate(dateFmtHdr));
    mailheader.replace(F("$espeasyversion"), getSystemBuildString());

    // Make sure to replace the char '\r' and not the string "\r"
    // See: https://github.com/letscontrolit/ESPEasy/issues/4967
    removeChar(aMesg, '\r');
    aMesg.replace(String('\n'), F("<br/>")); // re-write line breaks for Content-type: text/html

    // Wait for Client to Start Sending
    // The MTA Exchange
    while (true) {
      if (!NPlugin_001_MTA(client, EMPTY_STRING, 220)) { break; }

      if (!NPlugin_001_MTA(client, strformat(F("EHLO %s"), notificationsettings.Domain), 250)) { break; }

      if (!NPlugin_001_Auth(client, notificationsettings.User, notificationsettings.Pass)) { break; }

      if (!NPlugin_001_MTA(client, strformat(F("MAIL FROM:<%s>"), email_address.c_str()), 250)) { break; }

      bool   nextAddressAvailable = true;
      int    i                    = 0;
      String emailTo;
      const String receiver(notificationsettings.Receiver);

      if (!getNextMailAddress(receiver, emailTo, i)) {
        addLog(LOG_LEVEL_ERROR, F("Email: No recipient given"));
        break;
      }

      while (nextAddressAvailable) {
        if (loglevelActiveFor(LOG_LEVEL_INFO)) {
          addLogMove(LOG_LEVEL_INFO, concat(F("Email: To "), emailTo));
        }

        if (!NPlugin_001_MTA(client, strformat(F("RCPT TO:<%s>"), emailTo.c_str()), 250)) { break; }
        ++i;
        nextAddressAvailable = getNextMailAddress(receiver, emailTo, i);
      }

      if (!NPlugin_001_MTA(client, F("DATA"), 354)) { break; }

      if (!NPlugin_001_MTA(client, strformat(F("%s%s\r\n.\r\n"), mailheader.c_str(), aMesg.c_str()), 250)) { break; }

      myStatus = true;

      NPlugin_001_MTA(client, F("QUIT"), 221); // Sent successfully, close SMTP protocol, ignore failure
      break;
    }

    client.flush();
    client.stop();

    if (myStatus == true) {
      addLog(LOG_LEVEL_INFO, F("EMAIL: Connection Closed Successfully"));
    } else {
      if (loglevelActiveFor(LOG_LEVEL_ERROR)) {
        addLogMove(LOG_LEVEL_ERROR, concat(F("EMAIL: Connection Closed With Error. Used header: "),  mailheader));
      }
    }
  }
  return myStatus;
}

bool NPlugin_001_Auth(WiFiClient& client, const String& user, const String& pass)
{
  if (user.isEmpty() || pass.isEmpty()) {
    // No user/password given.
    return true;
  }
  base64 encoder;

  return NPlugin_001_MTA(client, F("AUTH LOGIN"), 334) &&
         NPlugin_001_MTA(client, encoder.encode(user), 334) &&
         NPlugin_001_MTA(client, encoder.encode(pass), 235);
}

bool NPlugin_001_MTA(WiFiClient& client, const String& aStr, uint16_t aWaitForPattern)
{
  # ifndef BUILD_NO_DEBUG
  addLog(LOG_LEVEL_DEBUG, aStr);
  # endif // ifndef BUILD_NO_DEBUG

  if (aStr.length()) { client.println(aStr); }

  // Wait For Response
  unsigned long timer = millis() + NPLUGIN_001_TIMEOUT;

  backgroundtasks();

  const String aWaitForPattern_str = strformat(F("%d "), aWaitForPattern);

  while (true) {
    if (timeOutReached(timer)) {
      if (loglevelActiveFor(LOG_LEVEL_ERROR)) {
        addLogMove(LOG_LEVEL_ERROR,
                   concat(F("NPlugin_001_MTA: timeout. "), aStr));
      }
      break;
    }

    delay(0);

    String line;
    safeReadStringUntil(client, line, '\n');

    const bool patternFound = line.indexOf(aWaitForPattern_str) >= 0;

    # ifndef BUILD_NO_DEBUG
    addLogMove(LOG_LEVEL_DEBUG, line);
    # endif // ifndef BUILD_NO_DEBUG

    return patternFound;
  }

  return false;
}

bool getNextMailAddress(const String& data, String& address, int index)
{
  int found          = 0;
  int strIndex[]     = { 0, -1 };
  const int maxIndex = data.length() - 1;

  for (int i = 0; i <= maxIndex && found <= index; i++) {
    if ((data.charAt(i) == ',') || (i == maxIndex)) {
      found++;
      strIndex[0] = strIndex[1] + 1;
      strIndex[1] = (i == maxIndex) ? i + 1 : i;
    }
  }

  if (found > index) {
    address = data.substring(strIndex[0], strIndex[1]);
    return true;
  }
  return false;
}

#endif // ifdef USES_N001
