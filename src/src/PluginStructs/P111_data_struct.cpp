#include "../PluginStructs/P111_data_struct.h"

#ifdef USES_P111

#include "../PluginStructs/P111_data_struct.h"
// Needed also here for PlatformIO's library finder as the .h file 
// is in a directory which is excluded in the src_filter

#include <MFRC522.h>

P111_data_struct::P111_data_struct(byte csPin, byte rstPin) : mfrc522(nullptr), _csPin(csPin), _rstPin(rstPin)
{}

void P111_data_struct::init() {
  if (mfrc522 == nullptr){
    mfrc522 = new MFRC522 (_csPin, _rstPin);   // Instantiate a MFRC522
    mfrc522->PCD_Init();  // Initialize MFRC522 reader
  }
}

/**
 * read status and tag
 */
byte P111_data_struct::readCardStatus(unsigned long *key, bool *removedTag) {

  byte error = 0;

  uint8_t uid[] = { 0, 0, 0, 0, 0, 0, 0 };
  uint8_t uidLength;

  error = readPassiveTargetID(uid, &uidLength);

  switch(error) {
    case 1: // Read error
    {
      errorCount++;
      removedState = false;
      String log = F("MFRC522: Read error: ");
      log += errorCount;
      addLog(LOG_LEVEL_ERROR, log);
      break;
    } 
    case 2: // No tag found
      if (!removedState) {
        removedState = true;
        *removedTag  = true;
        error = 0; // pass through that removal just once
      }
      errorCount = 0;
      break;
    default:  // Read a tag correctly
      errorCount = 0;
      removedState = false; // No longer removed
      break;
  }

  if (errorCount > 2) { // if three consecutive read errors, reset MFRC522
    reset(_csPin,_rstPin);
  }
  unsigned long tmpKey = uid[0];
  for (uint8_t i = 1; i < 4; i++) {
    tmpKey <<= 8;
    tmpKey += uid[i];
  }
  *key = tmpKey;

  return error;
}

/**
 * Returns last read card (type) name
 */
String P111_data_struct::getCardName() {
  return mfrc522->PICC_GetTypeName(mfrc522->PICC_GetType(mfrc522->uid.sak));
}

/*********************************************************************************************\
 * MFRC522 init
\*********************************************************************************************/
bool P111_data_struct::reset(int8_t csPin, int8_t resetPin) {
  if (resetPin != -1) {
    String log = F("MFRC522: Reset on pin: ");
    log += resetPin;
    addLog(LOG_LEVEL_INFO, log);
    pinMode(resetPin, OUTPUT);
    digitalWrite(resetPin, LOW);
    delay(100);
    digitalWrite(resetPin, HIGH);
    pinMode(resetPin, INPUT_PULLUP);
    delay(10);
  }

  pinMode(csPin, OUTPUT);
  digitalWrite(csPin, LOW);
    
  mfrc522->PCD_Init(csPin, resetPin);   // Init MFRC522 module

  //If you set Antenna Gain to Max it will increase reading distance
  mfrc522->PCD_SetAntennaGain(mfrc522->RxGain_max);
  
  bool result = mfrc522->PCD_PerformSelfTest(); // perform the test
  
  if (result) {
    //String log = F("RC522: Found");
    // Get the MFRC522 software version
    byte v = mfrc522->PCD_ReadRegister(mfrc522->VersionReg);
    
    // When 0x00 or 0xFF is returned, communication probably failed
    if ((v == 0x00) || (v == 0xFF)) {
      String log=F("MFRC522: Communication failure, is the MFRC522 properly connected?");
      addLog(LOG_LEVEL_ERROR,log);
      return false;
    } else {
      String log=F("MFRC522: Software Version: ");
      if (v == 0x91)
        log+=F(" = v1.0");
      else if (v == 0x92)
        log+=F(" = v2.0");
      else
        log+=F(" (unknown),probably a chinese clone?");
            
      addLog(LOG_LEVEL_INFO, log);
    }
    return true;
  }
  return false;
}

/*********************************************************************************************\
 * RC522 read tag ID
\*********************************************************************************************/
byte P111_data_struct::readPassiveTargetID(uint8_t *uid, uint8_t *uidLength) { //needed ? see above (not PN532)
  // Getting ready for Reading PICCs
  if ( ! mfrc522->PICC_IsNewCardPresent()) { //If a new PICC placed to RFID reader continue
    return 2;
  }
  addLog(LOG_LEVEL_INFO, F("MFRC522: New Card Detected"));
  if ( ! mfrc522->PICC_ReadCardSerial()) {   //Since a PICC placed get Serial and continue
    return 1;
  }
  
  // There are Mifare PICCs which have 4 byte or 7 byte UID care if you use 7 byte PICC
  // I think we should assume every PICC as they have 4 byte UID
  // Until we support 7 byte PICCs
  addLog(LOG_LEVEL_INFO, F("MFRC522: Scanned PICC's UID"));
  for (uint8_t i = 0; i < 4; i++) {  //
    uid[i] = mfrc522->uid.uidByte[i];
  }
  *uidLength = 4;
  mfrc522->PICC_HaltA(); // Stop reading
  return 0;
}


#endif // ifdef USES_P111
