//#######################################################################################################
//#################################### Plugin 082: MQ5 sensors ###############################################
//#######################################################################################################
#ifdef USES_P082
#define PLUGIN_082
#define PLUGIN_ID_082         82
#define PLUGIN_NAME_082       "Analog input - MQ5 smoke sensor"
#define PLUGIN_VALUENAME1_082 "Hydrogen"
#define PLUGIN_VALUENAME2_082 "LPG"
#define PLUGIN_VALUENAME3_082 "Methane"
#define PLUGIN_VALUENAME4_082 "CO"
#define PLUGIN_VALUENAME5_082 "Alcohol"

/************************Hardware Related Macros************************************/
#define         RL_VALUE                     (1)     //define the load resistance on the board, in kilo ohms
#define         RO_CLEAN_AIR_FACTOR          (6.455) //RO_CLEAR_AIR_FACTOR=(Sensor resistance in clean air)/RO,
                                                     //which is derived from the chart in datasheet

/***********************Software Related Macros************************************/
#define         CALIBARAION_SAMPLE_TIMES     (50)    //define how many samples you are going to take in the calibration phase
#define         CALIBRATION_SAMPLE_INTERVAL  (500)   //define the time interal(in milisecond) between each samples in the
                                                     //cablibration phase
#define         READ_SAMPLE_INTERVAL         (50)    //define how many samples you are going to take in normal operation
#define         READ_SAMPLE_TIMES            (5)     //define the time interal(in milisecond) between each samples in
                                                     //normal operation

/**********************Application Related Macros**********************************/
#define         GAS_HYDROGEN                 (0)
#define         GAS_LPG                      (1)
#define         GAS_METHANE                  (2)
#define         GAS_CARBON_MONOXIDE          (3)
#define         GAS_ALCOHOL                  (4)
// #define         accuracy                     (0)    //for linearcurves
#define         accuracy                   (1)    //for nonlinearcurves, un comment this line and comment the above line if calculations
                                                    //are to be done using non linear curve equations

float Ro = 10;                          //Ro is initialized to 10 kilo ohms

boolean Plugin_082(byte function, struct EventStruct *event, String& string)
{
  boolean success = false;

  switch (function)
  {

    case PLUGIN_DEVICE_ADD:
      {
        Device[++deviceCount].Number = PLUGIN_ID_082;
        Device[deviceCount].Type = DEVICE_TYPE_ANALOG;
        Device[deviceCount].VType = SENSOR_TYPE_TRIPLE;
        Device[deviceCount].Ports = 0;
        Device[deviceCount].PullUpOption = false;
        Device[deviceCount].InverseLogicOption = false;
        Device[deviceCount].FormulaOption = true;
        Device[deviceCount].ValueCount = 5;
        Device[deviceCount].SendDataOption = true;
        Device[deviceCount].TimerOption = true;
        Device[deviceCount].GlobalSyncOption = true;
        break;
      }

    case PLUGIN_GET_DEVICENAME:
      {
        string = F(PLUGIN_NAME_082);
        break;
      }

    case PLUGIN_GET_DEVICEVALUENAMES:
      {
        strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[0], PSTR(PLUGIN_VALUENAME1_082));
        strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[1], PSTR(PLUGIN_VALUENAME2_082));
        strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[2], PSTR(PLUGIN_VALUENAME3_082));
        strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[3], PSTR(PLUGIN_VALUENAME4_082));
        strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[4], PSTR(PLUGIN_VALUENAME5_082));
        break;
      }

    case PLUGIN_INIT:
      {
        int pinNum = Settings.TaskDevicePin1[event->TaskIndex];
        String log = F("Calibrating...\n");
        addLog(LOG_LEVEL_INFO,log);
        Ro = MQCalibration(pinNum);

        log = F("Calibration is done...\n");
        log = F("Ro=");
        log += Ro;
        log += F(" kohm\n");
        addLog(LOG_LEVEL_INFO,log);
      }

    case PLUGIN_READ:
      {
        String log = "";
        int MQ5PIN = Settings.TaskDevicePin1[event->TaskIndex];

        UserVar[event->BaseVarIndex] = (float) MQGetGasPercentage(MQRead(MQ5PIN)/Ro,GAS_HYDROGEN);
        UserVar[event->BaseVarIndex+1] = (float) MQGetGasPercentage(MQRead(MQ5PIN)/Ro,GAS_LPG);
        UserVar[event->BaseVarIndex+2] = (float) MQGetGasPercentage(MQRead(MQ5PIN)/Ro,GAS_METHANE);
        UserVar[event->BaseVarIndex+3] = (float) MQGetGasPercentage(MQRead(MQ5PIN)/Ro,GAS_CARBON_MONOXIDE);
        UserVar[event->BaseVarIndex+3] = (float) MQGetGasPercentage(MQRead(MQ5PIN)/Ro,GAS_ALCOHOL);

        //affect result
        success = true;
        break;
      }
    case PLUGIN_WEBFORM_LOAD:
      {
        #if defined(ESP32)
          addHtml(F("<TR><TD>Analog Pin:<TD>"));
          addPinSelect(false, F("taskdevicepin1"), Settings.TaskDevicePin1[event->TaskIndex]);
        #endif
        success = true;
        break;
      }
  }
  return success;
}

/****************** MQResistanceCalculation ****************************************
Input:   raw_adc - raw value read from adc, which represents the voltage
Output:  the calculated sensor resistance
Remarks: The sensor and the load resistor forms a voltage divider. Given the voltage
         across the load resistor and its resistance, the resistance of the sensor
         could be derived.
************************************************************************************/
float MQResistanceCalculation(int raw_adc)
{
  return ( ((float)RL_VALUE*(1023-raw_adc)/raw_adc));
}

/***************************** MQCalibration ****************************************
Input:   mq_pin - analog channel
Output:  Ro of the sensor
Remarks: This function assumes that the sensor is in clean air. It use
         MQResistanceCalculation to calculates the sensor resistance in clean air
         and then divides it with RO_CLEAN_AIR_FACTOR. RO_CLEAN_AIR_FACTOR is about
         10, which differs slightly between different sensors.
************************************************************************************/
float MQCalibration(int mq_pin)
{
  int i;
  float RS_AIR_val=0,r0;

  for (i=0;i<CALIBARAION_SAMPLE_TIMES;i++) {                     //take multiple samples
    RS_AIR_val += MQResistanceCalculation(analogRead(mq_pin));
    delay(CALIBRATION_SAMPLE_INTERVAL);
  }
  RS_AIR_val = RS_AIR_val/CALIBARAION_SAMPLE_TIMES;              //calculate the average value

  r0 = RS_AIR_val/RO_CLEAN_AIR_FACTOR;                      //RS_AIR_val divided by RO_CLEAN_AIR_FACTOR yields the Ro
                                                                 //according to the chart in the datasheet

  return r0;
}
/*****************************  MQRead *********************************************
Input:   mq_pin - analog channel
Output:  Rs of the sensor
Remarks: This function use MQResistanceCalculation to caculate the sensor resistenc (Rs).
         The Rs changes as the sensor is in the different consentration of the target
         gas. The sample times and the time interval between samples could be configured
         by changing the definition of the macros.
************************************************************************************/
float MQRead(int mq_pin)
{
  int i;
  float rs=0;

  for (i=0;i<READ_SAMPLE_TIMES;i++) {
    rs += MQResistanceCalculation(analogRead(mq_pin));
    delay(READ_SAMPLE_INTERVAL);
  }

  rs = rs/READ_SAMPLE_TIMES;

  return rs;
}

/*****************************  MQGetGasPercentage **********************************
Input:   rs_ro_ratio - Rs divided by Ro
         gas_id      - target gas type
Output:  ppm of the target gas
Remarks: This function uses different equations representing curves of each gas to
         calculate the ppm (parts per million) of the target gas.
************************************************************************************/
int MQGetGasPercentage(float rs_ro_ratio, int gas_id)
{
  if ( accuracy == 0 ) {
    if ( gas_id == GAS_HYDROGEN ) {
      return (pow(10,((-3.986*(log10(rs_ro_ratio))) + 3.075)));
    } else if ( gas_id == GAS_LPG ) {
      return (pow(10,((-2.513*(log10(rs_ro_ratio))) + 1.878)));
    } else if ( gas_id == GAS_METHANE ) {
      return (pow(10,((-2.554*(log10(rs_ro_ratio))) + 2.265 )));
    } else if ( gas_id == GAS_CARBON_MONOXIDE ) {
      return (pow(10,((-6.900*(log10(rs_ro_ratio))) + 6.241)));
    } else if ( gas_id == GAS_ALCOHOL ) {
      return (pow(10,((-4.590*(log10(rs_ro_ratio))) + 4.851)));
    }
  } else if ( accuracy == 1 ) {
    if ( gas_id == GAS_HYDROGEN ) {
      return (pow(10,(-22.89*pow((log10(rs_ro_ratio)), 3) + 8.873*pow((log10(rs_ro_ratio)), 2) - 3.587*(log10(rs_ro_ratio)) + 2.948)));
    } else if ( gas_id == GAS_LPG ) {
      return (pow(10,((-2.513*(log10(rs_ro_ratio))) + 1.878)));
    } else if ( gas_id == GAS_METHANE ) {
      return (pow(10,(-0.428*pow((log10(rs_ro_ratio)), 2) - 2.867*(log10(rs_ro_ratio)) + 2.224)));
    } else if ( gas_id == GAS_CARBON_MONOXIDE ) {
      return (pow(10,(1401*pow((log10(rs_ro_ratio)), 4) - 2777*pow((log10(rs_ro_ratio)), 3) + 2059*pow((log10(rs_ro_ratio)), 2) - 682.5*(log10(rs_ro_ratio)) + 88.81)));
    } else if ( gas_id == GAS_ALCOHOL ) {
      return (pow(10,(14.90*pow((log10(rs_ro_ratio)), 3) - 19.26*pow((log10(rs_ro_ratio)), 2) + 3.108*(log10(rs_ro_ratio)) + 3.922)));
    }
  }
  return 0;
}
#endif // USES_P082