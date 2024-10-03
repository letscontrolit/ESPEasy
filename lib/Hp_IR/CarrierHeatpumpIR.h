/*
    Carrier 42NQV035G / 38NYV035H2 heatpump control (remote control P/N WH-L05SE)
    Carrier 42MCA009515LS A/C control (remote control P/N R11CG/E)
       * Qlima (remote control P/N ABS10FP)
*/
#ifndef CarrierHeatpumpIR_h
#define CarrierHeatpumpIR_h

#include <HeatpumpIR.h>

// Carrier (42NQV035G / 38NYV035H2) timing constants (remote control P/N WH-L05SE)
#define CARRIER_AIRCON1_HDR_MARK   4320
#define CARRIER_AIRCON1_HDR_SPACE  4350
#define CARRIER_AIRCON1_BIT_MARK   500
#define CARRIER_AIRCON1_ONE_SPACE  1650
#define CARRIER_AIRCON1_ZERO_SPACE 550
#define CARRIER_AIRCON1_MSG_SPACE  7400

// Carrier codes
#define CARRIER_AIRCON1_MODE_AUTO  0x00 // Operating mode
#define CARRIER_AIRCON1_MODE_HEAT  0xC0
#define CARRIER_AIRCON1_MODE_COOL  0x80
#define CARRIER_AIRCON1_MODE_DRY   0x40
#define CARRIER_AIRCON1_MODE_FAN   0x20
#define CARRIER_AIRCON1_MODE_OFF   0xE0 // Power OFF
#define CARRIER_AIRCON1_FAN_AUTO   0x00 // Fan speed
#define CARRIER_AIRCON1_FAN1       0x02
#define CARRIER_AIRCON1_FAN2       0x06
#define CARRIER_AIRCON1_FAN3       0x01
#define CARRIER_AIRCON1_FAN4       0x05
#define CARRIER_AIRCON1_FAN5       0x03


// Carrier (42MCA009515LS) timing constants (remote control P/N R11CG/E)
#define CARRIER_AIRCON2_HDR_MARK   4510
#define CARRIER_AIRCON2_HDR_SPACE  4470
#define CARRIER_AIRCON2_BIT_MARK   600
#define CARRIER_AIRCON2_ONE_SPACE  1560
#define CARRIER_AIRCON2_ZERO_SPACE 500

#define CARRIER_AIRCON2_MODE_AUTO    0x10 // Operating mode
#define CARRIER_AIRCON2_MODE_COOL    0x00
#define CARRIER_AIRCON2_MODE_DRY     0x20
#define CARRIER_AIRCON2_MODE_FAN     0x20
#define CARRIER_AIRCON2_MODE_HEAT    0x30
#define CARRIER_AIRCON2_MODE_OFF     0x00 // Power OFF
#define CARRIER_AIRCON2_MODE_ON      0x20 // Power ON
#define CARRIER_AIRCON2_FAN_DRY_AUTO 0x00 // Fan speed, AUTO or DRY modes
#define CARRIER_AIRCON2_FAN1         0x01
#define CARRIER_AIRCON2_FAN2         0x02
#define CARRIER_AIRCON2_FAN3         0x04
#define CARRIER_AIRCON2_FAN_AUTO     0x05
#define CARRIER_AIRCON2_FAN_OFF      0x06


#define MODEL_CARRIER_MCA            1
#define MODEL_QLIMA_1                2
#define MODEL_QLIMA_2                3

class CarrierHeatpumpIR : public HeatpumpIR
{
  protected:
    CarrierHeatpumpIR();
    uint8_t _carrierModel;

  public:
    virtual void send(IRSender& IR, uint8_t powerModeCmd, uint8_t operatingModeCmd, uint8_t fanSpeedCmd, uint8_t temperatureCmd, uint8_t swingVCmd, uint8_t swingHCmd);
};

class CarrierNQVHeatpumpIR : public CarrierHeatpumpIR
{
  public:
    CarrierNQVHeatpumpIR();

  public:
    void send(IRSender& IR, uint8_t powerModeCmd, uint8_t operatingModeCmd, uint8_t fanSpeedCmd, uint8_t temperatureCmd, uint8_t swingVCmd, uint8_t swingHCmd);

  private:
    void sendCarrier(IRSender& IR, uint8_t operatingMode, uint8_t fanSpeed, uint8_t temperature);
};

class CarrierMCAHeatpumpIR : public CarrierHeatpumpIR
{
  public:
    CarrierMCAHeatpumpIR();

  public:
    void send(IRSender& IR, uint8_t powerModeCmd, uint8_t operatingModeCmd, uint8_t fanSpeedCmd, uint8_t temperatureCmd, uint8_t swingVCmd, uint8_t swingHCmd);
    void send(IRSender& IR, uint8_t powerModeCmd, uint8_t operatingModeCmd, uint8_t fanSpeedCmd, uint8_t temperatureCmd, uint8_t swingVCmd, uint8_t swingHCmd, bool turboMode);

  private:
    void sendCarrier(IRSender& IR, uint8_t powerMode, uint8_t operatingMode, uint8_t fanSpeed, uint8_t temperature, bool maintenanceMode, bool turboMode);
};

class Qlima1HeatpumpIR : public CarrierMCAHeatpumpIR
{
  public:
    Qlima1HeatpumpIR();
};

class Qlima2HeatpumpIR : public CarrierMCAHeatpumpIR
{
  public:
    Qlima2HeatpumpIR();
};

#endif
