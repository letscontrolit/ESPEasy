#ifdef PLUGIN_BUILD_DEV

//#######################################################################################################
//#################################### Plugin 102: Nodo Event Bridge V2 #################################
//#######################################################################################################

#define PLUGIN_102
#define PLUGIN_ID_102         102
#define PLUGIN_NAME_102       "Nodo Event Bridge V2 [DEVELOPMENT]"
#define PLUGIN_VALUENAME1_102 ""

boolean Plugin_102(byte function, struct EventStruct *event, String& string)
{
  boolean success = false;
  static byte unicastTargetUnit = 0;

  switch (function)
  {

    case PLUGIN_DEVICE_ADD:
      {
        Device[++deviceCount].Number = PLUGIN_ID_102;
        Device[deviceCount].Custom = true;
        break;
      }

    case PLUGIN_GET_DEVICENAME:
      {
        string = F(PLUGIN_NAME_102);
        break;
      }

    case PLUGIN_GET_DEVICEVALUENAMES:
      {
        strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[0], PSTR(PLUGIN_VALUENAME1_102));
        break;
      }

    case PLUGIN_UDP_IN:
      {
        // event->Data is pointer to UDP buffer
        if (event->Data[0] == 255 && event->Data[1] == 254)
        {
          if (event->Data[11] == 0) // normal traffic, Nodo flags are 0
          {
            unicastTargetUnit = 0;
          }
          else
          {
            unicastTargetUnit = event->Data[9]; // remote source Nodo unit will be unicast target
          }
          for (byte x = 0; x < 16; x++)
            Serial.write(event->Data[x]);
        }
        success = true;
        break;
      }

    case PLUGIN_SERIAL_IN:
      {
        if (Serial.peek() == 255)
        {
          delay(20); // wait for message to complete
          if (Serial.read() == 255 && Serial.read() == 254)
          {
            byte data[16];
            data[0] = 255;
            data[1] = 254;
            byte count = 2;
            while (Serial.available() && count < 16)
              data[count++] = Serial.read();

            IPAddress sendIP(255, 255, 255, 255);

            if (data[11] != 0) // flags set, set to unicast mode
            {
              if ((unicastTargetUnit != 0) && (Nodes[unicastTargetUnit].ip[0] != 0))
                for(byte x=0; x <4; x++)
                  sendIP[x] = Nodes[unicastTargetUnit].ip[x];
            }

            portUDP.beginPacket(sendIP, Settings.UDPPort);
            portUDP.write(data, 16);
            portUDP.endPacket();
          }
          success = true;
        }
        break;
      }

  }
  return success;
}

#endif
