#ifndef HELPERS_ESPEASYMUTEX_H
#define HELPERS_ESPEASYMUTEX_H


// ESP8266 does not support std::mutex, so we need to implement our own.
// Used the ESP8266 mutex implemented here: https://github.com/raburton/esp8266/tree/master/mutex
// This library is just linked in, not part of ESPEasy.
//
// We just need to make a wrapper to make sure we can use the same code for ESP8266 and ESP32.


#ifdef ESP8266
//# include <esp8266_mutex.h>
// No longer used, see: https://github.com/letscontrolit/ESPEasy/issues/3693#issuecomment-868847669
#endif // ifdef ESP8266

#ifdef ESP32
# include <mutex>
#endif // ifdef ESP32

struct ESPEasy_Mutex {
  // Don't allow to copy or move the mutex
  ESPEasy_Mutex(const ESPEasy_Mutex& other)            = delete;
  ESPEasy_Mutex(ESPEasy_Mutex&& other)                 = delete;
  ESPEasy_Mutex& operator=(const ESPEasy_Mutex& other) = delete;
  ESPEasy_Mutex& operator=(ESPEasy_Mutex&& other)      = delete;

#ifdef ESP8266

  // ESP8266 doesn't have a proper mutex system.
  // This is by no means a perfect implementation, but the assembly implementation was causing lots of issues with esp8266/Arduino 3.0.0
  // See: https://github.com/letscontrolit/ESPEasy/issues/3693#issuecomment-868847669
  ESPEasy_Mutex() {
    _mutex = 1;
  }

  // May cause deadlock, perhaps add a timeout?
  void lock() {
    while (_mutex == 0) {}
    _mutex = 0;
  }

  bool try_lock() {
    if (_mutex == 1) {
      _mutex = 0;
      return true;
    }
    return false;
  }

  void unlock() {
    _mutex = 1;
  }

private:

volatile  uint32_t _mutex = 0;
#endif // ifdef ESP8266

#ifdef ESP32
  ESPEasy_Mutex() = default;

  void lock() {
    _mutex.lock();
  }

  bool try_lock() {
    return _mutex.try_lock();
  }

  void unlock() {
    _mutex.unlock();
  }

private:

  std::mutex _mutex;


#endif // ifdef ESP32
};


#endif // ifndef HELPERS_ESPEASYMUTEX_H
