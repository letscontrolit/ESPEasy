/*-------------------------------------------------------------------------
NeoPixel library helper functions for Esp8266.

Written by Michael C. Miller.

I invest time and resources providing this open source code,
please support me by dontating (see https://github.com/Makuna/NeoPixelBus)

-------------------------------------------------------------------------
This file is part of the Makuna/NeoPixelBus library.

NeoPixelBus is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as
published by the Free Software Foundation, either version 3 of
the License, or (at your option) any later version.

NeoPixelBus is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public
License along with NeoPixel.  If not, see
<http://www.gnu.org/licenses/>.
-------------------------------------------------------------------------*/

#pragma once

#ifdef ARDUINO_ARCH_ESP8266
#include "NeoEsp8266I2sMethodCore.h"


class NeoEsp8266I2sDmx512SpeedBase
{
public:
    // 4 us bit send, 250Kbps
    static const uint32_t I2sClockDivisor = 20; // 0-63
    static const uint32_t I2sBaseClockDivisor = 32; // 0-63
    static const uint32_t ByteSendTimeUs = 44; // us it takes to send a single pixel element of 11 bits
    static const uint32_t BreakMabUs = 96; // Break min 92, Mab min 12
    static const size_t BreakMabSize = 4; // roundupby((BreakMabUs/4us)/8,4) count of bytes needed for the Break+Mab timing
    static const uint32_t MtbpUs = 11; // Mtbp, min 0, buy we use at least one byte of space (8*1.35)
    static const size_t MtbpSize = 1; // (MtbpUs/1.35)/8 count of bytes needed for the Mtbp timing
    // DMX requires the first slot to be zero
    static const size_t HeaderSize = 1;
};

class NeoEsp8266I2sDmx512Speed : public NeoEsp8266I2sDmx512SpeedBase
{
public:
    static const uint8_t MtbpLevel = 0x1; // high
    static const uint8_t StartBit = 0b00000000; 
    static const uint8_t StopBits = 0b00000011; 
    static const uint32_t Break = 0x00000000; // Break
    static const uint32_t BreakMab = 0x00000007; // Break + Mab

    static uint8_t Convert(uint8_t value)
    {
        // DMX requires LSB order
        return NeoUtil::Reverse8Bits( value );
    }
};

class NeoEsp8266I2sDmx512InvertedSpeed : public NeoEsp8266I2sDmx512SpeedBase
{
public:
    static const uint8_t MtbpLevel = 0x00; // low
    static const uint8_t StartBit = 0b00000001;
    static const uint8_t StopBits = 0b00000000;
    static const uint32_t Break = 0xffffffff; // Break
    static const uint32_t BreakMab = 0xfffffff8; // Break + Mab

    static uint8_t Convert(uint8_t value)
    {
        // DMX requires LSB order
        return NeoUtil::Reverse8Bits( ~value );
    }
};


class NeoEsp8266I2sWs2821SpeedBase
{
public:
    // 1.35 us bit send, 750Kbps
    static const uint32_t I2sClockDivisor = 27; // 0-63
    static const uint32_t I2sBaseClockDivisor = 8; // 0-63
    static const uint32_t ByteSendTimeUs = 15; // us it takes to send a single pixel element of 11 bits
    static const uint32_t BreakMabUs = 92; // Break min 88, Mab min 4
    static const size_t BreakMabSize = 12; // roundupby((BreakMabUs/1.35)/8,4) count of bytes needed for the Break+Mab timing
    static const uint32_t MtbpUs = 88; // Mtbp, min 88
    static const size_t MtbpSize = 9; // (MtbpUs/1.35)/8 count of bytes needed for the Mtbp timing
    
    // DMX/WS2821 requires the first slot to be zero
    static const size_t HeaderSize = 1;
};

class NeoEsp8266I2sWs2821Speed : public NeoEsp8266I2sWs2821SpeedBase
{
public:
    static const uint8_t MtbpLevel = 0x1; // high
    static const uint8_t StartBit = 0b00000000;
    static const uint8_t StopBits = 0b00000011;
    static const uint32_t Break = 0x00000000; // Break
    static const uint32_t BreakMab = 0x00000007; // Break + Mab (4~12us/1.35us)

    static uint8_t Convert(uint8_t value)
    {
        // DMX requires LSB order
        return NeoUtil::Reverse8Bits(value);
    }
};

class NeoEsp8266I2sWs2821InvertedSpeed : public NeoEsp8266I2sWs2821SpeedBase
{
public:
    static const uint8_t MtbpLevel = 0x00; // low
    static const uint8_t StartBit = 0b00000001;
    static const uint8_t StopBits = 0b00000000;
    static const uint32_t Break = 0xffffffff; // Break
    static const uint32_t BreakMab = 0xfffffff8; // Break + Mab

    static uint8_t Convert(uint8_t value)
    {
        // DMX requires LSB order
        return NeoUtil::Reverse8Bits(~value);
    }
};

template<typename T_SPEED> class NeoEsp8266I2sDmx512MethodBase : NeoEsp8266I2sMethodCore
{
public:
    typedef NeoNoSettings SettingsObject;

    NeoEsp8266I2sDmx512MethodBase(uint16_t pixelCount, size_t elementSize, size_t settingsSize) :
        _sizeData(pixelCount * elementSize + settingsSize + T_SPEED::HeaderSize)
    {
        size_t dmaPixelBits = I2sBitsPerPixelBytes * elementSize;
        size_t dmaSettingsBits = I2sBitsPerPixelBytes * (settingsSize + T_SPEED::HeaderSize);

        // bits + half rounding byte of bits / bits per byte
        size_t i2sBufferSize = (pixelCount * dmaPixelBits + dmaSettingsBits + 4) / 8;

        i2sBufferSize = i2sBufferSize + T_SPEED::BreakMabSize;

        // size is rounded up to nearest c_I2sByteBoundarySize
        i2sBufferSize = NeoUtil::RoundUp(i2sBufferSize, c_I2sByteBoundarySize);
        
        // size of a looping silent space rounded up to nearest c_I2sByteBoundarySize
        size_t i2sResetSize = NeoUtil::RoundUp(T_SPEED::MtbpSize, c_I2sByteBoundarySize);

        // protocol limits use of full block size to c_I2sByteBoundarySize
        size_t is2BufMaxBlockSize = (c_maxDmaBlockSize / c_I2sByteBoundarySize) * c_I2sByteBoundarySize;

        _data = static_cast<uint8_t*>(malloc(_sizeData));
        // first "slot" cleared due to protocol requiring it to be zero
        memset(_data, 0x00, 1);

        AllocateI2s(i2sBufferSize, i2sResetSize, is2BufMaxBlockSize, T_SPEED::MtbpLevel);
    }

    NeoEsp8266I2sDmx512MethodBase([[maybe_unused]] uint8_t pin, uint16_t pixelCount, size_t elementSize, size_t settingsSize) : 
        NeoEsp8266I2sDmx512MethodBase(pixelCount, elementSize, settingsSize)
    {
    }

    ~NeoEsp8266I2sDmx512MethodBase()
    {
        uint8_t waits = 1;
        while (!IsReadyToUpdate())
        {
            waits = 2;
            yield();
        }

        // wait for any pending sends to complete
        // due to internal i2s caching/send delays, this can more that once the data size
        uint32_t time = micros();
        while ((micros() - time) < ((getPixelTime() + T_SPEED::MtbpUs) * waits))
        {
            yield();
        }

        FreeI2s();

        free(_data);
    }

    bool IsReadyToUpdate() const
    {
        return IsIdle();
    }

    void Initialize()
    {
        InitializeI2s(T_SPEED::I2sClockDivisor, T_SPEED::I2sBaseClockDivisor);
    }

    void IRAM_ATTR Update(bool)
    {
        // wait for not actively sending data
        while (!IsReadyToUpdate())
        {
            yield();
        }
        FillBuffers();
        
        WriteI2s();
    }

    bool AlwaysUpdate()
    {
        // this method requires update to be called only if changes to buffer
        return false;
    }

    uint8_t* getData() const
    {
        return _data + T_SPEED::HeaderSize;
    };

    size_t getDataSize() const
    {
        return _sizeData - T_SPEED::HeaderSize;
    }

    void applySettings([[maybe_unused]] const SettingsObject& settings)
    {
    }

private:
    // given 11 sending bits per pixel byte, 
    static const uint16_t I2sBitsPerPixelBytes = 11;

    const size_t  _sizeData;    // Size of '_data' buffer 
    uint8_t*  _data;        // Holds LED color values

    // encodes the data with start and stop bits
    // input buffer is bytes
    // output stream is uint31_t
    static void Encoder(const uint8_t* pSrc, const uint8_t* pSrcEnd, 
            uint32_t* pOutput, const uint32_t* pOutputEnd)
    {
        static const uint32_t Mtbp = 0xffffffff * T_SPEED::MtbpLevel;
        const uint8_t* pData = pSrc;

        int8_t outputBit = 32;
        uint32_t output = 0;

        // DATA stream, one start, two stop
        while (pData < pSrcEnd)
        {
            uint8_t data = T_SPEED::Convert( *(pData++) );

            if (outputBit > 10)
            {
                // simple
                outputBit -= 1;
                output |= T_SPEED::StartBit << outputBit;

                outputBit -= 8;
                output |= data << outputBit;

                outputBit -= 2;
                output |= T_SPEED::StopBits << outputBit;
            }
            else
            {
                // split across an output uint32_t
                // handle start bit
                if (outputBit < 1)
                {
                    *(pOutput++) = output;
                    output = 0;
                    outputBit += 32;
                }
                outputBit -= 1;
                output |= (T_SPEED::StartBit << outputBit);

                // handle data bits
                if (outputBit < 8)
                {
                    output |= data >> (8 - outputBit);

                    *(pOutput++) = output;
                    output = 0;
                    outputBit += 32;
                }
                outputBit -= 8;
                output |= data << outputBit;

                // handle stop bits
                if (outputBit < 2)
                {
                    output |= T_SPEED::StopBits >> (2 - outputBit);

                    *(pOutput++) = output;
                    output = 0;
                    outputBit += 32;
                }
                outputBit -= 2;
                output |= T_SPEED::StopBits << outputBit;
            }
        }
        if (outputBit > 0)
        {
            // padd last output uint32_t with Mtbp
            output |= Mtbp >> (32 - outputBit);
            *(pOutput++) = output;
        }
        // fill the rest of the output with Mtbp
        while (pOutput < pOutputEnd)
        {
            *(pOutput++) = Mtbp;
        }
    }


    void FillBuffers()
    {
        uint32_t* pDma32 = reinterpret_cast<uint32_t*>(_i2sBuffer);
        const uint32_t* pDma32End = reinterpret_cast<uint32_t*>(_i2sBuffer + _i2sBufferSize);

        // first insert Break space as needed
        for (size_t count = 1; 
            count < (T_SPEED::BreakMabSize/sizeof(T_SPEED::Break));
            count++)
        {
            *(pDma32++) = T_SPEED::Break;
        }
        // then tail of break with mab
        *(pDma32++) = T_SPEED::BreakMab;
        
        Encoder(_data, _data + _sizeData, pDma32, pDma32End);
    }

    uint32_t getPixelTime() const
    {
        return (T_SPEED::ByteSendTimeUs * this->_sizeData);
    };

};


// normal
typedef NeoEsp8266I2sDmx512MethodBase<NeoEsp8266I2sDmx512Speed> NeoEsp8266Dmx512Method;
typedef NeoEsp8266I2sDmx512MethodBase<NeoEsp8266I2sWs2821Speed> NeoEsp8266Ws2821Method;

// inverted
typedef NeoEsp8266I2sDmx512MethodBase<NeoEsp8266I2sDmx512InvertedSpeed> NeoEsp8266Dmx512InvertedMethod;
typedef NeoEsp8266I2sDmx512MethodBase<NeoEsp8266I2sWs2821InvertedSpeed> NeoEsp8266Ws2821InvertedMethod;

#endif
