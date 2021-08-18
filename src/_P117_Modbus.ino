#include "_Plugin_Helper.h"
#ifdef USES_P117
//#######################################################################################################
//############################### Plugin 117: Data OUT (MODBUS-Analog)###################################
//#######################################################################################################

#include "src/Helpers/Rules_calculate.h"
#include "src/WebServer/WebServer.h"
#include "src/Modbus/ModbusIP_ESP8266.h"

#define PLUGIN_117
#define PLUGIN_ID_117        117
#define PLUGIN_NAME_117       "Data OUT (MODBUS-Analog)"
#define PLUGIN_VALUENAME1_117 "Dev"
#define PLUGIN_VALUENAME2_117 "Analog"
#define PLUGIN_VALUENAME3_117 "Switch"



//ModbusIP object
ModbusIP mb;
//Khai bao cac bien
int RateOutPut;
int Switch_update;
int ModBus_control = 0;


// int state = 0; //Gia tri tran thai Switch ban dau

//Dinh nghia cac thanh nghi thuong la cac bien dung chung cho toan phan mem
#define Dev_Ir          PCONFIG_FLOAT(0) // Thanh ghi du lieu thiet bi dau vao thanh Ghi INPUT
#define Switch_Ir       PCONFIG_FLOAT(1) // Thanh thai GIPO dieu khien thanh gi InPut
#define ModBus_Hr       PCONFIG_FLOAT(2) // Thanh ghi Hold de lay tin hieu dieu khien Switch
#define Pin_out         PCONFIG_FLOAT(7) // Chan GIPO de out tin hieu dieu khien

boolean Plugin_117(byte function, struct EventStruct *event, String& string)
{
  boolean success = false;
  //static byte switchstate[TASKS_MAX];
  switch (function)
  {

    case PLUGIN_DEVICE_ADD:
      {
        Device[++deviceCount].Number = PLUGIN_ID_117;
        Device[deviceCount].Type = DEVICE_TYPE_SINGLE; //Khai bao may thiet bi vao
        Device[deviceCount].VType = Sensor_VType::SENSOR_TYPE_TRIPLE;
        Device[deviceCount].Ports = 0;
        Device[deviceCount].PullUpOption = false;
        Device[deviceCount].InverseLogicOption = false;
        Device[deviceCount].FormulaOption = true;
        Device[deviceCount].ValueCount = 3;
        Device[deviceCount].SendDataOption = true;
        Device[deviceCount].TimerOption = false;
        break;
      }

    case PLUGIN_GET_DEVICENAME:
      {
        string = F(PLUGIN_NAME_117);
        break;
      }

    case PLUGIN_GET_DEVICEVALUENAMES:
      {
        strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[0], PSTR(PLUGIN_VALUENAME1_117));
        strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[1], PSTR(PLUGIN_VALUENAME2_117));
        strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[2], PSTR(PLUGIN_VALUENAME3_117));

        break;
      }

  //Cai dat GPIO de xuat tin hieu
    case PLUGIN_GET_DEVICEGPIONAMES:
      {
        event->String1 = formatGpioName_output(F("ON/OFF"));

        break;
      }


// Hien thi cac thong so cai dat tren nen Web
    case PLUGIN_WEBFORM_LOAD:
      {
        //Load form lay du lieu tu thiet bi khac
        // char tmpString[128];
        addHtml(F("<TR><TD>Check Task:<TD>"));
        addTaskSelect(F("P117_task"), PCONFIG(0));

        LoadTaskSettings(PCONFIG(0)); // we need to load the values from another task for selection!
        addHtml(F("<TR><TD>Check Value:<TD>"));
        addTaskValueSelect(F("P117_value"), PCONFIG(1), PCONFIG(0));

        //add cac thanh ghi
        addFormSubHeader(F("Thanh ghi ModBus (Min = 100) - 04:Read Input Registers")); //hien thi dong nay tren web
        addFormTextBox(F("Thiet Bi"), F("P117_DevIr"), String(PCONFIG_FLOAT(0)), 8);
      	addFormTextBox(F("Switch Status"), F("P117_SwitchIr"), String(PCONFIG_FLOAT(1)), 8);
        addFormSubHeader(F("Digital OUT (=100:No || =101:Device || >101:Modbus) - 03:Read Holding Registers")); //hien thi dong nay tren web
        addFormTextBox(F("Control"), F("P117_ModBusHr"), String(PCONFIG_FLOAT(2)), 8);

        //add he so nhan gia tri do de tang do chinh xac
        addFormSubHeader(F("He so tinh chinh - y=a(x+b)")); //hien thi dong nay tren web
        addFormTextBox(F("He so nhan - a(1)"), F("P117_HSa"), String(PCONFIG_FLOAT(8)), 8);
        addFormTextBox(F("He so bu - b(0)"), F("P117_HSb"), String(PCONFIG_FLOAT(9)), 8);
        addFormSubHeader(F("Bien do dieu khien (=(1-Err%)*Max:OFF || =(1+Err%)*Max:ON)")); //hien thi dong nay tren web
        addFormTextBox(F("Sai so - Err(%)"), F("P117_HSerr"), String(PCONFIG_FLOAT(10)), 8);


        //add cac gia tri dieu Khien
        addFormSubHeader(F("Analog OUT"));
        addFormTextBox(F("1. Range (max 1024)"), F("P117_rate"), String(PCONFIG_FLOAT(3)), 8);
        addFormTextBox(F("2. Min"), F("P117_DevMin"), String(PCONFIG_FLOAT(4)), 8);
        addFormTextBox(F("3. Max"), F("P117_DevMax"), String(PCONFIG_FLOAT(5)), 8);

        addFormSubHeader(F("Cai dat GPIO (12,13,14,16) - Analog OUT")); //hien thi dong nay tren web
        addFormTextBox(F("Chan GPIO"), F("P117_Switch"), String(PCONFIG_FLOAT(7)),8);


        //Test tin hieu dau ra
        addFormSubHeader(F("Kiem tra tin hieu OUTPUT (> Range:Do || <=Range:Test)"));
        addFormTextBox(F("Test"), F("P117_Test"), String(PCONFIG_FLOAT(6)), 8);

        // we need to restore our original taskvalues!
        LoadTaskSettings(event->TaskIndex);
        success = true;
        break;
      }

      //Luu cac gia tri cai vao cac bien
      case PLUGIN_WEBFORM_SAVE:
      {
        PCONFIG(0) = getFormItemInt(F("P117_task")); //Ten gia tri dau vao
        PCONFIG(1) = getFormItemInt(F("P117_value")); //Gia tri dau vao

        PCONFIG_FLOAT(0) = getFormItemFloat(F("P117_DevIr")); //Thanh gi dau vao
        PCONFIG_FLOAT(1) = getFormItemFloat(F("P117_SwitchIr")); //Thanh ghi Swicth
        PCONFIG_FLOAT(2) = getFormItemFloat(F("P117_ModBusHr")); //Thanh ghi mobus dieu khien
        PCONFIG_FLOAT(3) = getFormItemFloat(F("P117_rate"));
        PCONFIG_FLOAT(4) = getFormItemFloat(F("P117_DevMin"));
        PCONFIG_FLOAT(5) = getFormItemFloat(F("P117_DevMax"));
        PCONFIG_FLOAT(6) = getFormItemFloat(F("P117_Test"));
        PCONFIG_FLOAT(7) = getFormItemFloat(F("P117_Switch"));
        PCONFIG_FLOAT(8) = getFormItemFloat(F("P117_HSa"));
        PCONFIG_FLOAT(9) = getFormItemFloat(F("P117_HSb"));
        PCONFIG_FLOAT(10) = getFormItemFloat(F("P117_HSerr"));


        success = true;
        break;
      }

      //Gia tri cai dat de dieu khien switch owr day la gia tri MAX
      case PLUGIN_SET_CONFIG:
        {
          String command = parseString(string, 1);
          if (command == F("setlevel"))
          {
            String value = parseString(string, 2);
            double result=0;
            if (!isError(Calculate(value, result))) {
              PCONFIG_FLOAT(5) = result;
              SaveSettings();
              success = true;
            }
          }
          break;
        }

      case PLUGIN_GET_CONFIG:
        {
          String command = parseString(string, 1);
          if (command == F("getlevel"))
          {
            string = PCONFIG_FLOAT(5);
            success = true;
          }
          break;
        }

      //Khu vuc chay ham giong nhu Void Setup ben Adruino
      case PLUGIN_INIT:
      {
        pinMode(CONFIG_PIN1, OUTPUT); //khai bao chan GPIO de dieu khien Switch

        pinMode(Pin_out, OUTPUT); //khai bao chan GPIO Out Analog

        //Loai Modbus
        mb.server();

        //Add thanh ghi
        mb.addIreg(Dev_Ir);
        mb.addIreg(Switch_Ir);
        mb.addHreg(ModBus_Hr);

        //Nhap trang thai cac thanh mac dinh
        mb.Ireg(Dev_Ir,0);
        mb.Ireg(Switch_Ir,0);
        mb.Hreg(ModBus_Hr,0);

        success = true;
        break;
      }

      //Dong lenh Void loop
      case PLUGIN_TEN_PER_SECOND:
        {

      //Call once inside loop() - all magic here
          mb.task();

      // Lay gia tri tin hieu cua thiet bi dau vao de tinh
          int ModBus_Hr_Add = PCONFIG_FLOAT(2);
          taskIndex_t TaskIndex = PCONFIG(0);
          byte BaseVarIndex = TaskIndex * VARS_PER_TASK + PCONFIG(1);
          float Dev_in = PCONFIG_FLOAT(8)*(UserVar[BaseVarIndex]+PCONFIG_FLOAT(9)); //Gia tri do cua thiet bi

      // Tinh toan Analog
          float valueLowThreshold = (1-PCONFIG_FLOAT(10)/100)*PCONFIG_FLOAT(8)*(PCONFIG_FLOAT(5)+PCONFIG_FLOAT(9));
          float valueHighThreshold = (1+PCONFIG_FLOAT(10)/100)*PCONFIG_FLOAT(8)*(PCONFIG_FLOAT(5)+PCONFIG_FLOAT(9));
          float RatePerDev_in = PCONFIG_FLOAT(3)/(PCONFIG_FLOAT(5) -PCONFIG_FLOAT(4));

          if (PCONFIG_FLOAT(6) >= PCONFIG_FLOAT(3)) {
              RateOutPut = int(round(Dev_in*RatePerDev_in));
          }
          if (PCONFIG_FLOAT(6) <= PCONFIG_FLOAT(3)) {
              RateOutPut = int(PCONFIG_FLOAT(6));
          }


     //Khong dieu khien chi lay tin hieu Analog (ModBusHr = 0 PCONFIG_FLOAT(2))
          if (ModBus_Hr_Add == 100)
          {
            Switch_update = 0;
          }

    //Dieu khien thu cong (ModBusHr >= 51 PCONFIG_FLOAT(2))
          if (ModBus_Hr_Add >= 102)
          {
            Switch_update = mb.Hreg(ModBus_Hr);
          }

    //Khi vuot chuan se dieu khien
          if (ModBus_Hr_Add == 101)
          { if (Dev_in <= valueLowThreshold)
            Switch_update = 0;
            if (Dev_in >= valueHighThreshold)
            Switch_update = 1;
          }

     //Xuat tin hieu khi co gia tri do hoac thanh ghi
          if ((Dev_in != UserVar[event->BaseVarIndex+0])||(ModBus_control != mb.Hreg(ModBus_Hr)))
            {
              //Ghi du lieu thiet bi dau vao vao thanh ghi
              ModBus_control = mb.Hreg(ModBus_Hr);
              analogWrite(Pin_out,RateOutPut) ;
              digitalWrite(CONFIG_PIN1, Switch_update);
              mb.Ireg(Dev_Ir,Dev_in);
              mb.Ireg(Switch_Ir,digitalRead(CONFIG_PIN1));
              if (loglevelActiveFor(LOG_LEVEL_INFO))
                {
                  String log = F("Dev: ");
                  log += Dev_in;
                  log += F(" - Analog: ");
                  log += RateOutPut;
                  log += F(" - Switch: ");
                  log += Switch_update;
                  addLog(LOG_LEVEL_INFO, log);
                }
                UserVar[event->BaseVarIndex+0] = Dev_in;
                UserVar[event->BaseVarIndex+1] = RateOutPut;
                UserVar[event->BaseVarIndex+2] = digitalRead(CONFIG_PIN1);
                sendData(event);
            }

      // Gan gia tri hai chan GPIO
          success = true;
          break;
        }
    }
    return success;
  }
#endif // USES_P117
