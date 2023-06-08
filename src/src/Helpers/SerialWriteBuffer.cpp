#include "../Helpers/SerialWriteBuffer.h"

#include "../Helpers/Memory.h"

void SerialWriteBuffer_t::add(const String& line)
{
  // When the buffer is too full, try to dump at least the size of what we try to add.
  const bool mustPop = _buffer.size() > _maxSize;
  {
    #ifdef USE_SECOND_HEAP

    // Allow to store the logs in 2nd heap if present.
    HeapSelectIram ephemeral;
    #endif // ifdef USE_SECOND_HEAP
    int roomLeft = getRoomLeft();

    auto it = line.begin();

    while (roomLeft > 0 && it != line.end()) {
      if (mustPop) {
        _buffer.pop_front();
      }
      _buffer.push_back(*it);
      --roomLeft;
      ++it;
    }
  }
}

void SerialWriteBuffer_t::add(const __FlashStringHelper *line)
{
  add(String(line));
}

void SerialWriteBuffer_t::add(char c)
{
  if (_buffer.size() > _maxSize) {
    _buffer.pop_front();
  }
  _buffer.push_back(c);
}

void SerialWriteBuffer_t::addNewline()
{
  add('\r');
  add('\n');
}

void SerialWriteBuffer_t::clear()
{
  _buffer.clear();
}

int SerialWriteBuffer_t::availableForWrite() const
{
  return _buffer.size();
}

size_t SerialWriteBuffer_t::write(Stream& stream, size_t nrBytesToWrite)
{
  size_t bytesWritten     = 0;
  const size_t bufferSize = _buffer.size();

  if (bufferSize == 0) {
    return bytesWritten;
  }

  if (nrBytesToWrite > 0) {
    if (nrBytesToWrite > bufferSize) {
      nrBytesToWrite = bufferSize;
    }

    while (nrBytesToWrite > 0 && !_buffer.empty()) {
      const char c = _buffer.front();

      if (stream.write((uint8_t)c) == 0) {
        return bytesWritten;
      }
      _buffer.pop_front();
      --nrBytesToWrite;
      ++bytesWritten;
    }
  }
  return bytesWritten;
}

int SerialWriteBuffer_t::getRoomLeft() const {
  #ifdef USE_SECOND_HEAP

  // If stored in 2nd heap, we must check this for space
  HeapSelectIram ephemeral;
  #endif // ifdef USE_SECOND_HEAP

  int roomLeft = getMaxFreeBlock();

  if (roomLeft < 1000) {
    roomLeft = 0;                    // Do not append to buffer.
  } else if (roomLeft < 4000) {
    roomLeft = 128 - _buffer.size(); // 1 buffer.
  } else {
    roomLeft -= 4000;                // leave some free for normal use.
  }
  return roomLeft;
}
