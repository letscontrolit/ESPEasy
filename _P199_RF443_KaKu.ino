#ifdef PLUGIN_BUILD_DEV

//#######################################################################################################
//#################################### Plugin 199: RF KaKu receiver/sender ##############################
//#######################################################################################################

// Purpose: Control Klik-Aan-Klik-Uit RF 433MHz devices directly from ESP Easy (receive and send)
// Status : "Proof of concept"

// Connect the RF Receiver data pin to the first pin selected in the webgui
// Connect the RF Transmitter data pin to the second pin selected in the webgui

// Events:
//   newKaku_<address>#<Channel>=<state> (0=off, 1-15=dimvalue, 16=on)
//   Kaku_<address>#<Channel>=<state> (0=off, 1=on)
//   HE300EU_<address>#<Channel>=<state> (0=off, 1=on)

// Commands:
//   newKakuSend <address>, <Channel>, <state/dim>
//   KakuSend <address>, <Channel>, <state>

// This is a Work in Progress mini project!
// It has limited use because in most cases, your fancy Home Automation controller can handle 433MHz devices quite well using RFLink.
// It was implemented because in some cases i would like to have local "Klik-Aan-Klik-Uit" support using a standalone ESP Easy.
//   (Just because i own quite a lot of these Kaku devices)

// Current state / limitations:
//  Implemented send and receive support for KaKu with automatic code (no code wheel)
//  Implemented send and receive support for old KaKu unit's with code wheels
//  Implemented receive support for HomeEasy HE300EU remotes
//  RF Sender and RF receiver each need their own antenna! (as opposed to using a transceiver)

#define MIN_PULSE_LENGTH           100  // Too short pulses are considered to be noise...
#define SIGNAL_TIMEOUT               5  // gap between transmissions
#define MIN_RAW_PULSES              32  // Minimum number of pulses to be received, otherwise considered to be noise...
#define RAW_BUFFER_SIZE            256
#define RAWSIGNAL_MULTIPLY          25

void RF_ISR() ICACHE_RAM_ATTR;

// We need our own rawsignal buffer here.
// During plugin rawsignal checks, the IRQ routine will alPlugin_199_ready be working on the next signal burst...
volatile byte Plugin_199_RFBuffer[RAW_BUFFER_SIZE];
volatile boolean Plugin_199_ready = false;
volatile byte Plugin_199_pulses[RAW_BUFFER_SIZE + 2];
volatile int Plugin_199_number;
unsigned long Plugin_199_codeHash;
unsigned long Plugin_199_lastTime;
int8_t Plugin_199_RXpin = -1;
int8_t Plugin_199_TXpin = -1;

#define PLUGIN_199
#define PLUGIN_ID_199        199
#define PLUGIN_NAME_199       "RF Receiver/Sender [DEVELOPMENT]"
#define PLUGIN_VALUENAME1_199 "Address"
#define PLUGIN_VALUENAME2_199 "Channel"
#define PLUGIN_VALUENAME3_199 "State"

boolean Plugin_199(byte function, struct EventStruct *event, String& string)
{
  boolean success = false;

  switch (function)
  {

    case PLUGIN_DEVICE_ADD:
      {
        Device[++deviceCount].Number = PLUGIN_ID_199;
        Device[deviceCount].Type = DEVICE_TYPE_DUAL;
        Device[deviceCount].VType = SENSOR_TYPE_DUAL;
        Device[deviceCount].Ports = 0;
        Device[deviceCount].PullUpOption = false;
        Device[deviceCount].InverseLogicOption = false;
        Device[deviceCount].FormulaOption = false;
        Device[deviceCount].ValueCount = 3;
        break;
      }

    case PLUGIN_GET_DEVICENAME:
      {
        string = F(PLUGIN_NAME_199);
        break;
      }

    case PLUGIN_GET_DEVICEVALUENAMES:
      {
        strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[0], PSTR(PLUGIN_VALUENAME1_199));
        strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[1], PSTR(PLUGIN_VALUENAME2_199));
        strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[2], PSTR(PLUGIN_VALUENAME3_199));
        break;
      }

    case PLUGIN_INIT:
      {
        if (Settings.TaskDevicePin1[event->TaskIndex] != -1)
        {
          Plugin_199_RXpin = Settings.TaskDevicePin1[event->TaskIndex];
          pinMode(Plugin_199_RXpin, INPUT_PULLUP);
          attachInterrupt(Plugin_199_RXpin, RF_ISR, CHANGE);
          success = true;
        }
        if (Settings.TaskDevicePin1[event->TaskIndex] != -1)
        {
          Plugin_199_TXpin = Settings.TaskDevicePin2[event->TaskIndex];
          pinMode(Plugin_199_TXpin, OUTPUT);
        }
        break;
      }

    case PLUGIN_TEN_PER_SECOND:
      {
        if (Plugin_199_ready)
        {
          if (decodeNewKaku(event->BaseVarIndex));
          else if (decodeKaku());
          else if (decodeHE300EU());
          Plugin_199_ready = false;
        }
        break;
      }

    case PLUGIN_WRITE:
      {
        String command = parseString(string, 1);
        if (command == F("newkakusend"))
        {
          success = true;
          sendNewKaku(event->Par1, event->Par2, event->Par3);
        }
        if (command == F("kakusend"))
        {
          success = true;
          sendKaku(event->Par1, event->Par2, event->Par3);
        }
        break;
      }

  }
  return success;
}


//********************************************************************************
//  Interrupt handler for RF messages
//********************************************************************************
void RF_ISR()
{
  static unsigned int counter = 0;
  static unsigned long TimeStamp = 0;
  unsigned long TimeElapsed = 0;

  TimeElapsed = micros() - TimeStamp;
  TimeStamp = micros();

  if (TimeElapsed > MIN_PULSE_LENGTH && counter < RAW_BUFFER_SIZE)
  {
    counter++;
    Plugin_199_RFBuffer[counter] = TimeElapsed / 25;
  }
  else
    counter = 0;

  if (TimeElapsed > (SIGNAL_TIMEOUT * 1000) )
  {
    if (counter > MIN_RAW_PULSES)
    {
      Plugin_199_number = counter;

      // copy IRQ RF working buffer to RawSignal struct
      for (unsigned int x = 0; x <= counter; x++)
        Plugin_199_pulses[x] = Plugin_199_RFBuffer[x];

      Plugin_199_ready = true;
      counter = 0;
    }
    else
      counter = 0;
  }

  if (counter >= RAW_BUFFER_SIZE)
    counter = 0;
}


//********************************************************************************
//  Transmit pulses using pulse array
//********************************************************************************
void RawSendRF(void)
{
  if (Plugin_199_RXpin != -1)
    detachInterrupt(Plugin_199_RXpin);
  int x;
  //digitalWrite(PIN_RF_RX_VCC,LOW);                      // Turn off RF receiver
  //digitalWrite(PIN_RF_TX_VCC,HIGH);                     // Turn on RF sender

  delay(5);                                               // small delay between switching send/receive

  Plugin_199_pulses[Plugin_199_number] = 1;               // force last duration as 1 msec

  for (byte y = 0; y < 7; y++)                            // repeats RF code
  {
    x = 1;
    noInterrupts();
    while (x < Plugin_199_number)
    {
      digitalWrite(Plugin_199_TXpin, HIGH);
      delayMicroseconds(Plugin_199_pulses[x++] * 25 - 5);
      digitalWrite(Plugin_199_TXpin, LOW);
      delayMicroseconds(Plugin_199_pulses[x++] * 25 - 7);
    }
    interrupts();
    delay(20);// Delay must run outside interrupt blocked code.
  }

  delay(5);

  //digitalWrite(PIN_RF_TX_VCC,LOW);  // turn off RF sender
  //digitalWrite(PIN_RF_RX_VCC,HIGH); // turn on RF receiver
  if (Plugin_199_RXpin != -1)
    attachInterrupt(Plugin_199_RXpin, RF_ISR, CHANGE);
}


//********************************************************************************
//  Decode NewKaku protocol (without code wheel)
//********************************************************************************
#define NewKAKU_RawSignalLength      132
#define NewKAKUdim_RawSignalLength   148
#define NewKAKU_1T                   275        // us
#define NewKAKU_mT                   650        // us
#define NewKAKU_4T                  1100        // us
#define NewKAKU_8T                  2200        // us

boolean decodeNewKaku(byte BaseVarIndex)
{
  boolean success = false;
  byte Par1 = 0;
  unsigned long Par2 = 0;

  unsigned long bitstream = 0L;
  unsigned long address = 0L;
  byte channel = 0;
  byte command = 0;
  boolean Bit;
  int i;
  int P0, P1, P2, P3;
  Par1 = 0;

  if (Plugin_199_number == NewKAKU_RawSignalLength || Plugin_199_number == NewKAKUdim_RawSignalLength)
  {
    i = 3; // Plugin_199_pulses[3] is de eerste van een T,xT,T,xT combinatie

    do
    {
      P0 = Plugin_199_pulses[i]    * 25;
      P1 = Plugin_199_pulses[i + 1]  * 25;
      P2 = Plugin_199_pulses[i + 2]  * 25;
      P3 = Plugin_199_pulses[i + 3]  * 25;

      if     (P0 < NewKAKU_mT && P1 < NewKAKU_mT && P2 < NewKAKU_mT && P3 > NewKAKU_mT)Bit = 0; // T,T,T,4T
      else if (P0 < NewKAKU_mT && P1 > NewKAKU_mT && P2 < NewKAKU_mT && P3 < NewKAKU_mT)Bit = 1; // T,4T,T,T
      else if (P0 < NewKAKU_mT && P1 < NewKAKU_mT && P2 < NewKAKU_mT && P3 < NewKAKU_mT) // T,T,T,T Deze hoort te zitten op i=111 want: 27e NewKAKU bit maal 4 plus 2 posities voor startbit
      {
        if (Plugin_199_number != NewKAKUdim_RawSignalLength)                 // als de dim-bits er niet zijn
          return false;
      }
      else
        return false;                                                       // andere mogelijkheden zijn niet geldig in NewKAKU signaal.

      if (i < 130)                                                          // alle bits die tot de 32-bit pulstrein behoren 32bits * 4posities per bit + pulse/space voor startbit
        bitstream = (bitstream << 1) | Bit;
      else                                                                  // de resterende vier bits die tot het dimlevel behoren
        Par1 = (Par1 << 1) | Bit;

      i += 4;                                                               // volgende pulsenquartet
    } while (i < Plugin_199_number - 2);                                    //-2 omdat de space/pulse van de stopbit geen deel meer van signaal uit maakt.

    if (i > 140)                                                            // Commando en Dim deel
      Par1++;                                                        // Dim level. +1 omdat gebruiker dim level begint bij één.
    else
      Par1 = ((bitstream >> 4) & 0x01) ? 16 : 0;       // On/Off bit omzetten naar een Nodo waarde.

    Par2 = bitstream;
    address = bitstream >> 6;
    channel = (bitstream & 0x0f) + 1;
    command = (bitstream >> 4) & 0x03;
    if (command > 1)
      channel = 0;
    // valid signal, remember timestamp to suppress repeats...
    elapsed = millis() - Plugin_199_lastTime;
    Plugin_199_lastTime = millis();
    unsigned long codeHash = Par2 + Par1;
    if (codeHash != Plugin_199_codeHash || (codeHash == Plugin_199_codeHash && elapsed > 250))
    {
      UserVar[BaseVarIndex] = address;
      UserVar[BaseVarIndex + 1] = channel;
      UserVar[BaseVarIndex + 2] = Par1;
      String eventString = F("NewKaku_");
      eventString += address;
      eventString += F("#");
      eventString += channel;
      eventString += F("=");
      eventString += Par1;
      rulesProcessing(eventString);
      success = true;
    }
    Plugin_199_codeHash = Par2 + Par1;
  }
  return success;
}


//********************************************************************************
//  Send NewKaku protocol (without code wheel)
//********************************************************************************
void sendNewKaku(unsigned long address, byte channel, byte state)
{
  unsigned long bitstream = 0L;
  byte i = 1;
  byte x;                                                                   // aantal posities voor pulsen/spaces in RawSignal

  bitstream = address << 6;
  bitstream |= (channel - 1);

  //RawSignal.Repeats = 7;                                                    // Aantal herhalingen van het signaal.
  //RawSignal.Delay = 20;                                                     // Tussen iedere pulsenreeks enige tijd rust.

  if (state == 16 || state == 0)
  {
    bitstream |= (state == 16) << 4;                            // bit-5 is het on/off commando in KAKU signaal
    x = 130;                                                                // verzend startbit + 32-bits = 130
  }
  else
    x = 146;                                                                // verzend startbit + 32-bits = 130 + 4dimbits = 146

  // bitstream bevat nu de KAKU-bits die verzonden moeten worden.

  for (i = 3; i <= x; i++)Plugin_199_pulses[i] = NewKAKU_1T / 25; // De meeste tijden in signaal zijn T. Vul alle pulstijden met deze waarde. Later worden de 4T waarden op hun plek gezet

  i = 1;
  Plugin_199_pulses[i++] = NewKAKU_1T / 25;                  //pulse van de startbit
  Plugin_199_pulses[i++] = NewKAKU_8T / 25;                  //space na de startbit

  byte y = 31;                                                              // bit uit de bitstream
  while (i < x)
  {
    if ((bitstream >> (y--)) & 1)
      Plugin_199_pulses[i + 1] = NewKAKU_4T / 25;            // Bit=1; // T,4T,T,T
    else
      Plugin_199_pulses[i + 3] = NewKAKU_4T / 25;            // Bit=0; // T,T,T,4T

    if (x == 146)                                                           // als het een dim opdracht betreft
    {
      if (i == 111)                                                         // Plaats van de Commando-bit uit KAKU
        Plugin_199_pulses[i + 3] = NewKAKU_1T / 25;          // moet een T,T,T,T zijn bij een dim commando.
      if (i == 127)                                                         // als alle pulsen van de 32-bits weggeschreven zijn
      {
        bitstream = (unsigned long)state;                         //  nog vier extra dim-bits om te verzenden.
        y = 3;
      }
    }
    i += 4;
  }
  Plugin_199_pulses[i++] = NewKAKU_1T / 25;                  //pulse van de stopbit
  Plugin_199_pulses[i] = 0;                                                  //space van de stopbit
  Plugin_199_number = i;                                                     // aantal bits*2 die zich in het opgebouwde RawSignal bevinden
  RawSendRF();
}


//********************************************************************************
//  Decode Kaku protocol (with code wheel)
//********************************************************************************
#define KAKU_CodeLength             12
#define KAKU_T                     350
boolean decodeKaku()
{
  boolean success = false;
  byte Par1 = 0;
  unsigned long Par2 = 0;

  int i, j;
  unsigned long bitstream = 0;

  if (Plugin_199_number != (KAKU_CodeLength * 4) + 2)return false;           // conventionele KAKU bestaat altijd uit 12 data bits plus stop. Ongelijk, dan geen KAKU!

  for (i = 0; i < KAKU_CodeLength; i++)
  {
    j = (KAKU_T * 2) / 25;

    if      (Plugin_199_pulses[4 * i + 1] < j && Plugin_199_pulses[4 * i + 2] > j && Plugin_199_pulses[4 * i + 3] < j && Plugin_199_pulses[4 * i + 4] > j) {
      bitstream = (bitstream >> 1); // 0
    }
    else if (Plugin_199_pulses[4 * i + 1] < j && Plugin_199_pulses[4 * i + 2] > j && Plugin_199_pulses[4 * i + 3] > j && Plugin_199_pulses[4 * i + 4] < j) {
      bitstream = (bitstream >> 1 | (1 << (KAKU_CodeLength - 1)));  // 1
    }
    else if (Plugin_199_pulses[4 * i + 1] < j && Plugin_199_pulses[4 * i + 2] > j && Plugin_199_pulses[4 * i + 3] < j && Plugin_199_pulses[4 * i + 4] < j) {
      bitstream = (bitstream >> 1);  // Short 0, Groep commando op 2e bit.
      Par1 = 2;
    }
    else {
      return false; // foutief signaal
    }
  }

  if ((bitstream & 0x600) == 0x600)                                         // twee vaste bits van KAKU gebruiken als checksum
  { // Alles is in orde, bouw event op
    Par2          = bitstream & 0xFF;
    Par1         |= (bitstream >> 11) & 0x01;

    // valid signal, remember timestamp to suppress repeats...
    elapsed = millis() - Plugin_199_lastTime;
    Plugin_199_lastTime = millis();
    unsigned long codeHash = Par2 + Par1;
    if (codeHash != Plugin_199_codeHash || (codeHash == Plugin_199_codeHash && elapsed > 250))
    {
      String eventString = F("Kaku_");
      eventString += (Par2 & 0x0f) + 1;
      eventString += F("#");
      eventString += (Par2 >> 4) + 1;
      eventString += F("=");
      eventString += Par1;
      rulesProcessing(eventString);
      success = true;
    }
    Plugin_199_codeHash = Par2 + Par1;
  }
  return success;
}


//********************************************************************************
//  Send Kaku protocol (with code wheel)
//********************************************************************************
void sendKaku(unsigned long address, byte channel, byte state)
{
  byte Par1 = state;
  unsigned long Par2 = ((channel-1) << 4) + address-1;

  unsigned long Bitstream = Par2 | (0x600 | ((Par1 & 1 /*Commando*/) << 11)); // Stel een bitstream samen

  // loop de 12-bits langs en vertaal naar pulse/space signalen.
  for (byte i = 0; i < KAKU_CodeLength; i++)
  {
    Plugin_199_pulses[4 * i + 1] = KAKU_T / RAWSIGNAL_MULTIPLY;
    Plugin_199_pulses[4 * i + 2] = (KAKU_T * 3) / RAWSIGNAL_MULTIPLY;

    if (((Par1 >> 1) & 1) /* Groep */ && i >= 4 && i < 8)
    {
      Plugin_199_pulses[4 * i + 3] = KAKU_T / RAWSIGNAL_MULTIPLY;
      Plugin_199_pulses[4 * i + 4] = KAKU_T / RAWSIGNAL_MULTIPLY;
    } // short 0
    else
    {
      if ((Bitstream >> i) & 1) // 1
      {
        Plugin_199_pulses[4 * i + 3] = (KAKU_T * 3) / RAWSIGNAL_MULTIPLY;
        Plugin_199_pulses[4 * i + 4] = KAKU_T / RAWSIGNAL_MULTIPLY;
      }
      else //0
      {
        Plugin_199_pulses[4 * i + 3] = KAKU_T / RAWSIGNAL_MULTIPLY;
        Plugin_199_pulses[4 * i + 4] = (KAKU_T * 3) / RAWSIGNAL_MULTIPLY;
      }
    }
    // Stopbit
    Plugin_199_pulses[4 * KAKU_CodeLength + 1] = KAKU_T / RAWSIGNAL_MULTIPLY;
    Plugin_199_pulses[4 * KAKU_CodeLength + 2] = KAKU_T / RAWSIGNAL_MULTIPLY;
  }

  Plugin_199_number=KAKU_CodeLength*4+2;
  RawSendRF();
}


//********************************************************************************
//  Decode HomeEasy 3xx series EU protocol (without code wheel)
//********************************************************************************
boolean decodeHE300EU()
{
  boolean success = false;
  byte Par1 = 0;
  unsigned long Par2 = 0;

  unsigned long address = 0;
  unsigned long bitstream = 0;
  int counter = 0;
  byte rfbit = 0;
  byte state = 0;
  unsigned long channel = 0;

  // valid messages are 116 pulses
  if (Plugin_199_number != 116) return false;

  for (byte x = 1; x <= Plugin_199_number; x = x + 2)
  {
    if ((Plugin_199_pulses[x] * 25 < 500) & (Plugin_199_pulses[x + 1] * 25 > 500))
      rfbit = 1;
    else
      rfbit = 0;

    if ((x >= 23) && (x <= 86)) address = (address << 1) | rfbit;
    if ((x >= 87) && (x <= 114)) bitstream = (bitstream << 1) | rfbit;

  }
  state = ((bitstream >> 8) & 0x3) - 1;
  channel = (bitstream) & 0x3f;

  Par1 = state;
  Par2 = address + channel;

  // valid signal, remember timestamp to suppress repeats...
  elapsed = millis() - Plugin_199_lastTime;
  Plugin_199_lastTime = millis();
  unsigned long codeHash = Par2 + Par1;
  if (codeHash != Plugin_199_codeHash || (codeHash == Plugin_199_codeHash && elapsed > 250))
  {
    String eventString = F("HE300EU_");
    eventString += address;
    eventString += F("#");
    eventString += channel;
    eventString += F("=");
    eventString += Par1;
    rulesProcessing(eventString);
    success = true;
  }
  Plugin_199_codeHash = Par2 + Par1;
}


//********************************************************************************
//  Decode generic protocol, deriving hash value
//********************************************************************************
boolean decodeUnknown()
{
  boolean success = false;
  byte Par1 = 0;
  unsigned long Par2 = 0;

  int x;
  unsigned int MinPulse = 0xffff;
  unsigned int MinSpace = 0xffff;
  unsigned long CodeM = 0L;
  unsigned long CodeS = 0L;

  if (Plugin_199_number < MIN_RAW_PULSES) return false;

  for (x = 5; x < Plugin_199_number - 2; x += 2)
  {
    if (Plugin_199_pulses[x]  < MinPulse)MinPulse = Plugin_199_pulses[x];
    if (Plugin_199_pulses[x + 1] < MinSpace)MinSpace = Plugin_199_pulses[x + 1];
  }

  MinPulse += (MinPulse * 100) / 100;
  MinSpace += (MinSpace * 100) / 100;

  // Data kan zowel in de mark als de space zitten. Daarom pakken we beide voor data opbouw.
  for (x = 3; x <= Plugin_199_number; x += 2)
  {
    CodeM = (CodeM << 1) | (Plugin_199_pulses[x]   > MinPulse);
    CodeS = (CodeS << 1) | (Plugin_199_pulses[x + 1] > MinSpace);
  }

  // Data kan zowel in de mark als de space zitten. We nemen de grootste waarde voor de data.
  if (CodeM > CodeS)
    Par2 = CodeM;
  else
    Par2 = CodeS;

  // valid signal, remember timestamp to suppress repeats...
  elapsed = millis() - Plugin_199_lastTime;
  Plugin_199_lastTime = millis();
  unsigned long codeHash = Par2;
  if (codeHash != Plugin_199_codeHash || (codeHash == Plugin_199_codeHash && elapsed > 250))
  {
    String eventString = F("UnknownRF_");
    eventString += Par2;
    rulesProcessing(eventString);
    success = true;
  }
  Plugin_199_codeHash = Par2;
}

#endif
