#include "../Helpers/RulesHelper.h"

#include "../Globals/Settings.h"
#include "../Helpers/ESPEasy_Storage.h"


RulesHelperClass::RulesHelperClass()
{}

RulesHelperClass::~RulesHelperClass()
{
  closeAllFiles();
}

#ifdef ESP32

void RulesHelperClass::closeAllFiles() {
  for (auto it = _fileHandleMap.begin(); it != _fileHandleMap.end();) {
    it = _fileHandleMap.erase(it);
  }
}

size_t RulesHelperClass::read(const String& filename, size_t& pos, uint8_t *buffer, size_t length)
{
  if (!Settings.UseRules || !fileExists(filename)) {
    return 0;
  }
  auto it = _fileHandleMap.find(filename);

  if (it == _fileHandleMap.end()) {
    // No open file handle found, so try to open it.
    fs::File f = tryOpenFile(filename, "r+");

    if (f) {
      String fileContent;

      if (fileContent.reserve(f.size())) {
        while (f.available()) {
          fileContent += static_cast<char>(f.read());
        }
        _fileHandleMap.emplace(std::make_pair(filename, std::move(fileContent)));
        it = _fileHandleMap.find(filename);
      }
      f.close();
    }
  }

  if (it == _fileHandleMap.end()) {
    return 0;
  }

  size_t bytesRead = 0;

  while (pos < it->second.length() && bytesRead < length) {
    buffer[bytesRead] = it->second[pos];
    ++pos;
    ++bytesRead;
  }
  return bytesRead;
}

#else // ifdef ESP32

void RulesHelperClass::closeAllFiles() {
  for (auto it = _fileHandleMap.begin(); it != _fileHandleMap.end();) {
    it->second.close();
    it = _fileHandleMap.erase(it);
  }
}

size_t RulesHelperClass::read(const String& filename, size_t& pos, uint8_t *buffer, size_t length)
{
  if (!Settings.UseRules || !fileExists(filename)) {
    return 0;
  }
  auto it = _fileHandleMap.find(filename);

  if (it == _fileHandleMap.end()) {
    // No open file handle found, so try to open it.
    _fileHandleMap.emplace(filename, tryOpenFile(filename, "r+"));
    it = _fileHandleMap.find(filename);
  }

  if (!it->second) {
    return 0;
  }

  if (it->second.position() != pos) {
    it->second.seek(pos);
  }
  const size_t ret = it->second.read(buffer, length);

  pos = it->second.position();
  return ret;
}

#endif // ifdef ESP32
