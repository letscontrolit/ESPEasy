#include <ESPEasySC16IS752_Serial.h>

#include <map>


// Use a self-made smart pointer type, as I don't see how to create a map of smart pointers (or shared pointers)
// which will be cleared automatically when destroying the object.
// Problem here is that we can have 2 (max) references to the same object for channel A and B
// stored in a map or list shared among all instances of this object.
typedef std::pair<uint8_t, SC16IS752 *> SC16IS752_ptr_t;
static std::map<ESPEasySC16IS752_Serial::I2C_address, SC16IS752_ptr_t> SC16IS752_map;


ESPEasySC16IS752_Serial::ESPEasySC16IS752_Serial(ESPEasySC16IS752_Serial::I2C_address       addr,
                                                 ESPEasySC16IS752_Serial::SC16IS752_channel ch) : _address(addr), _channel(ch)
{
  auto it = SC16IS752_map.find(_address);

  if (it == SC16IS752_map.end()) {
    SC16IS752_ptr_t ptr;
    ptr.first               = 0; // link count
    ptr.second              = new SC16IS752(SC16IS750_PROTOCOL_I2C, _address);
    SC16IS752_map[_address] = ptr;
    it                      = SC16IS752_map.find(_address);
  }
  it->second.first++; // Increase link count
  _i2cuart = it->second.second;
}

ESPEasySC16IS752_Serial::~ESPEasySC16IS752_Serial()
{
  auto it = SC16IS752_map.find(_address);

  if (it != SC16IS752_map.end()) {
    // Should always return one....
    it->second.first--;

    if (it->second.first == 0) {
      // We are the last one.
      delete it->second.second;
      SC16IS752_map.erase(_address);
    }
  }
}

bool ESPEasySC16IS752_Serial::initialized() const
{
  if (_i2cuart) {
    return _pingReplied;
  }
  return false;
}

void ESPEasySC16IS752_Serial::begin(long speed)
{
  if (!_i2cuart) {
    return;
  }

  if (!_pingReplied) {
    _pingReplied = _i2cuart->ping();
  }

  if (_pingReplied) {
    if (_channel == SC16IS752_CHANNEL_A) {
      _i2cuart->beginA(speed);
    }
    else {
      _i2cuart->beginB(speed);
    }
  }
}

void ESPEasySC16IS752_Serial::end()
{
  // Not implemented
}

int ESPEasySC16IS752_Serial::peek(void)
{
  if (initialized()) {
    return _i2cuart->peek(_channel);
  }
  return -1;
}

size_t ESPEasySC16IS752_Serial::write(uint8_t val)
{
  if (initialized()) {
    return _i2cuart->write(_channel, val);
  }
  return 0;
}

size_t ESPEasySC16IS752_Serial::write(const uint8_t *buffer, size_t size)
{
  size_t count = 0;

  for (size_t i = 0; i < size; ++i) {
    size_t written = write(buffer[i]);

    if (written == 0) { return count; }
    count += written;
  }
  return count;
}

int ESPEasySC16IS752_Serial::read(void)
{
  if (initialized()) {
    return _i2cuart->read(_channel);
  }
  return -1;
}

size_t ESPEasySC16IS752_Serial::readBytes(char *buffer, size_t size)
{
  size_t count = 0;

  for (size_t i = 0; i < size; ++i) {
    int res = read();

    if (res < 0) { return count; }
    buffer[i] = static_cast<char>(res & 0xFF);
    ++count;
  }
  return count;
}

int ESPEasySC16IS752_Serial::available(void)
{
  if (initialized()) {
    return _i2cuart->available(_channel);
  }
  return 0;
}

void ESPEasySC16IS752_Serial::flush(void)
{
  if (initialized()) {
    _i2cuart->flush(_channel);
  }
}
