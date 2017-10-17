/********************************************************************************************\
* Get data from Serial Interface
\*********************************************************************************************/
#define INPUT_BUFFER_SIZE          128

byte SerialInByte;
int SerialInByteCounter = 0;
char InputBuffer_Serial[INPUT_BUFFER_SIZE + 2];

void serial()
{
  while (Serial.available())
  {
    yield();
    SerialInByte = Serial.read();
    if (SerialInByte == 255) // binary data...
    {
      Serial.flush();
      return;
    }

    if (isprint(SerialInByte))
    {
      if (SerialInByteCounter < INPUT_BUFFER_SIZE) // add char to string if it still fits
        InputBuffer_Serial[SerialInByteCounter++] = SerialInByte;
    }

    if (SerialInByte == '\r' || SerialInByte == '\n')
    {
      if (SerialInByteCounter == 0)   //empty command?
        break;
      InputBuffer_Serial[SerialInByteCounter] = 0; // serial data completed
      Serial.write('>');
      Serial.println(InputBuffer_Serial);
      String action = InputBuffer_Serial;
      struct EventStruct TempEvent;
      parseCommandString(&TempEvent, action);
      TempEvent.Source = VALUE_SOURCE_SERIAL;
      if (!PluginCall(PLUGIN_WRITE, &TempEvent, action))
        ExecuteCommand(VALUE_SOURCE_SERIAL, InputBuffer_Serial);
      SerialInByteCounter = 0;
      InputBuffer_Serial[0] = 0; // serial data processed, clear buffer
    }
  }
}
