/*
    Gree heatpump control (remote control P/N zzz)
*/
#ifndef GreeHeatpumpIR_h
#define GreeHeatpumpIR_h

#include <HeatpumpIR.h>

// Power state
#define GREE_AIRCON1_POWER_OFF  0x00
#define GREE_AIRCON1_POWER_ON   0x08

// Operating modes
// Gree codes
#define GREE_AIRCON1_MODE_AUTO  0x00
#define GREE_AIRCON1_MODE_COOL  0x01
#define GREE_AIRCON1_MODE_DRY   0x02
#define GREE_AIRCON1_MODE_FAN   0x03
#define GREE_AIRCON1_MODE_HEAT  0x04

// Fan speeds. Note that some heatpumps have less than 5 fan speeds
#define GREE_AIRCON1_FAN_AUTO   0x00 // Fan speed
#define GREE_AIRCON1_FAN1       0x10 // * low
#define GREE_AIRCON1_FAN2       0x20 // * med
#define GREE_AIRCON1_FAN3       0x30 // * high
#define GREE_AIRCON1_TURBO      0x80 // * turbo mode on YAN

// Only available on YAN and YT
// Vertical air directions. Note that these cannot be set on all heat pumps
#define GREE_VDIR_AUTO   0x00
#define GREE_VDIR_MANUAL 0x00
#define GREE_VDIR_SWING  0x01
#define GREE_VDIR_UP     0x02
#define GREE_VDIR_MUP    0x03
#define GREE_VDIR_MIDDLE 0x04
#define GREE_VDIR_MDOWN  0x05
#define GREE_VDIR_DOWN   0x06

// Only available on YAC
// Horizontal air directions. Note that these cannot be set on all heat pumps
#define GREE_HDIR_AUTO   0x00
#define GREE_HDIR_MANUAL 0x00
#define GREE_HDIR_SWING  0x01
#define GREE_HDIR_LEFT   0x02
#define GREE_HDIR_MLEFT  0x03
#define GREE_HDIR_MIDDLE 0x04
#define GREE_HDIR_MRIGHT 0x05
#define GREE_HDIR_RIGHT  0x06

#define GREE_IFEEL_BIT 0x08

// Only available on YAA, YAC, and YT
// byte 0
#define GREE_VSWING     (1 << 6)
// byte 2
#define GREE_TURBO_BIT  (1 << 4)
#define GREE_LIGHT_BIT  (1 << 5)
#define GREE_HEALTH_BIT (1 << 6)
#define GREE_XFAN_BIT   (1 << 7) // aka BLOW on some remotes

// Gree model codes
#define GREE_GENERIC 0
#define GREE_YAN     1
#define GREE_YAA     2
#define GREE_YAC     3
#define GREE_YT      4


class GreeHeatpumpIR : public HeatpumpIR
{
  protected:
    struct Timings {
        int hdr_mark;
        int hdr_space;
        int bit_mark;
        int one_space;
        int zero_space;
        int msg_space;
        int ifeel_hdr_mark;
        int ifeel_hdr_space;
        int ifeel_bit_mark;
    };

    GreeHeatpumpIR();

    virtual const Timings & getTimings() const;

    virtual void generateCommand(uint8_t * buffer,
            uint8_t powerMode, uint8_t operatingMode,
            uint8_t fanSpeed, uint8_t temperature,
            uint8_t swingV, uint8_t swingH,
            bool turboMode, bool iFeelMode);

    virtual void calculateChecksum(uint8_t * buffer);

    virtual void sendGree(
            IRSender& IR,
            uint8_t powerMode, uint8_t operatingMode,
            uint8_t fanSpeed, uint8_t temperature,
            uint8_t swingV, uint8_t swingH,
            bool turboMode, bool iFeelMode);

    virtual void sendBuffer(
            IRSender& IR,
            const uint8_t * buffer,
            size_t len = 8);

  public:
    void send(
            IRSender& IR,
            uint8_t powerModeCmd, uint8_t operatingModeCmd,
            uint8_t fanSpeedCmd, uint8_t temperatureCmd,
            uint8_t swingVCmd, uint8_t swingHCmd) override {
        send(
            IR,
            powerModeCmd, operatingModeCmd,
            fanSpeedCmd, temperatureCmd,
            swingVCmd, swingHCmd, false);
    }

    virtual void send(
            IRSender& IR,
            uint8_t powerModeCmd, uint8_t operatingModeCmd,
            uint8_t fanSpeedCmd, uint8_t temperatureCmd,
            uint8_t swingVCmd, uint8_t swingHCmd,
            bool turboMode, bool iFeelMode = false);
};

class GreeGenericHeatpumpIR : public GreeHeatpumpIR
{
  public:
    GreeGenericHeatpumpIR();
};

class GreeYANHeatpumpIR : public GreeHeatpumpIR
{
  public:
    GreeYANHeatpumpIR();

  protected:
    virtual void generateCommand(uint8_t * buffer,
            uint8_t powerMode, uint8_t operatingMode,
            uint8_t fanSpeed, uint8_t temperature,
            uint8_t swingV, uint8_t swingH,
            bool turboMode, bool iFeelMode) override;

    virtual void calculateChecksum(uint8_t * buffer) override;
};

class GreeYAAHeatpumpIR : public GreeHeatpumpIR
{
  public:
    GreeYAAHeatpumpIR();

  protected:
    virtual void generateCommand(uint8_t * buffer,
            uint8_t powerMode, uint8_t operatingMode,
            uint8_t fanSpeed, uint8_t temperature,
            uint8_t swingV, uint8_t swingH,
            bool turboMode, bool iFeelMode) override;
};

class GreeiFeelHeatpumpIR : public GreeHeatpumpIR
{
  public:
    using GreeHeatpumpIR::send;
    void send(IRSender& IR, uint8_t currentTemperature) override;

  protected:
    virtual void generateCommand(uint8_t * buffer,
            uint8_t powerMode, uint8_t operatingMode,
            uint8_t fanSpeed, uint8_t temperature,
            uint8_t swingV, uint8_t swingH,
            bool turboMode, bool iFeelMode) override;
};

class GreeYACHeatpumpIR : public GreeiFeelHeatpumpIR
{
  public:
    GreeYACHeatpumpIR();

  protected:
    virtual void generateCommand(uint8_t * buffer,
            uint8_t powerMode, uint8_t operatingMode,
            uint8_t fanSpeed, uint8_t temperature,
            uint8_t swingV, uint8_t swingH,
            bool turboMode, bool iFeelMode) override;
};

class GreeYTHeatpumpIR : public GreeiFeelHeatpumpIR
{
  public:
    GreeYTHeatpumpIR();

  protected:
    virtual void generateCommand(uint8_t * buffer,
            uint8_t powerMode, uint8_t operatingMode,
            uint8_t fanSpeed, uint8_t temperature,
            uint8_t swingV, uint8_t swingH,
            bool turboMode, bool iFeelMode) override;
};

class GreeYAPHeatpumpIR : public GreeiFeelHeatpumpIR
{
  public:
    GreeYAPHeatpumpIR();

    using GreeiFeelHeatpumpIR::send;

    virtual void send(
            IRSender& IR,
            uint8_t powerModeCmd, uint8_t operatingModeCmd,
            uint8_t fanSpeedCmd, uint8_t temperatureCmd,
            uint8_t swingVCmd, uint8_t swingHCmd,
            bool turboMode, bool iFeelMode = false) override {
        send(
            IR,
            powerModeCmd, operatingModeCmd,
            fanSpeedCmd, temperatureCmd,
            swingVCmd, swingHCmd,
            turboMode, iFeelMode,
            true);
    }

    virtual void send(
            IRSender& IR,
            uint8_t powerModeCmd, uint8_t operatingModeCmd,
            uint8_t fanSpeedCmd, uint8_t temperatureCmd,
            uint8_t swingVCmd, uint8_t swingHCmd,
            bool turboMode, bool iFeelMode,
            bool light, bool xfan = false,
            bool health = false, bool valve = false,
            bool sthtMode = false, bool enableWiFi = true);

  protected:
    virtual const Timings & getTimings() const override;

    virtual void generateCommand(uint8_t * buffer,
            uint8_t powerMode, uint8_t operatingMode,
            uint8_t fanSpeed, uint8_t temperature,
            uint8_t swingV, uint8_t swingH,
            bool turboMode, bool iFeelMode) override;

    void generateCommand(uint8_t * buffer,
            uint8_t powerMode, uint8_t operatingMode,
            uint8_t fanSpeed, uint8_t temperature,
            uint8_t swingV, uint8_t swingH,
            bool turboMode, bool iFeelMode,

            bool light, bool xfan = false,
            bool health = false, bool valve = false,
            bool sthtMode = false, bool enableWiFi = true
        );

    virtual void sendGree(
            IRSender& IR,
            uint8_t powerMode, uint8_t operatingMode,
            uint8_t fanSpeed, uint8_t temperature,
            uint8_t swingV, uint8_t swingH,
            bool turboMode, bool iFeelMode) override;

    void sendGree(
            IRSender& IR,
            uint8_t powerMode, uint8_t operatingMode,
            uint8_t fanSpeed, uint8_t temperature,
            uint8_t swingV, uint8_t swingH,
            bool turboMode, bool iFeelMode,

            bool light, bool xfan = false,
            bool health = false, bool valve = false,
            bool sthtMode = false, bool enableWiFi = true);
};

#endif
