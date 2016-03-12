//#######################################################################################################
//#################################### Plugin 030: Output IR #############################################
//#######################################################################################################

#include <IRremoteESP8266.h>
IRsend *irSend; 
#define PLUGIN_030
#define PLUGIN_ID_030         30
#define PLUGIN_NAME_030       "Infrared output"
#define PLUGIN_VALUENAME1_030 "IR"
#define PLUGIN_MAXDEVICES     8

byte IRPins[PLUGIN_MAXDEVICES];
boolean Plugin_030(byte function, struct EventStruct *event, String& string)
{
  boolean success = false;

  switch (function)
  {
    case PLUGIN_DEVICE_ADD:
      {
        Device[++deviceCount].Number = PLUGIN_ID_030;
        Device[deviceCount].Type = DEVICE_TYPE_SINGLE;
        Device[deviceCount].VType = SENSOR_TYPE_SINGLE;
        Device[deviceCount].Ports = 0;
        Device[deviceCount].PullUpOption = false;
        Device[deviceCount].InverseLogicOption = false;
        Device[deviceCount].FormulaOption = false;
        Device[deviceCount].ValueCount = 0;
        Device[deviceCount].SendDataOption = false;
        Device[deviceCount].TimerOption = false;
        break;
      }

    case PLUGIN_GET_DEVICENAME:
      {
        string = F(PLUGIN_NAME_030);
        break;
      }

    case PLUGIN_GET_DEVICEVALUENAMES:
      {
        strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[0], PSTR(PLUGIN_VALUENAME1_030));
        break;
      }

    case PLUGIN_INIT:
      {
        int irPin = Settings.TaskDevicePin1[event->TaskIndex];
        if (irSend == 0 && irPin != -1)
        {
          Serial.println(F("INIT : IR Output"));
          irSend= new IRsend(irPin);
          irSend->begin();
          for (byte i = 0; i < PLUGIN_MAXDEVICES; i++) {
            if (IRPins[i]==0){
              IRPins[i]=irPin;
              break;
            }
          }
        }
        if (irSend != 0 && irPin == -1)
        {
          Serial.println(F("IR Output Removed"));
          delete irSend;
          irSend=0;
        }
        success = true;
        break;
      }
    case PLUGIN_WRITE:
      {
        String tmpString  = string;
        int argIndex = tmpString.indexOf(',');
        if (argIndex)
          tmpString = tmpString.substring(0, argIndex);
        if (tmpString.equalsIgnoreCase(F("IR")) && Plugin_030_pinCheck(event->Par1))
        {
           if (Plugin_030_Write(event->Par1, event->Par2,event->Par3,string.substring(argIndex+1)))
           {
              success = true;
              if (printToWeb)
              {
                printWebString += F("IR Command Send<BR>");
              }
           }
           else{
              success = false;
              if (printToWeb)
              {
                printWebString += F("<BR>Error in IR Command<BR>");
              }
           }
        }
        break;
      }
  }
  return success;
}
boolean Plugin_030_pinCheck(boolean pin){
  boolean success=false;
  for(byte i=0;i<PLUGIN_MAXDEVICES;i++){
    if (pin==IRPins[i]){
      success=true;
      break;
    }
  }
  return success;
}
boolean Plugin_030_Write(byte irPin,byte irt, byte nbits,String string){
  boolean success = false;
  string = string.substring(string.indexOf(',')+1);
  string = string.substring(string.indexOf(',')+1);
  string = string.substring(string.indexOf(',')+1);
  String irs = string.substring(0,string.indexOf(','));
  string  = string.substring(string.indexOf(',')+1);
  int rep = string.substring(0,string.indexOf(',')).toInt();
  if(rep==0) rep=1;
  if (irs.length() != 11 && irs.indexOf("0x") >=0 && rep<=100 && nbits>=1 && nbits<=32)
  {
    success = true;
    irSend= new IRsend(irPin);
    char irc[11];
    irs.toCharArray(irc, 11);
    unsigned long int iri = strtoul(irc,0,16);
    for (int i=0; i < rep; i++){
      switch (irt){
        case NEC:
          irSend->sendNEC(iri,nbits);
          break;
        case SONY:
          irSend->sendSony(iri,nbits);
          break;
        case RC5:
          irSend->sendRC5(iri,nbits);
          break;
        case RC6:
          irSend->sendRC6(iri,nbits);
          break;
        case DISH:
          irSend->sendDISH(iri,nbits);
          break;
        case SHARP:
          irSend->sendSharpRaw(iri,nbits);
          break;
        case SAMSUNG:
          irSend->sendSAMSUNG(iri,nbits);
          break;
        case WHYNTER:
        irSend->sendWhynter(iri,nbits); 
        break;
      }
      delay(80);
    }
  }
  return success;
}


