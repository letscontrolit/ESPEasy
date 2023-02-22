// ----------------------------------------------------------------------------
// P145 "Gases - MQxxx (MQ135 CO2, MQ3 Alcohol)"
// Definition of sensor abstraction
// See _P145_MQxxx.ino
// 2023 By flashmark
// ----------------------------------------------------------------------------
#ifndef PLUGINSTRUCTS_P145_DATA_STRUCT_H
#define PLUGINSTRUCTS_P145_DATA_STRUCT_H


#include "../../_Plugin_Helper.h"
#ifdef USES_P145

// Maximum number of types that can be selected from
#define P145_MAXTYPES  10

// Analog input pin for sensor value
#define P145_SENSOR_PIN A0

// Supply voltage applied to the sensor
#define P145_VCC        (5.0)
#define P145_VCCLOW     (3.3)
// Max input scale voltage of the ADC
#define P145_VMAX       (3.3)

// Calibration interval in milli seconds (once per day)
#define P145_CALIBRATION_INTERVAL (24*60*60*1000)

// Selection of the conversion algorithm Rs/R0 => value
enum P145_algorithm 
{
  p145AlgNone,  // Undefined/free algorithm
  p145AlgA,     // MQ-135 with optional temp/hum correction 
  p145AlgB,     // miguel5612 Exponential
  p145AlgC      // miguel5612 Linear
};

// Structure with sensor specific conversion data
// A table of sensors is stored in PROGMEM, see P145_data_struct.cpp
// Members are preferrably sorted by memory alignment
struct P145_SENSORDEF
{
  float cleanRatio;    // Rs/R0 ratio in clean air
  float para;          // PARA scaling factor value
  float parb;          // PARB exponent value
  float cora;          // CORA
  float corb;          // CORB
  float corc;          // CORC
  float cord;          // CORD
  float core;          // CORE
  float corf;          // CORF
  float corg;          // CORG
  P145_algorithm alg;  // Preferred/tuned algorithm
  int   name;      // Sensor type name
  int   gas;       // Measured gas 
};

// State for heater control algorithm
enum P145_heaterState
{
  P145HeaterDisabled,   // Heater control is disabled (default)
  P145HeaterWarmup,     // Warming up after switching on
  P145HeaterHighVolt,   // Apply high voltage to heater
  P145HeaterLowVolt,    // Apply low volgate to heater
  P145HeaterMeasure,    // Measurement phase
};

struct P145_data_struct : public PluginTaskData_base 
{
    private:
    struct P145_SENSORDEF sensordef = {}; // Sensor type specific data

    /* Oversampling static data */
    long   ovs_value = 0;   // Oversampling algorithm summed value
    uint   ovs_min = 0;     // Oversampling algorithm minimum value
    uint   ovs_max = 0;     // Oversampling algorithm maximum value
    uint   ovs_cnt = 0;     // Oversampling algorithm sample counter
    float  last_ain = 0.0;  // Oversampling algorithm last measured analog input value
    /* Calibration static data */
    ulong  last_cal = 0U;   // Last calibration timestamp
    float  cal_data = 0.0f; // Building calibration resistance [Ohm]
    float  rcal_act = 0.0f; // Rzero estimation for actual measurement assuming refLevel
    /* Sensor value conversion parameters */
    float rload = 0.0f;     // Rload, load resistor [Ohm]
    float rzero = 0.0f;     // R0, reference resistance [Ohm]
    float refLevel = 0.0f;  // Reference level for calibration [ppm]
    /* Sensor type & user options */
    bool compensation = false;            // Use temperature compensation
    bool calibration = false;             // Perform auto calibration
    bool lowvcc = false;                  // Sensor power is low voltage 3v3
    int  sensorType = -1;                 // Selected sensor type
    P145_algorithm algorithm = p145AlgNone; // conversion algorithm
    int  analogPin = P145_SENSOR_PIN;     // Analog input pin connected to the sensor
    /* Heater control */
    P145_heaterState heaterState = P145HeaterDisabled;  // Statemachine 
    int heaterPin = -1;                   // Digital output pin for heater control
    int lastHeaterPin = -1;               // Last actuated output pin for heater control
    ulong heaterChangeTime = 0U;          // Last time the heater control changed state
    float latchedAnalogInput = 0.0f;      // Analog input measured while heater control enabled

    public:
    float readValue(float temperature, float humidity);
    bool plugin_init();
    bool plugin_ten_per_second();
    void setSensorData(int stype, bool comp, bool cal, bool vcclow, float load, float zero, float ref);
    void setSensorPins(int analogPin, int heaterPin);
    float getCalibrationValue() const;
    float getAutoCalibrationValue() const;
    static const String getTypeName( int stype);
    static const String getGasName( int stype);
    static int getNbrOfTypes();
    void dump() const;
    void heaterControl(void);

    private:
    void  calibrate (float Rcal);
    float getAnalogValue();
    float getResistance(float val) const;
    float getRZero(float rSensor) const;
    float getTempHumCorrection(float temperature, float humidity) const;
    float getCorrectedRZero(float rSensor, float temperature, float humidity) const;
    float getPPM(float rSensor);
    float getCorrectedPPM(float rSensor, float temperature, float humidity);
    void  resetOversampling();
};

#endif  // PLUGINSTRUCTS_P145_DATA_STRUCT_H
#endif  // USES_P145