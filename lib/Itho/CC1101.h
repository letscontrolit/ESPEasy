/*
 * Author: Klusjesman, modified bij supersjimmie for Arduino/ESP8266
 * 2022-08-10 tonhuisman: Fixed perpetual blocking while loops by limiting these to 3000 msec.
 * 2022-08-11 tonhuisman: Change 3000 to #define ITHO_MAX_WAIT for easy adjustment
 */

#ifndef __CC1101_H__
#define __CC1101_H__

#include <stdio.h>
#include "CC1101Packet.h"
#include <SPI.h>

// On Arduino, SPI pins are predefined
#ifndef PIN_SPI_SS
# define PIN_SPI_SS   (15)
#endif // ifndef PIN_SPI_SS

#ifndef ITHO_MAX_WAIT
# define ITHO_MAX_WAIT                     1000 // Wait no longer than this nr of milliseconds
#endif // ifndef ITHO_MAX_WAIT

/*	Type of transfers */
#define CC1101_WRITE_BURST                 0x40
#define CC1101_READ_SINGLE                 0x80
#define CC1101_READ_BURST                  0xC0

/*	Type of register */
#define CC1101_CONFIG_REGISTER             CC1101_READ_SINGLE
#define CC1101_STATUS_REGISTER             CC1101_READ_BURST

/*	PATABLE & FIFO's */
#define CC1101_PATABLE                     0x3E // PATABLE address
#define CC1101_TXFIFO                      0x3F // TX FIFO address
#define CC1101_RXFIFO                      0x3F // RX FIFO address
#define CC1101_PA_LowPower                 0x60
#define CC1101_PA_LongDistance             0xC0

/*	Command strobes */
#define CC1101_SRES                        0x30 // Reset CC1101 chip
#define CC1101_SFSTXON                     0x31 // Enable and calibrate frequency synthesizer (if
                                                // MCSM0.FS_AUTOCAL=1). If in RX (with CCA): Go to a
                                                // wait state where only the synthesizer is running
                                                // (for quick RX / TX turnaround).
#define CC1101_SXOFF                       0x32 // Turn off crystal oscillator
#define CC1101_SCAL                        0x33 // Calibrate frequency synthesizer and turn it off.
                                                // SCAL can be strobed from IDLE mode without setting
                                                // manual calibration mode (MCSM0.FS_AUTOCAL=0)
#define CC1101_SRX                         0x34 // Enable RX. Perform calibration first if coming from
                                                // IDLE and MCSM0.FS_AUTOCAL=1
#define CC1101_STX                         0x35 // In IDLE state: Enable TX. Perform calibration first
                                                // if MCSM0.FS_AUTOCAL=1. If in RX state and CCA is
                                                // enabled: Only go to TX if channel is clear
#define CC1101_SIDLE                       0x36 // Exit RX / TX, turn off frequency synthesizer and
                                                // exit Wake-On-Radio mode if applicable
#define CC1101_SWOR                        0x38 // Start automatic RX polling sequence (Wake-on-Radio)
                                                // as described in Section 19.5 if WORCTRL.RC_PD=0
#define CC1101_SPWD                        0x39 // Enter power down mode when CSn goes high
#define CC1101_SFRX                        0x3A // Flush the RX FIFO buffer. Only issue SFRX in IDLE or
                                                // RXFIFO_OVERFLOW states
#define CC1101_SFTX                        0x3B // Flush the TX FIFO buffer. Only issue SFTX in IDLE or
                                                // TXFIFO_UNDERFLOW states
#define CC1101_SWORRST                     0x3C // Reset real time clock to Event1 value
#define CC1101_SNOP                        0x3D // No operation. May be used to get access to the chip
                                                // status byte

/*	CC1101 configuration registers */
#define CC1101_IOCFG2                      0x00 // GDO2 Output Pin Configuration
#define CC1101_IOCFG1                      0x01 // GDO1 Output Pin Configuration
#define CC1101_IOCFG0                      0x02 // GDO0 Output Pin Configuration
#define CC1101_FIFOTHR                     0x03 // RX FIFO and TX FIFO Thresholds
#define CC1101_SYNC1                       0x04 // Sync Word, High Byte
#define CC1101_SYNC0                       0x05 // Sync Word, Low Byte
#define CC1101_PKTLEN                      0x06 // Packet Length
#define CC1101_PKTCTRL1                    0x07 // Packet Automation Control
#define CC1101_PKTCTRL0                    0x08 // Packet Automation Control
#define CC1101_ADDR                        0x09 // Device Address
#define CC1101_CHANNR                      0x0A // Channel Number
#define CC1101_FSCTRL1                     0x0B // Frequency Synthesizer Control
#define CC1101_FSCTRL0                     0x0C // Frequency Synthesizer Control
#define CC1101_FREQ2                       0x0D // Frequency Control Word, High Byte
#define CC1101_FREQ1                       0x0E // Frequency Control Word, Middle Byte
#define CC1101_FREQ0                       0x0F // Frequency Control Word, Low Byte
#define CC1101_MDMCFG4                     0x10 // Modem Configuration
#define CC1101_MDMCFG3                     0x11 // Modem Configuration
#define CC1101_MDMCFG2                     0x12 // Modem Configuration
#define CC1101_MDMCFG1                     0x13 // Modem Configuration
#define CC1101_MDMCFG0                     0x14 // Modem Configuration
#define CC1101_DEVIATN                     0x15 // Modem Deviation Setting
#define CC1101_MCSM2                       0x16 // Main Radio Control State Machine Configuration
#define CC1101_MCSM1                       0x17 // Main Radio Control State Machine Configuration
#define CC1101_MCSM0                       0x18 // Main Radio Control State Machine Configuration
#define CC1101_FOCCFG                      0x19 // Frequency Offset Compensation Configuration
#define CC1101_BSCFG                       0x1A // Bit Synchronization Configuration
#define CC1101_AGCCTRL2                    0x1B // AGC Control
#define CC1101_AGCCTRL1                    0x1C // AGC Control
#define CC1101_AGCCTRL0                    0x1D // AGC Control
#define CC1101_WOREVT1                     0x1E // High Byte Event0 Timeout
#define CC1101_WOREVT0                     0x1F // Low Byte Event0 Timeout
#define CC1101_WORCTRL                     0x20 // Wake On Radio Control
#define CC1101_FREND1                      0x21 // Front End RX Configuration
#define CC1101_FREND0                      0x22 // Front End TX Configuration
#define CC1101_FSCAL3                      0x23 // Frequency Synthesizer Calibration
#define CC1101_FSCAL2                      0x24 // Frequency Synthesizer Calibration
#define CC1101_FSCAL1                      0x25 // Frequency Synthesizer Calibration
#define CC1101_FSCAL0                      0x26 // Frequency Synthesizer Calibration
#define CC1101_RCCTRL1                     0x27 // RC Oscillator Configuration
#define CC1101_RCCTRL0                     0x28 // RC Oscillator Configuration
#define CC1101_FSTEST                      0x29 // Frequency Synthesizer Calibration Control
#define CC1101_PTEST                       0x2A // Production Test
#define CC1101_AGCTEST                     0x2B // AGC Test
#define CC1101_TEST2                       0x2C // Various Test Settings
#define CC1101_TEST1                       0x2D // Various Test Settings
#define CC1101_TEST0                       0x2E // Various Test Settings

/*	Status registers */
#define CC1101_PARTNUM                     0x30 // Chip ID
#define CC1101_VERSION                     0x31 // Chip ID
#define CC1101_FREQEST                     0x32 // Frequency Offset Estimate from Demodulator
#define CC1101_LQI                         0x33 // Demodulator Estimate for Link Quality
#define CC1101_RSSI                        0x34 // Received Signal Strength Indication
#define CC1101_MARCSTATE                   0x35 // Main Radio Control State Machine State
#define CC1101_WORTIME1                    0x36 // High Byte of WOR Time
#define CC1101_WORTIME0                    0x37 // Low Byte of WOR Time
#define CC1101_PKTSTATUS                   0x38 // Current GDOx Status and Packet Status
#define CC1101_VCO_VC_DAC                  0x39 // Current Setting from PLL Calibration Module
#define CC1101_TXBYTES                     0x3A // Underflow and Number of Bytes
#define CC1101_RXBYTES                     0x3B // Overflow and Number of Bytes
#define CC1101_RCCTRL1_STATUS              0x3C // Last RC Oscillator Calibration Result
#define CC1101_RCCTRL0_STATUS              0x3D // Last RC Oscillator Calibration Result

/* Bit fields in the chip status byte */
#define CC1101_STATUS_CHIP_RDYn_BM              0x80
#define CC1101_STATUS_STATE_BM                  0x70
#define CC1101_STATUS_FIFO_BYTES_AVAILABLE_BM   0x0F

/* Masks to retrieve status bit */
#define CC1101_BITS_TX_FIFO_UNDERFLOW           0x80
#define CC1101_BITS_RX_BYTES_IN_FIFO            0x7F
#define CC1101_BITS_MARCSTATE                   0x1F


/* Marc states */
enum CC1101MarcStates
{
  CC1101_MARCSTATE_SLEEP            = 0x00,
  CC1101_MARCSTATE_IDLE             = 0x01,
  CC1101_MARCSTATE_XOFF             = 0x02,
  CC1101_MARCSTATE_VCOON_MC         = 0x03,
  CC1101_MARCSTATE_REGON_MC         = 0x04,
  CC1101_MARCSTATE_MANCAL           = 0x05,
  CC1101_MARCSTATE_VCOON            = 0x06,
  CC1101_MARCSTATE_REGON            = 0x07,
  CC1101_MARCSTATE_STARTCAL         = 0x08,
  CC1101_MARCSTATE_BWBOOST          = 0x09,
  CC1101_MARCSTATE_FS_LOCK          = 0x0A,
  CC1101_MARCSTATE_IFADCON          = 0x0B,
  CC1101_MARCSTATE_ENDCAL           = 0x0C,
  CC1101_MARCSTATE_RX               = 0x0D,
  CC1101_MARCSTATE_RX_END           = 0x0E,
  CC1101_MARCSTATE_RX_RST           = 0x0F,
  CC1101_MARCSTATE_TXRX_SWITCH      = 0x10,
  CC1101_MARCSTATE_RXFIFO_OVERFLOW  = 0x11,
  CC1101_MARCSTATE_FSTXON           = 0x12,
  CC1101_MARCSTATE_TX               = 0x13,
  CC1101_MARCSTATE_TX_END           = 0x14,
  CC1101_MARCSTATE_RXTX_SWITCH      = 0x15,
  CC1101_MARCSTATE_TXFIFO_UNDERFLOW = 0x16
};


/* Chip states */
enum CC1101ChipStates
{
  CC1101_STATE_MASK         = 0x70,
  CC1101_STATE_IDLE         = 0x00,
  CC1101_STATE_RX           = 0x10,
  CC1101_STATE_TX           = 0x20,
  CC1101_STATE_FSTXON       = 0x30,
  CC1101_STATE_CALIBRATE    = 0x40,
  CC1101_STATE_SETTLING     = 0x50,
  CC1101_STATE_RX_OVERFLOW  = 0x60,
  CC1101_STATE_TX_UNDERFLOW = 0x70
};


class CC1101 {
protected:

  // functions

public:

  CC1101(int8_t CSpin   = PIN_SPI_SS,
         int8_t MISOpin = MISO);
  virtual ~CC1101();

  // spi
  void    spi_waitMiso();

  // cc1101
  void    init();

  uint8_t writeCommand(uint8_t command);
  void    writeRegister(uint8_t address,
                        uint8_t data);

  uint8_t readRegister(uint8_t address,
                       uint8_t registerType);

  void    writeBurstRegister(uint8_t  address,
                             uint8_t *data,
                             uint8_t  length);
  void    readBurstRegister(uint8_t *buffer,
                            uint8_t  address,
                            uint8_t  length);

  void    sendData(CC1101Packet *packet);
  uint8_t receiveData(CC1101Packet *packet,
                      uint8_t       length);

private:

  CC1101(const CC1101& c);
  CC1101& operator=(const CC1101& c);

  // SPI helper functions
  void    select(void);
  void    deselect(void);

  int8_t _CSpin = PIN_SPI_SS;
  int8_t _MISOpin;

protected:

  uint8_t readRegister(uint8_t address);
  uint8_t readRegisterMedian3(uint8_t address);
  uint8_t readRegisterWithSyncProblem(uint8_t address,
                                      uint8_t registerType);

  void    reset();
}; // CC1101

#endif // __CC1101_H__
