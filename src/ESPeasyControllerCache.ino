
struct ControllerCache_struct {
  ControllerCache_struct() {}

  ~ControllerCache_struct() {
    if (_RTC_cache_handler != nullptr) {
      delete _RTC_cache_handler;
      _RTC_cache_handler = nullptr;
    }
  }

  // Write a single sample set to the buffer
  bool write(uint8_t *data, unsigned int size) {
    if (_RTC_cache_handler == nullptr) {
      return false;
    }
    return _RTC_cache_handler->write(data, size);
  }

  // Read a single sample set, either from file or buffer.
  // May delete a file if it is all read and not written to.
  bool read(uint8_t *data, unsigned int size) {
    return true;
  }

  // Dump whatever is in the buffer to the filesystem
  bool flush() {
    if (_RTC_cache_handler == nullptr) {
      return false;
    }
    return _RTC_cache_handler->flush();
  }

  void init() {
    if (_RTC_cache_handler == nullptr) {
      _RTC_cache_handler = new (std::nothrow) RTC_cache_handler_struct;
    }
  }

  bool isInitialized() {
    return _RTC_cache_handler != nullptr;
  }

  // Clear all caches
  void clearCache() {}

  bool deleteOldestCacheBlock() {
    if (_RTC_cache_handler != nullptr) {
      return _RTC_cache_handler->deleteOldestCacheBlock();
    }
    return false;
  }

  void resetpeek() {
    if (_RTC_cache_handler != nullptr) {
      _RTC_cache_handler->resetpeek();
    }
  }

  // Read data without marking it as being read.
  bool peek(uint8_t *data, unsigned int size) {
    if (_RTC_cache_handler == nullptr) {
      return false;
    }
    return _RTC_cache_handler->peek(data, size);
  }

  String getPeekCacheFileName(bool& islast) {
    if (_RTC_cache_handler == nullptr) {
      return "";
    }
    return _RTC_cache_handler->getPeekCacheFileName(islast);
  }

  int readFileNr = 0;
  int readPos    = 0;

private:

  RTC_cache_handler_struct *_RTC_cache_handler = nullptr;
};
