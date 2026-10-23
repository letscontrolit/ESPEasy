#include "../PluginStructs/P190_data_struct.h"

#ifdef USES_P190

# include "../../ESPEasy-Globals.h" // printToWebJSON

# define P190_FINGER_TIMEOUT_MS 20000 // Enrolment: max wait for placing / lifting the finger
# define P190_FINGER_POLL_MS    100   // Enrolment: GenImg interval while waiting for the finger
# define P190_TOUCH_RETRY_MS    100   // Touch GPIO active but no image yet: retry interval
# define P190_REINIT_MS         5000  // Retry interval when the module doesn't answer
# define P190_MAX_QUEUE         8
# define P190_MAX_READ_BYTES    64    // Serial bytes handled per loop() call: a full acknowledge (52 bytes max.)

using namespace P190_proto;

void P190_colorSelector(const __FlashStringHelper *label, const __FlashStringHelper *id, int selected) {
  const __FlashStringHelper *options[] = {
    F("Red"), F("Blue"), F("Purple"), F("Green"), F("Yellow"), F("Cyan"), F("White"),
  };
  const int optionValues[] = {
    COLOR_RED,   COLOR_BLUE,   COLOR_PURPLE, COLOR_GREEN,
    COLOR_YELLOW, COLOR_CYAN,  COLOR_WHITE,
  };
  const FormSelectorOptions selector(NR_ELEMENTS(optionValues), options, optionValues);

  selector.addFormSelector(label, id, selected);
}

P190_data_struct::P190_data_struct(struct EventStruct *event) {
  const int mult = P190_BAUD_MULT;

  _address      = P190_ADDRESS;
  _password     = P190_PASSWORD;
  _baud         = 9600ul * ((mult >= 1 && mult <= 12) ? mult : P190_DEF_BAUD_MULT);
  _touchPin     = validGpio(CONFIG_PIN3) ? CONFIG_PIN3 : -1;
  _ledFeedback  = P190_LED_FEEDBACK != 0;
  _matchColor   = constrain(static_cast<int>(P190_MATCH_COLOR), 1, 7);
  _noMatchColor = constrain(static_cast<int>(P190_NOMATCH_COLOR), 1, 7);
  _pollMs       = constrain(static_cast<int>(P190_POLL_MS), 50, 5000);
  _minScore     = max(0, static_cast<int>(P190_MIN_SCORE));
}

P190_data_struct::~P190_data_struct() {
  delete _serial;
}

bool P190_data_struct::init(struct EventStruct *event) {
  const int8_t            rx   = CONFIG_PIN1;
  const int8_t            tx   = CONFIG_PIN2;
  const ESPEasySerialPort port = static_cast<ESPEasySerialPort>(CONFIG_PORT);

  _serial = new (std::nothrow) ESPeasySerial(port, rx, tx);

  if (nullptr == _serial) { return false; }
  _serial->begin(_baud);
  serialHelper_log_GpioDescription(port, rx, tx);

  if (_touchPin >= 0) {
    pinMode(_touchPin, INPUT_PULLUP);
  }

  UserVar.setFloat(event->TaskIndex, P190_VALUE_ID,    -1);
  UserVar.setFloat(event->TaskIndex, P190_VALUE_SCORE, 0);
  UserVar.setFloat(event->TaskIndex, P190_VALUE_COUNT, 0);
  setEnrollState(event, P190_ENROLL_NONE);
  _nextInit = millis();
  return true;
}

bool P190_data_struct::touched() const {
  // The WAKEUP output of the module is low while a finger touches the sensor
  return _touchPin >= 0 && digitalRead(_touchPin) == LOW;
}

void P190_data_struct::addEvent(struct EventStruct *event, const __FlashStringHelper *name, int value) const {
  if (Settings.UseRules) {
    eventQueue.add(event->TaskIndex, name, value);
  }
}

void P190_data_struct::setEnrollState(struct EventStruct *event, int state) const {
  // Only the value: sendData() would also resend ID to the rules and controllers (and open the door again)
  UserVar.setFloat(event->TaskIndex, P190_VALUE_ENROLL, state);
}

void P190_data_struct::reply(struct EventStruct *event, const String& json) const {
  // Answer of /control?cmd=fingerprint,... (or the MQTT status); ignored when the command comes from the rules
  if (event->Source == EventValueSource::Enum::VALUE_SOURCE_HTTP) { printToWebJSON = true; }
  SendStatus(event, json);
}

void P190_data_struct::replyError(struct EventStruct *event, const String& message) const {
  addLog(LOG_LEVEL_ERROR, concat(F("Fingerprint: "), message));
  reply(event, concat(F("{\"error\":\""), message) + F("\"}"));
}

// ----------------------------------------------------------------------------------------------------
// Main loop, called 50 times per second
// ----------------------------------------------------------------------------------------------------

void P190_data_struct::loop(struct EventStruct *event) {
  if (nullptr == _serial) { return; }

  // Bounded: never more than P190_MAX_READ_BYTES per call, also if the module (or noise) keeps sending.
  // The rest is handled on the next call; a missing acknowledge ends with the response timeout below.
  for (int n = min(_serial->available(), P190_MAX_READ_BYTES); n > 0; --n) {
    const int c = _serial->read();

    if (c < 0) { break; }

    if (_parser.feed(static_cast<uint8_t>(c)) && _waiting && (_parser.packet().pid == PID_ACK)) {
      _waiting = false;
      handleAck(event, _parser.packet());
    }
  }

  if (_waiting) {
    if (timePassedSince(_sentAt) > static_cast<long>(_timeout)) {
      _waiting = false;
      handleTimeout(event);
    }
    return;
  }

  if (_cancel) {
    _cancel = false;

    if (_job == P190_Job::Enroll) {
      enrollDone(event, P190_ERR_CANCELLED);
      return;
    }
  }

  if (_step != P190_Step::None) {
    if (timeOutReached(_stepAt)) { sendStep(); }
    return;
  }

  idle(event);
}

void P190_data_struct::idle(struct EventStruct *event) {
  if (!_ready) {
    if (timeOutReached(_nextInit)) {
      _nextInit = millis() + P190_REINIT_MS;
      startJob(event, { P190_Job::Init, 0, {} });
    }
    return;
  }

  if (!_queue.empty()) {
    const P190_request req = _queue.front();
    _queue.pop_front();
    startJob(event, req);
    return;
  }

  if (_touchPin >= 0) {
    if (!touched()) {
      _waitLift = false;
      return;
    }

    if (_waitLift || !timeOutReached(_nextScan)) { return; }
    _nextScan = millis() + P190_TOUCH_RETRY_MS;
    startJob(event, { P190_Job::Scan, 0, {} });
    return;
  }

  // No touch GPIO: poll the module
  if (!timeOutReached(_nextScan)) { return; }
  _nextScan = millis() + _pollMs;

  if (_waitLift) {
    _job = P190_Job::Scan;
    startStep(P190_Step::ScanRemove);
  } else {
    startJob(event, { P190_Job::Scan, 0, {} });
  }
}

bool P190_data_struct::request(const P190_request& req) {
  if (_queue.size() >= P190_MAX_QUEUE) { return false; }
  _queue.push_back(req);
  return true;
}

bool P190_data_struct::enrollPending() const {
  if (_job == P190_Job::Enroll) { return true; }

  for (const auto& req : _queue) {
    if (req.job == P190_Job::Enroll) { return true; }
  }
  return false;
}

void P190_data_struct::refresh() {
  // Re-read the used slots (if the module can) and the template count
  _indexPage = 0;
  startStep(_indexSupported ? P190_Step::ReadIndex : P190_Step::ReadCount);
}

void P190_data_struct::setIndex(uint16_t id, bool used) {
  if (static_cast<size_t>(id >> 3) >= _index.size()) { return; }

  if (used) {
    _index[id >> 3] |= 1 << (id & 7);
  } else {
    _index[id >> 3] &= ~(1 << (id & 7));
  }
}

void P190_data_struct::startJob(struct EventStruct *event, const P190_request& req) {
  _req    = req;
  _job    = req.job;
  _cancel = false;

  switch (_job) {
    case P190_Job::Init:   startStep(P190_Step::VerifyPassword); break;
    case P190_Job::Scan:   startStep(P190_Step::ScanImage);      break;
    case P190_Job::Delete: startStep(P190_Step::Delete);         break;
    case P190_Job::Empty:  startStep(P190_Step::Empty);          break;
    case P190_Job::Count:  startStep(P190_Step::ReadCount);      break;
    case P190_Job::Led:
      led(req.led[0], req.led[2], req.led[1], req.led[3], P190_Step::None);
      break;
    case P190_Job::Enroll:
      _capture        = 1;
      _fingerDeadline = millis() + P190_FINGER_TIMEOUT_MS;
      addLog(LOG_LEVEL_INFO, concat(F("Fingerprint: enrolling ID "), _req.id));
      addEvent(event, F("EnrollScan"), _capture);
      setEnrollState(event, P190_ENROLL_PLACE1);

      if (_ledFeedback) {
        led(AURA_BREATHING, COLOR_PURPLE, 100, 0, P190_Step::EnrollImage);
      } else {
        startStep(P190_Step::EnrollImage);
      }
      break;
    case P190_Job::None:
      finish();
      break;
  }
}

void P190_data_struct::startStep(P190_Step step, uint32_t delayMs) {
  _step   = step;
  _stepAt = millis() + delayMs;
}

void P190_data_struct::finish() {
  _job     = P190_Job::None;
  _step    = P190_Step::None;
  _ledNext = P190_Step::None;
}

void P190_data_struct::led(uint8_t mode, uint8_t color, uint8_t speed, uint8_t times, P190_Step next) {
  _req.led[0] = mode;
  _req.led[1] = speed;
  _req.led[2] = color;
  _req.led[3] = times;
  _ledNext    = next;
  startStep(P190_Step::Led);
}

// ----------------------------------------------------------------------------------------------------
// Commands to the module
// ----------------------------------------------------------------------------------------------------

void P190_data_struct::sendCommand(uint8_t cmd, const uint8_t *params, size_t nparams, uint32_t timeoutMs) {
  // Drop late or unexpected bytes: only those already received, so a continuous stream can't block here
  for (int n = _serial->available(); n > 0; --n) { _serial->read(); }
  _parser.reset();

  uint8_t buf[MAX_CMD_SIZE];
  const size_t len = buildCommand(buf, _address, cmd, params, nparams);

  _serial->write(buf, len);
  _waiting = true;
  _sentAt  = millis();
  _timeout = timeoutMs;
}

void P190_data_struct::sendStep() {
  switch (_step) {
    case P190_Step::VerifyPassword:
    {
      const uint8_t p[] = {
        static_cast<uint8_t>(_password >> 24), static_cast<uint8_t>(_password >> 16),
        static_cast<uint8_t>(_password >> 8),  static_cast<uint8_t>(_password)
      };
      sendCommand(CMD_VFY_PWD, p, sizeof(p));
      break;
    }
    case P190_Step::ReadParams:
      sendCommand(CMD_READ_SYS_PARA, nullptr, 0);
      break;
    case P190_Step::ReadIndex:
    {
      const uint8_t p[] = { _indexPage };
      sendCommand(CMD_READ_INDEX, p, sizeof(p));
      break;
    }
    case P190_Step::ReadCount:
      sendCommand(CMD_TEMPLATE_NUM, nullptr, 0);
      break;
    case P190_Step::ScanImage:
    case P190_Step::ScanRemove:
    case P190_Step::EnrollImage:
    case P190_Step::EnrollRemove:
      sendCommand(CMD_GEN_IMG, nullptr, 0);
      break;
    case P190_Step::ScanFeatures:
    case P190_Step::EnrollFeatures:
    {
      const uint8_t p[] = { static_cast<uint8_t>(_step == P190_Step::ScanFeatures ? 1 : _capture) };
      sendCommand(CMD_IMG_2_TZ, p, sizeof(p));
      break;
    }
    case P190_Step::ScanSearch:
    {
      const uint8_t p[] = { 1, 0, 0, static_cast<uint8_t>(_capacity >> 8), static_cast<uint8_t>(_capacity) };
      sendCommand(CMD_SEARCH, p, sizeof(p), 3000);
      break;
    }
    case P190_Step::EnrollModel:
      sendCommand(CMD_REG_MODEL, nullptr, 0);
      break;
    case P190_Step::EnrollStore:
    {
      const uint8_t p[] = { 1, static_cast<uint8_t>(_req.id >> 8), static_cast<uint8_t>(_req.id) };
      sendCommand(CMD_STORE, p, sizeof(p));
      break;
    }
    case P190_Step::Delete:
    {
      const uint8_t p[] = { static_cast<uint8_t>(_req.id >> 8), static_cast<uint8_t>(_req.id), 0, 1 };
      sendCommand(CMD_DELETE, p, sizeof(p));
      break;
    }
    case P190_Step::Empty:
      sendCommand(CMD_EMPTY, nullptr, 0, 5000);
      break;
    case P190_Step::Led:
      sendCommand(CMD_AURA_LED, _req.led, sizeof(_req.led));
      break;
    case P190_Step::None:
      break;
  }
}

// ----------------------------------------------------------------------------------------------------
// Answers of the module
// ----------------------------------------------------------------------------------------------------

void P190_data_struct::handleAck(struct EventStruct *event, const Packet& pkt) {
  const uint8_t code = pkt.code();

  switch (_step) {
    case P190_Step::VerifyPassword:
      if (code == CODE_OK) {
        startStep(P190_Step::ReadParams);
      } else {
        addLog(LOG_LEVEL_ERROR, code == CODE_WRONG_PASSWORD
               ? String(F("Fingerprint: wrong module password"))
               : concat(F("Fingerprint: password check failed, code "), code));
        _nextInit = millis() + P190_REINIT_MS;
        finish();
      }
      break;

    case P190_Step::ReadParams:
      if (code == CODE_OK) {
        const uint16_t capacity = pkt.word(5);

        if ((capacity > 0) && (capacity <= 3000)) { _capacity = capacity; }
      }
      _index.assign((_capacity + 7) / 8, 0);
      _indexValid = false;
      refresh();
      break;

    case P190_Step::ReadIndex:
      if ((code == CODE_OK) && (pkt.size >= 1 + INDEX_PAGE_BYTES)) {
        for (size_t i = 0; i < INDEX_PAGE_BYTES; ++i) {
          const size_t pos = _indexPage * INDEX_PAGE_BYTES + i;

          if (pos < _index.size()) { _index[pos] = pkt.payload[1 + i]; }
        }
        ++_indexPage;

        if (_indexPage * INDEX_PAGE_TEMPLATES < _capacity) {
          startStep(P190_Step::ReadIndex);
          break;
        }
        _indexValid = true;
      } else {
        addLog(LOG_LEVEL_ERROR, concat(F("Fingerprint: reading the used slots failed, code "), code));
        _indexValid = false;
      }
      startStep(P190_Step::ReadCount);
      break;

    case P190_Step::ReadCount:
      if (code == CODE_OK) {
        _count = pkt.word(1);
        UserVar.setFloat(event->TaskIndex, P190_VALUE_COUNT, _count);
      }

      if (_job == P190_Job::Init) {
        _ready = true;
        addLogMove(LOG_LEVEL_INFO, strformat(F("Fingerprint: module ready, %d of %d templates used"), _count, _capacity));
      } else if (_job == P190_Job::Count) {
        addEvent(event, F("Count"), _count);
      }
      finish();
      break;

    case P190_Step::ScanImage:
      if (code == CODE_OK) {
        startStep(P190_Step::ScanFeatures);
      } else {
        finish(); // No finger (or bad image): try again at the next poll
      }
      break;

    case P190_Step::ScanFeatures:
      if (code == CODE_OK) {
        startStep(P190_Step::ScanSearch);
      } else {
        report(event, -1, 0); // Finger seen, but the image can't be used
      }
      break;

    case P190_Step::ScanSearch:
    {
      const int score = code == CODE_OK ? pkt.word(3) : 0;

      if ((code == CODE_OK) && (score >= _minScore)) {
        report(event, pkt.word(1), score);
      } else {
        report(event, -1, score);
      }
      break;
    }

    case P190_Step::ScanRemove:
      if (code == CODE_NO_FINGER) { _waitLift = false; }
      finish();
      break;

    case P190_Step::EnrollImage:
    case P190_Step::EnrollFeatures:
    case P190_Step::EnrollRemove:
    case P190_Step::EnrollModel:
    case P190_Step::EnrollStore:
      handleEnrollAck(event, code);
      break;

    case P190_Step::Delete:
    case P190_Step::Empty:
      if (code == CODE_OK) {
        addLog(LOG_LEVEL_INFO, _step == P190_Step::Empty
               ? String(F("Fingerprint: all templates deleted"))
               : concat(F("Fingerprint: deleted ID "), _req.id));
        addEvent(event, F("Deleted"), _step == P190_Step::Empty ? -1 : _req.id);

        if (_step == P190_Step::Empty) {
          std::fill(_index.begin(), _index.end(), 0);
        } else {
          setIndex(_req.id, false);
        }
        refresh();
      } else {
        addLog(LOG_LEVEL_ERROR, concat(F("Fingerprint: delete failed, code "), code));
        addEvent(event, F("Error"), code);
        finish();
      }
      break;

    case P190_Step::Led:
      if (code != CODE_OK) {
        addLog(LOG_LEVEL_ERROR, concat(F("Fingerprint: LED command failed, code "), code));

        if (_job == P190_Job::Led) { addEvent(event, F("Error"), code); }
      }

      if (_ledNext != P190_Step::None) {
        startStep(_ledNext);
        _ledNext = P190_Step::None;
      } else {
        finish();
      }
      break;

    case P190_Step::None:
      break;
  }
}

void P190_data_struct::handleEnrollAck(struct EventStruct *event, uint8_t code) {
  switch (_step) {
    case P190_Step::EnrollImage:
      if (code == CODE_OK) {
        startStep(P190_Step::EnrollFeatures);
      } else if (timeOutReached(_fingerDeadline)) {
        enrollDone(event, P190_ERR_FINGER_TIMEOUT);
      } else {
        startStep(P190_Step::EnrollImage, P190_FINGER_POLL_MS);
      }
      break;

    case P190_Step::EnrollFeatures:
      if ((code == CODE_OK) && (_capture == 2)) {
        setEnrollState(event, P190_ENROLL_STORING);
        startStep(P190_Step::EnrollModel);
        break;
      }

      if (code != CODE_OK) {
        // Poor image: lift the finger and repeat the same capture
        addLog(LOG_LEVEL_INFO, concat(F("Fingerprint: image not usable, place the finger again, code "), code));
      }
      addEvent(event, F("EnrollRemove"), _capture);
      setEnrollState(event, P190_ENROLL_LIFT);

      if (code == CODE_OK) { ++_capture; }
      _fingerDeadline = millis() + P190_FINGER_TIMEOUT_MS;
      startStep(P190_Step::EnrollRemove, P190_FINGER_POLL_MS);
      break;

    case P190_Step::EnrollRemove:
      if (code == CODE_NO_FINGER) {
        addEvent(event, F("EnrollScan"), _capture);
        setEnrollState(event, _capture == 1 ? P190_ENROLL_PLACE1 : P190_ENROLL_PLACE2);
        _fingerDeadline = millis() + P190_FINGER_TIMEOUT_MS;
        startStep(P190_Step::EnrollImage, P190_FINGER_POLL_MS);
      } else if (timeOutReached(_fingerDeadline)) {
        enrollDone(event, P190_ERR_FINGER_TIMEOUT);
      } else {
        startStep(P190_Step::EnrollRemove, P190_FINGER_POLL_MS);
      }
      break;

    case P190_Step::EnrollModel:
      if (code == CODE_OK) {
        startStep(P190_Step::EnrollStore);
      } else {
        enrollDone(event, code); // 0x0A: the two captures are not the same finger
      }
      break;

    case P190_Step::EnrollStore:
      enrollDone(event, code);
      break;

    default:
      break;
  }
}

void P190_data_struct::handleTimeout(struct EventStruct *event) {
  if (_step == P190_Step::ReadIndex) {
    // Module without ReadIndexTable: carry on without the used slots (fingerprint,enroll then needs an ID)
    addLog(LOG_LEVEL_INFO, F("Fingerprint: the module doesn't list the used slots"));
    _indexSupported = false;
    _indexValid     = false;
    startStep(P190_Step::ReadCount);
    return;
  }

  if (_ready || (_job != P190_Job::Init)) {
    addLog(LOG_LEVEL_ERROR, F("Fingerprint: no answer from the module, check wiring and baud rate"));
  }
  const P190_Job job = _job;

  _ready    = false;
  _nextInit = millis() + P190_REINIT_MS;

  if (job == P190_Job::Enroll) {
    addEvent(event, F("EnrollFailed"), P190_ERR_NO_RESPONSE);
    setEnrollState(event, -P190_ERR_NO_RESPONSE);
  } else if ((job == P190_Job::Delete) || (job == P190_Job::Empty) || (job == P190_Job::Led) || (job == P190_Job::Count)) {
    addEvent(event, F("Error"), P190_ERR_NO_RESPONSE);
  }
  finish();
}

// ----------------------------------------------------------------------------------------------------
// Results
// ----------------------------------------------------------------------------------------------------

void P190_data_struct::report(struct EventStruct *event, int id, int score) {
  UserVar.setFloat(event->TaskIndex, P190_VALUE_ID,    id);
  UserVar.setFloat(event->TaskIndex, P190_VALUE_SCORE, score);

  if (loglevelActiveFor(LOG_LEVEL_INFO)) {
    addLogMove(LOG_LEVEL_INFO, id >= 0
               ? strformat(F("Fingerprint: match ID %d, score %d"), id, score)
               : strformat(F("Fingerprint: no match (score %d)"), score));
  }
  sendData(event);
  _waitLift = true;

  if (_ledFeedback) {
    led(AURA_FLASHING, id >= 0 ? _matchColor : _noMatchColor, 30, 2, P190_Step::None);
  } else {
    finish();
  }
}

void P190_data_struct::enrollDone(struct EventStruct *event, int errorCode) {
  if (errorCode == CODE_OK) {
    addLog(LOG_LEVEL_INFO, concat(F("Fingerprint: enrolled ID "), _req.id));
    addEvent(event, F("Enrolled"), _req.id);
    setIndex(_req.id, true);
    setEnrollState(event, P190_ENROLL_DONE);
  } else {
    addLog(LOG_LEVEL_ERROR, concat(F("Fingerprint: enrolment failed, code "), errorCode));
    addEvent(event, F("EnrollFailed"), errorCode);
    setEnrollState(event, -errorCode);
  }
  _waitLift = true; // The enrolled finger may still be on the sensor: don't report it as a scan

  // Stored: re-read the used slots and the count
  const P190_Step next = errorCode != CODE_OK ? P190_Step::None
                         : _indexSupported ? P190_Step::ReadIndex : P190_Step::ReadCount;

  _indexPage = 0;

  if (_ledFeedback) {
    led(AURA_FLASHING, errorCode == CODE_OK ? _matchColor : _noMatchColor, 30, 3, next);
  } else if (next != P190_Step::None) {
    startStep(next);
  } else {
    finish();
  }
}

// ----------------------------------------------------------------------------------------------------
// Commands: fingerprint,<subcommand>,...
// ----------------------------------------------------------------------------------------------------

bool P190_data_struct::plugin_write(struct EventStruct *event, const String& string) {
  if (!equals(parseString(string, 1), F("fingerprint"))) { return false; }

  const String  sub = parseString(string, 2);
  P190_request  req{};
  int32_t       value = 0;

  if (equals(sub, F("enroll"))) {
    // fingerprint,enroll[,<id>]: without ID, the first free slot. Answer: {"enroll":<id>} or {"error":"..."}
    const String arg = parseString(string, 3);

    if (enrollPending()) {
      replyError(event, F("an enrolment is already running"));
      return true;
    }

    if (arg.isEmpty()) {
      if (!_ready || !_indexValid) {
        replyError(event, F("used slots unknown, use fingerprint,enroll,<id>"));
        return true;
      }
      value = indexFirstFree(_index.data(), _capacity);

      if (value < 0) {
        replyError(event, F("no free slot"));
        setEnrollState(event, -P190_ERR_LIBRARY_FULL);
        return true;
      }
    } else if (!validIntFromString(arg, value) || (value < 0) || (value >= _capacity)) {
      replyError(event, strformat(F("the ID must be 0..%d"), _capacity - 1));
      return true;
    }
    req.job = P190_Job::Enroll;
    req.id  = value;

    if (!request(req)) {
      replyError(event, F("too many commands queued"));
      return true;
    }
    _enrollSlot = value;
    setEnrollState(event, P190_ENROLL_NONE);
    reply(event, strformat(F("{\"enroll\":%d}"), value));
    return true;
  } else if (equals(sub, F("list"))) {
    // {"ready":true,"capacity":200,"count":2,"enroll":5,"slot":3,"used":[0,3]}
    // enroll/slot: state and slot of the last enrolment, used: null if the module can't list them
    String json = strformat(F("{\"ready\":%s,\"capacity\":%d,\"count\":%d,\"enroll\":%d,\"slot\":%d,\"used\":"),
                            _ready ? "true" : "false", _capacity, _count,
                            static_cast<int>(UserVar.getFloat(event->TaskIndex, P190_VALUE_ENROLL)), _enrollSlot);

    if (_indexValid) {
      bool first = true;
      json += '[';

      for (uint16_t id = 0; id < _capacity; ++id) {
        if (indexUsed(_index.data(), id)) {
          if (!first) { json += ','; }
          json += static_cast<int>(id);
          first = false;
        }
      }
      json += ']';
    } else {
      json += F("null");
    }
    json += '}';
    reply(event, json);
    return true;
  } else if (equals(sub, F("delete"))) {
    if (!validIntFromString(parseString(string, 3), value) || (value < 0) || (value >= _capacity)) {
      addLogMove(LOG_LEVEL_ERROR, strformat(F("Fingerprint: the ID must be 0..%d"), _capacity - 1));
      return true;
    }
    req.job = P190_Job::Delete;
    req.id  = value;
  } else if (equals(sub, F("deleteall"))) {
    req.job = P190_Job::Empty;
  } else if (equals(sub, F("count"))) {
    req.job = P190_Job::Count;
  } else if (equals(sub, F("led"))) {
    // fingerprint,led,<mode>,<color>[,<speed>[,<times>]]
    const uint8_t mode  = ledMode(parseString(string, 3).c_str());
    uint8_t       color = ledColor(parseString(string, 4).c_str());

    if ((mode == AURA_OFF) && (color == 0)) { color = COLOR_RED; }

    int32_t speed = 100, times = 0;
    const String speedArg = parseString(string, 5);
    const String timesArg = parseString(string, 6);

    if ((mode == 0) || (color == 0) ||
        (!speedArg.isEmpty() && !validIntFromString(speedArg, speed)) ||
        (!timesArg.isEmpty() && !validIntFromString(timesArg, times))) {
      addLog(LOG_LEVEL_ERROR, F("Fingerprint: use fingerprint,led,<mode>,<color>[,<speed>[,<times>]]"));
      return true;
    }
    req.job    = P190_Job::Led;
    req.led[0] = mode;
    req.led[1] = constrain(static_cast<int>(speed), 0, 255);
    req.led[2] = color;
    req.led[3] = constrain(static_cast<int>(times), 0, 255);
  } else if (equals(sub, F("cancel"))) {
    bool queued = false;

    for (auto it = _queue.begin(); it != _queue.end();) {
      if (it->job == P190_Job::Enroll) {
        it     = _queue.erase(it);
        queued = true;
      } else {
        ++it;
      }
    }
    _cancel = _job == P190_Job::Enroll;

    if (queued && !_cancel) { setEnrollState(event, -P190_ERR_CANCELLED); } // Never started: no enrollDone()
    return true;
  } else {
    addLog(LOG_LEVEL_ERROR, concat(F("Fingerprint: unknown command "), sub));
    return true;
  }

  if (!request(req)) {
    addLog(LOG_LEVEL_ERROR, F("Fingerprint: too many commands queued"));
  }
  return true;
}

void P190_data_struct::showValues(struct EventStruct *event) const {
  uint8_t varNr = VARS_PER_TASK;
  TaskValuesWriterHelper data(event);

  data.setWriteRegularTaskValuesFirst();
  data.writeCustom(varNr++, F("Module"), _ready ? String(F("OK")) : String(F("No answer")));
  data.writeCustom(varNr++, F("Templates"), strformat(F("%d / %d"), _count, _capacity), true);
}

#endif // ifdef USES_P190
