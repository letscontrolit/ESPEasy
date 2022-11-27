#ifndef PLUGINSTRUCTS_P092_DATA_STRUCT_H
#define PLUGINSTRUCTS_P092_DATA_STRUCT_H

#include "../../_Plugin_Helper.h"

#ifdef USES_P092

#include <Arduino.h>

# ifdef LIMIT_BUILD_SIZE
#  define P092_LIMIT_BUILD_SIZE
# endif // ifdef LIMIT_BUILD_SIZE

/*********************************************************************************************\
   DLBus subs to get values from the receiving bitstream
\*********************************************************************************************/

// one data frame has <P092_DataSettings.DataBytes> data bytes + SYNC, e.g. 64 * (8+1+1) + 16 = 656
// 656 * 2 = 1312 (twice as much as a data frame is saved
// so there's one complete data frame

#define DLbus_MaxDataBytes 64
#define DLbus_AdditionalRecBytes 2
#define DLbus_StopBits 1
#define DLbus_StartBits 1
#define DLBus_SyncBits 16
#define DLBus_ReserveBytes 20
#define DLBus_BitChangeFactor 2
#define DLbus_MaxDataBits (((DLbus_MaxDataBytes + DLbus_AdditionalRecBytes) * (DLbus_StartBits + 8 + DLbus_StopBits) + DLBus_SyncBits) * \
                           DLBus_BitChangeFactor) + DLBus_ReserveBytes

// MaxDataBits is double of the maximum bit length because each bit change is stored
// (((64+2) * (8+1+1) + 16) * 2) + 50 = 1402 bytes

enum class eP092pinmode {
  ePPM_Input          = 1,
  ePPM_InputPullUp    = 2
};

class DLBus {
public:

  DLBus();                                // constructor of DLBus object
  ~DLBus();                               // destructor of DLBus object

  volatile uint8_t ISR_DLB_Pin = 0xFF;
  volatile boolean ISR_Receiving = false; // receiving flag
  volatile boolean ISR_AllBitsReceived = false;
  volatile uint16_t ISR_PulseCount = 0;   // number of received pulses
  volatile uint16_t ISR_PulseNumber = 0;  // max naumber of the received pulses
  volatile uint16_t ISR_MinPulseWidth = 0;
  volatile uint16_t ISR_MaxPulseWidth = 0;
  volatile uint16_t ISR_MinDoublePulseWidth = 0;
  volatile uint16_t ISR_MaxDoublePulseWidth = 0;

  // identification bytes for each DL bus device
  uint8_t DeviceBytes[2] = { 0 };
  uint8_t ByteStream[DLbus_MaxDataBits / 8 + 1] = { 0 }; // every bit gets sorted into a bitmap
  boolean IsLogLevelInfo = false;
  boolean IsNoData = false;           // no data received (DL bus not connected), stop receiving until next call to PLUGIN_READ
  boolean IsISRset = false;           // ISR set flag, used for setting the ISR after network connected
  uint8_t LogLevelInfo   = 0xff;
  uint8_t LogLevelError  = 0xFF;
  void    attachDLBusInterrupt(void);
  void    StartReceiving(void);
  boolean CheckTimings(void);
  boolean Processing(void);
  boolean CheckCRC(uint8_t IdxCRC);

private:

  volatile uint32_t ISR_TimeLastBitChange = 0;      // remember time of last transition
  uint8_t DLbus_ChangeBitStream[DLbus_MaxDataBits] = { 0 }; // received bit change stream (each bit change is extended to uint8_t, containing the
                                                    // timing flags)
  uint16_t BitNumber = 0;                           // bit number of the received DLbus_ChangeBitStream
  static void ISR(void);
  void        ISR_PinChanged(void);
  void        ProcessBit(uint8_t b);
  int         Analyze(void);
  void        Invert(void);
  uint8_t     ReadBit(int pos);
  void        WriteBit(int     pos,
                       uint8_t set);
  void        Trim(int start_bit);
  boolean     CheckDevice(void);
  static DLBus *__instance;
# ifndef P092_LIMIT_BUILD_SIZE
  void        AddToInfoLog(const String& string);
# endif // ifndef P092_LIMIT_BUILD_SIZE
  void        AddToErrorLog(const String& string);
};
#endif // ifndef DLBus_H

#ifdef USES_P092

// decoding the manchester code
// pulse width @ 488hz: 1000ms/488 = 2,048ms = 2048µs
// 2048µs / 2 = 1024µs (2 pulses for one bit)
// pulse width @ 50hz: 1000ms/50 = 20ms = 20000µs
// 20000µs / 2 = 10000µs (2 pulses for one bit)
# define P092_pulse_width_488        1024  // µs
# define P092_pulse_width_50         10000 // µs

// % tolerance for variances at the pulse width
# define P092_percentage_variance    10

// 1001 or 0110 are two sequential pulses without transition
# define P092_double_pulse_width_488 (P092_pulse_width_488 * 2)
# define P092_double_pulse_width_50  (P092_pulse_width_50 * 2)

// calculating the tolerance limits for variances
# define P092_min_width_488          (P092_pulse_width_488 - (P092_pulse_width_488 *  P092_percentage_variance / 100))
# define P092_max_width_488          (P092_pulse_width_488 + (P092_pulse_width_488 * P092_percentage_variance / 100))
# define P092_double_min_width_488   (P092_double_pulse_width_488 - (P092_pulse_width_488 * P092_percentage_variance / 100))
# define P092_double_max_width_488   (P092_double_pulse_width_488 + (P092_pulse_width_488 * P092_percentage_variance / 100))
# define P092_min_width_50           (P092_pulse_width_50 - (P092_pulse_width_50 *  P092_percentage_variance / 100))
# define P092_max_width_50           (P092_pulse_width_50 + (P092_pulse_width_50 * P092_percentage_variance / 100))
# define P092_double_min_width_50    (P092_double_pulse_width_50 - (P092_pulse_width_50 * P092_percentage_variance / 100))
# define P092_double_max_width_50    (P092_double_pulse_width_50 + (P092_pulse_width_50 * P092_percentage_variance / 100))

# define P092_DLbus_OptionCount 8
# define P092_DLbus_DeviceCount 6


struct P092_data_struct : public PluginTaskData_base {
public:

  P092_data_struct() = default;
  virtual ~P092_data_struct();

  bool init(int8_t pin1, int P092DeviceIndex, eP092pinmode P092pinmode);

  typedef struct {
    uint8_t Idx = 0u;
    uint8_t mode = 0u;
    float   value = 0.0f;
  } sP092_ReadData;

  void    Plugin_092_SetIndices(int P092DeviceIndex);

  void    Plugin_092_StartReceiving(taskIndex_t taskindex);

  boolean P092_GetData(int             OptionIdx,
                       int             CurIdx,
                       sP092_ReadData *ReadData);


  boolean P092_fetch_sensor(int             number,
                            sP092_ReadData *ReadData);
  boolean P092_fetch_output(int             number,
                            sP092_ReadData *ReadData);    // digital output byte(s)
  boolean P092_fetch_speed(int             number,
                           sP092_ReadData *ReadData);     // speed byte(s)
  boolean P092_fetch_analog(int             number,
                            sP092_ReadData *ReadData);    // analog byte(s)
  boolean P092_fetch_heatpower(int             number,
                               sP092_ReadData *ReadData); // heat power(s)
  boolean P092_fetch_heatmeter(int             number,
                               sP092_ReadData *ReadData); // heat meters(s)


//  uint8_t  P092_Last_DLB_Pin;
  boolean  P092_ReceivedOK   = false;
  uint32_t P092_LastReceived = 0;
  struct _P092_DataStruct
  {
    uint8_t DataBytes = 0u;
    uint8_t DeviceByte0 = 0u;
    uint8_t DeviceByte1 = 0u;
    uint8_t DeviceBytes = 0u;
    uint8_t DontCareBytes = 0u;
    uint8_t TimeStampBytes = 0u;
    uint8_t MaxSensors = 0u;
    uint8_t MaxExtSensors = 0u;
    uint8_t OutputBytes = 0u;
    uint8_t SpeedBytes = 0u;
    uint8_t MaxAnalogOuts = 0u;
    uint8_t AnalogBytes = 0u;
    uint8_t VolumeBytes = 0u;
    uint8_t MaxHeatMeters = 0u;
    uint8_t CurrentHmBytes = 0u;
    uint8_t MWhBytes = 0u;

    uint16_t DLbus_MinPulseWidth = 0u;
    uint16_t DLbus_MaxPulseWidth = 0u;
    uint16_t DLbus_MinDoublePulseWidth = 0u;
    uint16_t DLbus_MaxDoublePulseWidth = 0u;
    uint8_t  IdxSensor = 0u;
    uint8_t  IdxExtSensor = 0u;
    uint8_t  IdxOutput = 0u;
    uint8_t  IdxDrehzahl = 0u;
    uint8_t  IdxAnalog = 0u;
    uint8_t  IdxHmRegister = 0u;
    uint8_t  IdxVolume = 0u;
    uint8_t  IdxHeatMeter1 = 0u;
    uint8_t  IdxkWh1 = 0u;
    uint8_t  IdxMWh1 = 0u;
    uint8_t  IdxHeatMeter2 = 0u;
    uint8_t  IdxkWh2 = 0u;
    uint8_t  IdxMWh2 = 0u;
    uint8_t  IdxHeatMeter3 = 0u;
    uint8_t  IdxkWh3 = 0u;
    uint8_t  IdxMWh3 = 0u;
    uint8_t  IdxCRC = 0u;
  } P092_DataSettings;

  // heat meter
  typedef struct {
    uint8_t IndexIsValid = 0u;
    int32_t power_index = 0;
    int32_t kwh_index = 0;
    int32_t mwh_index = 0;
  } sDLbus_HMindex;

  sDLbus_HMindex P092_CheckHmRegister(int number);

  DLBus *DLbus_Data = nullptr;
};


#endif // ifdef USES_P092
#endif // ifndef PLUGINSTRUCTS_P092_DATA_STRUCT_H