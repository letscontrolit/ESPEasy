#ifndef PLUGINSTRUCTS_P145_DATA_STRUCT_H
#define PLUGINSTRUCTS_P145_DATA_STRUCT_H


#include "../../_Plugin_Helper.h"
#ifdef USES_P145


// Select between various sensor types for conversion
// Used as index in arrays. Start with 0 and keep consecutive
// For each sensor a matching entry in sensorData must exist
#define SENSOR_USER     0
#define SENSOR_MQ135    1
#define SENSOR_MQ3      2
#define SENSOR_MAX      3

// Analog input pin for sensor value
#define SENSOR_PIN      A0

// Supply voltage applied to the sensor
#define VCC             (5.0)
// Max input scale voltage of the ADC
#define VMAX            (3.3)
// Max scale value of the ADC [MAXSCALE=>VMAX]
#define MAXSCALE        (1023)

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
struct SENSORDEF
{
  char name[8];       // Sensor type name
  P145_algorithm alg; // Preferred/tuned algorithm
  float cleanRatio;   // Rs/R0 ratio in clean air
  float para;         // PARA scaling factor value
  float parb;         // PARB exponent value
  float cora;         // CORA
  float corb;         // CORB
  float corc;         // CORC
  float cord;         // CORD
  float core;         // CORE
  float corf;         // CORF
  float corg;         // CORG
};

struct P145_data_struct : public PluginTaskData_base 
{
    private:
    struct SENSORDEF sensordef = {};       // Sensor type specific data

    /* Oversampling static data */
    long   ovs_value = 0;   // Oversampling algorithm summed value
    uint   ovs_min = 0;     // Oversampling algorithm minimum value
    uint   ovs_max = 0;     // Oversampling algorithm maximum value
    uint   ovs_cnt = 0;     // Oversampling algorithm sample counter
    /* Calibration static data */
    float  last_ain = 0.0;  // Oversampling algorithm last measured analog input value
    ulong  last_cal = 0;    // Last calibration timestamp
    /* Sensor value conversion parameters */
    float rload = 0.0;      // Rload, load resistor [Ohm]
    float rzero = 0.0;      // R0, reference resistance [Ohm]
    float rcal = 0.0;       // Rcal, calibration resistance [Ohm]
    float refLevel = 0.0;   // Reference level for calibration [ppm]
    /* Sensor type & user options */
    bool compensation = false;       // Use temperature compensation
    bool calibration = false;        // Perform auto calibration
    int  sensorType = SENSOR_MAX;    // Selected sensor type
    P145_algorithm algorithm = p145AlgA;  // conversion algorithm
    int  analogPin = SENSOR_PIN;     // Analog input pin connected to the sensor

    public:
    float readValue(float temperature, float humidity);
    bool plugin_init();
    bool plugin_ten_per_second();
    void setSensorData(int stype, bool comp, bool cal, float load, float zero, float ref);
    const __FlashStringHelper * getTypeName( int stype);
    void dump();
    
    private:
    void  calibrate (float Rcal);
    float getAnalogValue();
    float getResistance(float val);
    float getRZero(float rSensor);
    float getCorrectedRZero(float rSensor, float temperature, float humidity);
    float getPPM(float rSensor);
    float getCorrectedPPM(float rSensor, float temperature, float humidity);
    void  resetOversampling();
};

#endif  // PLUGINSTRUCTS_P145_DATA_STRUCT_H
#endif  // USES_P145