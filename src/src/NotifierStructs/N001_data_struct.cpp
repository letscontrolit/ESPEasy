#include "../NotifierStructs/N001_data_struct.h"

#ifdef USES_N001
#include "../DataStructs/ESPEasy_EventStruct.h"
#include "../ESPEasyCore/ESPEasy_Log.h"
#include "../ESPEasyCore/ESPEasy_backgroundtasks.h"
#include "../Globals/Settings.h"
#include "../Helpers/ESPEasy_Storage.h"
#include "../Helpers/ESPEasy_time_calc.h"
#include "../Helpers/Networking.h"
#include "../Helpers/StringGenerator_System.h"
#include "../Helpers/StringParser.h"
#include "../Helpers/_CPlugin_Helper.h" // safeReadStringUntil
#include "../Helpers/_NPlugin_init.h"

#if FEATURE_EMAIL_TLS

  # include <WiFiClientSecureLightBearSSL.h>
  # include "../CustomBuild/Certificate_CA.h"

#endif // if FEATURE_EMAIL_TLS

bool NPlugin_001_send(const NotificationSettingsStruct& notificationsettings, const String& aSub, String& aMesg)
{
  //  String& aDomain , String aTo, String aFrom, String aSub, String aMesg, String aHost, int aPort)
  bool myStatus = false;
  bool failFlag = false;

#if FEATURE_EMAIL_TLS

  // values are based on the NPLUGIN_001_PKT_SZ
  BearSSL::WiFiClientSecure_light client(4096, 4096);
  client.setUtcTime_fcn(getUnixTime);
  client.setCfgTime_fcn(get_build_unixtime);
  client.setTrustAnchor(Tasmota_TA, Tasmota_TA_size);

  client.setInsecure();

#else // if FEATURE_EMAIL_TLS

  // Use WiFiClient class to create TCP connections
  WiFiClient client;
#endif // if FEATURE_EMAIL_TLS

  #ifdef MUSTFIX_CLIENT_TIMEOUT_IN_SECONDS

  // See: https://github.com/espressif/arduino-esp32/pull/6676
  client.setTimeout((notificationsettings.Timeout_ms + 500) / 1000); // in seconds!!!!
  Client *pClient = &client;
  pClient->setTimeout(notificationsettings.Timeout_ms);
  #else // ifdef MUSTFIX_CLIENT_TIMEOUT_IN_SECONDS
  client.setTimeout(notificationsettings.Timeout_ms); // in msec as it should be!
  #endif // ifdef MUSTFIX_CLIENT_TIMEOUT_IN_SECONDS

  #ifndef BUILD_NO_DEBUG

  if (loglevelActiveFor(LOG_LEVEL_DEBUG)) {
    addLog(LOG_LEVEL_DEBUG, strformat(
             F("Email: Connecting to %s:%d"),
             notificationsettings.Server,
             notificationsettings.Port));
  }
  #endif // ifndef BUILD_NO_DEBUG

  if (!connectClient(client, notificationsettings.Server, notificationsettings.Port, notificationsettings.Timeout_ms)) {
    if (loglevelActiveFor(LOG_LEVEL_ERROR)) {
      addLog(LOG_LEVEL_ERROR, strformat(
               F("Email: Error connecting to %s:%d"),
               notificationsettings.Server,
               notificationsettings.Port));
    }
    myStatus = false;
    failFlag = true;
  } else {
    uint16_t clientTimeout = notificationsettings.Timeout_ms;

    if ((clientTimeout < NPLUGIN_001_MIN_TM) || (clientTimeout > NPLUGIN_001_MAX_TM)) {
      clientTimeout = NPLUGIN_001_DEF_TM;
    }

    addLog(LOG_LEVEL_DEBUG, concat(F("NPlugin_001_send: timeout: "), clientTimeout));

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
        #ifndef BUILD_NO_DEBUG

      if (loglevelActiveFor(LOG_LEVEL_DEBUG)) {
        addLog(LOG_LEVEL_DEBUG, strformat(F("Email: Substitute Receiver (ato): %s"), subAddr.c_str()));
      }
        #endif // ifndef BUILD_NO_DEBUG

      String subMsg = aMesg.substring(pos_brace2 + 1); // Remove substitute email address from subject line.
      subMsg.trim();

      if (subMsg.indexOf(',') == 0) {
        subMsg = subMsg.substring(1); // Remove leading comma.
        subMsg.trim();
      }

      if (!subMsg.length()) {
        subMsg = "ERROR: ESPEasy Notify Rule missing the message text. Please correct the rule.";
      }
        #ifndef BUILD_NO_DEBUG

      if (loglevelActiveFor(LOG_LEVEL_DEBUG)) {
        addLog(LOG_LEVEL_DEBUG, strformat(F("Email: Substitute Message: %s"), subMsg.c_str()));
      }
        #endif // ifndef BUILD_NO_DEBUG
      aMesg = subMsg;
    }
    else {
      tmp_ato = notificationsettings.Receiver; // Use plugin's receiver.
    }

    // Clean up receiver address.
    tmp_ato.replace(";", ",");
    tmp_ato.replace(" ", "");


    const String nodename_emailfrom = strformat(F("%s <%s>"), senderName.c_str(), email_address.c_str());
    String dateFmtHdr               = F("%sysweekday_s%, %sysday_0% %sysmonth_s% %sysyear% %systime% %systzoffset%");

    const String mailheader = strformat(
      F(
        "From: %s\r\n"
        "To: %s\r\n"
        "Subject: %s\r\n"
        "Reply-To: %s\r\n"
        "Date: %s\r\n"
        "MIME-VERSION: 1.0\r\n"
        "Content-type: text/html; charset=UTF-8\r\n"
        "X-Mailer: EspEasy v%s\r\n\r\n"
        ),
      nodename_emailfrom.c_str(),
      tmp_ato.c_str(),
      aSub.c_str(),
      nodename_emailfrom.c_str(),
      parseTemplate(dateFmtHdr).c_str(),
      getSystemBuildString().c_str());


    // Make sure to replace the char '\r' and not the string "\r"
    // See: https://github.com/letscontrolit/ESPEasy/issues/4967
    removeChar(aMesg, '\r');
    aMesg.replace(String('\n'), F("<br/>")); // re-write line breaks for Content-type: text/html

    // Wait for Client to Start Sending
    // The MTA Exchange

    if (!failFlag) {
      addLog(LOG_LEVEL_INFO, F("Email: Initializing ..."));

      #ifndef BUILD_NO_DEBUG
      addLog(LOG_LEVEL_INFO, strformat(F("Email: Max Allowed Timeout is %d secs"), clientTimeout / 1000));
      #endif // ifndef BUILD_NO_DEBUG

      while (true) { // FIXME TD-er: Use of while here can be useful so you can
                     // exit using break;
                     // However this is way too complex using both a failFlag and break
                     // and not even consistently.
        if (!NPlugin_001_MTA(client, EMPTY_STRING, 220, clientTimeout)) {
          #ifndef BUILD_NO_DEBUG

          if (loglevelActiveFor(LOG_LEVEL_DEBUG)) {
            addLog(LOG_LEVEL_DEBUG, F("Email: Initialization Fail"));
          }
          #endif // ifndef BUILD_NO_DEBUG
          failFlag = true;
          break;
        }

        if (!failFlag) {
          #ifndef BUILD_NO_DEBUG
          addLog(LOG_LEVEL_DEBUG, F("Email: Sending EHLO domain"));
          #endif // ifndef BUILD_NO_DEBUG

          const String astr = strformat(F("EHLO %s"), notificationsettings.Domain);

          if (!NPlugin_001_MTA(
                client,
                astr,
                250,
                clientTimeout)) {
            #ifndef BUILD_NO_DEBUG
            addLog(LOG_LEVEL_DEBUG, F("Email: EHLO Domain Fail"));
            #endif // ifndef BUILD_NO_DEBUG
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

          #ifndef BUILD_NO_DEBUG

          if (loglevelActiveFor(LOG_LEVEL_DEBUG)) {
            String log = strformat(F("Email: Packet Rcvd is: > %s <"), catStr.c_str());
            addLogMove(LOG_LEVEL_DEBUG, log);
          }
          #endif // ifndef BUILD_NO_DEBUG
        }

        if (!failFlag) {
          #ifndef BUILD_NO_DEBUG

          if (loglevelActiveFor(LOG_LEVEL_DEBUG)) {
            addLog(LOG_LEVEL_DEBUG, F("Email: Sending User/Pass"));
          }
          #endif // ifndef BUILD_NO_DEBUG

          if (!NPlugin_001_Auth(client, notificationsettings.User, notificationsettings.Pass, clientTimeout)) {
            #ifndef BUILD_NO_DEBUG

            addLog(LOG_LEVEL_DEBUG, F("Email: User/Pass Fail"));
            #endif // ifndef BUILD_NO_DEBUG
            failFlag = true;
            break;
          }
        }

        if (!failFlag) {
          #ifndef BUILD_NO_DEBUG
          addLog(LOG_LEVEL_DEBUG, F("Email: Sending email Addr"));
          #endif // ifndef BUILD_NO_DEBUG

          const String astr = strformat(F("MAIL FROM:<%s>"), email_address.c_str());

          if (!NPlugin_001_MTA(client, astr, 250, clientTimeout)) {
            #ifndef BUILD_NO_DEBUG
            addLog(LOG_LEVEL_DEBUG, F("Email: Addr Fail"));
            #endif // ifndef BUILD_NO_DEBUG
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

            if (!NPlugin_001_MTA(client, strformat(F("RCPT TO:<%s>"), emailTo.c_str()), 250, clientTimeout))
            {
              break;
            }
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

    //    client.PR_9453_FLUSH_TO_CLEAR();
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

  bool success = true;

  if (!NPlugin_001_MTA(client, F("AUTH LOGIN"), 334, timeout)) { success = false; }

  if (!NPlugin_001_MTA(client, encoder.encode(user), 334, timeout)) { success = false; }

  if (!NPlugin_001_MTA(client, encoder.encode(pass), 235, timeout)) { success = false; }

  if (success) {
    addLog(LOG_LEVEL_INFO, F("Email: Credentials Accepted"));
  }
  return success;
}

bool NPlugin_001_MTA(WiFiClient& client, const String& aStr, uint16_t aWaitForPattern, uint16_t timeout)
{
  #ifndef BUILD_NO_DEBUG

  if (loglevelActiveFor(LOG_LEVEL_DEBUG)) {
    addLog(LOG_LEVEL_DEBUG, strformat(
             F("NPlugin_001_MTA:  Waitfor: %u Timeout: %u ms Send: '%s'"),
             aWaitForPattern,
             timeout,
             aStr.c_str()));
  }
  #endif // ifndef BUILD_NO_DEBUG

  if (aStr.length()) {
    client.PR_9453_FLUSH_TO_CLEAR(); // have to send msg to server so flush data first
    client.println(aStr);
  }

  // Wait For Response
  const unsigned long timer = millis() + timeout;
  backgroundtasks();

  do { // FIXME TD-er: Why this while loop??? makes no sense as it will only be run once
    delay(0);

    String line;
    safeReadStringUntil(client, line, '\n', 1024); // , timeout);

    #ifndef BUILD_NO_DEBUG

    if (loglevelActiveFor(LOG_LEVEL_DEBUG)) {
      addLogMove(LOG_LEVEL_DEBUG, concat(F("NPlugin_001_MTA: read line: "), line));
    }
    #endif // ifndef BUILD_NO_DEBUG


    // response could be like: '220 domain', '220-domain','220+domain'
    const String pattern_str_space = strformat(F("%d "), aWaitForPattern);
    const String pattern_str_minus = strformat(F("%d-"), aWaitForPattern);
    const String pattern_str_plus  = strformat(F("%d+"), aWaitForPattern);

    const bool patternFound = line.indexOf(pattern_str_space) >= 0
                              || line.indexOf(pattern_str_minus) >= 0
                              || line.indexOf(pattern_str_plus) >= 0;

    if (patternFound) {
      return true;
    }
  } while (!timeOutReached(timer));

  if (timeOutReached(timer)) {
    if (loglevelActiveFor(LOG_LEVEL_ERROR)) {
      addLogMove(LOG_LEVEL_ERROR,
                 concat(F("NPlugin_001_MTA: timeout. "), aStr));
    }
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

#endif