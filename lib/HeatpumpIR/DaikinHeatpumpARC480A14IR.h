/*
    Daikin heatpump control (Daikin remote control P/N ARC480A14)
*/
#ifndef DaikinHeatpumpARC480A14IR_h
#define DaikinHeatpumpARC480A14IR_h

#include <HeatpumpIR.h>


// Daikin timing constants
#define DAIKIN_AIRCON_HDR_MARK   3360  // 3300
#define DAIKIN_AIRCON_HDR_SPACE  1760  // 1600
#define DAIKIN_AIRCON_BIT_MARK   360   // 310
#define DAIKIN_AIRCON_ONE_SPACE  1370  // 1220
#define DAIKIN_AIRCON_ZERO_SPACE 520   // 400
#define DAIKIN_AIRCON_MSG_SPACE  32300 // 30800

// Daikin codes
#define DAIKIN_AIRCON_MODE_AUTO      0x00 // Operating mode
#define DAIKIN_AIRCON_MODE_HEAT      0x40
#define DAIKIN_AIRCON_MODE_COOL      0x30
#define DAIKIN_AIRCON_MODE_DRY       0x20
#define DAIKIN_AIRCON_MODE_FAN       0x60
#define DAIKIN_AIRCON_MODE_OFF       0x00 // Power OFF
#define DAIKIN_AIRCON_MODE_ON        0x01
#define DAIKIN_AIRCON_FAN_AUTO       0xA0 // Fan speed
#define DAIKIN_AIRCON_FAN1           0x30
#define DAIKIN_AIRCON_FAN2           0x40
#define DAIKIN_AIRCON_FAN3           0x50
#define DAIKIN_AIRCON_FAN4           0x60
#define DAIKIN_AIRCON_FAN5           0x70
#define DAIKIN_AIRCON_FAN_SILENT     0xB0
#define DAIKIN_AIRCON_SWING_ON       0x0F //Swing
#define DAIKIN_AIRCON_SWING_OFF      0x00
#define DAIKIN_AIRCON_COMFORT_ON     0x02 //Comfort mode
#define DAIKIN_AIRCON_COMFORT_OFF    0x00
#define DAIKIN_AIRCON_ECONO_ON       0x04 //Econo mode
#define DAIKIN_AIRCON_ECONO_OFF      0x00
#define DAIKIN_AIRCON_SENSOR_ON      0x08 //Sensor mode
#define DAIKIN_AIRCON_SENSOR_OFF     0x00
#define DAIKIN_AIRCON_QUIET_ON       0x20 //Quite mode
#define DAIKIN_AIRCON_QUIET_OFF      0x00
#define DAIKIN_AIRCON_POWERFUL_ON    0x01 //Powerful mode
#define DAIKIN_AIRCON_POWERFUL_OFF   0x00


class DaikinHeatpumpARC480A14IR : public HeatpumpIR
{
  public:
    DaikinHeatpumpARC480A14IR();
    void send(IRSender& IR, uint8_t powerModeCmd, uint8_t operatingModeCmd, uint8_t fanSpeedCmd, uint8_t temperatureCmd, uint8_t swingVCmd, uint8_t swingHCmd);
    void send(IRSender& IR, uint8_t powerModeCmd, uint8_t operatingModeCmd, uint8_t fanSpeedCmd, uint8_t temperatureCmd, uint8_t swingVCmd, uint8_t swingHCmd, uint8_t comfortMode, uint8_t econo, uint8_t sensor, uint8_t quiet, uint8_t powerful);
  private:
    void sendDaikin(IRSender& IR, uint8_t operatingMode, uint8_t fanSpeed, uint8_t temperature, uint8_t swingV, uint8_t swingH, uint8_t comfortMode, uint8_t econo, uint8_t sensor, uint8_t quiet, uint8_t powerful);
};

#endif
