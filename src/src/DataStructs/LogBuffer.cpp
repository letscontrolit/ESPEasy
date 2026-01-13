#include "../DataStructs/LogBuffer.h"

#include "../Helpers/ESPEasy_time_calc.h"
#include "../Helpers/StringConverter.h"

void LogBuffer::add(LogEntry_t&& logEntry) {
  clearExpiredEntries();

  if (logEntry) {
    LogEntries.emplace_back(std::move(logEntry));
  }
}

bool LogBuffer::getNext(LogDestination logDestination, uint32_t& timestamp, String& message, uint8_t& loglevel)
{
  if (logDestination >= NR_LOG_TO_DESTINATIONS) { return false; }

  lastReadTimeStamp[logDestination] = millis();

  while (cache_iterator_pos[logDestination] < LogEntries.size())
  {
    const auto pos = cache_iterator_pos[logDestination];
    ++cache_iterator_pos[logDestination];

    if (LogEntries[pos].validForSubscriber(logDestination)) {
      timestamp = LogEntries[pos].getTimestamp();
      message   = LogEntries[pos].getMessage();
      loglevel  = LogEntries[pos].getLogLevel();
      LogEntries[pos].markReadBySubscriber(logDestination);
      clearExpiredEntries();
      return true;
    }
  }
  return false;
}

uint32_t LogBuffer::getNrMessages(LogDestination logDestination) const
{
  uint32_t res{};

  if (logDestination >= NR_LOG_TO_DESTINATIONS) { return res; }

  uint32_t pos = cache_iterator_pos[logDestination];

  for (; pos < LogEntries.size(); ++pos) {
    if (LogEntries[pos].validForSubscriber(logDestination)) {
      ++res;
    }
  }
  return res;
}

bool LogBuffer::logActiveRead(LogDestination logDestination) {
  if (logDestination >= NR_LOG_TO_DESTINATIONS) { return false; }
  return timePassedSince(lastReadTimeStamp[logDestination]) < LOG_BUFFER_ACTIVE_READ_TIMEOUT;
}

void LogBuffer::clearExpiredEntries() {
  for (auto it = LogEntries.begin(); it != LogEntries.end();)
  {
    it->updateSubscribers();

    if (it->isExpired()) {
      for (size_t i = 0; i < NR_LOG_TO_DESTINATIONS; ++i) {
        if (cache_iterator_pos[i]) {
          --cache_iterator_pos[i];
        }
      }

      it = LogEntries.erase(it);
    } else {
      return;
    }
  }
}
