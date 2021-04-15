#include "../Commands/HTTP.h"

#include "../../ESPEasy_common.h"

#include "../Commands/Common.h"

#include "../DataStructs/ControllerSettingsStruct.h"
#include "../DataStructs/SettingsStruct.h"

#include "../ESPEasyCore/ESPEasy_Log.h"
#include "../ESPEasyCore/ESPEasyNetwork.h"

#include "../Globals/Settings.h"

#include "../Helpers/_CPlugin_Helper.h"
#include "../Helpers/Misc.h"
#include "../Helpers/Networking.h"
#include "../Helpers/StringParser.h"


String Command_HTTP_SendToHTTP(struct EventStruct *event, const char* Line)
{
	if (NetworkConnected()) {
		String host = parseString(Line, 2);
		const int port = parseCommandArgumentInt(Line, 2);
		if (loglevelActiveFor(LOG_LEVEL_DEBUG)) {
			String log = F("SendToHTTP: Host: ");
			log += host;
			log += F(" port: ");
			log += port;
			addLog(LOG_LEVEL_DEBUG, log);
		}
		if (port < 0 || port > 65535) return return_command_failed();
		// FIXME TD-er: This is not using the tolerant settings option.
    // String path = tolerantParseStringKeepCase(Line, 4);
		String path = parseStringToEndKeepCase(Line, 4);
#ifndef BUILD_NO_DEBUG
		if (loglevelActiveFor(LOG_LEVEL_DEBUG)) {
			String log = F("SendToHTTP: Path: ");
			log += path;
			addLog(LOG_LEVEL_DEBUG, log);
		}
#endif
		WiFiClient client;
		client.setTimeout(CONTROLLER_CLIENTTIMEOUT_MAX);
		const bool connected = connectClient(client, host.c_str(), port);
		if (connected) {
			String hostportString = host;
			if (port != 0 && port != 80) {
				hostportString += ':';
				hostportString += port;
			}
			String request = do_create_http_request(hostportString, F("GET"), path);
#ifndef BUILD_NO_DEBUG
			addLog(LOG_LEVEL_DEBUG, request);
#endif
            bool mustCheckAck = Settings.SendToHttp_ack();
			send_via_http(F("Command_HTTP_SendToHTTP"), client, request, mustCheckAck);
			return return_command_success();
		}
		addLog(LOG_LEVEL_ERROR, F("SendToHTTP connection failed"));
	} else {
		addLog(LOG_LEVEL_ERROR, F("SendToHTTP Not connected to network"));
	}
	return return_command_failed();
}
