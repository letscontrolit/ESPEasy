/*
    Mitsubishi Heavy FDTCxxVF heatpump control (remote control P/N PJA502A704AA)
*/
#ifndef MitsubishiHeavyFDTCHeatpumpIR_h
#define MitsubishiHeavyFDTCHeatpumpIR_h

#include <HeatpumpIR.h>

// Mitsubishi Heavy timing constants
#define MITSUBISHI_HEAVY_FDTC_HDR_MARK          6000 
#define MITSUBISHI_HEAVY_FDTC_HDR_SPACE         7500 
#define MITSUBISHI_HEAVY_FDTC_BIT_MARK          500  
#define MITSUBISHI_HEAVY_FDTC_ONE_SPACE         3500 
#define MITSUBISHI_HEAVY_FDTC_ZERO_SPACE        1500

// Mitsubishi Heavy codes
#define MITSUBISHI_HEAVY_FDTC_MODE_AUTO         0x00 // Operating mode
#define MITSUBISHI_HEAVY_FDTC_MODE_HEAT         0x40
#define MITSUBISHI_HEAVY_FDTC_MODE_COOL         0x20
#define MITSUBISHI_HEAVY_FDTC_MODE_DRY          0x10
#define MITSUBISHI_HEAVY_FDTC_MODE_FAN          0x30

#define MITSUBISHI_HEAVY_FDTC_MODE_OFF          0x00 // Power OFF
#define MITSUBISHI_HEAVY_FDTC_MODE_ON           0x80 // Power ON

#define MITSUBISHI_HEAVY_FDTC_FAN1              0x00
#define MITSUBISHI_HEAVY_FDTC_FAN2              0x10
#define MITSUBISHI_HEAVY_FDTC_FAN3              0x20

#define MITSUBISHI_HEAVY_FDTC_VS_SWING          0x40 // Vertical swing
#define MITSUBISHI_HEAVY_FDTC_VS_UP             0x00
#define MITSUBISHI_HEAVY_FDTC_VS_MUP            0x10
#define MITSUBISHI_HEAVY_FDTC_VS_MDOWN          0x20
#define MITSUBISHI_HEAVY_FDTC_VS_DOWN           0x30

class MitsubishiHeavyFDTCHeatpumpIR : public HeatpumpIR
{
  public:
    MitsubishiHeavyFDTCHeatpumpIR();
    void send(IRSender& IR, uint8_t powerModeCmd, uint8_t operatingModeCmd, uint8_t fanSpeedCmd, uint8_t temperatureCmd, uint8_t swingVCmd, uint8_t swingHCmd);

  private:
    void sendMitsubishiHeavyFDTC(IRSender& IR, uint8_t powerMode, uint8_t operatingMode, uint8_t fanSpeed, uint8_t temperature, uint8_t swingV);
};

#endif
