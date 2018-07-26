#include <Arduino.h>
#include <vector>

//**************************************************************************/
// Object to store data to and from I2C devices
//**************************************************************************/
template <class T>
struct I2Cdata {
  I2Cdata(uint8_t length) : start_reg(0) {
    data.resize(length);
  }

  I2Cdata(uint8_t length, uint8_t start_register) : start_reg(start_register) {
    data.resize(length);
  }

  T operator[](uint8_t n) const {
    if (n < start_reg) return 0;
    if (n >= (data.size() + start_reg)) return 0;
    return data[(n - start_reg)];
  }

  bool addAtIndex(uint8_t index, T value) {
    if (index >= data.size()) return false;
    data[index] = value;
    return true;
  }

  bool addRegister(uint8_t reg, T value) {
    if (reg < start_reg) return false;
    return addAtIndex(reg - start_reg, value);
  }

  uint8_t getSize() const {
    return data.size();
  }

  T* get() {
    return &(data[0]);
  }

  uint8_t getRegister() const {
    return start_reg;
  }

private:
  uint8_t start_reg;
  std::vector<T> data;
};

typedef I2Cdata<uint8_t> I2Cdata_bytes;
typedef I2Cdata<uint16_t> I2Cdata_words;
