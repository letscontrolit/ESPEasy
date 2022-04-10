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


String RulesHelperClass::doReadLn(const String& filename,
                                  size_t      & pos,
                                  bool        & moreAvailable)
{
  std::vector<uint8_t> buf;

  buf.resize(RULES_BUFFER_SIZE);

  bool firstNonSpaceRead = false;
  bool commentFound      = false;

  // Try to get the best possible estimate on line length based on earlier parsing of the rules.
  static size_t longestLineSize = RULES_BUFFER_SIZE;
  String line;

  line.reserve(longestLineSize);

  bool done = false;

  while (!done) {
    const size_t startPos = pos;
    int len               = read(filename, pos, &buf[0], RULES_BUFFER_SIZE);
    moreAvailable = len != 0;

    if (!moreAvailable) { done = true; }

    for (int x = 0; x < len; x++) {
      int data = buf[x];

      switch (static_cast<char>(data))
      {
        case '\n':
        {
          // Line end, parse rule
          const size_t lineLength = line.length();
          line.trim();

          //          check_rules_line_user_errors(line);

          if (lineLength > longestLineSize) {
            longestLineSize = lineLength;
          }

          if ((lineLength > 0) && !line.startsWith(F("//"))) {
            // The line end may be at any point in the buffer
            // But for the next line we must start at the correct position.
            pos = startPos + x;
            return line;
          }

          // Prepare for new line
          line.clear();
          line.reserve(longestLineSize);
          firstNonSpaceRead = false;
          commentFound      = false;
          break;
        }
        case '\r': // Just skip this character
          break;
        case '\t': // tab
        case ' ':  // space
        {
          // Strip leading spaces.
          if (firstNonSpaceRead) {
            line += ' ';
          }
          break;
        }
        case '/':
        {
          if (!commentFound) {
            line += '/';

            if (line.endsWith(F("//"))) {
              // consider the rest of the line a comment
              commentFound = true;
            }
          }
          break;
        }
        default: // Any other character
        {
          firstNonSpaceRead = true;

          if (!commentFound) {
            line += char(data);
          }
          break;
        }
      }
    }
  }
  return line;
}

#ifdef ESP32
String RulesHelperClass::readLn(const String& filename,
                                size_t      & pos,
                                bool        & moreAvailable)
{
  // ToDo TD-er: Must read from the cached lines
  return doReadLn(filename, pos, moreAvailable);
}

#else // ifdef ESP32

String RulesHelperClass::readLn(const String& filename,
                                size_t      & pos,
                                bool        & moreAvailable)
{
  return doReadLn(filename, pos, moreAvailable);
}

#endif // ifdef ESP32
