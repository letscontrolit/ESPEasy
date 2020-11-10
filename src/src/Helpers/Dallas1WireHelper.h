#ifndef HELPERS_DALLAS1WIREHELPER_H
#define HELPERS_DALLAS1WIREHELPER_H

#include <Arduino.h>

// Used timings based on Maxim documentation.
// See https://www.maximintegrated.com/en/design/technical-documents/app-notes/1/126.html
// We use the "standard speed" timings, not the "Overdrive speed"



/*********************************************************************************************\
   Variables used to keep track of scanning the bus
   N.B. these should not be shared for simultaneous scans on different pins
\*********************************************************************************************/
extern unsigned char ROM_NO[8];
extern uint8_t LastDiscrepancy;
extern uint8_t LastFamilyDiscrepancy;
extern uint8_t LastDeviceFlag;


/*********************************************************************************************\
   Timings for diagnostics regarding the reset + presence detection
\*********************************************************************************************/
extern long usec_release;   // Time needed for the line to rise (typ: < 1 usec)
extern long presence_start; // Start presence condition after release by master (typ: 30 usec)
extern long presence_end;   // End presence condition (minimal 60 usec, typ: 100 usec)


/*********************************************************************************************\
   Format 1-wire address
\*********************************************************************************************/
String Dallas_getModel(uint8_t family);

String Dallas_format_address(const uint8_t addr[]);

/*********************************************************************************************\
   Dallas Scan bus
\*********************************************************************************************/
byte   Dallas_scan(byte     getDeviceROM,
                   uint8_t *ROM,
                   int8_t   gpio_pin);

// read power supply
bool Dallas_is_parasite(const uint8_t ROM[8],
                        int8_t        gpio_pin);

void Dallas_startConversion(const uint8_t ROM[8],
                            int8_t        gpio_pin);

/*********************************************************************************************\
*  Dallas data from scratchpad
\*********************************************************************************************/
bool Dallas_readTemp(const uint8_t ROM[8],
                     float        *value,
                     int8_t        gpio_pin);

bool Dallas_readiButton(const byte addr[8],
                        int8_t     gpio_pin);

bool Dallas_readCounter(const uint8_t ROM[8],
                        float        *value,
                        int8_t        gpio_pin,
                        uint8_t       counter);

/*********************************************************************************************\
* Dallas Get Resolution
\*********************************************************************************************/
byte Dallas_getResolution(const uint8_t ROM[8],
                          int8_t        gpio_pin);

/*********************************************************************************************\
* Dallas Set Resolution
\*********************************************************************************************/
bool Dallas_setResolution(const uint8_t ROM[8],
                          byte          res,
                          int8_t        gpio_pin);

/*********************************************************************************************\
*  Dallas Reset
\*********************************************************************************************/
uint8_t Dallas_reset(int8_t gpio_pin);


/*********************************************************************************************\
*  Dallas Reset Search
\*********************************************************************************************/
void    Dallas_reset_search();

/*********************************************************************************************\
*  Dallas Search bus
\*********************************************************************************************/
uint8_t Dallas_search(uint8_t *newAddr,
                      int8_t   gpio_pin);

/*********************************************************************************************\
*  Dallas Read byte
\*********************************************************************************************/
uint8_t Dallas_read(int8_t gpio_pin);

/*********************************************************************************************\
*  Dallas Write byte
\*********************************************************************************************/
void    Dallas_write(uint8_t ByteToWrite,
                     int8_t  gpio_pin);

/*********************************************************************************************\
*  Dallas Read bit
*  See https://github.com/espressif/arduino-esp32/issues/1335
\*********************************************************************************************/
uint8_t Dallas_read_bit(int8_t gpio_pin) ICACHE_RAM_ATTR;

/*********************************************************************************************\
*  Dallas Write bit
*  See https://github.com/espressif/arduino-esp32/issues/1335
\*********************************************************************************************/
void Dallas_write_bit(uint8_t v,
                      int8_t  gpio_pin) ICACHE_RAM_ATTR;

/*********************************************************************************************\
*  Standard function to initiate addressing a sensor.
\*********************************************************************************************/
bool Dallas_address_ROM(const uint8_t ROM[8],
                        int8_t        gpio_pin);

/*********************************************************************************************\
*  Dallas Calculate CRC8 and compare it of addr[0-7] and compares it to addr[8]
\*********************************************************************************************/
bool     Dallas_crc8(const uint8_t *addr);

/*********************************************************************************************\
*  Dallas Calculate CRC16
\*********************************************************************************************/
uint16_t Dallas_crc16(const uint8_t *input,
                      uint16_t       len,
                      uint16_t       crc);

#endif // ifndef HELPERS_DALLAS1WIREHELPER_H
