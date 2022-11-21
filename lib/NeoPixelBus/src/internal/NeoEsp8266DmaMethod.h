/*-------------------------------------------------------------------------
NeoPixel library helper functions for Esp8266.


Written by Michael C. Miller.
Thanks to g3gg0.de for porting the initial DMA support which lead to this.
Thanks to github/cnlohr for the original work on DMA support, which opend
all our minds to a better way (located at https://github.com/cnlohr/esp8266ws2812i2s).

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

#include "Arduino.h"

extern "C"
{
#include "osapi.h"
#include "ets_sys.h"

#include "i2s_reg.h"

#ifdef ARDUINO_ESP8266_MAJOR    //this define was added in ESP8266 Arduino Core version v3.0.1
  #include "core_esp8266_i2s.h" //for Arduino core >= 3.0.1
#else
  #include "i2s.h"              //for Arduino core <= 3.0.0
#endif

#include "eagle_soc.h"
#include "esp8266_peri.h"
#include "slc_register.h"

#include "osapi.h"
#include "ets_sys.h"
#include "user_interface.h"

#if !defined(__CORE_ESP8266_VERSION_H) || defined(ARDUINO_ESP8266_RELEASE_2_5_0)
    void rom_i2c_writeReg_Mask(uint32_t block, uint32_t host_id, uint32_t reg_add, uint32_t Msb, uint32_t Lsb, uint32_t indata);
#endif
}

struct slc_queue_item
{
    uint32  blocksize : 12;
    uint32  datalen : 12;
    uint32  unused : 5;
    uint32  sub_sof : 1;
    uint32  eof : 1;
    uint32  owner : 1;
    uint8*  buf_ptr;
    struct slc_queue_item*  next_link_ptr;
};

class NeoEsp8266DmaSpeedBase
{
public:
    static const uint8_t Level = 0x00;
    static uint16_t Convert(uint8_t value)
    {
        const uint16_t bitpatterns[16] =
        {
            0b1000100010001000, 0b1000100010001110, 0b1000100011101000, 0b1000100011101110,
            0b1000111010001000, 0b1000111010001110, 0b1000111011101000, 0b1000111011101110,
            0b1110100010001000, 0b1110100010001110, 0b1110100011101000, 0b1110100011101110,
            0b1110111010001000, 0b1110111010001110, 0b1110111011101000, 0b1110111011101110,
        };

        return bitpatterns[value];
    }
};

class NeoEsp8266DmaInvertedSpeedBase
{
public:
    static const uint8_t Level = 0xFF;
    static uint16_t Convert(uint8_t value)
    {
        const uint16_t bitpatterns[16] =
        {
            0b0111011101110111, 0b0111011101110001, 0b0111011100010111, 0b0111011100010001,
            0b0111000101110111, 0b0111000101110001, 0b0111000100010111, 0b0111000100010001,
            0b0001011101110111, 0b0001011101110001, 0b0001011100010111, 0b0001011100010001,
            0b0001000101110111, 0b0001000101110001, 0b0001000100010111, 0b0001000100010001,
        };

        return bitpatterns[value];
    }
};

class NeoEsp8266DmaSpeed800KbpsBase : public NeoEsp8266DmaSpeedBase
{
public:
    const static uint32_t I2sClockDivisor = 3;
    const static uint32_t I2sBaseClockDivisor = 16;
    const static uint32_t ByteSendTimeUs = 10; // us it takes to send a single pixel element at 800khz speed
};

class NeoEsp8266DmaSpeedWs2812x : public NeoEsp8266DmaSpeed800KbpsBase
{
public:
    const static uint32_t ResetTimeUs = 300;
};

class NeoEsp8266DmaSpeedSk6812 : public NeoEsp8266DmaSpeed800KbpsBase
{
public:
    const static uint32_t ResetTimeUs = 80;
};

class NeoEsp8266DmaInvertedSpeedTm1814 : public NeoEsp8266DmaSpeed800KbpsBase
{
public:
    const static uint32_t ResetTimeUs = 200;
};

class NeoEsp8266DmaInvertedSpeedTm1829 : public NeoEsp8266DmaSpeed800KbpsBase
{
public:
    const static uint32_t ResetTimeUs = 200;
};

class NeoEsp8266DmaSpeed800Kbps : public NeoEsp8266DmaSpeed800KbpsBase
{
public:
    const static uint32_t ResetTimeUs = 50;
};

class NeoEsp8266DmaSpeed400Kbps : public NeoEsp8266DmaSpeedBase
{
public:
    const static uint32_t I2sClockDivisor = 6;
    const static uint32_t I2sBaseClockDivisor = 16;
    const static uint32_t ByteSendTimeUs = 20; // us it takes to send a single pixel element at 400khz speed
    const static uint32_t ResetTimeUs = 50;
};

class NeoEsp8266DmaSpeedApa106 : public NeoEsp8266DmaSpeedBase
{
public:
    const static uint32_t I2sClockDivisor = 4; 
    const static uint32_t I2sBaseClockDivisor = 16;
    const static uint32_t ByteSendTimeUs = 17; // us it takes to send a single pixel element
    const static uint32_t ResetTimeUs = 50;
};



class NeoEsp8266DmaInvertedSpeed800KbpsBase : public NeoEsp8266DmaInvertedSpeedBase
{
public:
    const static uint32_t I2sClockDivisor = 3;
    const static uint32_t I2sBaseClockDivisor = 16;
    const static uint32_t ByteSendTimeUs = 10; // us it takes to send a single pixel element at 800khz speed
};

class NeoEsp8266DmaInvertedSpeedWs2812x : public NeoEsp8266DmaInvertedSpeed800KbpsBase
{
public:
    const static uint32_t ResetTimeUs = 300;
};

class NeoEsp8266DmaInvertedSpeedSk6812 : public NeoEsp8266DmaInvertedSpeed800KbpsBase
{
public:
    const static uint32_t ResetTimeUs = 80;
};

class NeoEsp8266DmaSpeedTm1814 : public NeoEsp8266DmaInvertedSpeed800KbpsBase
{
public:
    const static uint32_t ResetTimeUs = 200;
};

class NeoEsp8266DmaSpeedTm1829 : public NeoEsp8266DmaInvertedSpeed800KbpsBase
{
public:
    const static uint32_t ResetTimeUs = 200;
};

class NeoEsp8266DmaInvertedSpeed800Kbps : public NeoEsp8266DmaInvertedSpeed800KbpsBase
{
public:
    const static uint32_t ResetTimeUs = 50;
};

class NeoEsp8266DmaInvertedSpeed400Kbps : public NeoEsp8266DmaInvertedSpeedBase
{
public:
    const static uint32_t I2sClockDivisor = 6;
    const static uint32_t I2sBaseClockDivisor = 16;
    const static uint32_t ByteSendTimeUs = 20; // us it takes to send a single pixel element at 400khz speed
    const static uint32_t ResetTimeUs = 50;
};

class NeoEsp8266DmaInvertedSpeedApa106 : public NeoEsp8266DmaInvertedSpeedBase
{
public:
    const static uint32_t I2sClockDivisor = 4;
    const static uint32_t I2sBaseClockDivisor = 16;
    const static uint32_t ByteSendTimeUs = 17; // us it takes to send a single pixel element
    const static uint32_t ResetTimeUs = 50;
};

enum NeoDmaState
{
    NeoDmaState_Idle,
    NeoDmaState_Pending,
    NeoDmaState_Sending,
    NeoDmaState_Zeroing,
};
const uint16_t c_maxDmaBlockSize = 4095;
const uint16_t c_dmaBytesPerPixelBytes = 4;
const uint8_t c_I2sPin = 3; // due to I2S hardware, the pin used is restricted to this

class NeoEsp8266DmaMethodCore
{
protected:
    static NeoEsp8266DmaMethodCore* s_this; // for the ISR

    volatile NeoDmaState _dmaState;

    slc_queue_item* _i2sBufDesc;  // dma block descriptors
    uint16_t _i2sBufDescCount;   // count of block descriptors in _i2sBufDesc


    // This routine is called as soon as the DMA routine has something to tell us. All we
    // handle here is the RX_EOF_INT status, which indicate the DMA has sent a buffer whose
    // descriptor has the 'EOF' field set to 1.
    // in the case of this code, the second to last state descriptor
    static void IRAM_ATTR i2s_slc_isr(void)
    {
        ETS_SLC_INTR_DISABLE();

        uint32_t slc_intr_status = SLCIS;

        SLCIC = 0xFFFFFFFF;

        if ((slc_intr_status & SLCIRXEOF) && s_this)
        {
            switch (s_this->_dmaState)
            {
            case NeoDmaState_Idle:
                break;

            case NeoDmaState_Pending:
            {
                slc_queue_item* finished_item = (slc_queue_item*)SLCRXEDA;

                // data block has pending data waiting to send, prepare it
                // point last state block to top 
                (finished_item + 1)->next_link_ptr = s_this->_i2sBufDesc;

                s_this->_dmaState = NeoDmaState_Sending;
            }
            break;

            case NeoDmaState_Sending:
            {
                slc_queue_item* finished_item = (slc_queue_item*)SLCRXEDA;

                // the data block had actual data sent
                // point last state block to first state block thus
                // just looping and not sending the data blocks
                (finished_item + 1)->next_link_ptr = finished_item;

                s_this->_dmaState = NeoDmaState_Zeroing;
            }
            break;

            case NeoDmaState_Zeroing:
                s_this->_dmaState = NeoDmaState_Idle;
                break;
            }
        }

        ETS_SLC_INTR_ENABLE();
    }
};

template<typename T_SPEED> class NeoEsp8266DmaMethodBase : NeoEsp8266DmaMethodCore
{
public:
    typedef NeoNoSettings SettingsObject;

    NeoEsp8266DmaMethodBase(uint16_t pixelCount, size_t elementSize, size_t settingsSize) :
        _sizeData(pixelCount * elementSize + settingsSize)
    {
        uint16_t dmaPixelSize = c_dmaBytesPerPixelBytes * elementSize;
        uint16_t dmaSettingsSize = c_dmaBytesPerPixelBytes * settingsSize;

        _i2sBufferSize = pixelCount * dmaPixelSize + dmaSettingsSize;

        _data = static_cast<uint8_t*>(malloc(_sizeData));
        // data cleared later in Begin()

        _i2sBuffer = static_cast<uint8_t*>(malloc(_i2sBufferSize));
        // no need to initialize it, it gets overwritten on every send

        // _i2sBuffer[0] = 0b11101000; // debug, 1 bit then 0 bit

        memset(_i2sZeroes, T_SPEED::Level, sizeof(_i2sZeroes));

        _is2BufMaxBlockSize = (c_maxDmaBlockSize / dmaPixelSize) * dmaPixelSize;

        _i2sBufDescCount = (_i2sBufferSize / _is2BufMaxBlockSize) + 1 + 2; // need two more for state/latch blocks
        _i2sBufDesc = (slc_queue_item*)malloc(_i2sBufDescCount * sizeof(slc_queue_item));

        s_this = this; // store this for the ISR
    }

    NeoEsp8266DmaMethodBase(uint8_t pin, uint16_t pixelCount, size_t elementSize, size_t settingsSize) : 
        NeoEsp8266DmaMethodBase(pixelCount, elementSize, settingsSize)
    {
    }

    ~NeoEsp8266DmaMethodBase()
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
        while ((micros() - time) < ((getPixelTime() + T_SPEED::ResetTimeUs) * waits))
        {
            yield();
        }

        StopDma();

        s_this = nullptr;
        pinMode(c_I2sPin, INPUT);

        free(_data);
        free(_i2sBuffer);
        free(_i2sBufDesc);
    }

    bool IsReadyToUpdate() const
    {
        return (_dmaState == NeoDmaState_Idle);
    }

    void Initialize()
    {
        StopDma();

        pinMode(c_I2sPin, FUNCTION_1); // I2S0_DATA

        uint8_t* is2Buffer = _i2sBuffer;
        uint32_t is2BufferSize = _i2sBufferSize;
        uint16_t indexDesc;

        // prepare main data block decriptors that point into our one static dma buffer
        for (indexDesc = 0; indexDesc < (_i2sBufDescCount - 2); indexDesc++)
        {
            uint32_t blockSize = (is2BufferSize > _is2BufMaxBlockSize) ? _is2BufMaxBlockSize : is2BufferSize;

            _i2sBufDesc[indexDesc].owner = 1;
            _i2sBufDesc[indexDesc].eof = 0; // no need to trigger interrupt generally
            _i2sBufDesc[indexDesc].sub_sof = 0;
            _i2sBufDesc[indexDesc].datalen = blockSize;
            _i2sBufDesc[indexDesc].blocksize = blockSize;
            _i2sBufDesc[indexDesc].buf_ptr = is2Buffer;
            _i2sBufDesc[indexDesc].unused = 0;
            _i2sBufDesc[indexDesc].next_link_ptr = reinterpret_cast<struct slc_queue_item*>(&(_i2sBufDesc[indexDesc + 1]));

            is2Buffer += blockSize;
            is2BufferSize -= blockSize;
        }

        // prepare the two state/latch descriptors
        for (; indexDesc < _i2sBufDescCount; indexDesc++)
        {
            _i2sBufDesc[indexDesc].owner = 1;
            _i2sBufDesc[indexDesc].eof = 0; // no need to trigger interrupt generally
            _i2sBufDesc[indexDesc].sub_sof = 0;
            _i2sBufDesc[indexDesc].datalen = sizeof(_i2sZeroes);
            _i2sBufDesc[indexDesc].blocksize = sizeof(_i2sZeroes);
            _i2sBufDesc[indexDesc].buf_ptr = _i2sZeroes;
            _i2sBufDesc[indexDesc].unused = 0;
            _i2sBufDesc[indexDesc].next_link_ptr = reinterpret_cast<struct slc_queue_item*>(&(_i2sBufDesc[indexDesc + 1]));
        }

        // the first state block will trigger the interrupt
        _i2sBufDesc[indexDesc - 2].eof = 1;
        // the last state block will loop to the first state block by defualt
        _i2sBufDesc[indexDesc - 1].next_link_ptr = reinterpret_cast<struct slc_queue_item*>(&(_i2sBufDesc[indexDesc - 2]));

        // setup the rest of i2s DMA
        //
        ETS_SLC_INTR_DISABLE();

        // start off in sending state as that is what it will be all setup to be
        // for the interrupt
        _dmaState = NeoDmaState_Sending; 

        SLCC0 |= SLCRXLR | SLCTXLR;
        SLCC0 &= ~(SLCRXLR | SLCTXLR);
        SLCIC = 0xFFFFFFFF;

        // Configure DMA
        SLCC0 &= ~(SLCMM << SLCM); // clear DMA MODE
        SLCC0 |= (1 << SLCM); // set DMA MODE to 1
        SLCRXDC |= SLCBINR | SLCBTNR; // enable INFOR_NO_REPLACE and TOKEN_NO_REPLACE
        SLCRXDC &= ~(SLCBRXFE | SLCBRXEM | SLCBRXFM); // disable RX_FILL, RX_EOF_MODE and RX_FILL_MODE

        // Feed DMA the 1st buffer desc addr
        // To send data to the I2S subsystem, counter-intuitively we use the RXLINK part, not the TXLINK as you might
        // expect. The TXLINK part still needs a valid DMA descriptor, even if it's unused: the DMA engine will throw
        // an error at us otherwise. Just feed it any random descriptor.
        SLCTXL &= ~(SLCTXLAM << SLCTXLA); // clear TX descriptor address
        // set TX descriptor address. any random desc is OK, we don't use TX but it needs to be valid
        SLCTXL |= (uint32)&(_i2sBufDesc[_i2sBufDescCount-1]) << SLCTXLA; 
        SLCRXL &= ~(SLCRXLAM << SLCRXLA); // clear RX descriptor address
        // set RX descriptor address.  use first of the data addresses
        SLCRXL |= (uint32)&(_i2sBufDesc[0]) << SLCRXLA; 

        ETS_SLC_INTR_ATTACH(i2s_slc_isr, NULL);
        SLCIE = SLCIRXEOF; // Enable only for RX EOF interrupt

        ETS_SLC_INTR_ENABLE();

        //Start transmission
        SLCTXL |= SLCTXLS;
        SLCRXL |= SLCRXLS;

        I2S_CLK_ENABLE();
        I2SIC = 0x3F;
        I2SIE = 0;

        //Reset I2S
        I2SC &= ~(I2SRST);
        I2SC |= I2SRST;
        I2SC &= ~(I2SRST);

        // Set RX/TX FIFO_MOD=0 and disable DMA (FIFO only)
        I2SFC &= ~(I2SDE | (I2STXFMM << I2STXFM) | (I2SRXFMM << I2SRXFM)); 
        I2SFC |= I2SDE; //Enable DMA
        // Set RX/TX CHAN_MOD=0
        I2SCC &= ~((I2STXCMM << I2STXCM) | (I2SRXCMM << I2SRXCM)); 

        // set the rate
        uint32_t i2s_clock_div = T_SPEED::I2sClockDivisor & I2SCDM;
        uint8_t i2s_bck_div = T_SPEED::I2sBaseClockDivisor & I2SBDM;

        //!trans master, !bits mod, rece slave mod, rece msb shift, right first, msb right
        I2SC &= ~(I2STSM | I2SRSM | (I2SBMM << I2SBM) | (I2SBDM << I2SBD) | (I2SCDM << I2SCD));
        I2SC |= I2SRF | I2SMR | I2SRSM | I2SRMS | (i2s_bck_div << I2SBD) | (i2s_clock_div << I2SCD);

        I2SC |= I2STXS; // Start transmission
    }

    void IRAM_ATTR Update(bool)
    {
        // wait for not actively sending data
        while (!IsReadyToUpdate())
        {
            yield();
        }
        FillBuffers();
        
        // toggle state so the ISR reacts
        _dmaState = NeoDmaState_Pending;
    }

    uint8_t* getData() const
    {
        return _data;
    };

    size_t getDataSize() const
    {
        return _sizeData;
    }

    void applySettings(const SettingsObject& settings)
    {
    }

private:
    const size_t  _sizeData;    // Size of '_data' buffer 
    uint8_t*  _data;        // Holds LED color values

    size_t _i2sBufferSize; // total size of _i2sBuffer
    uint8_t* _i2sBuffer;  // holds the DMA buffer that is referenced by _i2sBufDesc

    // normally 24 bytes creates the minimum 50us latch per spec, but
    // with the new logic, this latch is used to space between mulitple states
    // buffer size = (24 * (reset time / 50)) / 6
    uint8_t _i2sZeroes[(24L * (T_SPEED::ResetTimeUs / 50L)) / 6L];

    uint16_t _is2BufMaxBlockSize; // max size based on size of a pixel of a single block


    void FillBuffers()
    {
        uint16_t* pDma = (uint16_t*)_i2sBuffer;
        uint8_t* pEnd = _data + _sizeData;
        for (uint8_t* pData = _data; pData < pEnd; pData++)
        {
            *(pDma++) = T_SPEED::Convert(((*pData) & 0x0f));
            *(pDma++) = T_SPEED::Convert(((*pData) >> 4) & 0x0f);
        }
    }

    void StopDma()
    {
        ETS_SLC_INTR_DISABLE();

        // Disable any I2S send or receive
        I2SC &= ~(I2STXS | I2SRXS);

        // Reset I2S
        I2SC &= ~(I2SRST);
        I2SC |= I2SRST;
        I2SC &= ~(I2SRST);

        
        SLCIC = 0xFFFFFFFF;
        SLCIE = 0;
        SLCTXL &= ~(SLCTXLAM << SLCTXLA); // clear TX descriptor address
        SLCRXL &= ~(SLCRXLAM << SLCRXLA); // clear RX descriptor address

        pinMode(c_I2sPin, INPUT);
    }

    uint32_t getPixelTime() const
    {
        return (T_SPEED::ByteSendTimeUs * this->_sizeData);
    };

};



// normal
typedef NeoEsp8266DmaMethodBase<NeoEsp8266DmaSpeedWs2812x> NeoEsp8266DmaWs2812xMethod;
typedef NeoEsp8266DmaMethodBase<NeoEsp8266DmaSpeedSk6812> NeoEsp8266DmaSk6812Method;
typedef NeoEsp8266DmaMethodBase<NeoEsp8266DmaSpeedTm1814> NeoEsp8266DmaTm1814Method;
typedef NeoEsp8266DmaMethodBase<NeoEsp8266DmaSpeedTm1829> NeoEsp8266DmaTm1829Method;
typedef NeoEsp8266DmaTm1814Method NeoEsp8266DmaTm1914Method;
typedef NeoEsp8266DmaMethodBase<NeoEsp8266DmaSpeed800Kbps> NeoEsp8266Dma800KbpsMethod;
typedef NeoEsp8266DmaMethodBase<NeoEsp8266DmaSpeed400Kbps> NeoEsp8266Dma400KbpsMethod;
typedef NeoEsp8266DmaMethodBase<NeoEsp8266DmaSpeedApa106> NeoEsp8266DmaApa106Method;


// inverted
typedef NeoEsp8266DmaMethodBase<NeoEsp8266DmaInvertedSpeedWs2812x> NeoEsp8266DmaInvertedWs2812xMethod;
typedef NeoEsp8266DmaMethodBase<NeoEsp8266DmaInvertedSpeedSk6812> NeoEsp8266DmaInvertedSk6812Method;
typedef NeoEsp8266DmaMethodBase<NeoEsp8266DmaInvertedSpeedTm1814> NeoEsp8266DmaInvertedTm1814Method;
typedef NeoEsp8266DmaMethodBase<NeoEsp8266DmaInvertedSpeedTm1829> NeoEsp8266DmaInvertedTm1829Method;
typedef NeoEsp8266DmaInvertedTm1814Method NeoEsp8266DmaInvertedTm1914Method;
typedef NeoEsp8266DmaMethodBase<NeoEsp8266DmaInvertedSpeed800Kbps> NeoEsp8266DmaInverted800KbpsMethod;
typedef NeoEsp8266DmaMethodBase<NeoEsp8266DmaInvertedSpeed400Kbps> NeoEsp8266DmaInverted400KbpsMethod;
typedef NeoEsp8266DmaMethodBase<NeoEsp8266DmaInvertedSpeedApa106> NeoEsp8266DmaInvertedApa106Method;

// Dma  method is the default method for Esp8266
typedef NeoEsp8266DmaWs2812xMethod NeoWs2813Method;
typedef NeoEsp8266DmaWs2812xMethod NeoWs2812xMethod;
typedef NeoEsp8266Dma800KbpsMethod NeoWs2812Method;
typedef NeoEsp8266DmaWs2812xMethod NeoWs2811Method;
typedef NeoEsp8266DmaSk6812Method NeoSk6812Method;
typedef NeoEsp8266DmaTm1814Method NeoTm1814Method;
typedef NeoEsp8266DmaTm1829Method NeoTm1829Method;
typedef NeoEsp8266DmaTm1914Method NeoTm1914Method;
typedef NeoEsp8266DmaSk6812Method NeoLc8812Method;
typedef NeoEsp8266DmaApa106Method NeoApa106Method;

typedef NeoEsp8266DmaWs2812xMethod Neo800KbpsMethod;
typedef NeoEsp8266Dma400KbpsMethod Neo400KbpsMethod;

// inverted
typedef NeoEsp8266DmaInvertedWs2812xMethod NeoWs2813InvertedMethod;
typedef NeoEsp8266DmaInvertedWs2812xMethod NeoWs2812xInvertedMethod;
typedef NeoEsp8266DmaInverted800KbpsMethod NeoWs2812InvertedMethod;
typedef NeoEsp8266DmaInvertedWs2812xMethod NeoWs2811InvertedMethod;
typedef NeoEsp8266DmaInvertedSk6812Method NeoSk6812InvertedMethod;
typedef NeoEsp8266DmaInvertedTm1814Method NeoTm1814InvertedMethod;
typedef NeoEsp8266DmaInvertedTm1829Method NeoTm1829InvertedMethod;
typedef NeoEsp8266DmaInvertedTm1914Method NeoTm1914InvertedMethod;
typedef NeoEsp8266DmaInvertedSk6812Method NeoLc8812InvertedMethod;
typedef NeoEsp8266DmaInvertedApa106Method NeoApa106InvertedMethod;

typedef NeoEsp8266DmaInvertedWs2812xMethod Neo800KbpsInvertedMethod;
typedef NeoEsp8266DmaInverted400KbpsMethod Neo400KbpsInvertedMethod;
#endif
