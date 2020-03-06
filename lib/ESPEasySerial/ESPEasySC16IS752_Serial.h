#ifndef ESPeasySC16IS752_Serial_h
#define ESPeasySC16IS752_Serial_h

#include <inttypes.h>
#include <Stream.h>

#include <SC16IS752.h>

class ESPEasySC16IS752_Serial : public Stream {
public:

  typedef uint8_t I2C_address;
  typedef uint8_t SC16IS752_channel;

  explicit ESPEasySC16IS752_Serial(I2C_address       addr,
                                   SC16IS752_channel ch);
  virtual ~ESPEasySC16IS752_Serial();

  void   begin(long speed);
  void   end();
  int    peek(void);
  size_t write(uint8_t val) override;
  size_t write(const uint8_t *buffer,
               size_t         size);
  int    read(void) override;
  size_t readBytes(char  *buffer,
                   size_t size) override;
  int    available(void) override;
  void   flush(void) override;

  using Print::write;

private:

  bool initialized() const;

  SC16IS752 *_i2cuart = nullptr;

  I2C_address _address;
  SC16IS752_channel _channel;
  bool _pingReplied = false;
};

#endif // ESPeasySC16IS752_Serial_h
