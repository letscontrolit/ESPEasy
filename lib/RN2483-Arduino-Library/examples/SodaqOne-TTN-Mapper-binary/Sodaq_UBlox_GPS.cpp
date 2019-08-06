/*
* Copyright (c) 2016 SODAQ. All rights reserved.
*
* This file is part of Sodaq_UBlox_GPS.
*
* Sodaq_UBlox_GPS is free software: you can redistribute it and/or modify
* it under the terms of the GNU Lesser General Public License as published
* by the Free Software Foundation, either version 3 of the License, or (at
* your option) any later version.
*
* Sodaq_UBlox_GPS is distributed in the hope that it will be useful, but
* WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
* or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public
* License for more details.
*
* You should have received a copy of the GNU Lesser General Public License
* along with Sodaq_UBlox_GPS.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <Arduino.h>
#include <Wire.h>
#include "Sodaq_UBlox_GPS.h"

#define DEBUG 1
#ifdef DEBUG
#define debugPrintLn(...) { if (this->_diagStream) this->_diagStream->println(__VA_ARGS__); }
#define debugPrint(...) { if (this->_diagStream) this->_diagStream->print(__VA_ARGS__); }
#else
#define debugPrintLn(...)
#define debugPrint(...)
#endif

static const size_t INPUT_BUFFER_SIZE = 128;    // TODO Check UBlox manual ReceiverDescProtSpec
static const uint8_t UBlox_I2C_addr = 0x42;

#define GPS_ENABLE_ON   HIGH
#define GPS_ENABLE_OFF  LOW

Sodaq_UBlox_GPS sodaq_gps;
const char Sodaq_UBlox_GPS::_fieldSep = ',';

static inline bool is_timedout(uint32_t from, uint32_t nr_ms) __attribute__((always_inline));
static inline bool is_timedout(uint32_t from, uint32_t nr_ms)
{
    return (millis() - from) > nr_ms;
}

Sodaq_UBlox_GPS::Sodaq_UBlox_GPS()
{
    _enablePin = -1;

    _diagStream = 0;
    _addr = UBlox_I2C_addr;

    _minNumOfLines = 0;
    _minNumSatellites = 0;

    resetValues();

    _trans_active = false;
    _inputBuffer = static_cast<char*>(malloc(INPUT_BUFFER_SIZE));
    _inputBufferSize = INPUT_BUFFER_SIZE;
}

void Sodaq_UBlox_GPS::resetValues()
{
    _seenLatLon = false;
    _seenAlt = false;
    _numSatellites = 0;
    _lat = 0;
    _lon = 0;

    _seenTime = false;
    _hh = 0;
    _mm = 0;
    _ss = 0;
    _yy = 0;
    _MM = 0;
    _dd = 0;
}

void Sodaq_UBlox_GPS::init(int8_t enable_pin)
{
    _enablePin = enable_pin;
    Wire.begin();
    digitalWrite(_enablePin, GPS_ENABLE_OFF);
    pinMode(_enablePin, OUTPUT);
}

/*!
 * Read the UBlox device until a fix is seen, or until
 * a timeout has been reached.
 */
bool Sodaq_UBlox_GPS::scan(bool leave_on, uint32_t timeout)
{
    bool retval = false;
    uint32_t start = millis();
    resetValues();

    on();
    delay(500);         // TODO Is this needed?

    size_t fix_count = 0;
    while (!is_timedout(start, timeout)) {
        if (!readLine()) {
            // TODO Maybe quit?
            continue;
        }
        parseLine(_inputBuffer);

        // Which conditions are required to quit the scan?
        if (_seenLatLon
                && _seenTime
                && _seenAlt
                && (_minNumSatellites == 0 || _numSatellites >= _minNumSatellites)) {
            ++fix_count;
            if (fix_count >= _minNumOfLines) {
                retval = true;
                break;
            }
        }
    }

    if (_numSatellites > 0) {
        debugPrintLn(String("[scan] num sats = ") + _numSatellites);
    }
    if (_seenTime) {
        debugPrintLn(String("[scan] datetime = ") + getDateTimeString());
    }
    if (_seenLatLon) {
        debugPrintLn(String("[scan] lat = ") + String(_lat, 7));
        debugPrintLn(String("[scan] lon = ") + String(_lon, 7));
    }

    if (!leave_on) {
        off();
    }
    return retval;
}

String Sodaq_UBlox_GPS::getDateTimeString()
{
    return num2String(getYear(), 4)
            + num2String(getMonth(), 2)
            + num2String(getDay(), 2)
            + num2String(getHour(), 2)
            + num2String(getMinute(), 2)
            + num2String(getSecond(), 2);
}

bool Sodaq_UBlox_GPS::parseLine(const char * line)
{
    //debugPrintLn(String("= ") + line);
    if (!computeCrc(line, false)) {
        // Redo the check, with logging
        computeCrc(line, true);
        return false;
    }
    String data = line + 1;
    data.remove(data.length() - 3, 3);  // Strip checksum *<hex><hex>

    if (data.startsWith("GPGGA")) {
        return parseGPGGA(data);
    }

    if (data.startsWith("GPGSA")) {
        return parseGPGSA(data);
    }

    if (data.startsWith("GPRMC")) {
        return parseGPRMC(data);
    }

    if (data.startsWith("GPGSV")) {
        return parseGPGSV(data);
    }

    if (data.startsWith("GPGLL")) {
        return parseGPGLL(data);
    }

    if (data.startsWith("GPVTG")) {
        return parseGPVTG(data);
    }

    if (data.startsWith("GPTXT")) {
        return parseGPTXT(data);
    }

    debugPrintLn(String("?? >> ") + line);
    return false;
}

/*!
 * Read the coordinates using $GPGGA
 * See also section 24.3 of u-blox 7, Receiver Description. Document number: GPS.G7-SW-12001-B
 *
 * See section "Decode of selected position sentences" at
 *   http://www.gpsinformation.org/dale/nmea.htm
 * The most important NMEA sentences include the GGA which provides the
 * current Fix data, the RMC which provides the minimum gps sentences
 * information, and the GSA which provides the Satellite status data.
 *
 * 0    $GPGGA
 * 1    time            hhmmss.ss       UTC time
 * 2    lat             ddmm.mmmmm      Latitude (degrees & minutes)
 * 3    NS              char            North/South indicator
 * 4    long            dddmm.mmmmm     Longitude (degrees & minutes)
 * 5    EW              char            East/West indicator
 * 6    quality         digit           Quality indicator for position fix: 0 No Fix, 6 Estimated, 1 Auto GNSS, 2 Diff GNSS
 * 7    numSV           num             Number of satellites used
 * 8    HDOP            num             Horizontal Dilution of Precision
 * 9    alt             num             Altitude above mean sea level
 * 10   uAlt            char            Altitude units: meters
 * 11   sep             num             Geoid separation: difference between geoid and mean sea level
 * 12   uSep            char            Separation units: meters
 * 13   diffAge         num             Age of differential corrections
 * 14   diffStation     num             ID of station providing differential corrections
 * 15   checksum        2 hex digits
 */
bool Sodaq_UBlox_GPS::parseGPGGA(const String & line)
{
    debugPrintLn("parseGPGGA");
    debugPrintLn(String(">> ") + line);
    if (getField(line, 6) != "0") {
        _lat = convertDegMinToDecDeg(getField(line, 2));
        if (getField(line, 3) == "S") {
            _lat = -_lat;
        }
        _lon = convertDegMinToDecDeg(getField(line, 4));
        if (getField(line, 5) == "W") {
            _lon = -_lon;
        }
        _seenLatLon = true;

        _hdop = getField(line, 8).toFloat();
        if(getField(line, 10) == "M") {
          _alt = getField(line, 9).toFloat();
          _seenAlt = true;
        }
    }

    _numSatellites = getField(line, 7).toInt();
    return true;
}

/*!
 * Parse GPGSA line
 * GNSS DOP and Active Satellites
 */
bool Sodaq_UBlox_GPS::parseGPGSA(const String & line)
{
    // Not (yet) used
    debugPrintLn("parseGPGSA");
    debugPrintLn(String(">> ") + line);
    return false;
}

/*!
 * Read the coordinates using $GPRMC
 * See also section 24.13 of u-blox 7, Receiver Description. Document number: GPS.G7-SW-12001-B
 *
 * 0    $GPRMC
 * 1    time            hhmmss.ss       UTC time
 * 2    status          char            Status, V = Navigation receiver warning, A = Data valid
 * 3    lat             ddmm.mmmmm      Latitude (degrees & minutes)
 * 4    NS              char            North/South
 * 5    long            dddmm.mmmmm     Longitude (degrees & minutes)
 * 6    EW              char            East/West
 * 7    spd             num             Speed over ground
 * 8    cog             num             Course over ground
 * 9    date            ddmmyy          Date in day, month, year format
 * 10   mv              num             Magnetic variation value
 * 11   mvEW            char            Magnetic variation E/W indicator
 * 12   posMode         char            Mode Indicator: 'N' No Fix, 'E' Estimate, 'A' Auto GNSS, 'D' Diff GNSS
 * 13   checksum        2 hex digits    Checksum
 */
bool Sodaq_UBlox_GPS::parseGPRMC(const String & line)
{
    debugPrintLn("parseGPRMC");
    debugPrintLn(String(">> ") + line);

    if (getField(line, 2) == "A" && getField(line, 12) != "N") {
        _lat = convertDegMinToDecDeg(getField(line, 3));
        if (getField(line, 4) == "S") {
            _lat = -_lat;
        }
        _lon = convertDegMinToDecDeg(getField(line, 5));
        if (getField(line, 6) == "W") {
            _lon = -_lon;
        }
        _seenLatLon = true;
    }

    String time = getField(line, 1);
    String date = getField(line, 9);
    setDateTime(date, time);

    return true;
}

/*!
 * Parse GPGSV line
 * See also section 24.12 of u-blox 7, Receiver Description. Document number: GPS.G7-SW-12001-B
 *
 * 0    $GPGSV
 * 1    numMsg          digit   Number of messages, total number of GSV messages being output
 * 2    msgNum          digit   Number of this message
 * 3    numSV           num     Number of satellites in view
 *
 * 4    sv              num     Satellite ID
 * 5    elv             num     Elevation (range 0-90)
 * 6    az              num     Azimuth, (range 0-359)
 * 7    cno             num     Signal strength (C/N0, range 0-99), blank when not tracking
 *
 * fields 4..7 are repeated for each satellite in this message
 */
bool Sodaq_UBlox_GPS::parseGPGSV(const String & line)
{
    debugPrintLn("parseGPGSV");
    debugPrintLn(String(">> ") + line);

    // We could/should only use msgNum == 1. However, all messages should have
    // the same numSV.
    _numSatellites = getField(line, 3).toInt();

    return true;
}

/*!
 * Parse GPGLL line
 * Latitude and longitude, with time of position fix and status
 */
bool Sodaq_UBlox_GPS::parseGPGLL(const String & line)
{
    // Not (yet) used
    debugPrintLn("parseGPGLL");
    debugPrintLn(String(">> ") + line);
    return false;
}

/*!
 * Parse GPVTG line
 * Course over ground and Ground speed
 */
bool Sodaq_UBlox_GPS::parseGPVTG(const String & line)
{
    // Not (yet) used
    debugPrintLn("parseGPVTG");
    debugPrintLn(String(">> ") + line);
    return false;
}

/*!
 * Parse GPTXT line
 * See also section 24.13 of u-blox 7, Receiver Description. Document number: GPS.G7-SW-12001-B
 *   $GPTXT,01,01,02,ANTSTATUS=INIT*25
 *   ...
 *   $GPTXT,01,01,02,ANTSTATUS=OK*3B
 *
 * 0    $GPTXT
 * 1    numMsg          num             Total number of messages in this transmission
 * 2    msgNum          num             Message number in this transmission
 * 3    msgType         num             Text identifier, 00: Error, 01: Warning, 02: Notice, 07: User
 * 4    text            string          Any ASCII text
 * 13   checksum        2 hex digits    Checksum
 */
bool Sodaq_UBlox_GPS::parseGPTXT(const String & line)
{
    //debugPrintLn("parseGPTXT");
    //debugPrintLn(String(">> ") + line);
    debugPrintLn(String("TXT: \"") + getField(line, 4) + "\"");
    return true;
}

/*!
 * Compute and verify the checksum
 *
 * Each line must start with '$'
 * Each line must end with '*' <hex> <hex>
 */
bool Sodaq_UBlox_GPS::computeCrc(const char * line, bool do_logging)
{
    if (do_logging) {
        debugPrint(line);
    }
    size_t len = strlen(line);
    if (len < 4) {
        if (do_logging) {
            debugPrint("  Invalid short: ");
            debugPrintLn(len);
        }
        return false;
    }
    if (line[0] != '$') {
        if (do_logging) {
            debugPrintLn("  Invalid$");
        }
        return false;
    }
    if (line[len - 3] != '*') {
        if (do_logging) {
            debugPrintLn("  Invalid*");
        }
        return false;
    }

    uint8_t crc = 0;
    for (size_t i = 1; i < len - 3; ++i) {
        crc ^= line[i];
    }

    uint8_t crc1 = getHex2(line, len - 2);
    if (crc != crc1) {
        if (do_logging) {
            debugPrint("  INVALID CRC ");
            debugPrint(crc1, HEX);
            debugPrint("  EXPECTED ");
            debugPrintLn(crc, HEX);
        }
        return false;
    }

    //debugSerial.println("  OK");
    return true;
}

uint8_t Sodaq_UBlox_GPS::getHex2(const char * s, size_t index)
{
    uint8_t val = 0;
    char c;
    c = s[index];
    if (c >= '0' && c <= '9') {
        val += c - '0';
    } else if (c >= 'a' && c <= 'f') {
        val += c - 'a' + 10;
    } else if (c >= 'A' && c <= 'F') {
        val += c - 'A' + 10;
    }
    val <<= 4;
    c = s[++index];
    if (c >= '0' && c <= '9') {
        val += c - '0';
    } else if (c >= 'a' && c <= 'f') {
        val += c - 'a' + 10;
    } else if (c >= 'A' && c <= 'F') {
        val += c - 'A' + 10;
    }
    return val;
}

String Sodaq_UBlox_GPS::num2String(int num, size_t width)
{
    String out;
    out = num;
    while (out.length() < width) {
        out = String("0") + out;
    }
    return out;
}

String Sodaq_UBlox_GPS::getField(const String & data, int index)
{
    int found = 0;
    int strIndex[] = { 0, -1 };
    int maxIndex = data.length() - 1;

    for (int i = 0; i <= maxIndex && found <= index; i++) {
        if (data.charAt(i) == _fieldSep || i == maxIndex) {
            found++;
            strIndex[0] = strIndex[1] + 1;
            strIndex[1] = (i == maxIndex) ? i + 1 : i;
        }
    }

    return found > index ? data.substring(strIndex[0], strIndex[1]) : "";
}

/*
 * Convert lat/long degree-minute format to decimal-degrees
 *
 * This code is from
 *   http://arduinodev.woofex.net/2013/02/06/adafruit_gps_forma/
 *
 * According to the NMEA Standard, Latitude and Longitude are output in the format Degrees, Minutes and
 * (Decimal) Fractions of Minutes. To convert to Degrees and Fractions of Degrees, or Degrees, Minutes, Seconds
 * and Fractions of seconds, the 'Minutes' and 'Fractional Minutes' parts need to be converted. In other words: If
 * the GPS Receiver reports a Latitude of 4717.112671 North and Longitude of 00833.914843 East, this is
 *   Latitude 47 Degrees, 17.112671 Minutes
 *   Longitude 8 Degrees, 33.914843 Minutes
 * or
 *   Latitude 47 Degrees, 17 Minutes, 6.76026 Seconds
 *   Longitude 8 Degrees, 33 Minutes, 54.89058 Seconds
 * or
 *   Latitude 47.28521118 Degrees
 *   Longitude 8.56524738 Degrees
 */
double Sodaq_UBlox_GPS::convertDegMinToDecDeg(const String & data)
{
    double degMin = data.toFloat();
    double min = 0.0;
    double decDeg = 0.0;

    //get the minutes, fmod() requires double
    min = fmod((double) degMin, 100.0);

    //rebuild coordinates in decimal degrees
    degMin = (int) (degMin / 100);
    decDeg = degMin + (min / 60);

    return decDeg;
}

void Sodaq_UBlox_GPS::setDateTime(const String & date, const String & time)
{
    if (time.length() == 9 && date.length() == 6) {
        _hh = time.substring(0, 2).toInt();
        _mm = time.substring(2, 4).toInt();
        _ss = time.substring(4, 6).toInt();
        _dd = date.substring(0, 2).toInt();
        _MM = date.substring(2, 4).toInt();
        _yy = date.substring(4, 6).toInt();
        _seenTime = true;
    }
}

/*!
 * Read one NMEA frame
 */
bool Sodaq_UBlox_GPS::readLine(uint32_t timeout)
{
    if (!_inputBuffer) {
        return false;
    }

    uint32_t start = millis();
    char c;
    char *ptr = _inputBuffer;
    size_t cnt = 0;
    *ptr = '\0';

    c = 0;
    while (!is_timedout(start, timeout)) {
        c = (char)read();
        if (c == '$') {
            break;
        }
    }
    if (c != '$') {
        return false;
    }
    *ptr++ = c;
    ++cnt;

    c = 0;
    while (!is_timedout(start, timeout)) {
        c = (char)read();
        if (c == '\r') {
            continue;
        }
        if (c == '\n') {
            break;
        }
        if (cnt < _inputBufferSize - 1) {
            *ptr++ = c;
            ++cnt;
        }
    }
    *ptr = '\0';
    if (c != '\n') {
        return false;
    }
    endTransmission();
    return true;
}

uint8_t Sodaq_UBlox_GPS::read()
{
    beginTransmission();

    uint8_t b = 0xFF;
    uint8_t nr_bytes;
    nr_bytes = Wire.requestFrom(_addr, 1, false);
    if (nr_bytes == 1) {
        b = Wire.read();
    }
    return b;
}

void Sodaq_UBlox_GPS::beginTransmission()
{
    if (_trans_active) {
        return;
    }
    Wire.beginTransmission(_addr);
    _trans_active = true;
}

void Sodaq_UBlox_GPS::endTransmission()
{
    if (!_trans_active) {
        return;
    }
    Wire.endTransmission();
    _trans_active = false;
}

void Sodaq_UBlox_GPS::on()
{
    digitalWrite(_enablePin, GPS_ENABLE_ON);
}

void Sodaq_UBlox_GPS::off()
{
    digitalWrite(_enablePin, GPS_ENABLE_OFF);
}
