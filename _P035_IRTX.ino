//#######################################################################################################
//#################################### Plugin 035: Output IR ############################################
//#######################################################################################################

#include <IRremoteESP8266.h>
IRsend *Plugin_035_irSender;

#define PLUGIN_035
#define PLUGIN_ID_035         35
#define PLUGIN_NAME_035       "Infrared Transmit"

boolean Plugin_035(byte function, struct EventStruct *event, String& string)
{
  boolean success = false;

  switch (function)
  {
    case PLUGIN_DEVICE_ADD:
      {
        Device[++deviceCount].Number = PLUGIN_ID_035;
        Device[deviceCount].Type = DEVICE_TYPE_SINGLE;
        Device[deviceCount].SendDataOption = false;
        break;
      }

    case PLUGIN_GET_DEVICENAME:
      {
        string = F(PLUGIN_NAME_035);
        break;
      }

    case PLUGIN_GET_DEVICEVALUENAMES:
      {
        break;
      }

    case PLUGIN_INIT:
      {
        int irPin = Settings.TaskDevicePin1[event->TaskIndex];
        if (Plugin_035_irSender == 0 && irPin != -1)
        {
          addLog(LOG_LEVEL_INFO, "INIT: IR TX");
          Plugin_035_irSender = new IRsend(irPin);
          Plugin_035_irSender->begin(); // Start the sender
        }
        if (Plugin_035_irSender != 0 && irPin == -1)
        {
          addLog(LOG_LEVEL_INFO, "INIT: IR TX Removed");
          delete Plugin_035_irSender;
          Plugin_035_irSender = 0;
        }
        success = true;
        break;
      }

    case PLUGIN_WRITE:
      {
        String IrType;
        unsigned long IrCode;
        unsigned int IrBits;
        char command[80];
        command[0] = 0;
        char TmpStr1[80];
        TmpStr1[0] = 0;
        string.toCharArray(command, 80);

        String tmpString = string;
        int argIndex = tmpString.indexOf(',');
        if (argIndex) tmpString = tmpString.substring(0, argIndex);

        if (GetArgv(command, TmpStr1, 2)) IrType = TmpStr1;
        if (GetArgv(command, TmpStr1, 3)) IrCode = strtoul(TmpStr1, NULL, 16); //(long) TmpStr1
        if (GetArgv(command, TmpStr1, 4)) IrBits = str2int(TmpStr1);

        if (tmpString.equalsIgnoreCase("IRSEND") && Plugin_035_irSender != 0)
        {
          success = true;
          if (irReceiver != 0) irReceiver->disableIRIn(); // Stop the receiver

          if (IrType.equalsIgnoreCase("NEC")) Plugin_035_irSender->sendNEC(IrCode, IrBits);
          if (IrType.equalsIgnoreCase("JVC")) Plugin_035_irSender->sendJVC(IrCode, IrBits, 2);
          if (IrType.equalsIgnoreCase("RC5")) Plugin_035_irSender->sendRC5(IrCode, IrBits);
          if (IrType.equalsIgnoreCase("RC6")) Plugin_035_irSender->sendRC6(IrCode, IrBits);
          if (IrType.equalsIgnoreCase("SAMSUNG")) Plugin_035_irSender->sendSAMSUNG(IrCode, IrBits);
          if (IrType.equalsIgnoreCase("SONY")) Plugin_035_irSender->sendSony(IrCode, IrBits);
          if (IrType.equalsIgnoreCase("PANASONIC")) Plugin_035_irSender->sendPanasonic(IrBits, IrCode);

          addLog(LOG_LEVEL_INFO, "IR Code Sent");
          if (printToWeb)
          {
            printWebString += F("IR Code Sent ");
            printWebString += IrType;
            printWebString += F("<BR>");
          }

          if (irReceiver != 0) irReceiver->enableIRIn(); // Start the receiver
        }
        break;
      }
  }
  return success;
}

