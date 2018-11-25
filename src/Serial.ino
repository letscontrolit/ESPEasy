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
    delay(0);
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
      serialPrintln(InputBuffer_Serial);
      String action = InputBuffer_Serial;
      struct EventStruct TempEvent;
      action=parseTemplate(action,action.length());  //@giig1967g: parseTemplate before executing the command bug#1977
      parseCommandString(&TempEvent, action);
      TempEvent.Source = VALUE_SOURCE_SERIAL;
      if (!PluginCall(PLUGIN_WRITE, &TempEvent, action))
        ExecuteCommand(VALUE_SOURCE_SERIAL, action.c_str());
      SerialInByteCounter = 0;
      InputBuffer_Serial[0] = 0; // serial data processed, clear buffer
    }
  }
}


void addToSerialBuffer(const char *line) {
  const size_t line_length = strlen(line);
  int roomLeft = ESP.getFreeHeap() - 5000;
  if (roomLeft > 0) {
    size_t pos = 0;
    while (pos < line_length && pos < static_cast<size_t>(roomLeft)) {
      serialWriteBuffer.push_back(line[pos]);
      ++pos;
    }
  }
}

void addNewlineToSerialBuffer() {
  serialWriteBuffer.push_back('\r');
  serialWriteBuffer.push_back('\n');
}

void process_serialWriteBuffer() {
  if (serialWriteBuffer.size() == 0) return;
  size_t snip = 128; // Some default, ESP32 doesn't have the availableForWrite function yet.
#if defined(ESP8266)
  snip = Serial.availableForWrite();
#endif
  if (snip > 0) {
    last_serial_writebuffer_read = millis();
    size_t bytes_to_write = serialWriteBuffer.size();
    if (snip < bytes_to_write) bytes_to_write = snip;
    for (size_t i = 0; i < bytes_to_write; ++i) {
      Serial.write(serialWriteBuffer.front());
      serialWriteBuffer.pop_front();
    }
  }
}

bool serialWriteBufferActiveRead() {
  if (!Settings.UseSerial) return false;
  // Some default, ESP32 doesn't have the availableForWrite function yet.
  // Not sure how to detect read activity on an ESP32.
  size_t tx_free = 128;
#if defined(ESP8266)
  tx_free = Serial.availableForWrite();
#endif
  static size_t prev_tx_free = 0;
  if (tx_free < prev_tx_free) {
    prev_tx_free = tx_free;
    tempDisableSerialLog(false);
    return true;
  }
  // Must always set it or else it will never recover from prev_tx_free == 0
  prev_tx_free = tx_free;
  if (timePassedSince(last_serial_writebuffer_read) > LOG_BUFFER_EXPIRE) {
    serialWriteBuffer.clear();
    // Just add some marker to get it going again when the serial buffer is
    // read again and the serial log level was temporary set to 0 since nothing was read.
    if (Settings.SerialLogLevel > 0) {
      serialWriteBuffer.push_back('\n');
    }
    tempDisableSerialLog(true);
    return false;
  }
  return true;
}

// For now, only send it to the serial buffer and try to process it.
// Later we may want to wrap it into a log.
void serialPrint(const String& text) {
  addToSerialBuffer(text.c_str());
  process_serialWriteBuffer();
}

void serialPrintln(const String& text) {
  addToSerialBuffer(text.c_str());
  addNewlineToSerialBuffer();
  process_serialWriteBuffer();
}

void serialPrintln() {
  addNewlineToSerialBuffer();
  process_serialWriteBuffer();
}



// Do not add helper functions for other types, since those types can only be
// explicit matched at a constructor, not a function declaration.
/*
void serialPrint(char c) {
  serialPrint(String(c));
}


void serialPrint(unsigned long value) {
  serialPrint(String(value));
}

void serialPrint(long value) {
  serialPrint(String(value));
}

void serialPrintln(unsigned long value) {
  serialPrintln(String(value));
}
*/
