#ifndef HELPERS_ESPEASYMUTEX_H
#define HELPERS_ESPEASYMUTEX_H


// ESP8266 does not support std::mutex, so we need to implement our own.
// Used the ESP8266 mutex implemented here: https://github.com/raburton/esp8266/tree/master/mutex
// This library is just linked in, not part of ESPEasy.
//
// We just need to make a wrapper to make sure we can use the same code for ESP8266 and ESP32.


#ifdef ESP8266
# include <esp8266_mutex.h>
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

  ESPEasy_Mutex() {
    CreateMutux(&_mutex);
  }

  // May cause deadlock, perhaps add a timeout?
  void lock() {
    while (!GetMutex(&_mutex)) {}
  }

  bool try_lock() {
    return GetMutex(&_mutex);
  }

  void unlock() {
    ReleaseMutex(&_mutex);
  }

private:

  mutex_t _mutex = 0;
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
