#include "../WebServer/NotificationPage.h"

#if FEATURE_NOTIFIER

// #######################################################################################################
// ############################### Notifification Page: Email ############################################
// #######################################################################################################

/** Changelog:
 * 2024-07-30 ThomasB   : Added Read-the-Docs Help Button to email and buzzer plugins.
 * 2024-07-01 ThomasB   : Added User Setting for SMTP email server timeout. Display in Seconds.
 * 2024-07-01 ThomasB   : Start of changelog, older changes not logged.
 */

# include "../WebServer/ESPEasy_WebServer.h"
# include "../WebServer/HTML_wrappers.h"
# include "../WebServer/Markup.h"
# include "../WebServer/Markup_Buttons.h"
# include "../WebServer/Markup_Forms.h"

# include "../DataStructs/ESPEasy_EventStruct.h"
# include "../DataStructs/NotificationSettingsStruct.h"

# include "../Helpers/ESPEasy_Storage.h"

# include "../Globals/ESPEasy_Scheduler.h"
# include "../Globals/Settings.h"


// ********************************************************************************
// Web Interface notifcations page
// ********************************************************************************


# include "../Globals/NPlugins.h"


void handle_notifications() {
  # ifndef BUILD_NO_RAM_TRACKER
  checkRAM(F("handle_notifications"));
  # endif // ifndef BUILD_NO_RAM_TRACKER

  if (!isLoggedIn()) { return; }
  navMenuIndex = MENU_INDEX_NOTIFICATIONS;
  TXBuffer.startStream();
  sendHeadandTail_stdtemplate(_HEAD);

  struct EventStruct TempEvent;

  // char tmpString[64];


  // 'index' value in the URL
  uint8_t notificationindex       = getFormItemInt(F("index"), 0);
  boolean notificationindexNotSet = notificationindex == 0;
  --notificationindex;

  const int notification_webarg_value = getFormItemInt(F("notification"), -1);

  if (!notificationindexNotSet && (notification_webarg_value != -1))
  {
    const npluginID_t notification = npluginID_t::toPluginID(notification_webarg_value);

    if (notification == INVALID_N_PLUGIN_ID) {
      Settings.Notification[notificationindex]        = INVALID_N_PLUGIN_ID.value;
      Settings.NotificationEnabled[notificationindex] = false;
    } else {
      MakeNotificationSettings(NotificationSettings);

      if (Settings.Notification[notificationindex] != notification.value)
      {
        Settings.Notification[notificationindex] = notification.value;
      }
      else
      {
        if (Settings.Notification[notificationindex] != INVALID_N_PLUGIN_ID.value)
        {
          nprotocolIndex_t NotificationProtocolIndex = getNProtocolIndex_from_NotifierIndex(notificationindex);

          if (validNProtocolIndex(NotificationProtocolIndex)) {
            String dummyString;
            NPlugin_ptr[NotificationProtocolIndex](NPlugin::Function::NPLUGIN_WEBFORM_SAVE, 0, dummyString);
          }

          // Reload & overwrite
          LoadNotificationSettings(notificationindex, reinterpret_cast<uint8_t *>(&NotificationSettings), sizeof(NotificationSettingsStruct));
          NotificationSettings.validate();
          NotificationSettings.Port = getFormItemInt(F("port"), 0);

          NotificationSettings.Timeout_ms                 = getFormItemInt(F("timeout"), NPLUGIN_001_DEF_TM);
          NotificationSettings.Pin1                       = getFormItemInt(F("pin1"), -1);
          NotificationSettings.Pin2                       = getFormItemInt(F("pin2"), -1);
          Settings.NotificationEnabled[notificationindex] = isFormItemChecked(F("notificationenabled"));
          strncpy_webserver_arg(NotificationSettings.Domain,   F("domain"));
          strncpy_webserver_arg(NotificationSettings.Server,   F("server"));
          strncpy_webserver_arg(NotificationSettings.Sender,   F("sender"));
          strncpy_webserver_arg(NotificationSettings.Receiver, F("receiver"));
          strncpy_webserver_arg(NotificationSettings.Subject,  F("subject"));
          strncpy_webserver_arg(NotificationSettings.User,     F("username"));
          strncpy_webserver_arg(NotificationSettings.Body,     F("body"));

          copyFormPassword(F("password"), NotificationSettings.Pass, sizeof(NotificationSettings.Pass));
        }
      }
      addHtmlError(SaveNotificationSettings(notificationindex, reinterpret_cast<const uint8_t *>(&NotificationSettings),
                                            sizeof(NotificationSettingsStruct)));
    }

    // Save the settings.
    addHtmlError(SaveSettings());

    if (hasArg(F("test"))) {
      // Perform tests with the settings in the form.
      nprotocolIndex_t NotificationProtocolIndex = getNProtocolIndex_from_NotifierIndex(notificationindex);

      if (validNProtocolIndex(NotificationProtocolIndex) &&
          Settings.NotificationEnabled[notificationindex])
      {
        // TempEvent.NotificationProtocolIndex = NotificationProtocolIndex;
        TempEvent.NotificationIndex = notificationindex;
        Scheduler.schedule_notification_event_timer(NotificationProtocolIndex, NPlugin::Function::NPLUGIN_NOTIFY, std::move(TempEvent));
      }
    }
  }

  html_add_form();

  if (notificationindexNotSet)
  {
    html_table_class_multirow();
    html_TR();
    html_table_header(F(""),        70);
    html_table_header(F("Nr"),      50);
    html_table_header(F("Enabled"), 100);
    html_table_header(F("Service"));
    html_table_header(F("Server"));
    html_table_header(F("Port"));

    MakeNotificationSettings(NotificationSettings);

    for (uint8_t x = 0; x < NOTIFICATION_MAX; x++)
    {
      LoadNotificationSettings(x, reinterpret_cast<uint8_t *>(&NotificationSettings), sizeof(NotificationSettingsStruct));
      NotificationSettings.validate();
      html_TR_TD();
      html_add_button_prefix();
      addHtml(F("notifications?index="));
      addHtmlInt(x + 1);
      addHtml(F("'>Edit</a>"));
      html_TD();
      addHtmlInt(x + 1);
      html_TD();

      if (Settings.Notification[x] != INVALID_N_PLUGIN_ID.value)
      {
        addEnabled(Settings.NotificationEnabled[x]);

        html_TD();
        uint8_t NotificationProtocolIndex = getNProtocolIndex(npluginID_t::toPluginID(Settings.Notification[x]));
        String  NotificationName          = F("(plugin not found?)");

        if (validNProtocolIndex(NotificationProtocolIndex))
        {
          NPlugin_ptr[NotificationProtocolIndex](NPlugin::Function::NPLUGIN_GET_DEVICENAME, 0, NotificationName);
        }
        addHtml(NotificationName);
        html_TD();
        addHtml(NotificationSettings.Server);
        html_TD();

        if (NotificationSettings.Port) {
          addHtmlInt(NotificationSettings.Port);
        } else {
          // MFD: we display the GPIO
          addGpioHtml(NotificationSettings.Pin1);

          if (NotificationSettings.Pin2 >= 0)
          {
            html_BR();
            addGpioHtml(NotificationSettings.Pin2);
          }
        }
      }
      else {
        html_TD(3);
      }
    }
    html_end_table();
    html_end_form();
  }
  else
  {
    html_table_class_normal();
    addFormHeader(F("Notification Settings"));
    addRowLabel(F("Notification"));
    uint8_t choice = Settings.Notification[notificationindex];
    addSelector_Head_reloadOnChange(F("notification"));
    addSelector_Item(F("- None -"), 0, false);

    for (uint8_t x = 0; x <= notificationCount; x++)
    {
      String NotificationName;
      NPlugin_ptr[x](NPlugin::Function::NPLUGIN_GET_DEVICENAME, 0, NotificationName);
      addSelector_Item(NotificationName,
                       Notification[x].Number,
                       choice == Notification[x].Number);
    }
    addSelector_Foot();

    addHelpButton(F("EasyNotifications"));
    addRTDHelpButton(F("Notify/_Notifications.html"));

    if (Settings.Notification[notificationindex] != INVALID_N_PLUGIN_ID.value)
    {
      MakeNotificationSettings(NotificationSettings);
      LoadNotificationSettings(notificationindex, reinterpret_cast<uint8_t *>(&NotificationSettings), sizeof(NotificationSettingsStruct));
      NotificationSettings.validate();

      nprotocolIndex_t NotificationProtocolIndex = getNProtocolIndex_from_NotifierIndex(notificationindex);

      if (validNProtocolIndex(NotificationProtocolIndex))
      {
        if (Notification[NotificationProtocolIndex].usesMessaging)
        {
          if (NotificationSettings.Port == 0) {
# if FEATURE_EMAIL_TLS
            NotificationSettings.Port = 465;
# else // if FEATURE_EMAIL_TLS
            NotificationSettings.Port = 25;
# endif // if FEATURE_EMAIL_TLS
          }

          addFormSubHeader(F("SMTP Server Settings"));
          addFormTextBox(F("Domain"), F("domain"), NotificationSettings.Domain, sizeof(NotificationSettings.Domain) - 1);
          addFormTextBox(F("Server"), F("server"), NotificationSettings.Server, sizeof(NotificationSettings.Server) - 1);
          addFormNumericBox(
            F("Port"), F("port"),
            NotificationSettings.Port,
            1,
            65535);
# if FEATURE_EMAIL_TLS
          addFormNote(F("default port SSL: 465"));
# else // if FEATURE_EMAIL_TLS
          addFormNote(F("default port: 25, SSL/TLS servers NOT supported!"));
# endif // if FEATURE_EMAIL_TLS

          if ((NotificationSettings.Timeout_ms < NPLUGIN_001_MIN_TM) ||
              (NotificationSettings.Timeout_ms > NPLUGIN_001_MAX_TM))
          {
            NotificationSettings.Timeout_ms = NPLUGIN_001_DEF_TM;
          }

          addFormNumericBox(
            F("Timeout"), F("timeout"),
            NotificationSettings.Timeout_ms,
            NPLUGIN_001_MIN_TM,
            NPLUGIN_001_MAX_TM
# if FEATURE_TOOLTIPS
            , F("Maximum Server Response Time")
# endif // if FEATURE_TOOLTIPS
            );

          addUnit(F("ms"));

          ZERO_TERMINATE(NotificationSettings.Pass);
          addFormSubHeader(F("Credentials"));

          addFormTextBox(F("Username"), F("username"), NotificationSettings.User, sizeof(NotificationSettings.User) - 1);
          addFormPasswordBox(F("Password"), F("password"), NotificationSettings.Pass, sizeof(NotificationSettings.Pass) - 1);

          addFormSubHeader(F("Email Attributes"));

          addFormTextBox(F("Sender"),   F("sender"),   NotificationSettings.Sender,   sizeof(NotificationSettings.Sender) - 1);
          addFormTextBox(F("Receiver"), F("receiver"), NotificationSettings.Receiver, sizeof(NotificationSettings.Receiver) - 1);
          addFormTextBox(F("Subject"),  F("subject"),  NotificationSettings.Subject,  sizeof(NotificationSettings.Subject) - 1);

          addRowLabel(F("Body"));
          addHtml(F("<textarea name='body' rows='20' size=512 wrap='off'>"));
          addHtml(NotificationSettings.Body);
          addHtml(F("</textarea>"));
        }

        if (Notification[NotificationProtocolIndex].usesGPIO > 0)
        {
          addRowLabel(F("1st GPIO"));
          addPinSelect(PinSelectPurpose::Generic, F("pin1"), NotificationSettings.Pin1);
        }

        addRowLabel(F("Enabled"));
        addCheckBox(F("notificationenabled"), Settings.NotificationEnabled[notificationindex]);

        TempEvent.NotificationIndex = notificationindex;
        String webformLoadString;
        NPlugin_ptr[NotificationProtocolIndex](NPlugin::Function::NPLUGIN_WEBFORM_LOAD, &TempEvent, webformLoadString);

        if (webformLoadString.length() > 0) {
          addHtmlError(F("Bug in NPlugin::Function::NPLUGIN_WEBFORM_LOAD, should not append to string, use addHtml() instead"));
        }
      }
    }

    addFormSeparator(2);

    html_TR_TD();
    html_TD();
    addButton(F("notifications"), F("Close"));
    addSubmitButton();
    addSubmitButton(F("Test"), F("test"));
    html_end_table();
    html_end_form();
  }
  sendHeadandTail_stdtemplate(_TAIL);
  TXBuffer.endStream();
}

#endif // FEATURE_NOTIFIER
