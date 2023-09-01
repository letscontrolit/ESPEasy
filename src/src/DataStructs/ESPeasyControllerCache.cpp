#include "../DataStructs/ESPEasyControllerCache.h"

#if FEATURE_RTC_CACHE_STORAGE

ControllerCache_struct::~ControllerCache_struct() {
  if (_RTC_cache_handler != nullptr) {
    delete _RTC_cache_handler;
    _RTC_cache_handler = nullptr;
  }
}

// Write a single sample set to the buffer
bool ControllerCache_struct::write(const uint8_t *data, unsigned int size) {
  if (_RTC_cache_handler == nullptr) {
    return false;
  }
  return _RTC_cache_handler->write(data, size);
}

// Read a single sample set, either from file or buffer.
// May delete a file if it is all read and not written to.
bool ControllerCache_struct::read(uint8_t *data, unsigned int size) {
  return true;
}

// Dump whatever is in the buffer to the filesystem
bool ControllerCache_struct::flush() {
  if (_RTC_cache_handler == nullptr) {
    return false;
  }
  return _RTC_cache_handler->flush();
}

void ControllerCache_struct::init() {
  if (_RTC_cache_handler == nullptr) {
    _RTC_cache_handler = new (std::nothrow) RTC_cache_handler_struct;
    if (_RTC_cache_handler != nullptr) {
      _RTC_cache_handler->init();
    }
  }
}

bool ControllerCache_struct::isInitialized() const {
  return _RTC_cache_handler != nullptr;
}

// Clear all caches
void ControllerCache_struct::clearCache() {}

bool ControllerCache_struct::deleteOldestCacheBlock() {
  if (_RTC_cache_handler != nullptr) {
    return _RTC_cache_handler->deleteOldestCacheBlock();
  }
  return false;
}

void ControllerCache_struct::closeOpenFiles() {
  if (_RTC_cache_handler != nullptr) {
    _RTC_cache_handler->closeOpenFiles();
  }
}

bool ControllerCache_struct::deleteAllCacheBlocks() {
  if (_RTC_cache_handler != nullptr) {
    return _RTC_cache_handler->deleteAllCacheBlocks();
  }
  return false;
}

bool ControllerCache_struct::deleteCacheBlock(int fileNr) {
  if (_RTC_cache_handler != nullptr) {
    return _RTC_cache_handler->deleteCacheBlock(fileNr);
  }
  return false;
}

void ControllerCache_struct::resetpeek() {
  if (_RTC_cache_handler != nullptr) {
    _RTC_cache_handler->resetpeek();
  }
}

bool ControllerCache_struct::peekDataAvailable() const {
  if (_RTC_cache_handler == nullptr) {
    return false;
  }
  return _RTC_cache_handler->peekDataAvailable();
}

int  ControllerCache_struct::getPeekFilePos(int& peekFileNr) const {
  if (_RTC_cache_handler != nullptr) {
    return _RTC_cache_handler->getPeekFilePos(peekFileNr);
  }
  return -1;
}

int  ControllerCache_struct::getPeekFileSize(int peekFileNr) const {
  if (_RTC_cache_handler != nullptr) {
    return _RTC_cache_handler->getPeekFileSize(peekFileNr);
  }
  return -1;
}

void ControllerCache_struct::setPeekFilePos(int peekFileNr, int peekReadPos) {
  if (_RTC_cache_handler != nullptr) {
    _RTC_cache_handler->setPeekFilePos(peekFileNr, peekReadPos);
  }
}

// Read data without marking it as being read.
bool ControllerCache_struct::peek(uint8_t *data, unsigned int size) const {
  if (_RTC_cache_handler == nullptr) {
    return false;
  }
  return _RTC_cache_handler->peek(data, size);
}

String ControllerCache_struct::getNextCacheFileName(int& fileNr, bool& islast) {
  if (_RTC_cache_handler == nullptr) {
    fileNr = -1;
    islast = true;
    return EMPTY_STRING;
  }
  return _RTC_cache_handler->getNextCacheFileName(fileNr, islast);
}

#endif