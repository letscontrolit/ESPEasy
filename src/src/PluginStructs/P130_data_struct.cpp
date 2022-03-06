#include "../PluginStructs/P130_data_struct.h"

#ifdef USES_P130

#define P130_ADS1X15_REG_POINTER_CONVERT         (0x00)
#define P130_ADS1X15_REG_POINTER_CONFIG          (0x01)
#define P130_ADS1X15_REG_POINTER_THRESH_LO       (0x02)
#define P130_ADS1X15_REG_POINTER_THRESH_HI       (0x03)

#define P130_ADS1015_MUX_DIFF_01            (0x0000)
#define P130_ADS1015_MUX_DIFF_23            (0x3000)
#define P130_ADS1015_PGA_1_024V             (0x0600) //< +/-1.024V range = Gain 4 => datasheet says 1bit = 0.5mV
#define P130_ADS1015_CONFIG_CQUE_NONE       (0x0003) // Disable the comparator (default val)
#define P130_ADS1015_CONFIG_CLAT_NONLAT     (0x0000) // Non-latching (default val)
#define P130_ADS1015_CONFIG_CPOL_ACTVLOW    (0x0000) // Alert/Rdy active low   (default val)
#define P130_ADS1015_CONFIG_CMODE_TRAD      (0x0000) // Traditional comparator (default val)
#define P130_ADS1015_RATE_3300SPS           (0x00C0) // < 3300 samples per second
#define P130_ADS1015_RATE_2400SPS           (0x00A0) // < 2400 samples per second
#define P130_ADS1015_RATE_1600SPS           (0x0080) // < 1600 samples per second
#define P130_ADS1015_CONV_MODE_CONTINUOUS   (0x0000) // Single-shot mode (default)
#define P130_ADS1015_CONV_MODE_SINGLE       (0x0100) // Single-shot mode (default)
#define P130_ADS1015_START_SINGLE_CONV      (0x8000) // Start a single conversion

P130_data_struct::P130_data_struct(uint8_t i2c_addr, uint8_t _calCurrent1, uint8_t _calCurrent2, float_t _calVoltage, uint8_t _currentFreq, uint8_t _nbSinus, uint8_t _convModeContinuous) : 
    i2cAddress(i2c_addr),
    calCurrent1(_calCurrent1),
    calCurrent2(_calCurrent2),
    calVoltage(_calVoltage),
    currentFreq(_currentFreq),
    nbSinus(_nbSinus),
    convModeContinuous (_convModeContinuous) {
        this->debug = 0;
    }

void P130_data_struct::setDebug(uint8_t _debug) {
    this->debug = _debug;
    if (this->debug) {
        String log;
        log = F("Irms - ADS1015 : Set internal plugin debug; _calCurrent1=");
        log += this->calCurrent1;
        log += F(" _calCurrent2=");
        log += this->calCurrent2;
        log += F(" voltEst.=");
        log += String(this->calVoltage);
        log += F(" frequency=");
        log += this->currentFreq;
        log += F(" nbSinus=");
        log += this->nbSinus;
        addLog(LOG_LEVEL_INFO, log);
    }
}

uint8_t P130_data_struct::getDebug() {
    return this->debug;
}

boolean P130_data_struct::writeRegister(uint8_t registerAddr, uint16_t registerValue) {
    boolean result = false;
    uint8_t res = 0;
    Wire.beginTransmission(this->i2cAddress);
    Wire.write((uint8_t)(registerAddr));
    Wire.write((uint8_t)(registerValue >> 8));
    Wire.write((uint8_t)(registerValue & 0xFF));
    res = Wire.endTransmission();
    if ( 0 == res ) {
        result = true;
    } else {
        String log = F("Irms - ADS1015 : writeRegister NOK, errcode= ");
        log += res ;
        addLog(LOG_LEVEL_INFO, log);
        result = false;
    }
    return result;
}

boolean P130_data_struct::readRegister(uint8_t registerAddr, uint16_t& registerValue) {
    boolean result = false;
    const uint8_t requestedByte = 2;
    Wire.beginTransmission(this->i2cAddress);
    Wire.write((uint8_t)(registerAddr));
    if ( 0 == Wire.endTransmission() ) {
        if ( requestedByte == Wire.requestFrom(this->i2cAddress, requestedByte) ) {
            result = true;
            registerValue = ((Wire.read() << 8) | Wire.read());
        }
    }
    return result;
}

uint16_t P130_data_struct::readRegisterFacility(uint8_t registerAddr) {
    uint16_t registerValue = 0;
    this->readRegister(registerAddr, registerValue);
    return registerValue;
}

uint16_t P130_data_struct::getDefaultADS1015ReadConfig(){
    return P130_ADS1015_CONFIG_CQUE_NONE |
            P130_ADS1015_CONFIG_CLAT_NONLAT |
            P130_ADS1015_CONFIG_CPOL_ACTVLOW |
            P130_ADS1015_CONFIG_CMODE_TRAD |
            P130_ADS1015_RATE_1600SPS |
            P130_ADS1015_PGA_1_024V;

}

boolean P130_data_struct::readAdcSingleValue(uint16_t muxConf, int16_t& adcValue) {
    boolean result = false;
    uint16_t registerValue = 0;
    uint16_t configGlobal = P130_data_struct::getDefaultADS1015ReadConfig();
    configGlobal |= muxConf;

    uint16_t configStartSingle = configGlobal | 
                        P130_ADS1015_CONV_MODE_SINGLE |
                        P130_ADS1015_START_SINGLE_CONV;
    uint16_t configStartContinuous = configGlobal | 
                        P130_ADS1015_CONV_MODE_CONTINUOUS |
                        P130_ADS1015_START_SINGLE_CONV;
    uint16_t configStop = configGlobal | 
                        P130_ADS1015_CONV_MODE_SINGLE;

    // write conf+ =>start reading
    if ( false == this->writeRegister(P130_ADS1X15_REG_POINTER_CONFIG, configStartSingle) ) {
        addLog(LOG_LEVEL_INFO, F("readadcsingle write register conf failed"));
        result = false;
    } else {
        // delay(9); // See https://github.com/letscontrolit/ESPEasy/issues/3159#issuecomment-660546091
        // requesting if conversion ended
        ulong readingLongUS = micros();
        while ( micros() < (readingLongUS + 1000) ) {
            if ( ( this->readRegisterFacility(P130_ADS1X15_REG_POINTER_CONFIG) & 0x8000) != 0 ) {
                if ( true == this->readRegister(P130_ADS1X15_REG_POINTER_CONVERT, registerValue) ) {
                    // Shift 12-bit results right 4 bits for the ADS1015,
                    // making sure we keep the sign bit intact
                    adcValue = ((int16_t)(registerValue >> 4));
                    if (adcValue > 0x07FF) {
                    // negative number - extend the sign to 16th bit
                        adcValue |= 0xF000;
                    }
                    result = true;
                }
                break;
            }
        }
    }
    return result;
}

boolean P130_data_struct::readAdcContinuousRmsValue(uint16_t muxConf, uint16_t period_ms, float_t& adcIrms, uint16_t& nbSample) {
    boolean success = false;
    boolean pbmWhileReading = false;
    uint16_t registerValue = 0;
    uint64_t sumI = 0, sqrI = 0;
    adcIrms = 0.;
    nbSample = 0;
    uint16_t configGlobal = P130_data_struct::getDefaultADS1015ReadConfig();
    configGlobal |= muxConf;

    uint16_t configStartSingle = configGlobal | 
                        P130_ADS1015_CONV_MODE_SINGLE |
                        P130_ADS1015_START_SINGLE_CONV;
    uint16_t configStartContinuous = configGlobal | 
                        P130_ADS1015_CONV_MODE_CONTINUOUS |
                        P130_ADS1015_START_SINGLE_CONV;
    uint16_t configStop = configGlobal | 
                        P130_ADS1015_CONV_MODE_SINGLE;

    // write conf+ =>start reading
    if ( false == this->writeRegister(P130_ADS1X15_REG_POINTER_CONFIG, configStartContinuous) ) {
        addLog(LOG_LEVEL_INFO, F("readadccontinuous write register conf failed"));
        success = false;
    } else {
        // delay(9); // See https://github.com/letscontrolit/ESPEasy/issues/3159#issuecomment-660546091
        // requesting if conversion ended
        pbmWhileReading = false;
        ulong endPeriod = millis() + period_ms;
        while ( millis() < endPeriod ) {
            if ( true != this->readRegister(P130_ADS1X15_REG_POINTER_CONVERT, registerValue) ) {
                pbmWhileReading = true;
                break;
            } else {
                nbSample++;
                // Shift 12-bit results right 4 bits for the ADS1015,
                // making sure we keep the sign bit intact
                int16_t adcValue = ((int16_t)(registerValue >> 4));
                if (adcValue > 0x07FF) {
                // negative number - extend the sign to 16th bit
                    adcValue |= 0xF000;
                }
                sqrI = adcValue * adcValue;
                sumI += sqrI;
                #ifdef P130_DEBUG_DEV
                String log = "readadccontinuous registerValue=" + String(registerValue);
                log += " adcValue=" + String(adcValue);
                log += " sqrI=" + String((unsigned long)(sqrI >>16), HEX) + String((unsigned long)(sqrI & 0x0000FFFF), HEX);
                log += " sumI=" + String((unsigned long)(sumI >>16), HEX) + String((unsigned long)(sumI & 0x0000FFFF), HEX);
                addLog(LOG_LEVEL_INFO, log);
                #endif
            }
        }
        if ( true == pbmWhileReading ) {
            success = false;
        } else {
            adcIrms = sqrt(1. * sumI / nbSample);
            success = true;
        }
    }
    if ( false == this->writeRegister(P130_ADS1X15_REG_POINTER_CONFIG, configStop) ) {
        addLog(LOG_LEVEL_INFO, F("readadccontinuous stop reading failed"));
    }
    return success;
}

float_t P130_data_struct::estimatePower(float_t current) {
    return calVoltage * current;
}

boolean P130_data_struct::readCurrent(uint8_t canal, float_t& currentValue) {
    boolean success = false;
    boolean calculatingRMSSuccess = true;
    uint16_t registerValue = 0;
    int16_t adc = 0;
    uint64_t sumI = 0;
    float_t adcIrms = .0;
    uint16_t nbSample = 0;
    uint16_t durationSample = (this->nbSinus * (1000 / this->currentFreq));
    float_t voltage = .0;
    float_t calCurrent =.0;
    unsigned long endTime = 0;
    if ( ( 1 != canal ) && ( 2 != canal ) )
        success = false;
    else {
        uint16_t muxConf = P130_ADS1015_MUX_DIFF_01;
        if ( 1 == canal )
            muxConf = P130_ADS1015_MUX_DIFF_01;
        else if ( 2 == canal )
            muxConf = P130_ADS1015_MUX_DIFF_23;
        else
            muxConf = P130_ADS1015_MUX_DIFF_01; //default value, should never happened

        if ( 1 != this->convModeContinuous ) {
            // start loop
            endTime = millis() + durationSample; // 2*20ms => 5sinus
            calculatingRMSSuccess = true;
            while ( millis() < endTime ) {
                if ( false == this->readAdcSingleValue(muxConf, adc) ) {
                    currentValue = 0;
                    calculatingRMSSuccess = false;
                    break;
                } else {
                    sumI += (adc * adc);
                    nbSample++;
                }
            }
            if ( true == calculatingRMSSuccess ) {
                adcIrms = sqrt(sumI / nbSample);
                voltage = adcIrms * 0.0005;
                calCurrent = (1==canal)?calCurrent1:calCurrent2;
                currentValue = voltage * calCurrent;
                success = true;
            }
        } else {
            adcIrms = 0.;
            nbSample = 0;
            success = this->readAdcContinuousRmsValue(muxConf, durationSample, adcIrms, nbSample);
            voltage = adcIrms * 0.0005;
            calCurrent = (1==canal)?calCurrent1:calCurrent2;
            currentValue = voltage * calCurrent;
        }
        if (false != this->debug) {
            String log;
            log = String("Irms - ADS1015 : "); log += ((true == success)?String("SUCCESS"):String("FAILED"));
            log += String(" readCurrent canal = "); log += canal; addLog(LOG_LEVEL_INFO, log);
            log = F("Irms - ADS1015 : Differential square/root: "); log += adcIrms; addLog(LOG_LEVEL_INFO, log);
            log = F("Irms - ADS1015 :  (voltage: "); log += voltage + String("mV )"); addLog(LOG_LEVEL_INFO, log);
            log = F("Irms - ADS1015 :  (sample duration: "); log += durationSample + String("ms )"); addLog(LOG_LEVEL_INFO, log);
            log = F("Irms - ADS1015 :  (sample count: "); log += nbSample + String(" )"); addLog(LOG_LEVEL_INFO, log);
            log = F("Irms - ADS1015 : Current: "); log += currentValue; addLog(LOG_LEVEL_INFO, log);
        }
    }
    return success;
}

#endif // ifdef USES_P130
