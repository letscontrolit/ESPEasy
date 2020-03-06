#include <ESPEasySC16IS752_Serial.h>





static std::list


ESPEasySC16IS752_Serial::ESPEasySC16IS752_Serial(/* args */)
{
}

ESPEasySC16IS752_Serial::~ESPEasySC16IS752_Serial()
{
}

SC16IS752_addr_ch ESPEasySC16IS752_Serial::getAddrCh() const
{
    // Usable addresses 0x90 .. 0xAE
    return ((_address - 0x90) << 1) + (_channel & 1);
}