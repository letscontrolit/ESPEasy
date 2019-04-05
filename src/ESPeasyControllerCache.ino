
struct ControllerCache_struct {

  ControllerCache_struct() {
  }

  // Write a single sample set to the buffer
  bool write(uint8_t* data, unsigned int size) {
    if ((_buffer_size + size) > _buffer.size()) {
      if (!flush()) {
        return false;
      }
    }
    for (int i = 0; i < size; ++i) {
      _buffer[_buffer_size] = data[i];
      ++_buffer_size;
    }
    return true;
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
    if (fw && fw.size() > 32500) {
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
        return false;
      }
    }

    if (fw && (_buffer_size > 0)) {
      if (fw.write(&_buffer[0], _buffer_size) < 0) {
        return false;
      }
      _buffer_size = 0;
      delay(0);
      fw.flush();
    }
    return true;
  }

  // Determine what files are present.
  void init() {
    _buffer.resize(256);
    _buffer_size = 0;


  }

  // Clear all caches
  void clearCache() {}

  int readFileNr = 0;
  int writeFileNr = 0;
  int readPos = 0;

private:

  std::vector<uint8_t> _buffer;
  uint32_t _buffer_size = 0;
  File fw;

};
