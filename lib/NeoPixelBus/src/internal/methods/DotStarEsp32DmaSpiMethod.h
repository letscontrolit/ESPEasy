/*-------------------------------------------------------------------------
NeoPixel library helper functions for DotStars using Esp32, DMA and SPI (APA102).

Written by Michael C. Miller.
DotStarEsp32DmaSpiMethod written by Louis Beaudoin (Pixelvation)

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

#include "driver/spi_master.h"

// API and type use require newer IDF versions
#if ESP_IDF_VERSION >= ESP_IDF_VERSION_VAL(4, 4, 1)

template<typename T_SPISPEED, typename T_SPIBUS> class DotStarEsp32DmaSpiMethodBase
{
public:
    typedef typename T_SPISPEED::SettingsObject SettingsObject;

    DotStarEsp32DmaSpiMethodBase(uint16_t pixelCount, size_t elementSize, size_t settingsSize) :
        _sizeStartFrame(4 * T_SPIBUS::ParallelBits),
        _sizePixelData(pixelCount * elementSize + settingsSize),
        _sizeEndFrame((pixelCount + 15) / 16 * T_SPIBUS::ParallelBits) // 16 = div 2 (bit for every two pixels) div 8 (bits to bytes)
    {
        _spiBufferSize = _sizeStartFrame + _sizePixelData + _sizeEndFrame;

        // must have a 4 byte aligned buffer for i2s
        uint32_t alignment = _spiBufferSize % 4;
        if (alignment)
        {
            _spiBufferSize += 4 - alignment;
        }

        _data = static_cast<uint8_t*>(malloc(_spiBufferSize));
        _dmadata = static_cast<uint8_t*>(heap_caps_malloc(_spiBufferSize, MALLOC_CAP_DMA));

        // data cleared later in NeoPixelBus::Begin()
    }

    // Support constructor specifying pins by ignoring pins
    DotStarEsp32DmaSpiMethodBase(uint8_t, uint8_t, uint16_t pixelCount, size_t elementSize, size_t settingsSize) :
        DotStarEsp32DmaSpiMethodBase(pixelCount, elementSize, settingsSize)
    {
    }

    ~DotStarEsp32DmaSpiMethodBase()
    {
        if (_spiHandle)
        {
            deinitSpiDevice();
            esp_err_t ret = spi_bus_free(T_SPIBUS::SpiHostDevice);
            ESP_ERROR_CHECK(ret);            
        }
        free(_data);
        heap_caps_free(_dmadata);
        _spiHandle = NULL;
    }

    bool IsReadyToUpdate() const
    {
        spi_transaction_t t;
        spi_transaction_t* tptr = &t;

        esp_err_t ret = spi_device_get_trans_result(_spiHandle, &tptr, 0);

        // we are ready if prev result is back (ESP_OK) or if we got a timeout and
        // transaction length of 0 (we didn't start a transaction)
        return (ret == ESP_OK || (ret == ESP_ERR_TIMEOUT && 0 == _spiTransaction.length));
    }

    void Initialize(int8_t sck, int8_t dat0, int8_t dat1, int8_t dat2, int8_t dat3, int8_t dat4, int8_t dat5, int8_t dat6, int8_t dat7, int8_t ss)
    {
        memset(_data, 0x00, _sizeStartFrame);
        memset(_data + _sizeStartFrame + _sizePixelData, 0x00, _spiBufferSize - (_sizeStartFrame + _sizePixelData));

        _ssPin = ss;

        esp_err_t ret;
        spi_bus_config_t buscfg = { 0 };
 
        buscfg.sclk_io_num = sck;
        buscfg.data0_io_num = dat0;
        buscfg.data1_io_num = dat1;
        buscfg.data2_io_num = dat2;
        buscfg.data3_io_num = dat3;
        buscfg.data4_io_num = dat4;
        buscfg.data5_io_num = dat5;
        buscfg.data6_io_num = dat6;
        buscfg.data7_io_num = dat7;
        buscfg.max_transfer_sz = _spiBufferSize;
        if (T_SPIBUS::ParallelBits == 8) 
        {
            buscfg.flags = SPICOMMON_BUSFLAG_OCTAL;
        }

        //Initialize the SPI bus
        ret = spi_bus_initialize(T_SPIBUS::SpiHostDevice, &buscfg, SPI_DMA_CH_AUTO);
        ESP_ERROR_CHECK(ret);

        _spiTransaction = { 0 };
        initSpiDevice();
    }

    void Initialize(int8_t sck, int8_t dat0, int8_t dat1, int8_t dat2, int8_t dat3, int8_t ss)
    {
        Initialize(sck, dat0, dat1, dat2, dat3, -1, -1, -1, -1, ss);
    }

    void Initialize(int8_t sck, int8_t miso, int8_t mosi, int8_t ss)
    {
        Initialize(sck, mosi, miso, -1, -1, ss);
    }

    // If pins aren't specified, initialize bus with just the default SCK and MOSI pins for the SPI peripheral (no SS, no >1-bit pins)
    void Initialize()
    {
#if !defined(CONFIG_IDF_TARGET_ESP32S2) && !defined(CONFIG_IDF_TARGET_ESP32C3) && !defined(CONFIG_IDF_TARGET_ESP32S3)
        if (T_SPIBUS::SpiHostDevice == VSPI_HOST)
        {
            Initialize(SCK, -1, MOSI, -1, -1, -1);
        }
        else
        {
            Initialize(14, -1, 13, -1, -1, -1);
        }
#else
        Initialize(SCK, -1, MOSI, -1, -1, -1);
#endif
    }

    void Update(bool)
    {
        while(!IsReadyToUpdate()) 
        {
            portYIELD();
        }

        memcpy(_dmadata, _data, _spiBufferSize);

        _spiTransaction = { 0 };
        _spiTransaction.length = (_spiBufferSize) * 8; // in bits not bytes!

        if (T_SPIBUS::ParallelBits == 1)
        {
            _spiTransaction.flags = 0;
        }
        if (T_SPIBUS::ParallelBits == 2)
        {
            _spiTransaction.flags = SPI_TRANS_MODE_DIO;
        }
        if (T_SPIBUS::ParallelBits == 4)
        {
            _spiTransaction.flags = SPI_TRANS_MODE_QIO;
        }
        if (T_SPIBUS::ParallelBits == 8)
        {
            _spiTransaction.flags = SPI_TRANS_MODE_OCT;
        }
        _spiTransaction.tx_buffer = _dmadata;

        esp_err_t ret = spi_device_queue_trans(_spiHandle, &_spiTransaction, 0);  //Transmit!
        ESP_ERROR_CHECK(ret);
    }

    bool AlwaysUpdate()
    {
        // this method requires update to be called only if changes to buffer
        return false;
    }

    uint8_t* getData() const
    {
        return _data + _sizeStartFrame;
    };

    size_t getDataSize() const
    {
        return _sizePixelData;
    };

    void applySettings([[maybe_unused]] const SettingsObject& settings)
    {
        _speed.applySettings(settings);
        if (_spiHandle)
        {
            deinitSpiDevice();
            initSpiDevice();
        }
    }

private:
    void initSpiDevice()
    {
        spi_device_interface_config_t devcfg = {0};

        devcfg.clock_speed_hz = _speed.Clock;
        devcfg.mode = 0;                 //SPI mode 0
        devcfg.spics_io_num = _ssPin;    //CS pin
        devcfg.queue_size = 1;
        if (T_SPIBUS::ParallelBits == 1)
        {
            devcfg.flags = 0;
        }
        if (T_SPIBUS::ParallelBits >= 2)
        {
            devcfg.flags = SPI_DEVICE_HALFDUPLEX;
        }

        //Allocate the LEDs on the SPI bus
        esp_err_t ret = spi_bus_add_device(T_SPIBUS::SpiHostDevice, &devcfg, &_spiHandle);
        ESP_ERROR_CHECK(ret);
    }

    void deinitSpiDevice()
    {
        while(!IsReadyToUpdate());
        esp_err_t ret = spi_bus_remove_device(_spiHandle);
        ESP_ERROR_CHECK(ret);
    }

    const size_t             _sizeStartFrame;
    const size_t             _sizePixelData;   // Size of '_data' buffer below, minus (_sizeStartFrame + _sizeEndFrame)
    const size_t             _sizeEndFrame;

    size_t                  _spiBufferSize;
    uint8_t*                _data;       // Holds start/end frames and LED color values
    uint8_t*                _dmadata;    // Holds start/end frames and LED color values
    spi_device_handle_t     _spiHandle = NULL;
    spi_transaction_t       _spiTransaction;
    T_SPISPEED              _speed;
    int8_t                  _ssPin;
};


// Unfortunately we have a bit of a mess with SPI bus names across different version of the ESP32
// e.g ESP32 has SPI, HSPI, and VSPI (1, 2, and 3), ESP32-S2 has SPI, FSPI, and HSPI (1, 2, and 3)
// and the S3 and C3 dropped the silly names entirely and just uses SPI1, SPI2, and SPI3.
//
// SPI1 can be only be used by ESP32 and supports up to 4 bit
// SPI2 supports up to 4 bit output across all of those devices (!) and supports 8 bit on S2 and S3
// SPI3 supports up to 4 bit output on ESP32 and S3, and 1 bit only on the S2

enum spi_bus_width_t {
    WIDTH1 = 1,
    WIDTH2 = 2,
    WIDTH4 = 4,
    WIDTH8 = 8,
};

template <spi_host_device_t bus, spi_bus_width_t bits = WIDTH1>
struct Esp32SpiBus
{
    const static spi_host_device_t SpiHostDevice = bus;
    const static int ParallelBits = bits;
};

// Define all valid ESP32 SPI Buses with a default speed

// SPI1 -- ESP32 Only
#if defined(CONFIG_IDF_TARGET_ESP32)
typedef Esp32SpiBus<SPI1_HOST, WIDTH1> Esp32Spi1Bus;
typedef Esp32SpiBus<SPI1_HOST, WIDTH2> Esp32Spi12BitBus;
typedef Esp32SpiBus<SPI1_HOST, WIDTH4> Esp32Spi14BitBus;

typedef DotStarEsp32DmaSpiMethodBase<SpiSpeed10Mhz, Esp32Spi1Bus> DotStarEsp32DmaSpi1Method;
typedef DotStarEsp32DmaSpiMethodBase<SpiSpeed10Mhz, Esp32Spi12BitBus> DotStarEsp32DmaSpi12BitMethod;
typedef DotStarEsp32DmaSpiMethodBase<SpiSpeed10Mhz, Esp32Spi14BitBus> DotStarEsp32DmaSpi14BitMethod;
#endif

// SPI2
typedef Esp32SpiBus<SPI2_HOST, WIDTH1> Esp32Spi2Bus;
typedef Esp32SpiBus<SPI2_HOST, WIDTH2> Esp32Spi22BitBus;
typedef Esp32SpiBus<SPI2_HOST, WIDTH4> Esp32Spi24BitBus;
#if SOC_SPI_SUPPORT_OCT
typedef Esp32SpiBus<SPI2_HOST, WIDTH8> Esp32Spi28BitBus;
#endif

typedef DotStarEsp32DmaSpiMethodBase<SpiSpeed10Mhz, Esp32Spi2Bus> DotStarEsp32DmaSpi2Method;
typedef DotStarEsp32DmaSpiMethodBase<SpiSpeed10Mhz, Esp32Spi22BitBus> DotStarEsp32DmaSpi22BitMethod;
typedef DotStarEsp32DmaSpiMethodBase<SpiSpeed10Mhz, Esp32Spi24BitBus> DotStarEsp32DmaSpi24BitMethod;
#if SOC_SPI_SUPPORT_OCT
typedef DotStarEsp32DmaSpiMethodBase<SpiSpeed10Mhz, Esp32Spi28BitBus> DotStarEsp32DmaSpi28BitMethod;
#endif


// SPI3
#if (defined(CONFIG_IDF_TARGET_ESP32) || defined(CONFIG_IDF_TARGET_ESP32S3))
typedef Esp32SpiBus<SPI3_HOST, WIDTH1> Esp32Spi3Bus;
typedef Esp32SpiBus<SPI3_HOST, WIDTH2> Esp32Spi32BitBus;
typedef Esp32SpiBus<SPI3_HOST, WIDTH4> Esp32Spi34BitBus;

typedef DotStarEsp32DmaSpiMethodBase<SpiSpeed10Mhz, Esp32Spi3Bus> DotStarEsp32DmaSpi3Method;
typedef DotStarEsp32DmaSpiMethodBase<SpiSpeed10Mhz, Esp32Spi32BitBus> DotStarEsp32DmaSpi32BitMethod;
typedef DotStarEsp32DmaSpiMethodBase<SpiSpeed10Mhz, Esp32Spi34BitBus> DotStarEsp32DmaSpi34BitMethod;
#endif

#if defined(CONFIG_IDF_TARGET_ESP32S2)
typedef Esp32SpiBus<SPI3_HOST, WIDTH1> Esp32Spi3Bus;
typedef DotStarEsp32DmaSpiMethodBase<SpiSpeed10Mhz, Esp32Spi3Bus> DotStarEsp32DmaSpi3Method;
#endif

// Default SpiDma methods if we don't care about bus. It's nice that every single ESP32 out there
// supports up to 4 bits on SPI2

typedef DotStarEsp32DmaSpi2Method DotStarEsp32DmaSpiMethod;
typedef DotStarEsp32DmaSpi22BitMethod DotStarEsp32DmaSpi2BitMethod;
typedef DotStarEsp32DmaSpi24BitMethod DotStarEsp32DmaSpi4BitMethod;
#if SOC_SPI_SUPPORT_OCT
typedef DotStarEsp32DmaSpi28BitMethod DotStarEsp32DmaSpi8BitMethod;
#endif

#endif // ESP_IDF_VERSION >= ESP_IDF_VERSION_VAL(4, 4, 1)