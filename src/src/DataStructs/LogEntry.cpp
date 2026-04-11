#include "../DataStructs/LogEntry.h"


#include "../Helpers/ESPEasy_time_calc.h"
#include "../Helpers/StringConverter.h"


LogEntry_t::LogEntry_t(const uint8_t              logLevel,
                       const __FlashStringHelper *message) :
  _message((void *)(message)),
  _timestamp(millis()),
  _strLength(message ? strlen_P((const char *)(message)) : 0),
  _isFlashString(true),
  _logLevel(logLevel),
  _subscriberPendingRead(0)
{}

LogEntry_t::LogEntry_t(const uint8_t logLevel,
                       const char   *message) :
  _message(nullptr),
  _timestamp(millis()),
  _strLength(message ? strlen_P((const char *)(message)) : 0),
  _isFlashString(false),
  _logLevel(logLevel),
  _subscriberPendingRead(0)
{
  if (_strLength && loglevelActiveFor(logLevel)) {
    _message = special_calloc(1, _strLength + 1);

    if (_message) {
      memcpy_P(_message, message, _strLength);
    }
  }
}

LogEntry_t::LogEntry_t(const uint8_t logLevel,
                       const String& message) :
  _message(nullptr),
  _timestamp(millis()),
  _strLength(message.length()),
  _isFlashString(false),
  _logLevel(logLevel),
  _subscriberPendingRead(0)
{
  if (_strLength && loglevelActiveFor(logLevel)) {

    _message = special_calloc(1, _strLength + 1);

    if (_message) {
      memcpy_P(_message, message.begin(), _strLength);
    }
  }
}

LogEntry_t::LogEntry_t(const uint8_t logLevel,
                       String     && message) :
  _message(nullptr),
  _timestamp(millis()),
  _strLength(message.length()),
  _isFlashString(false),
  _logLevel(logLevel),
  _subscriberPendingRead(0)
{
  // We can't move the allocated memory from 'message'.
  // Just use move so we make sure the memory is de-allocated after this call.
  String str = std::move(message);

  if (_strLength && loglevelActiveFor(logLevel)) {
    _message = special_calloc(1, _strLength + 1);

    if (_message) {
      memcpy_P(_message, str.begin(), _strLength);
    }
  }
}

LogEntry_t::~LogEntry_t()
{
  if (!_isFlashString && (_message != nullptr)) {
    free(_message);
  }
}

LogEntry_t::LogEntry_t(LogEntry_t&& rhs) :
  _message(rhs._message),
  _timestamp(rhs._timestamp),
  _flags(rhs._flags)
{
  rhs._message   = nullptr;
  rhs._timestamp = 0;
  rhs._flags     = 0;
}

LogEntry_t& LogEntry_t::operator=(LogEntry_t&& rhs)
{
  _message   = rhs._message;
  _timestamp = rhs._timestamp;
  _flags     = rhs._flags;

  rhs._message   = nullptr;
  rhs._timestamp = 0;
  rhs._flags     = 0;
  return *this;
}

void LogEntry_t::clear()
{
  if (!_isFlashString && (_message != nullptr)) {
    free(_message);
  }
  _message = nullptr;

  //  _strLength = 0;
}

void LogEntry_t::setSubscribers()
{
  if (isValid()) {
    for (uint32_t i = 0; i < NR_LOG_TO_DESTINATIONS; ++i) {
      bitWrite(_subscriberPendingRead, i,
               loglevelActiveFor(static_cast<LogDestination>(i), _logLevel));
    }
  }

  if (_subscriberPendingRead == 0) {
    clear();
  }
}

void LogEntry_t::updateSubscribers()
{
  if (isValid()) {
    for (uint32_t i = 0; i < NR_LOG_TO_DESTINATIONS; ++i) {
      if (bitRead(_subscriberPendingRead, i)) {
        if (!loglevelActiveFor(static_cast<LogDestination>(i), _logLevel)) {
          bitClear(_subscriberPendingRead, i);
        }
      }
    }
    if (_subscriberPendingRead == 0) {
      clear();
    }
  }
}

void LogEntry_t::markReadBySubscriber(uint8_t subscriber)
{
  if (subscriber < NR_LOG_TO_DESTINATIONS) {
    bitClear(_subscriberPendingRead, subscriber);
  }

  if (_subscriberPendingRead == 0) {
    clear();
  }
}

bool LogEntry_t::validForSubscriber(uint8_t subscriber) const
{
  return isValid() && (subscriber < NR_LOG_TO_DESTINATIONS) && bitRead(_subscriberPendingRead, subscriber);
}

size_t LogEntry_t::print(Print& out, size_t offset, size_t length) const
{
  if (offset > _strLength) { return 0; }

  const char*begin = (const char *)_message;
  const char*end   = begin;

  if (_strLength > (offset + length)) {
    end += (offset + length);
  }
  else {
    end += _strLength;
  }
  const char*pos = begin + offset;

  for (; pos != end; ++pos) {
    if ((*pos == '\0') || (out.write(*pos) == 0)) { return pos - begin - offset; }
  }
  return pos - begin - offset;
}

String LogEntry_t::getMessage() const
{
  if (_message == nullptr) { return EMPTY_STRING; }

  if (_isFlashString) {
    return String((const __FlashStringHelper *)_message);
  }
  return String((const char *)_message);
}

bool LogEntry_t::isExpired() const
{
  return
    !isValid() ||
    timePassedSince(_timestamp) >= LOG_BUFFER_EXPIRE;
}

bool LogEntry_t::isValid() const
{
  return _message && (_strLength != 0u);
}

uint8_t LogEntry_t::getLogLevel() const
{
  return _logLevel;
}
