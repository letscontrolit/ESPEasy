#ifdef USES_P004
// #######################################################################################################
// #################################### Plugin 004: TempSensor Dallas DS18B20  ###########################
// #######################################################################################################

// Maxim Integrated (ex Dallas) DS18B20 datasheet : https://datasheets.maximintegrated.com/en/ds/DS18B20.pdf

#if defined(ESP32)
  #define ESP32noInterrupts() {portMUX_TYPE mux = portMUX_INITIALIZER_UNLOCKED;portENTER_CRITICAL(&mux)
  #define ESP32interrupts() portEXIT_CRITICAL(&mux);}
#endif

#define PLUGIN_004
#define PLUGIN_ID_004         4
#define PLUGIN_NAME_004       "Environment - DS18b20"
#define PLUGIN_VALUENAME1_004 "Temperature"
#define SENSOR_SELECTOR_ITEMS_MAX 32  // Change it if you have more ds18b20 devices
#define SENSORS_MAX 4 // count of sensors per plugin, must be no more than VARS_PER_TASK

int8_t Plugin_004_DallasPin;

boolean Plugin_004(byte function, struct EventStruct * event, String& string)
{
    boolean success = false;

    switch (function)
    {
        case PLUGIN_DEVICE_ADD:
        {
            Device[++deviceCount].Number           = PLUGIN_ID_004;
            Device[deviceCount].Type               = DEVICE_TYPE_SINGLE;
            Device[deviceCount].VType              = SENSOR_TYPE_SINGLE;
            Device[deviceCount].Ports              = 0;
            Device[deviceCount].PullUpOption       = false;
            Device[deviceCount].InverseLogicOption = false;
            Device[deviceCount].FormulaOption      = true;
            Device[deviceCount].ValueCount         = SENSORS_MAX;
            Device[deviceCount].SendDataOption     = true;
            Device[deviceCount].TimerOption        = true;
            Device[deviceCount].GlobalSyncOption   = true;
            break;
        }

        case PLUGIN_GET_DEVICENAME:
        {
            string = F(PLUGIN_NAME_004);
            break;
        }

        case PLUGIN_GET_DEVICEVALUENAMES:
        {
            for (byte j = 0; j < SENSORS_MAX; j++){
                String valueName = F(PLUGIN_VALUENAME1_004);
                valueName += " ";
                valueName += j;
                const char* tmp = valueName.c_str();
                strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[j], tmp);
            }
            break;
        }

        case PLUGIN_GET_DEVICEGPIONAMES:
          {
            event->String1 = formatGpioName_bidirectional(F("1-Wire"));
            break;
          }

        case PLUGIN_WEBFORM_LOAD:
        {
            uint8_t savedAddress[SENSORS_MAX][8] = {{ 0 }};
            byte resolutionChoice = 0;
            // Scan the onewire bus and fill dropdown list with devicecount on this GPIO.
            Plugin_004_DallasPin = Settings.TaskDevicePin1[event->TaskIndex];

            if (Plugin_004_DallasPin != -1){
                // get currently saved address
                LoadCustomTaskSettings(event->TaskIndex, (byte*)&savedAddress, sizeof(savedAddress));

                // find all suitable devices
                uint8_t allSensorsList[SENSOR_SELECTOR_ITEMS_MAX][8] = {{ 0 }};
                uint8_t zeroAddr[8] = { 0 };
                uint8_t tmpAddress[8] = { 0 };
                byte sensorsFound = 0;
                Plugin_004_DS_reset();
                Plugin_004_DS_reset_search();
                while (Plugin_004_DS_search(tmpAddress)){
                    memcpy(allSensorsList[sensorsFound],tmpAddress,8 );
                    sensorsFound++;
                    if (sensorsFound >= SENSOR_SELECTOR_ITEMS_MAX){
                        break;
                    }
                }

                // add device selectors
                for (byte dev = 0; dev < SENSORS_MAX; dev++){
                    String label = F("Device Address ");
                    label += (dev+1);
                    addRowLabel(label);
                    String head = F("p004_dev_");
                    head += dev;
                    addSelector_Head(head, false);
                    addSelector_Item("", -1, false, false, "");
                    for (byte item = 0; item < sensorsFound; item++){
                        String option = "";
                        for (byte j = 0; j < 8; j++)
                        {
                            option += String(allSensorsList[item][j], HEX);
                            if (j < 7) option += '-';
                        }
                        bool selected = (memcmp(allSensorsList[item], savedAddress[dev], 8) == 0) ? true : false;
                        addSelector_Item(option, item, selected, false, "");
                    }
                    addSelector_Foot();

                    // get resolutionChoice from first found device
                    if (0 == resolutionChoice && memcmp(zeroAddr, savedAddress[dev], 8) != 0)
                        resolutionChoice = Plugin_004_DS_getResolution(savedAddress[dev]);
                }

                // Device Resolution select is common for all devices and equals resolution of first found device
                if (0 == resolutionChoice)
                    resolutionChoice = 9;
                String resultsOptions[4] = { F("9"), F("10"), F("11"), F("12") };
                int resultsOptionValues[4] = { 9, 10, 11, 12 };
                addFormSelector(F("Device Resolution"), F("p004_res"), 4, resultsOptions, resultsOptionValues, resolutionChoice);
                addHtml(F(" Bit"));
            }
            success = true;
            break;
        }

        case PLUGIN_WEBFORM_SAVE:
        {
            uint8_t allAddr[SENSORS_MAX][8] = {{ 0 }};

            // save the address for selected device and store into extra tasksettings
            Plugin_004_DallasPin = Settings.TaskDevicePin1[event->TaskIndex];
            if (Plugin_004_DallasPin != -1){
              for (byte dev = 0; dev < SENSORS_MAX; dev++){
                  String devForm = F("p004_dev_");
                  devForm += dev;
                  Plugin_004_DS_scan(getFormItemInt(devForm), allAddr[dev]);
                  Plugin_004_DS_setResolution(allAddr[dev], getFormItemInt(F("p004_res")));
                  Plugin_004_DS_startConversion(allAddr[dev]);
              }
            }
            SaveCustomTaskSettings(event->TaskIndex, (byte*)&allAddr, sizeof(allAddr));
            success = true;
            break;
        }

        case PLUGIN_WEBFORM_SHOW_CONFIG:
        {
            uint8_t savedAddress[SENSORS_MAX][8] = {{ 0 }};
            uint8_t zeroAddr[8] = { 0 };
            LoadCustomTaskSettings(event->TaskIndex, (byte*)&savedAddress, sizeof(savedAddress));
            for (byte dev = 0; dev < SENSORS_MAX; dev++){
                if (dev > 0){
                    string += F("<BR>");
                }

                if (memcmp(zeroAddr, savedAddress[dev], 8) == 0 )
                    continue;

                for (byte x = 0; x < 8; x++)
                {
                    if (x != 0)
                        string += '-';
                        string += String(savedAddress[dev][x], HEX);
                }
            }
            success = true;
            break;
        }

        case PLUGIN_INIT:
        {
            Plugin_004_DallasPin = Settings.TaskDevicePin1[event->TaskIndex];
            if (Plugin_004_DallasPin != -1){
              uint8_t savedAddress[SENSORS_MAX][8] = {{ 0 }};
              uint8_t zeroAddr[8] = { 0 };
              LoadCustomTaskSettings(event->TaskIndex, (byte*)&savedAddress, sizeof(savedAddress));
              for (byte dev = 0; dev < SENSORS_MAX; dev++){
                  if (memcmp(zeroAddr, savedAddress[dev], 8) == 0 )
                    continue;
                  Plugin_004_DS_startConversion(savedAddress[dev]);
              }
              delay(800); //give it time to do intial conversion
            }
            success = true;
            break;
        }

        case PLUGIN_READ:
        {
            Plugin_004_DallasPin = Settings.TaskDevicePin1[event->TaskIndex];
            if (Plugin_004_DallasPin != -1){
                uint8_t savedAddress[SENSORS_MAX][8] = {{ 0 }};
                uint8_t zeroAddr[8] = { 0 };
                LoadCustomTaskSettings(event->TaskIndex, (byte*)&savedAddress, sizeof(savedAddress));

                byte devicesToRead = 0;
                for (byte dev = 0; dev < SENSORS_MAX; dev++){
                    if (memcmp(zeroAddr, savedAddress[dev], 8) == 0 ){
                        UserVar[event->BaseVarIndex + dev] = NAN;
                        continue;
                    }

                    devicesToRead += 1;
                    String log  = F("reading DS device #");
                    float value = 0;
                    log  += (dev +1);
                    log += F(" (");
                    for (byte x = 0; x < 8; x++)
                    {
                        if (x != 0)
                            log += '-';
                        log += String(savedAddress[dev][x], HEX);
                    }
                    log += F(" Temperature: ");

                    if (Plugin_004_DS_readTemp(savedAddress[dev], &value))
                    {
                        UserVar[event->BaseVarIndex + dev] = value;
                        log    += value;
                        success = true;
                    }
                    else
                    {
                        UserVar[event->BaseVarIndex + dev] = NAN;
                        log += F("Error!");
                    }
                    Plugin_004_DS_startConversion(savedAddress[dev]);
                    addLog(LOG_LEVEL_INFO, log);
                }

                switch (devicesToRead) {
                    case 1: {
                        event->sensorType = SENSOR_TYPE_SINGLE;
                        break;
                    }
                    case 2: {
                        event->sensorType = SENSOR_TYPE_DUAL;
                        break;
                    }
                    case 3: {
                        event->sensorType = SENSOR_TYPE_TRIPLE;
                        break;
                    }
                    case 4: {
                        event->sensorType = SENSOR_TYPE_QUAD;
                        break;
                    }
                    // case 5: {
                    //     event->sensorType = SENSOR_TYPE_PENTA;
                    //     break;	
                    // }
                    default: {
                        event->sensorType = SENSOR_TYPE_NONE;
                    }
                }
            }
            break;
        }
    }
    return success;
}

/*********************************************************************************************\
   Dallas Scan bus
  \*********************************************************************************************/
byte Plugin_004_DS_scan(byte getDeviceROM, uint8_t* ROM)
{
    byte tmpaddr[8];
    byte devCount = 0;
    Plugin_004_DS_reset();

    Plugin_004_DS_reset_search();
    while (Plugin_004_DS_search(tmpaddr))
    {
        if (getDeviceROM == devCount)
            for (byte  i = 0; i < 8; i++)
                ROM[i] = tmpaddr[i];
        devCount++;
    }
    return devCount;
}

/*********************************************************************************************\
*  Dallas Start Temperature Conversion, expected duration:
*    9 bits resolution ->  93.75 ms
*   10 bits resolution -> 187.5 ms
*   11 bits resolution -> 375 ms
*   12 bits resolution -> 750 ms
\*********************************************************************************************/
void Plugin_004_DS_startConversion(uint8_t ROM[8])
{
    Plugin_004_DS_reset();
    Plugin_004_DS_write(0x55); // Choose ROM
    for (byte i = 0; i < 8; i++)
        Plugin_004_DS_write(ROM[i]);
    Plugin_004_DS_write(0x44); // Take temperature mesurement
}

/*********************************************************************************************\
*  Dallas Read temperature from scratchpad
\*********************************************************************************************/
boolean Plugin_004_DS_readTemp(uint8_t ROM[8], float * value)
{
    int16_t DSTemp;
    byte ScratchPad[12];

    Plugin_004_DS_reset();
    Plugin_004_DS_write(0x55); // Choose ROM
    for (byte i = 0; i < 8; i++)
        Plugin_004_DS_write(ROM[i]);

    Plugin_004_DS_write(0xBE); // Read scratchpad

    for (byte i = 0; i < 9; i++) // read 9 bytes
        ScratchPad[i] = Plugin_004_DS_read();

    bool crc_ok = Plugin_004_DS_crc8(ScratchPad);

    if (loglevelActiveFor(LOG_LEVEL_DEBUG)) {
        String log = F("DS: SP: ");

        for (byte x = 0; x < 9; x++)
        {
            if (x != 0)
                log += ',';
            log += String(ScratchPad[x], HEX);
        }

        if (crc_ok)
            log += F(",OK");
        addLog(LOG_LEVEL_DEBUG, log);
    }

    if (!crc_ok)
    {
        *value = 0;
        return false;
    }

    if ((ROM[0] == 0x28 ) || (ROM[0] == 0x3b) || (ROM[0] == 0x22)) // DS18B20 or DS1825 or DS1822
    {
        DSTemp = (ScratchPad[1] << 8) + ScratchPad[0];
        if (DSTemp == 0x550)      // power-on reset value
           return false;
        *value = (float(DSTemp) * 0.0625);
    }
    else if (ROM[0] == 0x10) // DS1820 DS18S20
    {
        if (ScratchPad[0] == 0xaa)        // power-on reset value
          return false;
        DSTemp = (ScratchPad[1] << 11) | ScratchPad[0] << 3;
        DSTemp = ((DSTemp & 0xfff0) << 3) - 16 +
                 (((ScratchPad[7] - ScratchPad[6]) << 7) / ScratchPad[7]);
        *value = float(DSTemp) * 0.0078125;
    }
    return true;
}

/*********************************************************************************************\
* Dallas Get Resolution
\*********************************************************************************************/
int Plugin_004_DS_getResolution(uint8_t ROM[8])
{
    // DS1820 and DS18S20 have no resolution configuration register
    if (ROM[0] == 0x10) return 12;

    byte ScratchPad[12];

    Plugin_004_DS_reset();
    Plugin_004_DS_write(0x55); // Choose ROM
    for (byte i = 0; i < 8; i++)
        Plugin_004_DS_write(ROM[i]);

    Plugin_004_DS_write(0xBE); // Read scratchpad

    for (byte i = 0; i < 9; i++) // read 9 bytes
        ScratchPad[i] = Plugin_004_DS_read();

    if (!Plugin_004_DS_crc8(ScratchPad))
        return 0;
    else
    {
        switch (ScratchPad[4])
        {
            case 0x7F: // 12 bit
                return 12;

            case 0x5F: // 11 bit
                return 11;

            case 0x3F: // 10 bit
                return 10;

            case 0x1F: //  9 bit
            default:
                return 9;
        }
    }
    return(0);
}

/*********************************************************************************************\
* Dallas Get Resolution
\*********************************************************************************************/
boolean Plugin_004_DS_setResolution(uint8_t ROM[8], byte res)
{
    // DS1820 and DS18S20 have no resolution configuration register
    if (ROM[0] == 0x10) return true;

    byte ScratchPad[12];

    Plugin_004_DS_reset();
    Plugin_004_DS_write(0x55); // Choose ROM
    for (byte i = 0; i < 8; i++)
        Plugin_004_DS_write(ROM[i]);

    Plugin_004_DS_write(0xBE); // Read scratchpad

    for (byte i = 0; i < 9; i++) // read 9 bytes
        ScratchPad[i] = Plugin_004_DS_read();

    if (!Plugin_004_DS_crc8(ScratchPad))
        return false;
    else
    {
    	byte old_configuration = ScratchPad[4];

        switch (res)
        {
            case 12:
                ScratchPad[4] = 0x7F; // 12 bits
                break;
            case 11:
                ScratchPad[4] = 0x5F; // 11 bits
                break;
            case 10:
                ScratchPad[4] = 0x3F; // 10 bits
                break;
            case 9:
            default:
                ScratchPad[4] = 0x1F; //  9 bits
                break;
        }

        if (ScratchPad[4] == old_configuration)
        	return true;

        Plugin_004_DS_reset();
        Plugin_004_DS_write(0x55); // Choose ROM
        for (byte i = 0; i < 8; i++)
            Plugin_004_DS_write(ROM[i]);

        Plugin_004_DS_write(0x4E);          // Write to EEPROM
        Plugin_004_DS_write(ScratchPad[2]); // high alarm temp
        Plugin_004_DS_write(ScratchPad[3]); // low alarm temp
        Plugin_004_DS_write(ScratchPad[4]); // configuration register

        Plugin_004_DS_reset();
        Plugin_004_DS_write(0x55); // Choose ROM
        for (byte i = 0; i < 8; i++)
            Plugin_004_DS_write(ROM[i]);

        // save the newly written values to eeprom
        Plugin_004_DS_write(0x48);
        delay(100); // <--- added 20ms delay to allow 10ms long EEPROM write operation (as specified by datasheet)
        Plugin_004_DS_reset();

        return true; // new value set
    }
}

/*********************************************************************************************\
*  Dallas Reset
\*********************************************************************************************/
uint8_t Plugin_004_DS_reset()
{
    uint8_t r;
    uint8_t retries = 125;
    #if defined(ESP32)
      ESP32noInterrupts();
    #endif
    pinMode(Plugin_004_DallasPin, INPUT);
    do // wait until the wire is high... just in case
    {
        if (--retries == 0)
            return 0;
        delayMicroseconds(2);
    }
    while (!digitalRead(Plugin_004_DallasPin));

    pinMode(Plugin_004_DallasPin, OUTPUT); digitalWrite(Plugin_004_DallasPin, LOW);
    delayMicroseconds(480);               // Dallas spec. = Min. 480uSec. Arduino 500uSec.
    pinMode(Plugin_004_DallasPin, INPUT); // Float
    delayMicroseconds(70);
    r = !digitalRead(Plugin_004_DallasPin);
    delayMicroseconds(410);
    #if defined(ESP32)
      ESP32interrupts();
    #endif
    return r;
}

#define FALSE 0
#define TRUE  1

unsigned char ROM_NO[8];
uint8_t LastDiscrepancy;
uint8_t LastFamilyDiscrepancy;
uint8_t LastDeviceFlag;


/*********************************************************************************************\
*  Dallas Reset Search
\*********************************************************************************************/
void Plugin_004_DS_reset_search()
{
    // reset the search state
    LastDiscrepancy       = 0;
    LastDeviceFlag        = FALSE;
    LastFamilyDiscrepancy = 0;
    for (byte i = 0; i < 8; i++)
        ROM_NO[i] = 0;
}


/*********************************************************************************************\
*  Dallas Search bus
\*********************************************************************************************/
uint8_t Plugin_004_DS_search(uint8_t * newAddr)
{
    uint8_t id_bit_number;
    uint8_t last_zero, rom_byte_number, search_result;
    uint8_t id_bit, cmp_id_bit;
    unsigned char rom_byte_mask, search_direction;

    // initialize for search
    id_bit_number   = 1;
    last_zero       = 0;
    rom_byte_number = 0;
    rom_byte_mask   = 1;
    search_result   = 0;

    // if the last call was not the last one
    if (!LastDeviceFlag)
    {
        // 1-Wire reset
        if (!Plugin_004_DS_reset())
        {
            // reset the search
            LastDiscrepancy       = 0;
            LastDeviceFlag        = FALSE;
            LastFamilyDiscrepancy = 0;
            return FALSE;
        }

        // issue the search command
        Plugin_004_DS_write(0xF0);

        // loop to do the search
        do
        {
            // read a bit and its complement
            id_bit     = Plugin_004_DS_read_bit();
            cmp_id_bit = Plugin_004_DS_read_bit();

            // check for no devices on 1-wire
            if ((id_bit == 1) && (cmp_id_bit == 1))
                break;
            else
            {
                // all devices coupled have 0 or 1
                if (id_bit != cmp_id_bit)
                    search_direction = id_bit; // bit write value for search
                else
                {
                    // if this discrepancy if before the Last Discrepancy
                    // on a previous next then pick the same as last time
                    if (id_bit_number < LastDiscrepancy)
                        search_direction = ((ROM_NO[rom_byte_number] & rom_byte_mask) > 0);
                    else
                        // if equal to last pick 1, if not then pick 0
                        search_direction = (id_bit_number == LastDiscrepancy);

                    // if 0 was picked then record its position in LastZero
                    if (search_direction == 0)
                    {
                        last_zero = id_bit_number;

                        // check for Last discrepancy in family
                        if (last_zero < 9)
                            LastFamilyDiscrepancy = last_zero;
                    }
                }

                // set or clear the bit in the ROM byte rom_byte_number
                // with mask rom_byte_mask
                if (search_direction == 1)
                    ROM_NO[rom_byte_number] |= rom_byte_mask;
                else
                    ROM_NO[rom_byte_number] &= ~rom_byte_mask;

                // serial number search direction write bit
                Plugin_004_DS_write_bit(search_direction);

                // increment the byte counter id_bit_number
                // and shift the mask rom_byte_mask
                id_bit_number++;
                rom_byte_mask <<= 1;

                // if the mask is 0 then go to new SerialNum byte rom_byte_number and reset mask
                if (rom_byte_mask == 0)
                {
                    rom_byte_number++;
                    rom_byte_mask = 1;
                }
            }
        }
        while (rom_byte_number < 8); // loop until through all ROM bytes 0-7

        // if the search was successful then
        if (!(id_bit_number < 65))
        {
            // search successful so set LastDiscrepancy,LastDeviceFlag,search_result
            LastDiscrepancy = last_zero;

            // check for last device
            if (LastDiscrepancy == 0)
                LastDeviceFlag = TRUE;

            search_result = TRUE;
        }
    }

    // if no device found then reset counters so next 'search' will be like a first
    if (!search_result || !ROM_NO[0])
    {
        LastDiscrepancy       = 0;
        LastDeviceFlag        = FALSE;
        LastFamilyDiscrepancy = 0;
        search_result         = FALSE;
    }

    for (int i = 0; i < 8; i++)
        newAddr[i] = ROM_NO[i];

    return search_result;
}

/*********************************************************************************************\
*  Dallas Read byte
\*********************************************************************************************/
uint8_t Plugin_004_DS_read(void)
{
    uint8_t bitMask;
    uint8_t r = 0;

    for (bitMask = 0x01; bitMask; bitMask <<= 1)
        if (Plugin_004_DS_read_bit())
            r |= bitMask;

    return r;
}

/*********************************************************************************************\
*  Dallas Write byte
\*********************************************************************************************/
void Plugin_004_DS_write(uint8_t ByteToWrite)
{
    uint8_t bitMask;
    for (bitMask = 0x01; bitMask; bitMask <<= 1)
        Plugin_004_DS_write_bit( (bitMask & ByteToWrite) ? 1 : 0);
}

/*********************************************************************************************\
*  Dallas Read bit
\*********************************************************************************************/
uint8_t Plugin_004_DS_read_bit(void)
{
    uint8_t r;

    #if defined(ESP32)
       ESP32noInterrupts();
    #endif
    pinMode(Plugin_004_DallasPin, OUTPUT);
    digitalWrite(Plugin_004_DallasPin, LOW);
    delayMicroseconds(3);
    pinMode(Plugin_004_DallasPin, INPUT); // let pin float, pull up will raise
    delayMicroseconds(10);
    r = digitalRead(Plugin_004_DallasPin);
    #if defined(ESP32)
       ESP32interrupts();
    #endif
    delayMicroseconds(53);
    return r;
}

/*********************************************************************************************\
*  Dallas Write bit
\*********************************************************************************************/
void Plugin_004_DS_write_bit(uint8_t v)
{
    if (v & 1)
    {
        #if defined(ESP32)
          ESP32noInterrupts();
        #endif
        digitalWrite(Plugin_004_DallasPin, LOW);
        pinMode(Plugin_004_DallasPin, OUTPUT);
        delayMicroseconds(10);
        digitalWrite(Plugin_004_DallasPin, HIGH);
        #if defined(ESP32)
          ESP32interrupts();
        #endif
        delayMicroseconds(55);
    }
    else
    {
        #if defined(ESP32)
          ESP32noInterrupts();
        #endif
        digitalWrite(Plugin_004_DallasPin, LOW);
        pinMode(Plugin_004_DallasPin, OUTPUT);
        delayMicroseconds(65);
        digitalWrite(Plugin_004_DallasPin, HIGH);
        #if defined(ESP32)
           ESP32interrupts();
        #endif
        delayMicroseconds(5);
    }
}

/*********************************************************************************************\
*  Dallas Calculate CRC8 and compare it of addr[0-7] and compares it to addr[8]
\*********************************************************************************************/
boolean Plugin_004_DS_crc8(uint8_t * addr)
{
  uint8_t crc = 0;
  uint8_t len = 8;

    while (len--)
    {
        uint8_t inbyte = *addr++; // from 0 to 7
        for (uint8_t i = 8; i; i--)
        {
            uint8_t mix = (crc ^ inbyte) & 0x01;
            crc >>= 1;
            if (mix) crc ^= 0x8C;
            inbyte >>= 1;
        }
    }
    return crc == *addr; // addr 8
}
#endif // USES_P004
