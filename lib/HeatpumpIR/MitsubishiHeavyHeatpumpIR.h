/*
    Mitsubishi Heavy SRKxxZJ-S heatpump control (remote control P/N RKX502A001C)
*/
#ifndef MitsubishiHeavyHeatpumpIR_h
#define MitsubishiHeavyHeatpumpIR_h

#include <HeatpumpIR.h>


// Mitsubishi Heavy timing constants
#define MITSUBISHI_HEAVY_HDR_MARK          3200 // 3200
#define MITSUBISHI_HEAVY_HDR_SPACE         1600 // 1600
#define MITSUBISHI_HEAVY_BIT_MARK          400  // 400
#define MITSUBISHI_HEAVY_ONE_SPACE         1200 // 1200
#define MITSUBISHI_HEAVY_ZERO_SPACE        400  // 400


// Mitsubishi Heavy codes
#define MITSUBISHI_HEAVY_MODE_AUTO         0x07 // Operating mode
#define MITSUBISHI_HEAVY_MODE_HEAT         0x03
#define MITSUBISHI_HEAVY_MODE_COOL         0x06
#define MITSUBISHI_HEAVY_MODE_DRY          0x05
#define MITSUBISHI_HEAVY_MODE_FAN          0x04
#define MITSUBISHI_HEAVY_ZMP_MODE_FAN      0xD4 //ZMP model seems to have a different value for operating mode 'fan'
#define MITSUBISHI_HEAVY_ZMP_MODE_MAINT    0x06 //We use the maintenance mode to activate cleaning.

#define MITSUBISHI_HEAVY_MODE_OFF          0x08 // Power OFF
#define MITSUBISHI_HEAVY_MODE_ON           0x00 // Power ON

#define MITSUBISHI_HEAVY_ZJ_FAN_AUTO       0xE0 // Fan speed
#define MITSUBISHI_HEAVY_ZJ_FAN1           0xA0
#define MITSUBISHI_HEAVY_ZJ_FAN2           0x80
#define MITSUBISHI_HEAVY_ZJ_FAN3           0x60
#define MITSUBISHI_HEAVY_ZJ_HIPOWER        0x40 
#define MITSUBISHI_HEAVY_ZJ_ECONO          0x00

#define MITSUBISHI_HEAVY_ZM_FAN_AUTO       0x0F // Fan speed
#define MITSUBISHI_HEAVY_ZM_FAN1           0x0E
#define MITSUBISHI_HEAVY_ZM_FAN2           0x0D
#define MITSUBISHI_HEAVY_ZM_FAN3           0x0C
#define MITSUBISHI_HEAVY_ZM_FAN4           0x0B
#define MITSUBISHI_HEAVY_ZM_HIPOWER        0x07
#define MITSUBISHI_HEAVY_ZM_ECONO          0x09

#define MITSUBISHI_HEAVY_ZMP_FAN_AUTO       0xE0 // Fan speed
#define MITSUBISHI_HEAVY_ZMP_FAN1           0xA0
#define MITSUBISHI_HEAVY_ZMP_FAN2           0x80
#define MITSUBISHI_HEAVY_ZMP_FAN3           0x60
#define MITSUBISHI_HEAVY_ZMP_HIPOWER        0x20 
#define MITSUBISHI_HEAVY_ZMP_ECONO          0x00

#define MITSUBISHI_HEAVY_CLEAN_ON          0x00
#define MITSUBISHI_HEAVY_ZMP_CLEAN_ON      0xDF
#define MITSUBISHI_HEAVY_ZJ_CLEAN_OFF      0x20
#define MITSUBISHI_HEAVY_ZM_CLEAN_OFF      0x60
#define MITSUBISHI_HEAVY_ZMP_CLEAN_OFF     0x20

#define MITSUBISHI_HEAVY_ZM_3DAUTO_ON      0x00 // Only available in Auto, Cool and Heat mode
#define MITSUBISHI_HEAVY_ZM_3DAUTO_OFF     0x12

#define MITSUBISHI_HEAVY_ZJ_SILENT_ON      0x00
#define MITSUBISHI_HEAVY_ZM_SILENT_ON      0x00 // NOT available in Fan or Dry mode
#define MITSUBISHI_HEAVY_ZM_SILENT_OFF     0x80
#define MITSUBISHI_HEAVY_ZMP_SILENT_ON     0x00

#define MITSUBISHI_HEAVY_ZJ_VS_SWING       0x0A // Vertical swing
#define MITSUBISHI_HEAVY_ZJ_VS_UP          0x02
#define MITSUBISHI_HEAVY_ZJ_VS_MUP         0x18
#define MITSUBISHI_HEAVY_ZJ_VS_MIDDLE      0x10
#define MITSUBISHI_HEAVY_ZJ_VS_MDOWN       0x08
#define MITSUBISHI_HEAVY_ZJ_VS_DOWN        0x00
#define MITSUBISHI_HEAVY_ZJ_VS_STOP        0x1A

#define MITSUBISHI_HEAVY_ZM_VS_SWING       0xE0 // Vertical swing
#define MITSUBISHI_HEAVY_ZM_VS_UP          0xC0
#define MITSUBISHI_HEAVY_ZM_VS_MUP         0xA0
#define MITSUBISHI_HEAVY_ZM_VS_MIDDLE      0x80
#define MITSUBISHI_HEAVY_ZM_VS_MDOWN       0x60
#define MITSUBISHI_HEAVY_ZM_VS_DOWN        0x40
#define MITSUBISHI_HEAVY_ZM_VS_STOP        0x20

#define MITSUBISHI_HEAVY_ZMP_VS_SWING       0x0A // Vertical swing
#define MITSUBISHI_HEAVY_ZMP_VS_UP          0x02
#define MITSUBISHI_HEAVY_ZMP_VS_MUP         0x18
#define MITSUBISHI_HEAVY_ZMP_VS_MIDDLE      0x10
#define MITSUBISHI_HEAVY_ZMP_VS_MDOWN       0x08
#define MITSUBISHI_HEAVY_ZMP_VS_DOWN        0x00
#define MITSUBISHI_HEAVY_ZMP_VS_STOP        0x1A

#define MITSUBISHI_HEAVY_ZJ_HS_SWING       0x4C // Horizontal swing - 3D AUTO
#define MITSUBISHI_HEAVY_ZJ_HS_MIDDLE      0x48
#define MITSUBISHI_HEAVY_ZJ_HS_LEFT        0xC8
#define MITSUBISHI_HEAVY_ZJ_HS_MLEFT       0x88
#define MITSUBISHI_HEAVY_ZJ_HS_MRIGHT      0x08
#define MITSUBISHI_HEAVY_ZJ_HS_RIGHT       0xC4
#define MITSUBISHI_HEAVY_ZJ_HS_STOP        0xCC
#define MITSUBISHI_HEAVY_ZJ_HS_LEFTRIGHT   0x84
#define MITSUBISHI_HEAVY_ZJ_HS_RIGHTLEFT   0x44
#define MITSUBISHI_HEAVY_ZJ_HS_3DAUTO      0x04

#define MITSUBISHI_HEAVY_ZM_HS_SWING       0x0F // Horizontal swing
#define MITSUBISHI_HEAVY_ZM_HS_MIDDLE      0x0C
#define MITSUBISHI_HEAVY_ZM_HS_LEFT        0x0E
#define MITSUBISHI_HEAVY_ZM_HS_MLEFT       0x0D
#define MITSUBISHI_HEAVY_ZM_HS_MRIGHT      0x0B
#define MITSUBISHI_HEAVY_ZM_HS_RIGHT       0x0A
#define MITSUBISHI_HEAVY_ZM_HS_STOP        0x07
#define MITSUBISHI_HEAVY_ZM_HS_LEFTRIGHT   0x08
#define MITSUBISHI_HEAVY_ZM_HS_RIGHTLEFT   0x09

//ZMP model does not support horizontal swing
#define MITSUBISHI_HEAVY_ZMP_HS_STOP       0xCC

// MitsubishiHeavy model codes
#define MITSUBISHIHEAVY_ZJ 0
#define MITSUBISHIHEAVY_ZM 1
#define MITSUBISHIHEAVY_ZMP 2


class MitsubishiHeavyHeatpumpIR : public HeatpumpIR
{
  protected: // Cannot create generic MitsubishiHeavy heatpump instances
    MitsubishiHeavyHeatpumpIR();
    uint8_t _mitsubishiModel;  // Tells whether this is ZJ or ZM (or other supported model...)

  public:
    void send(IRSender& IR, uint8_t powerModeCmd, uint8_t operatingModeCmd, uint8_t fanSpeedCmd, uint8_t temperatureCmd, uint8_t swingVCmd, uint8_t swingHCmd);
    virtual void send(IRSender& IR, uint8_t powerModeCmd, uint8_t operatingModeCmd, uint8_t fanSpeedCmd, uint8_t temperatureCmd, uint8_t swingVCmd, uint8_t swingHCmd, bool cleanModeCmd, bool silentModeCmd, bool _3DAutoCmd);
};

class MitsubishiHeavyZJHeatpumpIR : public MitsubishiHeavyHeatpumpIR
{
  public:
    MitsubishiHeavyZJHeatpumpIR();
    void send(IRSender& IR, uint8_t powerModeCmd, uint8_t operatingModeCmd, uint8_t fanSpeedCmd, uint8_t temperatureCmd, uint8_t swingVCmd, uint8_t swingHCmd, bool cleanModeCmd, bool silentModeCmd, bool _3DAutoCmd);

  private:
    void sendMitsubishiHeavy(IRSender& IR, uint8_t powerMode, uint8_t operatingMode, uint8_t fanSpeed, uint8_t temperature, uint8_t swingV, uint8_t swingH, uint8_t cleanMode);
};

class MitsubishiHeavyZMHeatpumpIR : public MitsubishiHeavyHeatpumpIR
{
  public:
    MitsubishiHeavyZMHeatpumpIR();
    void send(IRSender& IR, uint8_t powerModeCmd, uint8_t operatingModeCmd, uint8_t fanSpeedCmd, uint8_t temperatureCmd, uint8_t swingVCmd, uint8_t swingHCmd, bool cleanModeCmd, bool silentModeCmd, bool _3DAutoCmd);

  private:
    void sendMitsubishiHeavy(IRSender& IR, uint8_t powerMode, uint8_t operatingMode, uint8_t fanSpeed, uint8_t temperature, uint8_t swingV, uint8_t swingH, uint8_t cleanMode, uint8_t silentMode, uint8_t _3DAuto);
};

class MitsubishiHeavyZMPHeatpumpIR : public MitsubishiHeavyHeatpumpIR
{
  public:
    MitsubishiHeavyZMPHeatpumpIR();
    void send(IRSender& IR, uint8_t powerModeCmd, uint8_t operatingModeCmd, uint8_t fanSpeedCmd, uint8_t temperatureCmd, uint8_t swingVCmd, uint8_t swingHCmd, bool cleanModeCmd, bool silentModeCmd, bool _3DAutoCmd);

  private:
    void sendMitsubishiHeavy(IRSender& IR, uint8_t powerMode, uint8_t operatingMode, uint8_t fanSpeed, uint8_t temperature, uint8_t swingV, uint8_t swingH, uint8_t cleanMode);
};

#endif
