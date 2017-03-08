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
          String log = F("INIT: IR TX");
          addLog(LOG_LEVEL_INFO, log);
          Plugin_035_irSender = new IRsend(irPin);
          Plugin_035_irSender->begin(); // Start the sender
        }
        if (Plugin_035_irSender != 0 && irPin == -1)
        {
          String log = F("INIT: IR TX Removed");
          addLog(LOG_LEVEL_INFO, log);
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
        char log[120];

        char command[120];
        command[0] = 0;
        char TmpStr1[100];
        TmpStr1[0] = 0;
        string.toCharArray(command, 120);

        String cmdCode = string;
        int argIndex = cmdCode.indexOf(',');
        if (argIndex) cmdCode = cmdCode.substring(0, argIndex);

        if (cmdCode.equalsIgnoreCase("IRSEND") && Plugin_035_irSender != 0)
        {
          success = true;
          if (irReceiver != 0) irReceiver->disableIRIn(); // Stop the receiver

          if (GetArgv(command, TmpStr1, 2)) IrType = TmpStr1;

          if (IrType.equalsIgnoreCase("RAW")) {
            String IrRaw;
            unsigned int IrHz;
            unsigned int IrPLen;
            unsigned int IrBLen;

            if (GetArgv(command, TmpStr1, 3)) IrRaw = TmpStr1;
            if (GetArgv(command, TmpStr1, 4)) IrHz = str2int(TmpStr1);
            if (GetArgv(command, TmpStr1, 5)) IrPLen = str2int(TmpStr1);
            if (GetArgv(command, TmpStr1, 6)) IrBLen = str2int(TmpStr1);

            printWebString += F("<a href='https://en.wikipedia.org/wiki/Base32#base32hex'>Base32Hex</a> RAW Code: ");
            printWebString += IrRaw;
            printWebString += F("<BR>");

            printWebString += F("kHz: ");
            printWebString += IrHz;
            printWebString += F("<BR>");

            printWebString += F("Pulse Len: ");
            printWebString += IrPLen;
            printWebString += F("<BR>");

            printWebString += F("Blank Len: ");
            printWebString += IrBLen;
            printWebString += F("<BR>");

            unsigned int buf[200];
            unsigned int idx = 0;
            unsigned int c0 = 0;
            unsigned int c1 = 0;

            printWebString += F("Interpreted RAW Code: ");
            for(int i = 0; i < IrRaw.length(); i++)
            {
              char c = ((IrRaw[i] | ('A' ^ 'a')) - '0') % 39;

              for (unsigned int shft = 1; shft < 6; shft++)
              {
                if ((c & 16) != 0) {
                  c1++;
                  if (c0 > 0) {
                    buf[idx++] = c0 * IrBLen;
                    for (uint t = 0; t < c0; t++)
                      printWebString += F("0");
                  }
                  c0 = 0;
                } else {
                  if (c0+c1 != 0) {
                    c0++;
                    if (c1 > 0) {
                      buf[idx++] = c1 * IrPLen;
                      for (uint t = 0; t < c1; t++)
                        printWebString += F("1");
                    }
                    c1 = 0;
                  }
                }
                c <<= 1;
              }
            }

            if (c0 > 0) {
              buf[idx] = c0 * IrBLen;
              for (uint t = 0; t < c0; t++)
                printWebString += F("0");
            }
            if (c1 > 0) {
              buf[idx] = c1 * IrPLen;
              for (uint t = 0; t < c1; t++)
                printWebString += F("1");
            }

            printWebString += F("<BR>");

            Plugin_035_irSender->sendRaw(buf, idx+1, IrHz);

            //sprintf_P(log, PSTR("IR Params1: Hz:%u - PLen: %u - BLen: %u"), IrHz, IrPLen, IrBLen);
            //addLog(LOG_LEVEL_INFO, log);
            //sprintf_P(log, PSTR("IR Params2: RAW Code:%s"), IrRaw.c_str());
            //addLog(LOG_LEVEL_INFO, log);
          } else {
            if (GetArgv(command, TmpStr1, 2)) IrType = TmpStr1;
            if (GetArgv(command, TmpStr1, 3)) IrCode = strtoul(TmpStr1, NULL, 16); //(long) TmpStr1
            if (GetArgv(command, TmpStr1, 4)) IrBits = str2int(TmpStr1);

            if (IrType.equalsIgnoreCase("NEC")) Plugin_035_irSender->sendNEC(IrCode, IrBits);
            if (IrType.equalsIgnoreCase("JVC")) Plugin_035_irSender->sendJVC(IrCode, IrBits, 2);
            if (IrType.equalsIgnoreCase("RC5")) Plugin_035_irSender->sendRC5(IrCode, IrBits);
            if (IrType.equalsIgnoreCase("RC6")) Plugin_035_irSender->sendRC6(IrCode, IrBits);
            if (IrType.equalsIgnoreCase("SAMSUNG")) Plugin_035_irSender->sendSAMSUNG(IrCode, IrBits);
            if (IrType.equalsIgnoreCase("SONY")) Plugin_035_irSender->sendSony(IrCode, IrBits);
            if (IrType.equalsIgnoreCase("PANASONIC")) Plugin_035_irSender->sendPanasonic(IrBits, IrCode);
          }

          String log = F("IRTX :IR Code Sent");
          addLog(LOG_LEVEL_INFO, log);
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

