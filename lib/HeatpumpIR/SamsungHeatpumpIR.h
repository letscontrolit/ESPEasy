/*
    Samsung AQV12PSBN / AQV09ASA heatpump control (remote control P/N zzz)
    Samsung FJM (RJ040F2HXEA / MH026FNEA) heatpump control (remote control P/N ARH-465)
*/
#ifndef SamsungHeatpumpIR_h
#define SamsungHeatpumpIR_h

#include <HeatpumpIR.h>

// Samsung timing constants
#define SAMSUNG_AIRCON1_HDR_MARK   3000
#define SAMSUNG_AIRCON1_HDR_SPACE  9000
#define SAMSUNG_AIRCON1_BIT_MARK   500
#define SAMSUNG_AIRCON1_ONE_SPACE  1500
#define SAMSUNG_AIRCON1_ZERO_SPACE 500
#define SAMSUNG_AIRCON1_MSG_SPACE  2000
/* Orig
#define SAMSUNG_AIRCON2_HDR_MARK   3000
#define SAMSUNG_AIRCON2_HDR_SPACE  9150
#define SAMSUNG_AIRCON2_BIT_MARK   360
#define SAMSUNG_AIRCON2_ONE_SPACE  1600
#define SAMSUNG_AIRCON2_ZERO_SPACE 630
*/
#define SAMSUNG_AIRCON2_HDR_MARK   2920
#define SAMSUNG_AIRCON2_HDR_SPACE  8960
#define SAMSUNG_AIRCON2_BIT_MARK   490
#define SAMSUNG_AIRCON2_ONE_SPACE  1560
#define SAMSUNG_AIRCON2_ZERO_SPACE 546

// Samsung codes
#define SAMSUNG_AIRCON1_MODE_AUTO  0x00 // Operating mode
#define SAMSUNG_AIRCON1_MODE_HEAT  0x40
#define SAMSUNG_AIRCON1_MODE_COOL  0x10
#define SAMSUNG_AIRCON1_MODE_DRY   0x20
#define SAMSUNG_AIRCON1_MODE_FAN   0x30
#define SAMSUNG_AIRCON1_MODE_OFF   0xC0 // Power OFF
#define SAMSUNG_AIRCON1_MODE_ON    0xF0 // Power ON
#define SAMSUNG_AIRCON1_FAN_AUTO   0x01 // Fan speed
#define SAMSUNG_AIRCON1_FAN1       0x05 // * low
#define SAMSUNG_AIRCON1_FAN2       0x09 // * med
#define SAMSUNG_AIRCON1_FAN3       0x0B // * high
#define SAMSUNG_AIRCON2_FAN4       0x0B // * very high
#define SAMSUNG_AIRCON1_VS_SWING   0xAE // Vertical swing
#define SAMSUNG_AIRCON1_VS_AUTO    0xFE

#define SAMSUNG_AIRCON2_VS_SWING   0xA0 // Vertical swing
#define SAMSUNG_AIRCON2_VS_AUTO    0xF0
#define SAMSUNG_AIRCON2_TURBO      0x06 // 30 minutes of full power


class SamsungHeatpumpIR : public HeatpumpIR
{
  protected:
    SamsungHeatpumpIR();
  
  public:
    virtual void send(IRSender& IR, uint8_t powerModeCmd, uint8_t operatingModeCmd, uint8_t fanSpeedCmd, uint8_t temperatureCmd, uint8_t swingVCmd, uint8_t swingHCmd);

  private:
    void sendSamsung(IRSender& IR, uint8_t powerMode, uint8_t operatingMode, uint8_t fanSpeed, uint8_t temperature, uint8_t swingV);
};


class SamsungAQVHeatpumpIR : public SamsungHeatpumpIR
{
  public:
    SamsungAQVHeatpumpIR();
    void send(IRSender& IR, uint8_t powerModeCmd, uint8_t operatingModeCmd, uint8_t fanSpeedCmd, uint8_t temperatureCmd, uint8_t swingVCmd, uint8_t swingHCmd);
    
  private:
    void sendSamsung(IRSender& IR, uint8_t powerMode, uint8_t operatingMode, uint8_t fanSpeed, uint8_t temperature, uint8_t swingV);    
};

class SamsungFJMHeatpumpIR : public SamsungHeatpumpIR
{
  public:
    SamsungFJMHeatpumpIR();
    void send(IRSender& IR, uint8_t powerModeCmd, uint8_t operatingModeCmd, uint8_t fanSpeedCmd, uint8_t temperatureCmd, uint8_t swingVCmd, uint8_t swingHCmd);
    void send(IRSender& IR, uint8_t powerModeCmd, uint8_t operatingModeCmd, uint8_t fanSpeedCmd, uint8_t temperatureCmd, uint8_t swingVCmd, uint8_t swingHCmd, bool turboModeCmd);
    
  private:
    void sendSamsung(IRSender& IR, uint8_t powerMode, uint8_t operatingMode, uint8_t fanSpeed, uint8_t temperature, uint8_t swingV, bool turboMode);    
};

#endif
