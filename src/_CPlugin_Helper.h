#ifndef CPLUGIN_HELPER_H
#define CPLUGIN_HELPER_H CPLUGIN_HELPER_H

// These element classes should be defined as class, to be used as template.

/*********************************************************************************************\
 * MQTT_queue_element for all MQTT base controllers
\*********************************************************************************************/
class MQTT_queue_element {
public:
  MQTT_queue_element() : controller_idx(0), _retained(false) {}

  MQTT_queue_element(int ctrl_idx,
    const String& topic, const String& payload, boolean retained) :
    controller_idx(ctrl_idx), _topic(topic), _payload(payload), _retained(retained)
     {}

  int controller_idx;
  String _topic;
  String _payload;
  boolean _retained;
};

/*********************************************************************************************\
 * C001_queue_element for queueing requests for C001.
\*********************************************************************************************/
class C001_queue_element {
public:
  C001_queue_element() : controller_idx(0) {}
  C001_queue_element(int ctrl_idx, const String& req) : controller_idx(ctrl_idx), url(req) {}

  int controller_idx;
  String url;
};

/*********************************************************************************************\
 * C003_queue_element for queueing requests for C003 Nodo Telnet.
\*********************************************************************************************/
class C003_queue_element {
public:
  C003_queue_element() : controller_idx(0) {}
  C003_queue_element(int ctrl_idx, const String& req) : controller_idx(ctrl_idx), url(req) {}

  int controller_idx;
  String url;
};

/*********************************************************************************************\
 * C004_queue_element for queueing requests for C004 ThingSpeak.
 *   Typical use case for Thingspeak is to only send values every N seconds/minutes.
 *   So we just store everything needed to recreate the event when the time is ready.
\*********************************************************************************************/
class C004_queue_element {
public:
  C004_queue_element() : controller_idx(0), TaskIndex(0), idx(0), sensorType(0) {}
  C004_queue_element(const struct EventStruct* event) :
    controller_idx(event->ControllerIndex),
    TaskIndex(event->TaskIndex),
    idx(event->idx),
    sensorType(event->sensorType) {}

  int controller_idx;
  byte TaskIndex;
  int idx;
  byte sensorType;
};

/*********************************************************************************************\
 * C007_queue_element for queueing requests for C007 Emoncms
\*********************************************************************************************/
class C007_queue_element {
public:
  C007_queue_element() : controller_idx(0), TaskIndex(0), idx(0), sensorType(0) {}
  C007_queue_element(const struct EventStruct* event) :
    controller_idx(event->ControllerIndex),
    TaskIndex(event->TaskIndex),
    idx(event->idx),
    sensorType(event->sensorType) {}

  int controller_idx;
  byte TaskIndex;
  int idx;
  byte sensorType;
};

/*********************************************************************************************\
 * C008_queue_element for queueing requests for 008: Generic HTTP
 * This controller only sends a single value per request and thus needs to keep track of the
 * number of values already sent.
\*********************************************************************************************/
class C008_queue_element {
public:
  C008_queue_element() : controller_idx(0), TaskIndex(0), idx(0), valuesSent(0) {}
  C008_queue_element(const struct EventStruct* event, byte value_count) :
    controller_idx(event->ControllerIndex),
    TaskIndex(event->TaskIndex),
    idx(event->idx),
    valuesSent(0),
    valueCount(value_count) {}

  bool checkDone(bool succesfull) const {
    if (succesfull) ++valuesSent;
    return (valuesSent == valueCount);
  }

  int controller_idx;
  byte TaskIndex;
  int idx;
  String url[VARS_PER_TASK];
  mutable byte valuesSent;
  byte valueCount;
};

/*********************************************************************************************\
 * C009_queue_element for queueing requests for C009: FHEM HTTP.
\*********************************************************************************************/
class C009_queue_element {
public:
  C009_queue_element() : controller_idx(0) {}
  C009_queue_element(int ctrl_idx, const String& URI, const String& JSON) :
     controller_idx(ctrl_idx), url(URI), json(JSON) {}

  int controller_idx;
  String url;
  String json;
};




/*********************************************************************************************\
 * ControllerDelayHandlerStruct
\*********************************************************************************************/
template<class T>
struct ControllerDelayHandlerStruct {
  ControllerDelayHandlerStruct() :
      lastSend(0), minTimeBetweenMessages(100), max_queue_depth(10), attempt(0), max_retries(10), delete_oldest(false) {}

  void configureControllerSettings(const ControllerSettingsStruct& settings) {
    minTimeBetweenMessages = settings.MinimalTimeBetweenMessages;
    max_queue_depth = settings.MaxQueueDepth;
    max_retries = settings.MaxRetry;
    delete_oldest = settings.DeleteOldest;
    // Set some sound limits
    if (max_queue_depth == 0) max_queue_depth = 10;
    if (max_retries == 0) max_retries = 10;
    if (minTimeBetweenMessages == 0) minTimeBetweenMessages = 100;
    if (minTimeBetweenMessages < 10) minTimeBetweenMessages = 10;
  }

  // Try to add to the queue, if permitted by "delete_oldest"
  // Return false when no item was added.
  bool addToQueue(const T& element) {
    if (delete_oldest) {
      forceAddToQueue(element);
      return true;
    }
    if (sendQueue.size() < max_queue_depth) {
      sendQueue.emplace_back(element);
      return true;
    }
    if (loglevelActiveFor(LOG_LEVEL_DEBUG)) {
      String log = get_formatted_Controller_number(element.controller_idx);
      log += " : queue full";
      addLog(LOG_LEVEL_DEBUG, log);
    }
    return false;
  }

  // Force add to the queue.
  // If max buffer is reached, the oldest in the queue (first to be served) will be removed.
  // Return true when no elements removed from queue.
  bool forceAddToQueue(const T& element) {
    sendQueue.emplace_back(element);
    if (sendQueue.size() <= max_queue_depth) {
      return true;
    }
    sendQueue.pop_front();
    return false;
  }

  // Get the next element.
  // Remove front element when max_retries is reached.
  bool getNext(T& element) {
    if (sendQueue.empty()) return false;
    if (attempt > max_retries) {
      sendQueue.pop_front();
      attempt = 0;
      if (sendQueue.empty()) return false;
    }
    element = sendQueue.front();
    return true;
  }

  // Mark as processed and return time to schedule for next process.
  // Return 0 when nothing to process.
  // @param remove_from_queue indicates whether the elements should be removed from the queue.
  unsigned long markProcessed(bool remove_from_queue) {
    if (sendQueue.empty()) return 0;
    if (remove_from_queue) {
      sendQueue.pop_front();
      attempt = 0;
    } else {
      ++attempt;
    }
    lastSend = millis();
    return getNextScheduleTime();
  }

  unsigned long getNextScheduleTime() const {
    if (sendQueue.empty()) return 0;
    unsigned long nextTime = lastSend + minTimeBetweenMessages;
    if (timePassedSince(nextTime) > 0) {
      nextTime = millis();
    }
    if (nextTime == 0) nextTime = 1; // Just to make sure it will be executed
    return nextTime;
  }

  std::list<T> sendQueue;
  unsigned long lastSend;
  unsigned int minTimeBetweenMessages;
  byte max_queue_depth;
  byte attempt;
  byte max_retries;
  bool delete_oldest;
};

ControllerDelayHandlerStruct<MQTT_queue_element> MQTTDelayHandler;


// This macro defines the code needed to create the 'process_c##NNN##_delay_queue()'
// function and all needed objects and forward declarations.
// It is a macro to prevent common typo errors.
// This function will perform the (re)scheduling and mark if it is processed (and can be removed)
// The controller itself must implement the 'do_process_c004_delay_queue' function to actually
// send the data.
// Its return value must state whether it can be marked 'Processed'.
// N.B. some controllers only can send one value per iteration, so a returned "false" can mean it
//      was still successful. The controller should keep track of the last value sent
//      in the element stored in the queue.
#define DEFINE_Cxxx_DELAY_QUEUE_MACRO(NNN) \
                ControllerDelayHandlerStruct<C##NNN##_queue_element> C##NNN##_DelayHandler; \
                bool do_process_c##NNN##_delay_queue(const C##NNN##_queue_element& element, ControllerSettingsStruct& ControllerSettings); \
                void process_c##NNN##_delay_queue() { \
                  C##NNN##_queue_element element; \
                  if (!C##NNN##_DelayHandler.getNext(element)) return; \
                  ControllerSettingsStruct ControllerSettings; \
                  LoadControllerSettings(element.controller_idx, (byte*)&ControllerSettings, sizeof(ControllerSettings)); \
                  C##NNN##_DelayHandler.configureControllerSettings(ControllerSettings); \
                  if (!WiFiConnected(100)) { \
                    scheduleNextDelayQueue(TIMER_C##NNN##_DELAY_QUEUE, C##NNN##_DelayHandler.getNextScheduleTime()); \
                    return; \
                  } \
                  START_TIMER; \
                  C##NNN##_DelayHandler.markProcessed(do_process_c##NNN##_delay_queue(element, ControllerSettings)); \
                  STOP_TIMER(C##NNN##_DELAY_QUEUE); \
                  scheduleNextDelayQueue(TIMER_C##NNN##_DELAY_QUEUE, C##NNN##_DelayHandler.getNextScheduleTime()); \
                }

// Define the function wrappers to handle the calling to Cxxx_DelayHandler etc.
#ifdef USES_C001
  DEFINE_Cxxx_DELAY_QUEUE_MACRO(001)
#endif
#ifdef USES_C003
  DEFINE_Cxxx_DELAY_QUEUE_MACRO(003)
#endif
#ifdef USES_C004
  DEFINE_Cxxx_DELAY_QUEUE_MACRO(004)
#endif
#ifdef USES_C007
  DEFINE_Cxxx_DELAY_QUEUE_MACRO(007)
#endif
#ifdef USES_C008
  DEFINE_Cxxx_DELAY_QUEUE_MACRO(008)
#endif
#ifdef USES_C009
  DEFINE_Cxxx_DELAY_QUEUE_MACRO(009)
#endif
/*
#ifdef USES_C010
  DEFINE_Cxxx_DELAY_QUEUE_MACRO(010)
#endif
#ifdef USES_C011
  DEFINE_Cxxx_DELAY_QUEUE_MACRO(011)
#endif
#ifdef USES_C012
  DEFINE_Cxxx_DELAY_QUEUE_MACRO(012)
#endif
#ifdef USES_C013
  DEFINE_Cxxx_DELAY_QUEUE_MACRO(013)
#endif
*/

/*********************************************************************************************\
 * Helper functions used in a number of controllers
\*********************************************************************************************/
bool safeReadStringUntil(Stream &input, String &str, char terminator, unsigned int maxSize = 1024, unsigned int timeout = 1000)
{
	int c;
	const unsigned long timer = millis() + timeout;
	str = "";

	do {
		//read character
		c = input.read();
		if (c >= 0) {
			//found terminator, we're ok
			if (c == terminator) {
				return(true);
			}
			//found character, add to string
			else{
				str += char(c);
				//string at max size?
				if (str.length() >= maxSize) {
					addLog(LOG_LEVEL_ERROR, F("Not enough bufferspace to read all input data!"));
					return(false);
				}
			}
		}
		yield();
	} while (!timeOutReached(timer));

	addLog(LOG_LEVEL_ERROR, F("Timeout while reading input data!"));
	return(false);
}

String get_formatted_Controller_number(int controller_index) {
  String result = F("C");
  if (controller_index < 100) result += '0';
  if (controller_index < 10) result += '0';
  result += controller_index;
  return result;
}

String get_auth_header(int controller_index) {
  String authHeader = "";
  if ((SecuritySettings.ControllerUser[controller_index][0] != 0) &&
      (SecuritySettings.ControllerPassword[controller_index][0] != 0))
  {
    base64 encoder;
    String auth = SecuritySettings.ControllerUser[controller_index];
    auth += ":";
    auth += SecuritySettings.ControllerPassword[controller_index];
    authHeader = F("Authorization: Basic ");
    authHeader += encoder.encode(auth);
    authHeader += F(" \r\n");
  }
  return authHeader;
}

String do_create_http_request(
    const String& hostportString,
    const String& method, const String& uri,
    const String& auth_header, const String& additional_options,
    int content_length) {
  int estimated_size = hostportString.length() + method.length()
                       + uri.length() + auth_header.length()
                       + additional_options.length()
                       + 42;
  if (content_length >= 0) estimated_size += 25;
  String request;
  request.reserve(estimated_size);
  request += method;
  request += ' ';
  if (!uri.startsWith("/")) request += '/';
  request += uri;
  request += F(" HTTP/1.1\r\n");
  if (content_length >= 0) {
    request += F("Content-Length: ");
    request += content_length;
    request += F("\r\n");
  }
  request += F("Host: ");
  request += hostportString;
  request += F("\r\n");
  request += auth_header;
  request += additional_options;
  request += F("Connection: close\r\n");
  request += F("\r\n");
  return request;
}

String do_create_http_request(
    const String& hostportString,
    const String& method, const String& uri) {
  return do_create_http_request(hostportString, method, uri,
    "", // auth_header
    "", // additional_options
    -1  // content_length
  );
}

String do_create_http_request_no_portnr(
    int controller_index, ControllerSettingsStruct& ControllerSettings,
    const String& method, const String& uri,
    bool include_auth_header, int content_length) {
  return do_create_http_request(
    ControllerSettings.getHost(),
    method,
    uri,
    include_auth_header ? get_auth_header(controller_index) : "",
    "", // additional_options
    content_length);
}

String do_create_http_request(
    int controller_index, ControllerSettingsStruct& ControllerSettings,
    const String& method, const String& uri,
    bool include_auth_header, int content_length) {
  const bool defaultport = ControllerSettings.Port == 0 || ControllerSettings.Port == 80;
  return do_create_http_request(
    defaultport ? ControllerSettings.getHost() : ControllerSettings.getHostPortString(),
    method,
    uri,
    include_auth_header ? get_auth_header(controller_index) : "",
    "", // additional_options
    content_length);
}

String create_http_get_request(int controller_index, ControllerSettingsStruct& ControllerSettings,
    const String& uri) {
  return do_create_http_request(controller_index, ControllerSettings, F("GET"), uri, false, -1);
}

String create_http_request_auth(int controller_index, ControllerSettingsStruct& ControllerSettings,
    const String& method, const String& uri) {
  return do_create_http_request(controller_index, ControllerSettings, method, uri, true, -1);
}

String create_http_request_auth(int controller_index, ControllerSettingsStruct& ControllerSettings,
    const String& method, const String& uri, int content_length) {
  return do_create_http_request(controller_index, ControllerSettings, method, uri, true, content_length);
}

String create_http_request_auth_no_portnr(int controller_index, ControllerSettingsStruct& ControllerSettings,
    const String& method, const String& uri) {
  return do_create_http_request_no_portnr(controller_index, ControllerSettings, method, uri, true, -1);
}

String create_http_request_auth_no_portnr(int controller_index, ControllerSettingsStruct& ControllerSettings,
    const String& method, const String& uri, int content_length) {
  return do_create_http_request_no_portnr(controller_index, ControllerSettings, method, uri, true, content_length);
}

bool try_connect_host(int controller_index, WiFiClient& client, ControllerSettingsStruct& ControllerSettings) {
  // Use WiFiClient class to create TCP connections
  String log = F("HTTP : ");
  log += get_formatted_Controller_number(controller_index);
  log += F(" connecting to ");
  log += ControllerSettings.getHostPortString();
  addLog(LOG_LEVEL_DEBUG, log);
  if (!ControllerSettings.connectToHost(client))
  {
    connectionFailures++;
    if (loglevelActiveFor(LOG_LEVEL_ERROR)) {
      log = F("HTTP : ");
      log += get_formatted_Controller_number(controller_index);
      log += F(" connection failed");
      addLog(LOG_LEVEL_ERROR, log);
    }
    return false;
  }
  statusLED(true);
  if (connectionFailures)
    connectionFailures--;
  return true;
}

bool send_via_http(const String& logIdentifier, WiFiClient& client, const String& postStr) {
  bool success = false;
  // This will send the request to the server
  client.print(postStr);

  unsigned long timer = millis() + 200;
  while (!client.available() && !timeOutReached(timer))
    delay(1);

  // Read all the lines of the reply from server and print them to Serial
  while (client.available() && !success) {
    //   String line = client.readStringUntil('\n');
    String line;
    safeReadStringUntil(client, line, '\n');

    if (loglevelActiveFor(LOG_LEVEL_DEBUG_MORE)) {
      if (line.length() > 80) {
        addLog(LOG_LEVEL_DEBUG_MORE, line.substring(0, 80));
      } else {
        addLog(LOG_LEVEL_DEBUG_MORE, line);
      }
    }
    if (line.startsWith(F("HTTP/1.1 2")))
    {
      success = true;
      if (loglevelActiveFor(LOG_LEVEL_DEBUG)) {
        String log = F("HTTP : ");
        log += logIdentifier;
        log += F(" Success! ");
        log += line;
        addLog(LOG_LEVEL_DEBUG, log);
      }
    } else if (line.startsWith(F("HTTP/1.1 4"))) {
      if (loglevelActiveFor(LOG_LEVEL_ERROR)) {
        String log = F("HTTP : ");
        log += logIdentifier;
        log += F(" Error: ");
        log += line;
        addLog(LOG_LEVEL_ERROR, log);
      }
      addLog(LOG_LEVEL_DEBUG_MORE, postStr);
    }
    yield();
  }
  if (loglevelActiveFor(LOG_LEVEL_DEBUG)) {
    String log = F("HTTP : ");
    log += logIdentifier;
    log += F(" closing connection");
    addLog(LOG_LEVEL_DEBUG, log);
  }

  client.flush();
  client.stop();
  return success;
}

bool send_via_http(int controller_index, WiFiClient& client, const String& postStr) {
  return send_via_http(get_formatted_Controller_number(controller_index), client, postStr);
}


#endif // CPLUGIN_HELPER_H
