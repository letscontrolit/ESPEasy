#include "../DataStructs/LogBuffer.h"

#include "../Helpers/ESPEasy_time_calc.h"
#include "../Helpers/StringConverter.h"

LogBuffer::LogBuffer()
{
  for (size_t i = 0; i < NR_LOG_TO_DESTINATIONS; ++i) {
    cache_iterator_pos[i] = LogEntries.begin();
  }
}

void LogBuffer::add(LogEntry_t&& logEntry) {
  clearExpiredEntries();

  if (logEntry) {
    const auto oldEnd = LogEntries.end();
    LogEntries.emplace_back(std::move(logEntry));
    {
      auto newit = LogEntries.end();
      --newit; // We don't have a function to get an iterator to the last element

      for (size_t i = 0; i < NR_LOG_TO_DESTINATIONS; ++i) {
        if (cache_iterator_pos[i] == oldEnd) {
          cache_iterator_pos[i] = newit;
        }
      }
    }
  }
}

bool LogBuffer::getNext(LogDestination logDestination, uint32_t& timestamp, String& message, uint8_t& loglevel)
{
  if (logDestination >= NR_LOG_TO_DESTINATIONS) { return false; }

  lastReadTimeStamp[logDestination] = millis();

  while (cache_iterator_pos[logDestination] != LogEntries.end())
  {
    const auto pos = cache_iterator_pos[logDestination];
    ++cache_iterator_pos[logDestination];

    if (pos->validForSubscriber(logDestination)) {
      timestamp = pos->getTimestamp();
      message   = pos->getMessage();
      loglevel  = pos->getLogLevel();
      pos->markReadBySubscriber(logDestination);
      clearExpiredEntries();
      return true;
    }
  }
  return false;
}

bool LogBuffer::hasMessages(LogDestination logDestination)
{
  if (logDestination >= NR_LOG_TO_DESTINATIONS) { return false; }

  clearExpiredEntries(); // Cleanup the old stuff first

  auto pos = cache_iterator_pos[logDestination];

  for (; pos != LogEntries.end(); ++pos) {
    if (pos->validForSubscriber(logDestination)) {
      cache_iterator_pos[logDestination] = pos;
      return true;
    }
  }

  lastReadTimeStamp[logDestination] = millis(); // Reset if we aren't going to fetch a next message
  return false;
}

bool LogBuffer::logActiveRead(LogDestination logDestination) const {
  if (logDestination >= NR_LOG_TO_DESTINATIONS) { return false; }
  return timePassedSince(lastReadTimeStamp[logDestination]) < LOG_BUFFER_ACTIVE_READ_TIMEOUT;
}

void LogBuffer::clearExpiredEntries() {

  #ifdef ESP32

  if (xPortInIsrContext()) {
    // When called from an ISR, you should not try to erase log entries
    // Messing with memory from within an ISR is a big no-no.
    return;
  }
  #endif // ifdef ESP32

  static bool clearExpiredEntriesRunning{};

  if (clearExpiredEntriesRunning) { return; }
  clearExpiredEntriesRunning = true;

  for (auto it = LogEntries.begin(); it != LogEntries.end();)
  {
    it->updateSubscribers();

    if (it->isExpired()) {
      auto next = LogEntries.erase(it);

      for (size_t i = 0; i < NR_LOG_TO_DESTINATIONS; ++i) {
        if (it == cache_iterator_pos[i]) {
          cache_iterator_pos[i] = next;
        }
      }
      it = next;
    } else {
      ++it;
    }
  }
  clearExpiredEntriesRunning = false;
}
