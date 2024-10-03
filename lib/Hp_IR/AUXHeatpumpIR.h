/*
    AUX heatpump control (remote control YKR-N/002E, YKR-P/002E)
*/
#ifndef AUXHeatpumpIR_h
#define AUXHeatpumpIR_h

#include <HeatpumpIR.h>


// AUX timing constants
#define AUX_AIRCON1_HDR_MARK    8800 // 9260 <-- Arduino decoder printouts
#define AUX_AIRCON1_HDR_SPACE   4580 // 4540
#define AUX_AIRCON1_BIT_MARK    490  // 540
#define AUX_AIRCON1_ONE_SPACE   1740 // 1640
#define AUX_AIRCON1_ZERO_SPACE  620  // 480

// AUX codes
#define AUX_AIRCON1_MODE_AUTO   0x00 // Operating mode
#define AUX_AIRCON1_MODE_HEAT   0x80
#define AUX_AIRCON1_MODE_COOL   0x20
#define AUX_AIRCON1_MODE_DRY    0x40
#define AUX_AIRCON1_MODE_FAN    0xC0
#define AUX_AIRCON1_MODE_OFF    0x00 // Power
#define AUX_AIRCON1_MODE_ON     0x20
#define AUX_AIRCON1_FAN_AUTO    0xA0 // Fan speed
#define AUX_AIRCON1_FAN1        0x60
#define AUX_AIRCON1_FAN2        0x40
#define AUX_AIRCON1_FAN3        0x20
#define AUX_AIRCON1_VDIR_MANUAL 0x00 // Air direction modes
#define AUX_AIRCON1_VDIR_SWING  0x07
#define AUX_AIRCON1_HDIR_MANUAL 0x00
#define AUX_AIRCON1_HDIR_SWING  0xE0


class AUXHeatpumpIR : public HeatpumpIR
{
  public:
    AUXHeatpumpIR();
    void send(IRSender& IR, uint8_t powerModeCmd, uint8_t operatingModeCmd, uint8_t fanSpeedCmd, uint8_t temperatureCmd, uint8_t swingVCmd, uint8_t swingHCmd);

  private:
    void sendAUX(IRSender& IR, uint8_t powerMode, uint8_t operatingMode, uint8_t fanSpeed, uint8_t temperature, uint8_t swingV, uint8_t swingH);
};

#endif
