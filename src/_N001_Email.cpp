#include "ESPEasy_common.h"

#ifdef USES_N001

// #######################################################################################################
// ########################### Notification Plugin 001: Email ############################################
// #######################################################################################################

/** Changelog:
 * 2024-07-01 ThomasB   : Modified to support new email server protocol used by some ISP hosting providers
 *                        Add support for (also) supplying an alternate email address via the notify command
 *                        Now uses Plugin's new Timeout Setting for SMTP server response.
 *                        Can now use substitute email address(s), provided within Notify command rule.
 * 2024-04-06 tonhuisman: Add support for (also) supplying a custom subject when sending an email notification via the notify command
 *                        Log reply from mailserver at DEBUG level
 *                        Code improvements
 * 2022-12-29 tonhuisman: Add Date: field to email header to reduce spam score, see https://github.com/letscontrolit/ESPEasy/issues/3865
 * 2022-12-29 tonhuisman: Start changelog
 */

# define NPLUGIN_001
# define NPLUGIN_ID_001         1
# define NPLUGIN_NAME_001       "Email (SMTP)"

# define NPLUGIN_001_PKT_SZ     256

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
                      const String& pass,
                      uint16_t      timeout);
bool NPlugin_001_MTA(WiFiClient  & client,
                     const String& aStr,
                     uint16_t      aWaitForPattern,
                     uint16_t      timeout);
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
  bool failFlag = false;

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
             F("Email: Connecting to %s:%d"),
             notificationsettings.Server,
             notificationsettings.Port));
  }
  # endif // ifndef BUILD_NO_DEBUG

  if (!connectClient(client, notificationsettings.Server, notificationsettings.Port, CONTROLLER_CLIENTTIMEOUT_DFLT)) {
    if (loglevelActiveFor(LOG_LEVEL_ERROR)) {
      addLog(LOG_LEVEL_ERROR, strformat(
               F("Email: Error connecting to %s:%d"),
               notificationsettings.Server,
               notificationsettings.Port));
    }
    myStatus = false;
    failFlag = true;
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

    uint16_t clientTimeout = notificationsettings.Timeout * 1000; // Convert to mS.

    if ((clientTimeout < NPLUGIN_001_MIN_TM) || (clientTimeout > NPLUGIN_001_MAX_TM)) {
      clientTimeout = NPLUGIN_001_DEF_TM;
    }

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


    // Use Notify Command's destination email address(s) if provided in Command rules.
    // Sample Rule: Notify 1, "{email1@domain.com;email2@domain.net}Test email from %sysname%.<br/> How are you?<br/>Have a good day.<br/>"
    String subAddr;
    String tmp_ato;
    int    pos_brace1 = aMesg.indexOf('{');
    int    pos_amper  = aMesg.indexOf('@');
    int    pos_brace2 = aMesg.indexOf('}');

    if ((pos_brace1 == 0) && (pos_amper > pos_brace1) && (pos_brace2 > pos_amper)) {
      subAddr = aMesg.substring(pos_brace1 + 1, pos_brace2);
      subAddr.trim();
      tmp_ato = subAddr;
        # ifndef BUILD_NO_DEBUG

      if (loglevelActiveFor(LOG_LEVEL_DEBUG)) {
        addLog(LOG_LEVEL_DEBUG, strformat(F("Email: Substitute Receiver (ato): %s"), subAddr.c_str()));
      }
        # endif // ifndef BUILD_NO_DEBUG

      String subMsg = aMesg.substring(pos_brace2 + 1); // Remove substitute email address from subject line.
      subMsg.trim();

      if (subMsg.indexOf(',') == 0) {
        subMsg = subMsg.substring(1); // Remove leading comma.
        subMsg.trim();
      }

      if (!subMsg.length()) {
        subMsg = "ERROR: ESPEasy Notify Rule missing the message text. Please correct the rule.";
      }
        # ifndef BUILD_NO_DEBUG

      if (loglevelActiveFor(LOG_LEVEL_DEBUG)) {
        addLog(LOG_LEVEL_DEBUG, strformat(F("Email: Substitute Message: %s"), subMsg.c_str()));
      }
        # endif // ifndef BUILD_NO_DEBUG
      aMesg = subMsg;
    }
    else {
      tmp_ato = notificationsettings.Receiver; // Use plugin's receiver.
    }

    // Clean up receiver address.
    tmp_ato.replace(";", ",");
    tmp_ato.replace(" ", "");

    mailheader.replace(F("$nodename"),       senderName);
    mailheader.replace(F("$emailfrom"),      email_address);
    mailheader.replace(F("$ato"),            tmp_ato);
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

    if (!failFlag) {
      addLog(LOG_LEVEL_INFO, F("Email: Initializing ..."));

      # ifndef BUILD_NO_DEBUG
      addLog(LOG_LEVEL_INFO, strformat(F("Email: Max Allowed Timeout is %d secs"), clientTimeout / 1000));
      # endif // ifndef BUILD_NO_DEBUG

      while (true) {
        if (!NPlugin_001_MTA(client, EMPTY_STRING, 220, clientTimeout)) {
          # ifndef BUILD_NO_DEBUG

          if (loglevelActiveFor(LOG_LEVEL_DEBUG)) {
            addLog(LOG_LEVEL_DEBUG, F("Email: Initialization Fail"));
          }
          # endif // ifndef BUILD_NO_DEBUG
          failFlag = true;
          break;
        }

        if (!failFlag) {
          # ifndef BUILD_NO_DEBUG
          addLog(LOG_LEVEL_DEBUG, F("Email: Sending EHLO domain"));
          # endif // ifndef BUILD_NO_DEBUG

          const String astr = strformat(F("EHLO %s"), notificationsettings.Domain);

          if (!NPlugin_001_MTA(
                client,
                astr,
                250,
                clientTimeout)) {
            # ifndef BUILD_NO_DEBUG
            addLog(LOG_LEVEL_DEBUG, F("Email: EHLO Domain Fail"));
            # endif // ifndef BUILD_NO_DEBUG
            failFlag = true;
          }
        }

        // Must retrieve SMTP Reply Packet. Data not used, ignored.
        if (!failFlag) {
          const unsigned long timer = millis() + clientTimeout;
          String replyStr;
          String catStr;

          bool done = false;

          while (client.available() && !done) {
            if (timeOutReached(timer)) {
              failFlag = true;
              break;
            }
            done    = safeReadStringUntil(client, replyStr, '\n', NPLUGIN_001_PKT_SZ);
            catStr += replyStr;
          }

          if (!catStr.length()) {
            catStr = F("Empty!");
          }

          # ifndef BUILD_NO_DEBUG

          if (loglevelActiveFor(LOG_LEVEL_DEBUG)) {
            String log = strformat(F("Email: Packet Rcvd is: > %s <"), catStr.c_str());
            addLogMove(LOG_LEVEL_DEBUG, log);
          }
          # endif // ifndef BUILD_NO_DEBUG
        }

        if (!failFlag) {
          # ifndef BUILD_NO_DEBUG

          if (loglevelActiveFor(LOG_LEVEL_DEBUG)) {
            addLog(LOG_LEVEL_DEBUG, F("Email: Sending User/Pass"));
          }
          # endif // ifndef BUILD_NO_DEBUG

          if (!NPlugin_001_Auth(client, notificationsettings.User, notificationsettings.Pass, clientTimeout)) {
            # ifndef BUILD_NO_DEBUG

            addLog(LOG_LEVEL_DEBUG, F("Email: User/Pass Fail"));
            # endif // ifndef BUILD_NO_DEBUG
            failFlag = true;
            break;
          }
        }

        if (!failFlag) {
          # ifndef BUILD_NO_DEBUG
          addLog(LOG_LEVEL_DEBUG, F("Email: Sending email Addr"));
          # endif // ifndef BUILD_NO_DEBUG

          const String astr = strformat(F("MAIL FROM:<%s>"), email_address.c_str());

          if (!NPlugin_001_MTA(client, astr, 250, clientTimeout)) {
            # ifndef BUILD_NO_DEBUG
            addLog(LOG_LEVEL_DEBUG, F("Email: Addr Fail"));
            # endif // ifndef BUILD_NO_DEBUG
            failFlag = true;
            break;
          }
        }

        if (!failFlag) {
          bool   nextAddressAvailable = true;
          int    i                    = 0;
          String emailTo;
          const String receiver(tmp_ato);

          addLog(LOG_LEVEL_INFO, strformat(F("Email: Receiver(s): %s"), receiver.c_str()));

          if (!getNextMailAddress(receiver, emailTo, i)) {
            addLog(LOG_LEVEL_ERROR, F("Email: Receiver missing!"));
            break;
          }

          while (nextAddressAvailable) {
            if (loglevelActiveFor(LOG_LEVEL_INFO)) {
              addLog(LOG_LEVEL_INFO, concat(F("Email: To "), emailTo));
            }

            if (!NPlugin_001_MTA(client, strformat(F("RCPT TO:<%s>"), emailTo.c_str()), 250, clientTimeout)) { break; }
            ++i;
            nextAddressAvailable = getNextMailAddress(receiver, emailTo, i);
          }
        }

        if (!failFlag) {
          if (!NPlugin_001_MTA(client, F("DATA"), 354, clientTimeout)) {
            failFlag = true;
            break;
          }
        }

        if (!failFlag) {
          if (!NPlugin_001_MTA(client, strformat(F("%s%s\r\n.\r\n"), mailheader.c_str(), aMesg.c_str()), 250, clientTimeout)) {
            failFlag = true;
            break;
          }
        }

        // Email Sent. Do some final housekeeping, tell server we're leaving.
        if (!failFlag) {
          myStatus = true;
        }

        NPlugin_001_MTA(client, F("QUIT"), 221, clientTimeout); // Sent successfully, close SMTP protocol, ignore failure
        break;
      }
    }
    client.PR_9453_FLUSH_TO_CLEAR();
    client.stop();

    if (myStatus == true) {
      addLog(LOG_LEVEL_INFO, F("Email: Connection Closed Successfully"));
    } else {
      if (loglevelActiveFor(LOG_LEVEL_ERROR)) {
        addLogMove(LOG_LEVEL_ERROR, concat(F("Email: Connection Closed With Error. Used header: "),  mailheader));
      }
    }
  }
  return myStatus;
}

bool NPlugin_001_Auth(WiFiClient& client, const String& user, const String& pass, uint16_t timeout)
{
  if (user.isEmpty() || pass.isEmpty()) {
    // No user/password given.
    return true;
  }
  base64 encoder;

  bool mta1 = NPlugin_001_MTA(client, F("AUTH LOGIN"), 334, timeout);
  bool mta2 = NPlugin_001_MTA(client, encoder.encode(user), 334, timeout);
  bool mta3 = NPlugin_001_MTA(client, encoder.encode(pass), 235, timeout);

  if (mta1 && mta2 && mta3) {
    addLog(LOG_LEVEL_INFO, F("Email: Credentials Accepted"));
  }
  return mta1 && mta2 && mta3;
}

bool NPlugin_001_MTA(WiFiClient& client, const String& aStr, uint16_t aWaitForPattern, uint16_t timeout)
{
  # ifndef BUILD_NO_DEBUG

  if (loglevelActiveFor(LOG_LEVEL_DEBUG)) {
    addLog(LOG_LEVEL_DEBUG, aStr);
  }
  # endif // ifndef BUILD_NO_DEBUG

  client.PR_9453_FLUSH_TO_CLEAR();

  if (aStr.length()) { client.println(aStr); }

  // Wait For Response
  unsigned long timer = millis() + timeout;

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
    safeReadStringUntil(client, line, '\n', 1024, timeout);

    line.replace("-", " "); // Must Remove optional dash from MTA response code.

    const bool patternFound = line.indexOf(aWaitForPattern_str) >= 0;

    # ifndef BUILD_NO_DEBUG

    if (loglevelActiveFor(LOG_LEVEL_DEBUG)) {
      addLogMove(LOG_LEVEL_DEBUG, line);
    }
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
