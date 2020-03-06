#ifndef ESPeasySC16IS752_Serial_h
#define ESPeasySC16IS752_Serial_h

#include <inttypes.h>
#include <Stream.h>

class ESPEasySC16IS752_Serial : public Stream
{
public:

    typedef uint8_t I2C_address;
    typedef uint8_t SC16IS752_channel;
    typedef uint8_t SC16IS752_addr_ch;

    ESPEasySC16IS752_Serial(/* args */);
    virtual ~ESPEasySC16IS752_Serial();

private:

    SC16IS752_addr_ch getAddrCh() const;

    I2C_address _address;
    SC16IS752_channel _channel;


};

#endif // ESPeasySC16IS752_Serial_h
