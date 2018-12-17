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

  size_t getSize() const {
    return sizeof(this) + _topic.length() + _payload.length();
  }

  int controller_idx;
  String _topic;
  String _payload;
  boolean _retained;
};

/*********************************************************************************************\
 * Simple queue element, only storing controller index and some String
\*********************************************************************************************/
class simple_queue_element_string_only {
public:
  simple_queue_element_string_only() : controller_idx(0) {}
  simple_queue_element_string_only(int ctrl_idx, const String& req) :
       controller_idx(ctrl_idx), txt(req) {}

  size_t getSize() const {
    return sizeof(this) + txt.length();
  }

  int controller_idx;
  String txt;
};


/*********************************************************************************************\
 * C001_queue_element for queueing requests for C001.
\*********************************************************************************************/
#define C001_queue_element simple_queue_element_string_only

/*********************************************************************************************\
 * C003_queue_element for queueing requests for C003 Nodo Telnet.
\*********************************************************************************************/
#define C003_queue_element simple_queue_element_string_only

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

  size_t getSize() const {
    return sizeof(this);
  }

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

  size_t getSize() const {
    return sizeof(this);
  }

  int controller_idx;
  byte TaskIndex;
  int idx;
  byte sensorType;
};

/*********************************************************************************************\
 * Base class for controllers that only send a single value per request and thus needs to
 * keep track of the number of values already sent.
\*********************************************************************************************/
class queue_element_single_value_base {
public:
  queue_element_single_value_base() : controller_idx(0), TaskIndex(0), idx(0), valuesSent(0) {}
  queue_element_single_value_base(const struct EventStruct* event, byte value_count) :
    controller_idx(event->ControllerIndex),
    TaskIndex(event->TaskIndex),
    idx(event->idx),
    valuesSent(0),
    valueCount(value_count) {}

  bool checkDone(bool succesfull) const {
    if (succesfull) ++valuesSent;
    return (valuesSent >= valueCount || valuesSent >= VARS_PER_TASK);
  }

  size_t getSize() const {
    size_t total = sizeof(this);
    for (int i = 0; i < VARS_PER_TASK; ++i) {
      total += txt[i].length();
    }
    return total;
  }

  String txt[VARS_PER_TASK];
  int controller_idx;
  byte TaskIndex;
  int idx;
  mutable byte valuesSent;  // Value must be set by const function checkDone()
  byte valueCount;
};


/*********************************************************************************************\
 * C008_queue_element for queueing requests for 008: Generic HTTP
 * Using queue_element_single_value_base
\*********************************************************************************************/
#define C008_queue_element queue_element_single_value_base

/*********************************************************************************************\
 * C009_queue_element for queueing requests for C009: FHEM HTTP.
\*********************************************************************************************/
class C009_queue_element {
public:
  C009_queue_element() : controller_idx(0), TaskIndex(0), idx(0), sensorType(0) {}
  C009_queue_element(const struct EventStruct* event) :
    controller_idx(event->ControllerIndex),
    TaskIndex(event->TaskIndex),
    idx(event->idx),
    sensorType(event->sensorType) {}

  size_t getSize() const {
    size_t total = sizeof(this);
    for (int i = 0; i < VARS_PER_TASK; ++i) {
      total += txt[i].length();
    }
    return total;
  }

  String txt[VARS_PER_TASK];
  int controller_idx;
  byte TaskIndex;
  int idx;
  byte sensorType;
};


/*********************************************************************************************\
 * C010_queue_element for queueing requests for 010: Generic UDP
 * Using queue_element_single_value_base
\*********************************************************************************************/
#define C010_queue_element queue_element_single_value_base

/*********************************************************************************************\
 * C011_queue_element for queueing requests for 011: Generic HTTP Advanced
\*********************************************************************************************/
#define C011_queue_element simple_queue_element_string_only

/*********************************************************************************************\
 * C012_queue_element for queueing requests for 012: Blynk
 * Using queue_element_single_value_base
\*********************************************************************************************/
#define C012_queue_element queue_element_single_value_base



/*********************************************************************************************\
 * ControllerDelayHandlerStruct
\*********************************************************************************************/
template<class T>
struct ControllerDelayHandlerStruct {
  ControllerDelayHandlerStruct() :
      lastSend(0),
      minTimeBetweenMessages(CONTROLLER_DELAY_QUEUE_DELAY_DFLT),
      max_queue_depth(CONTROLLER_DELAY_QUEUE_DEPTH_DFLT),
      attempt(0),
      max_retries(CONTROLLER_DELAY_QUEUE_RETRY_DFLT),
      delete_oldest(false),
      must_check_reply(false) {}

  void configureControllerSettings(const ControllerSettingsStruct& settings) {
    minTimeBetweenMessages = settings.MinimalTimeBetweenMessages;
    max_queue_depth = settings.MaxQueueDepth;
    max_retries = settings.MaxRetry;
    delete_oldest = settings.DeleteOldest;
    must_check_reply = settings.MustCheckReply;
    // Set some sound limits when not configured
    if (max_queue_depth == 0) max_queue_depth = CONTROLLER_DELAY_QUEUE_DEPTH_DFLT;
    if (max_retries == 0) max_retries = CONTROLLER_DELAY_QUEUE_RETRY_DFLT;
    if (minTimeBetweenMessages == 0) minTimeBetweenMessages = CONTROLLER_DELAY_QUEUE_DELAY_DFLT;
    // No less than 10 msec between messages.
    if (minTimeBetweenMessages < 10) minTimeBetweenMessages = 10;
  }

  bool queueFull(const T& element) const {
    if (sendQueue.size() >= max_queue_depth) return true;

    // Number of elements is not exceeding the limit, check memory
    int freeHeap = ESP.getFreeHeap();
    if (freeHeap > 5000) return false; // Memory is not an issue.
    if (loglevelActiveFor(LOG_LEVEL_DEBUG)) {
      String log = "Controller-";
      log += element.controller_idx +1;
      log += " : Memory used: ";
      log += getQueueMemorySize();
      log += " bytes ";
      log += sendQueue.size();
      log += " items ";
      log += freeHeap;
      log += " free";
      addLog(LOG_LEVEL_DEBUG, log);
    }
    return true;
  }

  // Try to add to the queue, if permitted by "delete_oldest"
  // Return false when no item was added.
  bool addToQueue(const T& element) {
    if (delete_oldest) {
      // Force add to the queue.
      // If max buffer is reached, the oldest in the queue (first to be served) will be removed.
      while (queueFull(element)) {
        sendQueue.pop_front();
      }
      sendQueue.emplace_back(element);
      return true;
    }
    if (!queueFull(element)) {
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

  // Get the next element.
  // Remove front element when max_retries is reached.
  T* getNext() {
    if (sendQueue.empty()) return NULL;
    if (attempt > max_retries) {
      sendQueue.pop_front();
      attempt = 0;
      if (sendQueue.empty()) return NULL;
    }
    return &sendQueue.front();
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

  size_t getQueueMemorySize() const {
    size_t totalSize = 0;
    for (auto it = sendQueue.begin(); it != sendQueue.end(); ++it) {
      totalSize += it->getSize();
    }
    return totalSize;
  }

  std::list<T> sendQueue;
  unsigned long lastSend;
  unsigned int minTimeBetweenMessages;
  byte max_queue_depth;
  byte attempt;
  byte max_retries;
  bool delete_oldest;
  bool must_check_reply;
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
#define DEFINE_Cxxx_DELAY_QUEUE_MACRO(NNN, M) \
                ControllerDelayHandlerStruct<C##NNN##_queue_element> C##NNN##_DelayHandler; \
                bool do_process_c##NNN##_delay_queue(int controller_number, const C##NNN##_queue_element& element, ControllerSettingsStruct& ControllerSettings); \
                void process_c##NNN##_delay_queue() { \
                  C##NNN##_queue_element* element(C##NNN##_DelayHandler.getNext()); \
                  if (element == NULL) return; \
                  MakeControllerSettings(ControllerSettings); \
                  LoadControllerSettings(element->controller_idx, ControllerSettings); \
                  C##NNN##_DelayHandler.configureControllerSettings(ControllerSettings); \
                  if (!WiFiConnected(100)) { \
                    scheduleNextDelayQueue(TIMER_C##NNN##_DELAY_QUEUE, C##NNN##_DelayHandler.getNextScheduleTime()); \
                    return; \
                  } \
                  START_TIMER; \
                  C##NNN##_DelayHandler.markProcessed(do_process_c##NNN##_delay_queue(M, *element, ControllerSettings)); \
                  STOP_TIMER(C##NNN##_DELAY_QUEUE); \
                  scheduleNextDelayQueue(TIMER_C##NNN##_DELAY_QUEUE, C##NNN##_DelayHandler.getNextScheduleTime()); \
                }

// Define the function wrappers to handle the calling to Cxxx_DelayHandler etc.
// If someone knows how to add leading zeros in macros, please be my guest :)
#ifdef USES_C001
  DEFINE_Cxxx_DELAY_QUEUE_MACRO(001, 1)
#endif
#ifdef USES_C003
  DEFINE_Cxxx_DELAY_QUEUE_MACRO(003, 3)
#endif
#ifdef USES_C004
  DEFINE_Cxxx_DELAY_QUEUE_MACRO(004, 4)
#endif
#ifdef USES_C007
  DEFINE_Cxxx_DELAY_QUEUE_MACRO(007, 7)
#endif
#ifdef USES_C008
  DEFINE_Cxxx_DELAY_QUEUE_MACRO(008, 8)
#endif
#ifdef USES_C009
  DEFINE_Cxxx_DELAY_QUEUE_MACRO(009, 9)
#endif
#ifdef USES_C010
  DEFINE_Cxxx_DELAY_QUEUE_MACRO(010, 10)
#endif
#ifdef USES_C011
  DEFINE_Cxxx_DELAY_QUEUE_MACRO(011, 11)
#endif
#ifdef USES_C012
  DEFINE_Cxxx_DELAY_QUEUE_MACRO(012, 12)
#endif
/*
#ifdef USES_C013
  DEFINE_Cxxx_DELAY_QUEUE_MACRO(013, 13)
#endif
*/
// When extending this, also extend in Scheduler.ino:
// void process_interval_timer(unsigned long id, unsigned long lasttimer)


/*********************************************************************************************\
 * Helper functions used in a number of controllers
\*********************************************************************************************/
bool safeReadStringUntil(Stream &input, String &str, char terminator, unsigned int maxSize = 1024, unsigned int timeout = 1000)
{
	int c;
  const unsigned long start = millis();
	const unsigned long timer = start + timeout;
  unsigned long backgroundtasks_timer = start + 10;
	str = "";

	do {
		//read character
    if (input.available()) {
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
      // We must run the backgroundtasks every now and then.
      if (timeOutReached(backgroundtasks_timer)) {
        backgroundtasks_timer += 10;
        backgroundtasks();
      } else {
        delay(0);
      }
    } else {
      delay(0);
    }
	} while (!timeOutReached(timer));

	addLog(LOG_LEVEL_ERROR, F("Timeout while reading input data!"));
	return(false);
}

bool valid_controller_number(int controller_number) {
  if (controller_number < 0) return false;
  return true;
//  return getProtocolIndex(controller_number) <= protocolCount;
}

String get_formatted_Controller_number(int controller_number) {
  if (!valid_controller_number(controller_number)) {
    return F("C---");
  }
  String result = F("C");
  if (controller_number < 100) result += '0';
  if (controller_number < 10) result += '0';
  result += controller_number;
  return result;
}

String get_auth_header(int controller_index) {
  String authHeader = "";
  if (controller_index < CONTROLLER_MAX) {
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
  } else {
    addLog(LOG_LEVEL_ERROR, F("Invalid controller index"));
  }
  return authHeader;
}

String get_user_agent_request_header_field() {
  static unsigned int agent_size = 20;
  String request;
  request.reserve(agent_size);
  request = F("User-Agent: ");
  request += F("ESP Easy/");
  request += BUILD;
  request += '/';
  request += String(CRCValues.compileDate);
  request += ' ';
  request += String(CRCValues.compileTime);
  request += "\r\n";
  agent_size = request.length();
  return request;
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
  request += F(" HTTP/1.1");
  request += "\r\n";
  if (content_length >= 0) {
    request += F("Content-Length: ");
    request += content_length;
    request += "\r\n";
  }
  request += F("Host: ");
  request += hostportString;
  request += "\r\n";
  request += auth_header;
  request += additional_options;
  request += get_user_agent_request_header_field();
  request += F("Connection: close\r\n");
  request += "\r\n";
  addLog(LOG_LEVEL_DEBUG, request);
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

String do_create_http_request(
    int controller_number, ControllerSettingsStruct& ControllerSettings,
    const String& method, const String& uri,
    int content_length) {
  const bool defaultport = ControllerSettings.Port == 0 || ControllerSettings.Port == 80;
  return do_create_http_request(
    defaultport ? ControllerSettings.getHost() : ControllerSettings.getHostPortString(),
    method,
    uri,
    "", // auth_header
    "", // additional_options
    content_length);
}

String create_http_request_auth(
    int controller_number, int controller_index, ControllerSettingsStruct& ControllerSettings,
    const String& method, const String& uri,
    int content_length) {
  const bool defaultport = ControllerSettings.Port == 0 || ControllerSettings.Port == 80;
  return do_create_http_request(
    defaultport ? ControllerSettings.getHost() : ControllerSettings.getHostPortString(),
    method,
    uri,
    get_auth_header(controller_index),
    "", // additional_options
    content_length);
}

String create_http_get_request(int controller_number, ControllerSettingsStruct& ControllerSettings,
    const String& uri) {
  return do_create_http_request(controller_number, ControllerSettings, F("GET"), uri, -1);
}

String create_http_request_auth(int controller_number, int controller_index, ControllerSettingsStruct& ControllerSettings,
    const String& method, const String& uri) {
  return create_http_request_auth(controller_number, controller_index, ControllerSettings, method, uri, -1);
}

void log_connecting_to(const String& prefix, int controller_number, ControllerSettingsStruct& ControllerSettings) {
  if (loglevelActiveFor(LOG_LEVEL_DEBUG)) {
    String log = prefix;
    log += get_formatted_Controller_number(controller_number);
    log += F(" connecting to ");
    log += ControllerSettings.getHostPortString();
    addLog(LOG_LEVEL_DEBUG, log);
  }
}

void log_connecting_fail(const String& prefix, int controller_number, ControllerSettingsStruct& ControllerSettings) {
  if (loglevelActiveFor(LOG_LEVEL_ERROR)) {
    String log = prefix;
    log += get_formatted_Controller_number(controller_number);
    log += F(" connection failed");
    addLog(LOG_LEVEL_ERROR, log);
  }
}

bool count_connection_results(bool success, const String& prefix, int controller_number, ControllerSettingsStruct& ControllerSettings) {
  if (!success)
  {
    connectionFailures++;
    log_connecting_fail(prefix, controller_number, ControllerSettings);
    return false;
  }
  statusLED(true);
  if (connectionFailures)
    connectionFailures--;
  return true;
}

bool try_connect_host(int controller_number, WiFiUDP& client, ControllerSettingsStruct& ControllerSettings) {
  START_TIMER;
  client.setTimeout(ControllerSettings.ClientTimeout);
  log_connecting_to(F("UDP  : "), controller_number, ControllerSettings);
  bool success = ControllerSettings.beginPacket(client) != 0;
  const bool result = count_connection_results(
      success,
      F("UDP  : "), controller_number, ControllerSettings);
  STOP_TIMER(TRY_CONNECT_HOST_UDP);
  return result;
}

bool try_connect_host(int controller_number, WiFiClient& client, ControllerSettingsStruct& ControllerSettings) {
  START_TIMER;
  // Use WiFiClient class to create TCP connections
  client.setTimeout(ControllerSettings.ClientTimeout);
  log_connecting_to(F("HTTP : "), controller_number, ControllerSettings);
  bool success = ControllerSettings.connectToHost(client);
  const bool result = count_connection_results(
      success,
      F("HTTP : "), controller_number, ControllerSettings);
  STOP_TIMER(TRY_CONNECT_HOST_TCP);
  return result;
}

// Use "client.available() || client.connected()" to read all lines from slow servers.
// See: https://github.com/esp8266/Arduino/pull/5113
//      https://github.com/esp8266/Arduino/pull/1829
bool client_available(WiFiClient& client) {
  delay(0);
  return client.available() || client.connected();
}

bool send_via_http(const String& logIdentifier, WiFiClient& client, const String& postStr, bool must_check_reply) {
  bool success = !must_check_reply;
  // This will send the request to the server
  byte written = client.print(postStr);
  // as of 2018/11/01 the print function only returns one byte (upd to 256 chars sent). However if the string sent can be longer than this therefore we calculate modulo 256.
  // see discussion here https://github.com/letscontrolit/ESPEasy/pull/1979
  // and implementation here https://github.com/esp8266/Arduino/blob/561426c0c77e9d05708f2c4bf2a956d3552a3706/libraries/ESP8266WiFi/src/include/ClientContext.h#L437-L467
  // this needs to be adjusted if the WiFiClient.print method changes.
  if (written != (postStr.length()%256)) {
    if (loglevelActiveFor(LOG_LEVEL_ERROR)) {
      String log = F("HTTP : ");
      log += logIdentifier;
      log += F(" Error: could not write to client (");
      log += written;
      log += "/";
      log += postStr.length();
      log += ")";
      addLog(LOG_LEVEL_ERROR, log);
    }
    success = false;
  } else {
    if (loglevelActiveFor(LOG_LEVEL_DEBUG)) {
      String log = F("HTTP : ");
      log += logIdentifier;
      log += F(" written to client (");
      log += written;
      log += "/";
      log += postStr.length();
      log += ")";
      addLog(LOG_LEVEL_DEBUG, log);
    }
  }

  if (must_check_reply) {
    unsigned long timer = millis() + 200;
    while (!client_available(client)) {
      if (timeOutReached(timer)) return false;
      delay(1);
    }

    // Read all the lines of the reply from server and print them to Serial
    while (client_available(client) && !success) {
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
      delay(0);
    }
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

bool send_via_http(int controller_number, WiFiClient& client, const String& postStr, bool must_check_reply) {
  return send_via_http(get_formatted_Controller_number(controller_number), client, postStr, must_check_reply);
}


#endif // CPLUGIN_HELPER_H
