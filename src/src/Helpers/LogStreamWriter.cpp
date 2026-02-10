#include "../Helpers/LogStreamWriter.h"

#include "../Globals/Logging.h"

#include "../Helpers/ESPEasy_time_calc.h"
#include "../Helpers/Memory.h"
#include "../Helpers/StringConverter.h"

void LogStreamWriter::clear()
{
  _prefix.clear();
  free_string(_message);
  _timestamp = 0;
  _readpos   = 0;
}

bool LogStreamWriter::process(Stream*stream, size_t availableForWrite)
{
  if (stream == nullptr) { return false; }
  return write(*stream, availableForWrite) != 0;
}

bool LogStreamWriter::process()
{
  return false;
}

uint32_t LogStreamWriter::getNrMessages() const
{
  return Logging.getNrMessages(_log_destination);
}

size_t LogStreamWriter::write(Stream& stream, size_t nrBytesToWrite)
{
  size_t bytesWritten = 0;

  while (bytesWritten < nrBytesToWrite) {
    const size_t written = write_item(stream, (nrBytesToWrite - bytesWritten));

    if (written == 0) {
      return bytesWritten;
    }
    bytesWritten += written;
  }
  return bytesWritten;
}

size_t LogStreamWriter::write_single_item(Stream& stream,
                                          size_t  nrBytesToWrite)
{
  const size_t res = write_item(stream, nrBytesToWrite);

  clear();
  return res;
}

size_t LogStreamWriter::write_item(Stream& stream,
                                   size_t  nrBytesToWrite)
{
  size_t bytesWritten = 0;

  if ((_timestamp != 0) && (timePassedSince(_timestamp) > LOG_BUFFER_EXPIRE)) {
    clear();
    bytesWritten += write_skipping(stream);
  }

  if (_timestamp == 0) {
    _readpos = 0;

    // Need to fetch a line
    if (!Logging.getNext(_log_destination, _timestamp, _message, _loglevel) ||
        !loglevelActiveFor(_log_destination, _loglevel)) {
      free_string(_message);
      _timestamp = 0;
      return bytesWritten;
    }

    prepare_prefix();
  }

  const size_t maxToWrite = _prefix.length() + _message.length();

  if (nrBytesToWrite > maxToWrite) {
    nrBytesToWrite = maxToWrite;
  }

  bool done = false;

  while (!done && bytesWritten < nrBytesToWrite) {
    if (_readpos < _prefix.length()) {
      // Write prefix
      if (1 != stream.write(_prefix[_readpos])) { return bytesWritten; }
      ++bytesWritten;
      ++_readpos;
    } else if (!_prefix.isEmpty()) {
      // Clear prefix
      _prefix.clear();
      _readpos = 0;
    } else if (_readpos < _message.length()) {
      // Write message
      if (1 != stream.write(_message[_readpos])) { return bytesWritten; }
      ++bytesWritten;
      ++_readpos;
    } else {
      if ((bytesWritten + 2) > nrBytesToWrite) { return bytesWritten; }
      bytesWritten += stream.println();

      // Done with entry, cleanup and leave
      clear();
      done = true;
    }
  }
  return bytesWritten;
}

size_t LogStreamWriter::write_skipping(Stream& stream) { return 0; }

void   LogStreamWriter::prepare_prefix()               {}
