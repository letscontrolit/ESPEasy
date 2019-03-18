
struct ControllerCache_struct {

  ControllerCache_struct() {
  }

  // Write a single sample set to the buffer
  bool write(uint8_t* data, unsigned int size) {
    if (readFileNr == 0)
      readFileNr = _RTC_cache_handler.getFreeSpace();

    if (_RTC_cache_handler.getFreeSpace() < size) {
      if (!flush()) {
        return false;
      }
    }
    return _RTC_cache_handler.write(data, size);
  }

  // Read a single sample set, either from file or buffer.
  // May delete a file if it is all read and not written to.
  bool read(uint8_t* data, unsigned int size) {
    return true;
  }

  // Dump whatever is in the buffer to the filesystem
  bool flush() {
    if (SpiffsFull()) {
      return false;
    }
    // Write timings make quite a jump above 24k file size.
    bool fileFound = false;
    while (!fileFound) {
      if (fw && fw.size() > 24000) {
        fw.close();
      }
      if (!fw) {
        // Create a new file to write
        ++writeFileNr;
        String fname = "cache_";
        fname += String(writeFileNr);
        fname += ".bin";
        fw = SPIFFS.open(fname.c_str(), "a+");
        if (!fw) {
          addLog(LOG_LEVEL_ERROR, F("RTC  : error opening file"));
          return false;
        }
      }
      fileFound = fw && fw.size() < 24000;
      delay(0);
    }

    if (fw) {
      unsigned int size;
      uint8_t* data = _RTC_cache_handler.getBuffer(size);
      if (size > 0) {
        if (fw.write(data, size) < 0) {
          addLog(LOG_LEVEL_ERROR, F("RTC  : error writing file"));
          return false;
        }
        addLog(LOG_LEVEL_INFO, F("RTC  : Write RTC cache data to file"));
        addLog(LOG_LEVEL_INFO, String(readFileNr));
        delay(0);
        fw.flush();
        _RTC_cache_handler.flush();
      }
    }
    return true;
  }

  // Determine what files are present.
  void init() {

  }

  // Clear all caches
  void clearCache() {}

  int readFileNr = 0;
  int writeFileNr = 0;
  int readPos = 0;

private:
  RTC_cache_handler_struct _RTC_cache_handler;
  File fw;

};
