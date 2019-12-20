
struct ControllerCache_struct {
  ControllerCache_struct(void) {}

  ~ControllerCache_struct(void) {
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
  bool flush(void) {
    if (_RTC_cache_handler == nullptr) {
      return false;
    }
    return _RTC_cache_handler->flush(void);
  }

  void init(void) {
    if (_RTC_cache_handler == nullptr) {
      _RTC_cache_handler = new RTC_cache_handler_struct;
    }
  }

  bool isInitialized(void) {
    return _RTC_cache_handler != nullptr;
  }

  // Clear all caches
  void clearCache(void) {}

  bool deleteOldestCacheBlock(void) {
    if (_RTC_cache_handler != nullptr) {
      return _RTC_cache_handler->deleteOldestCacheBlock(void);
    }
    return false;
  }

  void resetpeek(void) {
    if (_RTC_cache_handler != nullptr) {
      _RTC_cache_handler->resetpeek(void);
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
