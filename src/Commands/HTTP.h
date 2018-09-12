#ifndef COMMAND_HTTP_H
#define COMMAND_HTTP_H


String Command_HTTP_SendToHTTP(struct EventStruct *event, const char* Line)
{
	if (wifiStatus == ESPEASY_WIFI_SERVICES_INITIALIZED) {
		String strLine = Line;
		String host = parseString(strLine, 2);
		String port = parseString(strLine, 3);
		if (loglevelActiveFor(LOG_LEVEL_DEBUG)) {
			String log = F("SendToHTTP: Host: ");
			log += host;
			log += F(" port: ");
			log += port;
			addLog(LOG_LEVEL_DEBUG, log);
		}
		if (!isInt(port)) return return_command_failed();
		String path = parseStringToEndKeepCase(strLine, 4);
		if (loglevelActiveFor(LOG_LEVEL_DEBUG)) {
			String log = F("SendToHTTP: Path: ");
			log += path;
			addLog(LOG_LEVEL_DEBUG, log);
		}
		WiFiClient client;
		const int port_int = port.toInt();
		const bool connected = client.connect(host.c_str(), port_int) == 1;
		if (connected) {
			String hostportString = host;
			if (port_int != 0 && port_int != 80) {
				hostportString += ':';
				hostportString += port_int;
			}
			String request = do_create_http_request(hostportString, F("GET"), path);
			addLog(LOG_LEVEL_DEBUG, request);
			send_via_http(F("Command_HTTP_SendToHTTP"), client, request);
		}
	}
	return return_command_success();
}

#endif // COMMAND_HTTP_H
