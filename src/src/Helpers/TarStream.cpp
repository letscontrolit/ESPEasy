#include "../Helpers/TarStream.h"

#if FEATURE_TARSTREAM_SUPPORT
# include "../Globals/ESPEasy_time.h"
# include "../Helpers/ESPEasy_Storage.h"
# include "../Helpers/StringConverter.h"

/**
 * TarStream : Create/receive a .tar file while streaming via http webserver
 * Copyright (c) 2023.. Ton Huisman for ESPEasy
 *
 * Changelog: See TarStream.h
 */

/**
 * TarFileInfo_struct implementation
 */
TarFileInfo_struct::TarFileInfo_struct(const String fname,
                                       size_t       fsize) :
  fileName(fname), fileSize(fsize) {
  tarSize = ((fsize % TAR_BLOCK_SIZE == 0 ? 0 : 1) + (fsize / TAR_BLOCK_SIZE)) * TAR_BLOCK_SIZE; // Multiple of block size
  # if TAR_STREAM_DEBUG

  if (loglevelActiveFor(LOG_LEVEL_INFO)) {
    addLog(LOG_LEVEL_INFO, strformat(F("Added file: %s, size: %d, tarSize: %d"), fileName.c_str(), fileSize, tarSize));
  }
  # endif // if TAR_STREAM_DEBUG
}

/**
 * TarStream implementation
 */
TarStream::TarStream() {}

TarStream::TarStream(const String fileName)
  : _fileName(fileName) {}

TarStream::TarStream(const String fileName, FileDestination_e destination)
  : _fileName(fileName), _destination(destination) {}

TarStream::~TarStream() {
  _filesList.clear();
}

size_t TarStream::write(uint8_t ch) {
  // TODO implement
  addLogMove(LOG_LEVEL_ERROR, F("TarStream: write(ch) NOT IMPLEMENTED YET."));
  return 1u;
}

size_t TarStream::write(const uint8_t *buf,
                        size_t         size) {
  size_t bufOffset  = 0u;   // Offset into the buffer
  bool   stayInLoop = true; // To allow processing the rest of the data

  # if TAR_STREAM_DEBUG
  const bool logInfo = loglevelActiveFor(LOG_LEVEL_INFO);
  # endif // if TAR_STREAM_DEBUG

  while (stayInLoop) {
    stayInLoop = false;

    switch (_streamState) {
      case TarStreamState_e::Initial: // Initial behaves like WritingHeader
      case TarStreamState_e::WritingHeader:
      {
        if (_headerPosition == 0) {
          clearHeader();
        }
        const size_t toMove = std::min(size - bufOffset, TAR_HEADER_SIZE - _headerPosition);

        memcpy(&_tarData[_headerPosition], &buf[bufOffset], toMove);
        _headerPosition += toMove;
        bufOffset       += toMove;

        if (_headerPosition == TAR_HEADER_SIZE) {
          bool allZeros = true;

          for (size_t n = 0; n < TAR_HEADER_SIZE && allZeros; ++n) {
            allZeros &= (_tarData[n] == 0u);
          }

          if (allZeros) {
            _headerPosition = 0;
            _streamState    = TarStreamState_e::WritingFinal;
            # if TAR_STREAM_DEBUG
            addLog(LOG_LEVEL_INFO, F("TarStream: Switch from Initial/WritingHeader to WritingFinal"));
            # endif // if TAR_STREAM_DEBUG
          } else {
            const String fname(_tarHeader.name);
            const size_t fsize   = strtoul(_tarHeader.size, nullptr, 8);         // Octal
            const bool   isValid = validateHeader() && fname.indexOf('/') == -1; // Don't _allow_ subdirectories

            if (loglevelActiveFor(LOG_LEVEL_INFO)) {
              # if TAR_STREAM_DEBUG
              addLog(LOG_LEVEL_INFO, strformat(F("TarStream: Write Receiving file %s size: %d"),
                                               fname.c_str(), fsize));
              # else // if TAR_STREAM_DEBUG
              addLog(LOG_LEVEL_INFO, concat(F("Tar   : Load file: "), fname));
              # endif // if TAR_STREAM_DEBUG
            }

            if (((_tarHeader.typeflag == REGTYPE) || (_tarHeader.typeflag == AREGTYPE)) &&
                isValid) {           // Checked: typeflag, magic & checksum
              addFile(fname, fsize); // Add to list
              _fileIndex++;
              _filesSizes += fsize;
              bool validConfig = true;

              bufOffset += TAR_BLOCK_SIZE - TAR_HEADER_SIZE; // Skip remaining bytes to start of next block

              if (matchFileType(fname, FileType::CONFIG_DAT)) {
                validConfig = validateUploadConfigDat(&buf[bufOffset]);
              }

              if (validConfig) {
                _streamState = TarStreamState_e::WritingFile;

                size_t available = UINT32_MAX;

                if (FileDestination_e::SD != _destination) { // Check flash storage only
                  available = SpiffsFreeSpace();             // Leave(s) at least 2 blocks free
                  fs::File tmpfile = tryOpenFile(_filesList[_fileIndex].fileName, F("r"), _destination);

                  if (tmpfile) {
                    available += tmpfile.size(); // Existing file will be deleted
                    tmpfile.close();
                  }
                }

                if (available > _filesList[_fileIndex].tarSize) { // Use rounded-up size
                  // delete and create file for write mode
                  if (fileExists(_filesList[_fileIndex].fileName) &&
                      !tryDeleteFile(_filesList[_fileIndex].fileName, _destination) &&
                      loglevelActiveFor(LOG_LEVEL_ERROR)) {
                    addLog(LOG_LEVEL_ERROR, concat(F("TarStream: Can't delete file: "), _filesList[_fileIndex].fileName));
                  }
                  _currentFile = tryOpenFile(_filesList[_fileIndex].fileName, F("w"), _destination);

                  if (!_currentFile && loglevelActiveFor(LOG_LEVEL_ERROR)) {
                    addLog(LOG_LEVEL_ERROR, concat(F("TarStream: Can't create file: "), _filesList[_fileIndex].fileName));
                  }
                } else {
                  if (loglevelActiveFor(LOG_LEVEL_ERROR)) {
                    addLog(LOG_LEVEL_ERROR, concat(F("TarStream: Not enough space to save file: "),
                                                   _filesList[_fileIndex].fileName));
                  }
                }

                # if TAR_STREAM_DEBUG
                addLog(LOG_LEVEL_INFO, F("TarStream: Switch from Initial/WritingHeader to WritingFile"));
                # endif // if TAR_STREAM_DEBUG
              } else {
                _streamState = TarStreamState_e::WritingSlack;

                if (loglevelActiveFor(LOG_LEVEL_ERROR)) {
                  addLog(LOG_LEVEL_ERROR, F("TarStream: Received invalid config.dat, ignored."));
                }
                _filesList[_fileIndex].fileName = F("(ignored)"); // Won't be recognized
                # if TAR_STREAM_DEBUG
                addLog(LOG_LEVEL_INFO, F("TarStream: Switch from Initial/WritingHeader to WritingSlack"));
                # endif // if TAR_STREAM_DEBUG
              }
              _writePosition = 0u; // Start at file-position 0
            } else {
              _streamState = TarStreamState_e::Error;

              if (loglevelActiveFor(LOG_LEVEL_ERROR)) {
                addLog(LOG_LEVEL_ERROR, strformat(F("TarStream: Unsupported file: %s, type: %c"),
                                                  fname.c_str(), _tarHeader.typeflag));
              }
              # if TAR_STREAM_DEBUG
              addLog(LOG_LEVEL_INFO, F("TarStream: Switch from Initial/WritingHeader to Error"));
              # endif // if TAR_STREAM_DEBUG
            }
          }
        }

        # if !defined(BUILD_NO_DEBUG) && TAR_STREAM_DEBUG

        if (loglevelActiveFor(TAR_LOG_LEVEL_DEBUG)) {
          addLog(TAR_LOG_LEVEL_DEBUG, strformat(F("TarStream: DEBUG WritingHeader size: %d, offset: %d, stay: %d"),
                                                size, bufOffset, stayInLoop));
        }
        # endif // if !defined(BUILD_NO_DEBUG) && TAR_STREAM_DEBUG

        break;
      }
      case TarStreamState_e::WritingFile:
      {
        const size_t toWrite = std::min(size - bufOffset, _filesList[_fileIndex].fileSize - _writePosition);

        // write to file
        if (_currentFile) {
          _currentFile.write(&buf[bufOffset], toWrite);
        }

        _writePosition += toWrite;
        bufOffset      += toWrite;
        # if !defined(BUILD_NO_DEBUG) && TAR_STREAM_DEBUG

        if (loglevelActiveFor(TAR_LOG_LEVEL_DEBUG)) {
          addLog(TAR_LOG_LEVEL_DEBUG,
                 strformat(F("TarStream: DEBUG WritingFile %d bytes of %d to file, pos: %d, size: %d, bufoff: %d, stay: %d"),
                           toWrite, _filesList[_fileIndex].fileSize,
                           _writePosition, size, bufOffset, stayInLoop));
        }
        # endif // if !defined(BUILD_NO_DEBUG) && TAR_STREAM_DEBUG

        if (_writePosition == _filesList[_fileIndex].fileSize) { // Done with this file?
          // close the file
          if (_currentFile) {
            _currentFile.close();
          }

          if (_writePosition < _filesList[_fileIndex].tarSize) {
            _streamState = TarStreamState_e::WritingSlack;
            # if TAR_STREAM_DEBUG

            if (logInfo) {
              addLog(LOG_LEVEL_INFO, strformat(F("TarStream: Switch from WritingFile to WritingSlack, bytes: %d"),
                                               _writePosition));
            }
            # endif // if TAR_STREAM_DEBUG
          } else {
            _streamState = TarStreamState_e::WritingHeader;
            # if TAR_STREAM_DEBUG

            if (logInfo) {
              addLog(LOG_LEVEL_INFO, strformat(F("TarStream: Switch from WritingFile to WritingHeader, bytes: %d"),
                                               _writePosition));
            }
            # endif // if TAR_STREAM_DEBUG
            _headerPosition = 0;
          }
        }

        break;
      }
      case TarStreamState_e::WritingSlack:
      {
        const size_t toSkip = std::min(size - bufOffset, _filesList[_fileIndex].tarSize - _writePosition);

        _writePosition += toSkip;
        bufOffset      += toSkip;
        # if !defined(BUILD_NO_DEBUG) && TAR_STREAM_DEBUG

        if (loglevelActiveFor(TAR_LOG_LEVEL_DEBUG)) {
          addLog(TAR_LOG_LEVEL_DEBUG, strformat(F("%sDEBUG WritingSlack %d bytes of %d to file"),
                                                F("TarStream: "), toSkip, _filesList[_fileIndex].tarSize));
        }
        # endif // if !defined(BUILD_NO_DEBUG) && TAR_STREAM_DEBUG

        if (_writePosition == _filesList[_fileIndex].tarSize) {
          _streamState = TarStreamState_e::WritingHeader;
          # if TAR_STREAM_DEBUG
          addLog(LOG_LEVEL_INFO, F("TarStream: Switch from WritingSlack to WritingHeader"));
          # endif // if TAR_STREAM_DEBUG
          _headerPosition = 0;
        }
        break;
      }
      case TarStreamState_e::WritingFinal:
      {
        // FIXME check entire 512 byte block or just ignore?
        const size_t toMove = std::min(size - bufOffset, TAR_HEADER_SIZE - _headerPosition);
        memcpy(&_tarData[_headerPosition], &buf[bufOffset], toMove);
        _headerPosition += toMove;
        bufOffset       += toMove;
        bool allZeros = true;

        if (_headerPosition == TAR_HEADER_SIZE) {
          for (size_t n = 0; n < TAR_HEADER_SIZE && allZeros; ++n) {
            allZeros &= (_tarData[n] == 0u);
          }
          _streamState = TarStreamState_e::WritingDone;
          # if TAR_STREAM_DEBUG

          if (logInfo) {
            addLog(LOG_LEVEL_INFO, strformat(F("%sWritingFinal to WritingDone, allZeros: %d"),
                                             F("TarStream: Switch from "), allZeros));
          }
          # endif // if TAR_STREAM_DEBUG
        }
        break;
      }
      case TarStreamState_e::WritingDone:
      {
        const size_t toSkip = size - bufOffset;
        bufOffset += toSkip;
        # if TAR_STREAM_DEBUG

        if (logInfo) {
          addLog(LOG_LEVEL_INFO, strformat(F("TarStream: WritingDone, skipping: %d"), toSkip));
        }
        # endif // if TAR_STREAM_DEBUG
        break;
      }
      case TarStreamState_e::ReadingHeader: // Not here
      case TarStreamState_e::ReadingFile:
      case TarStreamState_e::ReadingSlack:
      case TarStreamState_e::ReadingFinal:
        break;
      case TarStreamState_e::Error: // No real error state
        break;
    }

    if (bufOffset < size) { // We got leftover bytes
      stayInLoop = true;
    }
  }
  delay(0);
  _tarSize += size;
  return size;
}

int TarStream::available() {
  return _tarRemaining;
}

void TarStream::clearHeader() {
  memset(&_tarHeader, 0, TAR_HEADER_SIZE);
}

void TarStream::setupHeader() {
  constexpr size_t tarHeader_name_size = NR_ELEMENTS(_tarHeader.name);

  clearHeader();
  safe_strncpy(_tarHeader.name, _currentIndex.fileName.c_str(), tarHeader_name_size);
  sprintf(_tarHeader.mode,  PSTR("%07o"),  TUREAD + TUWRITE + TUEXEC + TGREAD + TGWRITE + TGEXEC + TOREAD + TOWRITE + TOEXEC);
  sprintf(_tarHeader.uid,   PSTR("%07o"),  0);
  sprintf(_tarHeader.gid,   PSTR("%07o"),  0);
  sprintf(_tarHeader.size,  PSTR("%011o"), _currentIndex.fileSize);
  sprintf(_tarHeader.mtime, PSTR("%011o"), node_time.getUnixTime()); // We don't have file-date/times, use current date/time
  _tarHeader.typeflag = REGTYPE;
  sprintf(_tarHeader.magic, PSTR("%s"),    TMAGIC);
  _tarHeader.version[0] = TVERSION[0]; _tarHeader.version[1] = TVERSION[1];

  const uint32_t chksum = clearAndCalculateHeaderChecksum();

  sprintf(_tarHeader.chksum, "%06o", chksum); // FIXME Compatible with 7-zip: 6 octal digits + 0x0 + space?
}

bool TarStream::validateHeader() {
  String magic(_tarHeader.magic);

  magic = magic.substring(0, 6);                                    // Only get magic part
  magic.trim();
  const size_t   expected = strtoul(_tarHeader.chksum, nullptr, 8); // Octal
  const uint32_t chksum   = clearAndCalculateHeaderChecksum();

  # if TAR_STREAM_DEBUG

  if (loglevelActiveFor(LOG_LEVEL_INFO)) {
    addLog(LOG_LEVEL_INFO, strformat(F("TarStream: Validate Header, magic: %s checksum: %06o expected: %06o (octal)"),
                                     magic.c_str(), chksum, expected));
  }
  # endif // if TAR_STREAM_DEBUG
  return magic.equals(F(TMAGIC)) && chksum == expected;
}

uint32_t TarStream::clearAndCalculateHeaderChecksum() {
  constexpr size_t tarHeader_chksum_size = NR_ELEMENTS(_tarHeader.chksum);
  uint32_t chksum                        = 0u;

  for (size_t c = 0; c < tarHeader_chksum_size; ++c) { // note: chksum content during calculation is spaces
    _tarHeader.chksum[c] = ' ';
  }

  for (uint16_t hdr = 0; hdr < TAR_HEADER_SIZE; ++hdr) {
    chksum += _tarData[hdr];
  }
  return chksum;
}

int TarStream::read() {
  int result = EOF;

  switch (_streamState) {
    case TarStreamState_e::Initial:
    {
      _currentIterator = _filesList.begin();
      _currentIndex    = *_currentIterator;
      _currentFile     = tryOpenFile(_currentIndex.fileName, F("r"));

      if (_currentFile) {
        # ifndef BUILD_NO_DEBUG

        if (loglevelActiveFor(LOG_LEVEL_INFO)) {
          addLog(LOG_LEVEL_INFO, concat(F("Tar   : Save file: "), _currentIndex.fileName));
        }
        # endif // ifndef BUILD_NO_DEBUG

        // Set up header
        setupHeader();

        // Header done
        _streamState    = TarStreamState_e::ReadingHeader;
        _headerPosition = 0;
        result          = _tarData[_headerPosition];
        # if TAR_STREAM_DEBUG
        addLog(LOG_LEVEL_INFO, F("TarStream: Switch from Initial to ReadingHeader"));
        # endif // if TAR_STREAM_DEBUG
      } else {
        _streamState = TarStreamState_e::Error;

        # if TAR_STREAM_DEBUG
        addLog(LOG_LEVEL_INFO, F("TarStream: Switch from Initial to Error"));
        # endif // if TAR_STREAM_DEBUG
      }
      break;
    }
    case TarStreamState_e::ReadingHeader:
    {
      _headerPosition++;

      if (_headerPosition < TAR_HEADER_SIZE) {
        result = _tarData[_headerPosition];
      } else if (_headerPosition < TAR_BLOCK_SIZE) {
        result = 0;
      } else {
        _tarPosition = 0;

        if (_currentFile) {
          if (_tarPosition < _currentFile.size()) {
            result       = _currentFile.read();
            _streamState = TarStreamState_e::ReadingFile;
            # if TAR_STREAM_DEBUG
            addLog(LOG_LEVEL_INFO, F("TarStream: Switch from ReadingHeader to ReadingFile"));
            # endif // if TAR_STREAM_DEBUG
          } else {
            _streamState = TarStreamState_e::ReadingSlack;
            # if TAR_STREAM_DEBUG
            addLog(LOG_LEVEL_INFO, F("TarStream: Switch from ReadingHeader to ReadingSlack 0"));
            # endif // if TAR_STREAM_DEBUG
            result = 0;
          }
        } else {
          _streamState = TarStreamState_e::Error;
          # if TAR_STREAM_DEBUG
          addLog(LOG_LEVEL_INFO, F("TarStream: Switch from ReadingHeader to Error"));
          # endif // if TAR_STREAM_DEBUG
        }
      }

      if (!((TarStreamState_e::ReadingSlack == _streamState) && (_currentIndex.fileSize == 0))) {
        break; // Special 0-file case
      }
    }
    case TarStreamState_e::ReadingFile:
    {
      // FIXME tonhuisman: This looks horrible... but we can't return a single byte for a 0-size file :-(
      if (!((TarStreamState_e::ReadingSlack == _streamState) && (_currentIndex.fileSize == 0))) {
        _tarPosition++;

        if (_tarPosition < _currentFile.size()) {
          result = _currentFile.read();
          break;
        } else if (_tarPosition < _currentIndex.tarSize) {
          _currentFile.close();
          result       = 0;
          _streamState = TarStreamState_e::ReadingSlack;
          # if TAR_STREAM_DEBUG
          addLog(LOG_LEVEL_INFO, F("TarStream: Switch from ReadingFile to ReadingSlack 1"));
          # endif // if TAR_STREAM_DEBUG
          break;
        }
        _currentFile.close();
        _tarPosition--; // revert 1 position
        _streamState = TarStreamState_e::ReadingSlack;
        # if TAR_STREAM_DEBUG
        addLog(LOG_LEVEL_INFO, F("TarStream: Switch from ReadingFile to ReadingSlack 2"));
        # endif // if TAR_STREAM_DEBUG
      }

      // Fall through
    }
    case TarStreamState_e::ReadingSlack:
    {
      _tarPosition++;

      if ((_tarPosition < _currentIndex.tarSize) && (_currentIndex.fileSize != 0)) {
        result = 0;
      } else {
        if (_currentIndex.fileSize == 0) {
          _tarPosition--; // Revert 1 position for 0 byte file
        }
        _currentIterator++;

        if (_currentIterator != _filesList.end()) {
          _currentIndex = *_currentIterator;
          _currentFile  = tryOpenFile(_currentIndex.fileName, F("r"));

          if (_currentFile) {
            if (loglevelActiveFor(LOG_LEVEL_INFO)) {
              addLog(LOG_LEVEL_INFO, concat(F("Tar   : Save file: "), _currentIndex.fileName));
            }

            // Set up header
            setupHeader();

            // Header done
            _streamState = TarStreamState_e::ReadingHeader;
            # if TAR_STREAM_DEBUG
            addLog(LOG_LEVEL_INFO, F("TarStream: Switch from ReadingSlack to ReadingHeader"));
            # endif // if TAR_STREAM_DEBUG
          } else {
            _streamState = TarStreamState_e::Error;
            # if TAR_STREAM_DEBUG
            addLog(LOG_LEVEL_INFO, F("TarStream: Switch from ReadingSlack to Error"));
            # endif // if TAR_STREAM_DEBUG
          }
        } else {
          clearHeader();
          _streamState = TarStreamState_e::ReadingFinal;
          # if TAR_STREAM_DEBUG
          addLog(LOG_LEVEL_INFO, F("TarStream: Switch from ReadingSlack to ReadingFinal"));
          # endif // if TAR_STREAM_DEBUG
        }
        _headerPosition = 0;
        result          = _tarData[_headerPosition];
      }
      break;
    }
    case TarStreamState_e::ReadingFinal:
    {
      _headerPosition++;
      result = 0;

      if (_headerPosition == (TAR_BLOCK_SIZE * 2)) {
        result = EOF;
        # if TAR_STREAM_DEBUG
        addLog(LOG_LEVEL_INFO, F("TarStream: Reached ReadingFinal EOF"));
        # endif // if TAR_STREAM_DEBUG
      } else if (_headerPosition > (TAR_BLOCK_SIZE * 2)) {
        result       = EOF;
        _streamState = TarStreamState_e::Error;
        # if TAR_STREAM_DEBUG
        addLog(LOG_LEVEL_INFO, F("TarStream: Switch from ReadingFinal to Error"));
        # endif // if TAR_STREAM_DEBUG
      }
      break;
    }
    case TarStreamState_e::WritingHeader: // Not here
    case TarStreamState_e::WritingFile:
    case TarStreamState_e::WritingSlack:
    case TarStreamState_e::WritingFinal:
    case TarStreamState_e::WritingDone:
      break;
    case TarStreamState_e::Error: // Endstate, something went wrong
      break;
  }

  if (_tarRemaining > 0u) {
    _tarRemaining--;
  }

  return result;
}

int TarStream::peek() {
  int result = 0;

  # if TAR_STREAM_PEEK

  switch (_streamState) {
    case TarStreamState_e::Initial:
      break;
    case TarStreamState_e::ReadingHeader:
    {
      if (_headerPosition < TAR_HEADER_SIZE - 1) {
        result = _tarData[_headerPosition];
      } else if (_headerPosition < TAR_BLOCK_SIZE - 1) {
        result = 0;
      } else if (_currentFile) {
        result = _currentFile.peek();
      }
      break;
    }
    case TarStreamState_e::ReadingFile:
    {
      if (_tarPosition < _currentIndex.fileSize + 1) {
        result = _currentFile.peek();
      }
      break;
    }
    case TarStreamState_e::ReadingFinal:
    {
      if (_headerPosition == (TAR_BLOCK_SIZE * 2) - 1) {
        result = EOF;
      }
      break;
    }
    case TarStreamState_e::ReadingSlack:
    case TarStreamState_e::WritingHeader: // Ignore
    case TarStreamState_e::WritingFile:
    case TarStreamState_e::WritingSlack:
    case TarStreamState_e::WritingFinal:
    case TarStreamState_e::WritingDone:
      break;
    case TarStreamState_e::Error:
      result = EOF; // Endstate, something went wrong
      break;
  }
  # endif // if TAR_STREAM_PEEK

  return result;
}

void TarStream::flush() {
  if (_currentFile) {
    _currentFile.flush();
  }
  _tarRemaining = 0u;
}

size_t TarStream::size() {
  return _tarSize;
}

const char * TarStream::name() {
  return _fileName.c_str();
}

bool TarStream::addFileIfExists(const String& fileName) {
  fs::File tryFile = tryOpenFile(fileName, F("r"));

  if (tryFile) {
    addFile(tryFile.name(), tryFile.size());

    tryFile.close();

    return true;
  }
  return false;
}

bool TarStream::addFile(const String& fileName,
                        size_t        fileSize) {
  const TarFileInfo_struct tarFileInfo(fileName, fileSize);

  _filesList.push_back(tarFileInfo);

  if (_tarSize == 0) {
    _tarSize = TAR_BLOCK_SIZE * 2;                      // Closing 2 blocks of 0s
  }
  _tarSize     += TAR_BLOCK_SIZE + tarFileInfo.tarSize; // Header block + rounded-up file-size
  _filesSizes  += tarFileInfo.fileSize;                 // Actual file-size
  _tarRemaining = _tarSize;

  return true;
}

bool TarStream::isFileIncluded(const String& filename) {
  if (!filename.isEmpty()) {
    for (auto it = _filesList.begin(); it != _filesList.end(); ++it) {
      if (it->fileName.equalsIgnoreCase(filename)) {
        return true;
      }
    }
  }
  return false;
}

size_t TarStream::getFileCount() const {
  return _filesList.size();
}

#endif // if FEATURE_TARSTREAM_SUPPORT
