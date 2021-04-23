#include "_CPlugin_Helper_webform.h"

#include "../DataStructs/ESPEasy_EventStruct.h"
#include "../DataTypes/ESPEasy_plugin_functions.h"
#include "../Globals/CPlugins.h"
#include "../Globals/SecuritySettings.h"
#include "../Globals/Settings.h"
#include "../Helpers/_CPlugin_Helper.h"
#include "../Helpers/Networking.h"
#include "../WebServer/WebServer.h"
#include "../WebServer/Markup.h"
#include "../WebServer/Markup_Forms.h"

/*********************************************************************************************\
* Functions to load and store controller settings on the web page.
\*********************************************************************************************/
String getControllerParameterName(protocolIndex_t                   ProtocolIndex,
                                  ControllerSettingsStruct::VarType parameterIdx,
                                  bool                              displayName,
                                  bool                            & isAlternative) {
  String name;

  if (displayName) {
    EventStruct tmpEvent;
    tmpEvent.idx = parameterIdx;

    if (CPluginCall(ProtocolIndex, CPlugin::Function::CPLUGIN_GET_PROTOCOL_DISPLAY_NAME, &tmpEvent, name)) {
      // Found an alternative name for it.
      isAlternative = true;
      return name;
    }
  }
  isAlternative = false;

  switch (parameterIdx) {
    case ControllerSettingsStruct::CONTROLLER_USE_DNS:                  name = F("Locate Controller");      break;
    case ControllerSettingsStruct::CONTROLLER_HOSTNAME:                 name = F("Controller Hostname");    break;
    case ControllerSettingsStruct::CONTROLLER_IP:                       name = F("Controller IP");          break;
    case ControllerSettingsStruct::CONTROLLER_PORT:                     name = F("Controller Port");        break;
    case ControllerSettingsStruct::CONTROLLER_USER:                     name = F("Controller User");        break;
    case ControllerSettingsStruct::CONTROLLER_PASS:                     name = F("Controller Password");    break;

    case ControllerSettingsStruct::CONTROLLER_MIN_SEND_INTERVAL:        name = F("Minimum Send Interval");  break;
    case ControllerSettingsStruct::CONTROLLER_MAX_QUEUE_DEPTH:          name = F("Max Queue Depth");        break;
    case ControllerSettingsStruct::CONTROLLER_MAX_RETRIES:              name = F("Max Retries");            break;
    case ControllerSettingsStruct::CONTROLLER_FULL_QUEUE_ACTION:        name = F("Full Queue Action");      break;
    case ControllerSettingsStruct::CONTROLLER_ALLOW_EXPIRE:             name = F("Allow Expire");           break;
    case ControllerSettingsStruct::CONTROLLER_DEDUPLICATE:              name = F("De-duplicate");           break;
    
    case ControllerSettingsStruct::CONTROLLER_CHECK_REPLY:              name = F("Check Reply");            break;

    case ControllerSettingsStruct::CONTROLLER_CLIENT_ID:                name = F("Controller Client ID");   break;
    case ControllerSettingsStruct::CONTROLLER_UNIQUE_CLIENT_ID_RECONNECT: name = F("Unique Client ID on Reconnect");   break;
    case ControllerSettingsStruct::CONTROLLER_RETAINFLAG:               name = F("Publish Retain Flag");    break;
    case ControllerSettingsStruct::CONTROLLER_SUBSCRIBE:                name = F("Controller Subscribe");   break;
    case ControllerSettingsStruct::CONTROLLER_PUBLISH:                  name = F("Controller Publish");     break;
    case ControllerSettingsStruct::CONTROLLER_LWT_TOPIC:                name = F("Controller LWT Topic");   break;
    case ControllerSettingsStruct::CONTROLLER_LWT_CONNECT_MESSAGE:      name = F("LWT Connect Message");    break;
    case ControllerSettingsStruct::CONTROLLER_LWT_DISCONNECT_MESSAGE:   name = F("LWT Disconnect Message"); break;
    case ControllerSettingsStruct::CONTROLLER_SEND_LWT:                 name = F("Send LWT to broker");     break;
    case ControllerSettingsStruct::CONTROLLER_WILL_RETAIN:              name = F("Will Retain");            break;
    case ControllerSettingsStruct::CONTROLLER_CLEAN_SESSION:            name = F("Clean Session");          break;
    case ControllerSettingsStruct::CONTROLLER_USE_EXTENDED_CREDENTIALS: name = F("Use Extended Credentials");  break;
    case ControllerSettingsStruct::CONTROLLER_SEND_BINARY:              name = F("Send Binary");            break;
    case ControllerSettingsStruct::CONTROLLER_TIMEOUT:                  name = F("Client Timeout");         break;
    case ControllerSettingsStruct::CONTROLLER_SAMPLE_SET_INITIATOR:     name = F("Sample Set Initiator");   break;

    case ControllerSettingsStruct::CONTROLLER_ENABLED:

      if (displayName) { name = F("Enabled"); }
      else {             name = F("controllerenabled"); }
      break;

    default:
      name = F("Undefined");
  }

  if (!displayName) {
    // Change name to lower case and remove spaces to make it an internal name.
    name.toLowerCase();
    name.replace(F(" "), F(""));
  }
  return name;
}

String getControllerParameterInternalName(protocolIndex_t ProtocolIndex, ControllerSettingsStruct::VarType parameterIdx) {
  bool isAlternative; // Dummy, not needed for internal name
  bool displayName = false;

  return getControllerParameterName(ProtocolIndex, parameterIdx, displayName, isAlternative);
}

String getControllerParameterDisplayName(protocolIndex_t ProtocolIndex, ControllerSettingsStruct::VarType parameterIdx, bool& isAlternative) {
  bool displayName = true;

  return getControllerParameterName(ProtocolIndex, parameterIdx, displayName, isAlternative);
}

void addControllerEnabledForm(controllerIndex_t controllerindex) {
  protocolIndex_t ProtocolIndex = getProtocolIndex_from_ControllerIndex(controllerindex);

  if (!validProtocolIndex(ProtocolIndex)) {
    return;
  }

  ControllerSettingsStruct::VarType varType = ControllerSettingsStruct::CONTROLLER_ENABLED;

  bool   isAlternativeDisplayName = false;
  String displayName              = getControllerParameterDisplayName(ProtocolIndex, varType, isAlternativeDisplayName);
  String internalName             = getControllerParameterInternalName(ProtocolIndex, varType);
  addFormCheckBox(displayName, internalName, Settings.ControllerEnabled[controllerindex]);
}

void addControllerParameterForm(const ControllerSettingsStruct& ControllerSettings, controllerIndex_t controllerindex, ControllerSettingsStruct::VarType varType) {
  protocolIndex_t  ProtocolIndex  = getProtocolIndex_from_ControllerIndex(controllerindex);
  if (!validProtocolIndex(ProtocolIndex)) {
    return;
  }

  bool   isAlternativeDisplayName = false;
  String displayName              = getControllerParameterDisplayName(ProtocolIndex, varType, isAlternativeDisplayName);
  String internalName             = getControllerParameterInternalName(ProtocolIndex, varType);

  switch (varType) {
    case ControllerSettingsStruct::CONTROLLER_USE_DNS:
    {
      byte   choice = ControllerSettings.UseDNS;
      String options[2];
      options[0] = F("Use IP address");
      options[1] = F("Use Hostname");
      addFormSelector(displayName, internalName, 2, options, NULL, NULL, choice, true);
      break;
    }
    case ControllerSettingsStruct::CONTROLLER_HOSTNAME:
    {
      addFormTextBox(displayName, internalName, ControllerSettings.HostName, sizeof(ControllerSettings.HostName) - 1);
      break;
    }
    case ControllerSettingsStruct::CONTROLLER_IP:
    {
      addFormIPBox(displayName, internalName, ControllerSettings.IP);
      break;
    }
    case ControllerSettingsStruct::CONTROLLER_PORT:
    {
      addFormNumericBox(displayName, internalName, ControllerSettings.Port, 1, 65535);
      break;
    }
    case ControllerSettingsStruct::CONTROLLER_USER:
    {
      size_t fieldMaxLength =
        ControllerSettings.useExtendedCredentials() ? EXT_SECURITY_MAX_USER_LENGTH : sizeof(SecuritySettings.ControllerUser[0]) - 1;
      addFormTextBox(displayName,
                     internalName,
                     getControllerUser(controllerindex, ControllerSettings),
                     fieldMaxLength);
      break;
    }
    case ControllerSettingsStruct::CONTROLLER_PASS:
    {
      size_t fieldMaxLength = ControllerSettings.useExtendedCredentials() ? EXT_SECURITY_MAX_PASS_LENGTH : sizeof(SecuritySettings.ControllerPassword[0]) - 1;
      if (isAlternativeDisplayName) {
        // It is not a regular password, thus use normal text field.
        addFormTextBox(displayName, internalName, 
                       getControllerPass(controllerindex, ControllerSettings),
                       fieldMaxLength);
      } else {
        addFormPasswordBox(displayName, internalName,
                           getControllerPass(controllerindex, ControllerSettings),
                           fieldMaxLength);
      }
      break;
    }
    case ControllerSettingsStruct::CONTROLLER_MIN_SEND_INTERVAL:
    {
      addFormNumericBox(displayName, internalName, ControllerSettings.MinimalTimeBetweenMessages, 1, CONTROLLER_DELAY_QUEUE_DELAY_MAX);
      addUnit(F("ms"));
      break;
    }
    case ControllerSettingsStruct::CONTROLLER_MAX_QUEUE_DEPTH:
    {
      addFormNumericBox(displayName, internalName, ControllerSettings.MaxQueueDepth, 1, CONTROLLER_DELAY_QUEUE_DEPTH_MAX);
      break;
    }
    case ControllerSettingsStruct::CONTROLLER_MAX_RETRIES:
    {
      addFormNumericBox(displayName, internalName, ControllerSettings.MaxRetry, 1, CONTROLLER_DELAY_QUEUE_RETRY_MAX);
      break;
    }
    case ControllerSettingsStruct::CONTROLLER_FULL_QUEUE_ACTION:
    {
      String options[2];
      options[0] = F("Ignore New");
      options[1] = F("Delete Oldest");
      addFormSelector(displayName, internalName, 2, options, NULL, NULL, ControllerSettings.DeleteOldest, false);
      break;
    }
    case ControllerSettingsStruct::CONTROLLER_ALLOW_EXPIRE:
      addFormCheckBox(displayName, internalName, ControllerSettings.allowExpire());
      break;
    case ControllerSettingsStruct::CONTROLLER_DEDUPLICATE:
      addFormCheckBox(displayName, internalName, ControllerSettings.deduplicate());
      break;
    case ControllerSettingsStruct::CONTROLLER_CHECK_REPLY:
    {
      String options[2];
      options[0] = F("Ignore Acknowledgement");
      options[1] = F("Check Acknowledgement");
      addFormSelector(displayName, internalName, 2, options, NULL, NULL, ControllerSettings.MustCheckReply, false);
      break;
    }
    case ControllerSettingsStruct::CONTROLLER_CLIENT_ID:
      addFormTextBox(displayName, internalName, ControllerSettings.ClientID, sizeof(ControllerSettings.ClientID) - 1);
      break;
    case ControllerSettingsStruct::CONTROLLER_UNIQUE_CLIENT_ID_RECONNECT:
      addFormCheckBox(displayName, internalName, ControllerSettings.mqtt_uniqueMQTTclientIdReconnect());
      break;
    case ControllerSettingsStruct::CONTROLLER_RETAINFLAG:
      addFormCheckBox(displayName, internalName, ControllerSettings.mqtt_retainFlag());
      break;
    case ControllerSettingsStruct::CONTROLLER_SUBSCRIBE:
      addFormTextBox(displayName, internalName, ControllerSettings.Subscribe,            sizeof(ControllerSettings.Subscribe) - 1);
      break;
    case ControllerSettingsStruct::CONTROLLER_PUBLISH:
      addFormTextBox(displayName, internalName, ControllerSettings.Publish,              sizeof(ControllerSettings.Publish) - 1);
      break;
    case ControllerSettingsStruct::CONTROLLER_LWT_TOPIC:
      addFormTextBox(displayName, internalName, ControllerSettings.MQTTLwtTopic,         sizeof(ControllerSettings.MQTTLwtTopic) - 1);
      break;
    case ControllerSettingsStruct::CONTROLLER_LWT_CONNECT_MESSAGE:
      addFormTextBox(displayName, internalName, ControllerSettings.LWTMessageConnect,    sizeof(ControllerSettings.LWTMessageConnect) - 1);
      break;
    case ControllerSettingsStruct::CONTROLLER_LWT_DISCONNECT_MESSAGE:
      addFormTextBox(displayName, internalName, ControllerSettings.LWTMessageDisconnect, sizeof(ControllerSettings.LWTMessageDisconnect) - 1);
      break;
    case ControllerSettingsStruct::CONTROLLER_SEND_LWT:
      addFormCheckBox(displayName, internalName, ControllerSettings.mqtt_sendLWT());
      break;
    case ControllerSettingsStruct::CONTROLLER_WILL_RETAIN:
      addFormCheckBox(displayName, internalName, ControllerSettings.mqtt_willRetain());
      break;
    case ControllerSettingsStruct::CONTROLLER_CLEAN_SESSION:
      addFormCheckBox(displayName, internalName, ControllerSettings.mqtt_cleanSession());
      break;
    case ControllerSettingsStruct::CONTROLLER_USE_EXTENDED_CREDENTIALS:
      addFormCheckBox(displayName, internalName, ControllerSettings.useExtendedCredentials());
      break;
    case ControllerSettingsStruct::CONTROLLER_SEND_BINARY:
      addFormCheckBox(displayName, internalName, ControllerSettings.sendBinary());
      break;
    case ControllerSettingsStruct::CONTROLLER_TIMEOUT:
      addFormNumericBox(displayName, internalName, ControllerSettings.ClientTimeout, 10, CONTROLLER_CLIENTTIMEOUT_MAX);
      addUnit(F("ms"));
      break;
    case ControllerSettingsStruct::CONTROLLER_SAMPLE_SET_INITIATOR:
      addTaskSelectBox(displayName, internalName, ControllerSettings.SampleSetInitiator);
      break;
    case ControllerSettingsStruct::CONTROLLER_ENABLED:
      addFormCheckBox(displayName, internalName, Settings.ControllerEnabled[controllerindex]);
      break;
  }
}

void saveControllerParameterForm(ControllerSettingsStruct        & ControllerSettings,
                                 controllerIndex_t                 controllerindex,
                                 ControllerSettingsStruct::VarType varType) {
  protocolIndex_t ProtocolIndex = getProtocolIndex_from_ControllerIndex(controllerindex);

  if (!validProtocolIndex(ProtocolIndex)) {
    return;
  }
  String internalName = getControllerParameterInternalName(ProtocolIndex, varType);

  switch (varType) {
    case ControllerSettingsStruct::CONTROLLER_USE_DNS:  ControllerSettings.UseDNS = getFormItemInt(internalName); break;
    case ControllerSettingsStruct::CONTROLLER_HOSTNAME:

      if (ControllerSettings.UseDNS)
      {
        strncpy_webserver_arg(ControllerSettings.HostName, internalName);
        IPAddress IP;
        resolveHostByName(ControllerSettings.HostName, IP, ControllerSettings.ClientTimeout);

        for (byte x = 0; x < 4; x++) {
          ControllerSettings.IP[x] = IP[x];
        }
      }
      break;
    case ControllerSettingsStruct::CONTROLLER_IP:

      if (!ControllerSettings.UseDNS)
      {
        String controllerip = web_server.arg(internalName);
        str2ip(controllerip, ControllerSettings.IP);
      }
      break;
    case ControllerSettingsStruct::CONTROLLER_PORT:
      ControllerSettings.Port = getFormItemInt(internalName, ControllerSettings.Port);
      break;
    case ControllerSettingsStruct::CONTROLLER_USER:
      setControllerUser(controllerindex, ControllerSettings, web_server.arg(internalName));
      break;
    case ControllerSettingsStruct::CONTROLLER_PASS:
    {
      String password;

      if (getFormPassword(internalName, password)) {
        setControllerPass(controllerindex, ControllerSettings, password);
      }
      break;
    }

    case ControllerSettingsStruct::CONTROLLER_MIN_SEND_INTERVAL:
      ControllerSettings.MinimalTimeBetweenMessages = getFormItemInt(internalName, ControllerSettings.MinimalTimeBetweenMessages);
      break;
    case ControllerSettingsStruct::CONTROLLER_MAX_QUEUE_DEPTH:
      ControllerSettings.MaxQueueDepth = getFormItemInt(internalName, ControllerSettings.MaxQueueDepth);
      break;
    case ControllerSettingsStruct::CONTROLLER_MAX_RETRIES:
      ControllerSettings.MaxRetry = getFormItemInt(internalName, ControllerSettings.MaxRetry);
      break;
    case ControllerSettingsStruct::CONTROLLER_FULL_QUEUE_ACTION:
      ControllerSettings.DeleteOldest = getFormItemInt(internalName, ControllerSettings.DeleteOldest);
      break;
    case ControllerSettingsStruct::CONTROLLER_ALLOW_EXPIRE:
      ControllerSettings.allowExpire(isFormItemChecked(internalName));
      break;
    case ControllerSettingsStruct::CONTROLLER_DEDUPLICATE:
      ControllerSettings.deduplicate(isFormItemChecked(internalName));
      break;
    case ControllerSettingsStruct::CONTROLLER_CHECK_REPLY:
      ControllerSettings.MustCheckReply = getFormItemInt(internalName, ControllerSettings.MustCheckReply);
      break;

    case ControllerSettingsStruct::CONTROLLER_CLIENT_ID:
      strncpy_webserver_arg(ControllerSettings.ClientID, internalName);
      break;
    case ControllerSettingsStruct::CONTROLLER_UNIQUE_CLIENT_ID_RECONNECT:
      ControllerSettings.mqtt_uniqueMQTTclientIdReconnect(isFormItemChecked(internalName));
      break;
    case ControllerSettingsStruct::CONTROLLER_RETAINFLAG:
      ControllerSettings.mqtt_retainFlag(isFormItemChecked(internalName));
      break;
    case ControllerSettingsStruct::CONTROLLER_SUBSCRIBE:
      strncpy_webserver_arg(ControllerSettings.Subscribe,            internalName);
      break;
    case ControllerSettingsStruct::CONTROLLER_PUBLISH:
      strncpy_webserver_arg(ControllerSettings.Publish,              internalName);
      break;
    case ControllerSettingsStruct::CONTROLLER_LWT_TOPIC:
      strncpy_webserver_arg(ControllerSettings.MQTTLwtTopic,         internalName);
      break;
    case ControllerSettingsStruct::CONTROLLER_LWT_CONNECT_MESSAGE:
      strncpy_webserver_arg(ControllerSettings.LWTMessageConnect,    internalName);
      break;
    case ControllerSettingsStruct::CONTROLLER_LWT_DISCONNECT_MESSAGE:
      strncpy_webserver_arg(ControllerSettings.LWTMessageDisconnect, internalName);
      break;
    case ControllerSettingsStruct::CONTROLLER_SEND_LWT:
      ControllerSettings.mqtt_sendLWT(isFormItemChecked(internalName));
      break;
    case ControllerSettingsStruct::CONTROLLER_WILL_RETAIN:
      ControllerSettings.mqtt_willRetain(isFormItemChecked(internalName));
      break;
    case ControllerSettingsStruct::CONTROLLER_CLEAN_SESSION:
      ControllerSettings.mqtt_cleanSession(isFormItemChecked(internalName));
      break;
    case ControllerSettingsStruct::CONTROLLER_USE_EXTENDED_CREDENTIALS:
      ControllerSettings.useExtendedCredentials(isFormItemChecked(internalName));
      break;
    case ControllerSettingsStruct::CONTROLLER_SEND_BINARY:
      ControllerSettings.sendBinary(isFormItemChecked(internalName));
      break;
    case ControllerSettingsStruct::CONTROLLER_TIMEOUT:
      ControllerSettings.ClientTimeout = getFormItemInt(internalName, ControllerSettings.ClientTimeout);
      break;
    case ControllerSettingsStruct::CONTROLLER_SAMPLE_SET_INITIATOR:
      ControllerSettings.SampleSetInitiator = getFormItemInt(internalName, ControllerSettings.SampleSetInitiator);
      break;
    case ControllerSettingsStruct::CONTROLLER_ENABLED:
      Settings.ControllerEnabled[controllerindex] = isFormItemChecked(internalName);
      break;
  }
}
