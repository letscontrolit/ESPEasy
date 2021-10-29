#include "../PluginStructs/P119_data_struct.h"

#ifdef USES_P119

// **************************************************************************/
// Constructor
// **************************************************************************/
P119_data_struct::P119_data_struct(uint8_t i2c_addr, bool rawData, uint8_t aSize)
  : _i2cAddress(i2c_addr), _rawData(rawData), _aSize(aSize) {
  if (_aSize == 0) { _aSize = 1; }
  _XA.resize(_aSize, 0);
  _YA.resize(_aSize, 0);
  _ZA.resize(_aSize, 0);
  _aUsed = 0;
  _aMax  = 0;
}

// **************************************************************************/
// Destructor
// **************************************************************************/
P119_data_struct::~P119_data_struct() {
  if (nullptr != itg3205) {
    delete itg3205;
    itg3205 = nullptr;
  }
}

// **************************************************************************/
// Initialize sensor and read data from ITG3205
// **************************************************************************/
bool P119_data_struct::read_sensor() {
  # if PLUGIN_119_DEBUG
  String log;
  # endif // if PLUGIN_119_DEBUG

  if (nullptr == itg3205) {
    init_sensor();

    # if PLUGIN_119_DEBUG

    if (loglevelActiveFor(LOG_LEVEL_DEBUG) &&
        log.reserve(55)) {
      log  = F("ITG3205: i2caddress: 0x");
      log += String(_i2cAddress, HEX);
      log += F(", initialized: ");
      log += String(nullptr != itg3205 ? F("true") : F("false"));
      log += F(", ID=0x");
      log += String(itg3205->readWhoAmI(), HEX);
      addLog(LOG_LEVEL_DEBUG, log);
    }
    # endif // if PLUGIN_119_DEBUG
  }

  if (nullptr != itg3205) {
    if (_rawData) {
      itg3205->readGyroRaw();
    } else {
      itg3205->readGyro();
    }
    _XA[_aUsed] = itg3205->g.x;
    _YA[_aUsed] = itg3205->g.y;
    _ZA[_aUsed] = itg3205->g.z;

    _aUsed++;

    if ((_aMax < _aUsed) && (_aUsed < _aSize)) {
      _aMax = _aUsed;
    }

    if (_aUsed == _aSize) {
      _aUsed = 0;
    }

    # if PLUGIN_119_DEBUG

    if (loglevelActiveFor(LOG_LEVEL_DEBUG) &&
        log.reserve(40)) {
      log  = F("ITG3205: ");
      log += (_rawData ? F("raw ") : F(""));
      log += F(", X: ");
      log += itg3205->g.x;
      log += F(", Y: ");
      log += itg3205->g.y;
      log += F(", Z: ");
      log += itg3205->g.z;
      addLog(LOG_LEVEL_DEBUG, log);
    }
    # endif // if PLUGIN_119_DEBUG
    return true;
  }
  return false;
}

// **************************************************************************/
// Average the measurements and return the results
// **************************************************************************/
bool P119_data_struct::read_data(int& X, int& Y, int& Z) {
  X = 0;
  Y = 0;
  Z = 0;

  if (nullptr != itg3205) {
    for (uint8_t n = 0; n <= _aMax; n++) {
      X += _XA[n];
      Y += _YA[n];
      Z += _ZA[n];
    }

    X /= _aMax; // Average available measurements
    Y /= _aMax;
    Z /= _aMax;

    # if PLUGIN_119_DEBUG

    if (loglevelActiveFor(LOG_LEVEL_DEBUG)) {
      String log;

      if (log.reserve(40)) {
        log  = F("ITG3205: averages, X: ");
        log += X;
        log += F(", Y: ");
        log += Y;
        log += F(", Z: ");
        log += Z;
        addLog(LOG_LEVEL_DEBUG, log);
      }
    }
    # endif // if PLUGIN_119_DEBUG
  }
  return nullptr != itg3205;
}

// **************************************************************************/
// Initialize ITG3205
// **************************************************************************/
bool P119_data_struct::init_sensor() {
  itg3205 = new (std::nothrow) ITG3205(_i2cAddress);

  if (nullptr != itg3205) {
    addLog(LOG_LEVEL_INFO, F("ITG3205: Initializing Gyro..."));
    itg3205->initGyro();
    addLog(LOG_LEVEL_INFO, F("ITG3205: Calibrating Gyro..."));
    itg3205->calibrate();
    addLog(LOG_LEVEL_INFO, F("ITG3205: Calibration done."));
  } else {
    addLog(LOG_LEVEL_ERROR, F("ITG3205: Initialization of Gyro failed."));
    return false;
  }

  if (loglevelActiveFor(LOG_LEVEL_DEBUG)) {
    String log;

    if (log.reserve(25)) {
      log  = F("ITG3205: Address: 0x");
      log += String(_i2cAddress, HEX);
      addLog(LOG_LEVEL_DEBUG, log);
    }
  }

  return true;
}

#endif // ifdef USES_P119
