#include "../PluginStructs/P103_data_struct.h"

#ifdef USES_P103

const __FlashStringHelper* toString(AtlasEZO_Sensors_e sensor) {
  switch (sensor) {
    case AtlasEZO_Sensors_e::PH: return F("pH (Potential of Hydrogen)");
    case AtlasEZO_Sensors_e::ORP: return F("ORP (Oxidation Reduction Potential)");
    case AtlasEZO_Sensors_e::EC: return F("EC (Electric conductivity)");
    case AtlasEZO_Sensors_e::DO: return F("DO (Dissolved Oxigen)");
    case AtlasEZO_Sensors_e::HUM: return F("HUM (Humidity)");
    # if P103_USE_RTD
    case AtlasEZO_Sensors_e::RTD: return F("RTD (Thermosensor)");
    # endif // if P103_USE_RTD
    # if P103_USE_FLOW
    case AtlasEZO_Sensors_e::FLOW: return F("FLOW (Flow meter)");
    # endif // if P103_USE_FLOW
    case AtlasEZO_Sensors_e::UNKNOWN: break;
  }
  return F("Unknown");
}

const __FlashStringHelper* P103_statusToString(char status) {
  switch (status) {
    case 'P':
      return F("powered off");
    case 'S':
      return F("software reset");
    case 'B':
      return F("brown out");
    case 'W':
      return F("watch dog");
    case 'U':
    default:
      break;
  }
  return F("unknown");
}

bool P103_send_I2C_command(uint8_t I2Caddress, const String& cmd, char *sensordata)
{
  sensordata[0] = '\0';

  uint16_t sensor_bytes_received = 0;

  uint8_t error;
  uint8_t i2c_response_code = 0;
  uint8_t in_char           = 0;

  # ifndef BUILD_NO_DEBUG

  if (loglevelActiveFor(LOG_LEVEL_DEBUG)) {
    addLogMove(LOG_LEVEL_DEBUG, concat(F("> cmd = "), cmd));
  }
  # endif // ifndef BUILD_NO_DEBUG
  Wire.beginTransmission(I2Caddress);

  for (size_t i = 0; i < cmd.length(); ++i)  {
    Wire.write(static_cast<uint8_t>(cmd[i]));
  }
  error = Wire.endTransmission();

  if (error != 0)
  {
    addLog(LOG_LEVEL_ERROR, F("Wire.endTransmission() returns error: Check Atlas shield, pH, ORP, EC, DO, HUM"
                              # if P103_USE_RTD
                              ", RTD"
                              # endif // if P103_USE_RTD
                              # if P103_USE_FLOW
                              ", FLOW"
                              # endif // if P103_USE_FLOW
                              " are supported."));
    return false;
  }

  // don't read answer if we want to go to sleep
  if (cmd.substring(0, 5).equalsIgnoreCase(F("Sleep")))
  {
    return true;
  }

  i2c_response_code = 254;

  while (i2c_response_code == 254)
  {
    Wire.requestFrom(I2Caddress, (uint8_t)(ATLAS_EZO_RETURN_ARRAY_SIZE - 1)); // call the circuit and request ATLAS_EZO_RETURN_ARRAY_SIZE -
                                                                              // 1 = 32 bytes (this is more then we need).
    i2c_response_code = Wire.read();                                          // read response code

    while (Wire.available())
    {                                                                         // read response
      in_char = Wire.read();

      if (in_char == 0)
      {   // if we receive a null character, we're done
        while (Wire.available())
        { // purge the data line if needed
          Wire.read();
        }

        break; // exit the while loop.
      }
      else
      {
        if (sensor_bytes_received >= ATLAS_EZO_RETURN_ARRAY_SIZE)
        {
          addLog(LOG_LEVEL_ERROR, F("< result array to short!"));
          return false;
        }
        sensordata[sensor_bytes_received] = in_char; // load this uint8_t into our array.
        sensor_bytes_received++;
      }
    }
    sensordata[sensor_bytes_received] = '\0';

    switch (i2c_response_code)
    {
      case 1:
        # ifndef BUILD_NO_DEBUG

        if (loglevelActiveFor(LOG_LEVEL_DEBUG)) {
          addLogMove(LOG_LEVEL_DEBUG, concat(F("< success, answer = "), String(sensordata)));
        }
        # endif // ifndef BUILD_NO_DEBUG
        break;

      case 2:
        # ifndef BUILD_NO_DEBUG
        addLog(LOG_LEVEL_DEBUG, F("< command failed"));
        # endif // ifndef BUILD_NO_DEBUG
        return false;

      case 254:
        # ifndef BUILD_NO_DEBUG
        addLog(LOG_LEVEL_DEBUG_MORE, F("< command pending"));
        # endif // ifndef BUILD_NO_DEBUG
        break;

      case 255:
        # ifndef BUILD_NO_DEBUG
        addLog(LOG_LEVEL_DEBUG, F("< no data"));
        # endif // ifndef BUILD_NO_DEBUG
        return false;
    }
  }

  return true;
}

int P103_getCalibrationPoints(uint8_t i2cAddress)
{
  int  nb_calibration_points = -1;
  char sensordata[ATLAS_EZO_RETURN_ARRAY_SIZE]{};

  if (P103_send_I2C_command(i2cAddress, F("Cal,?"), sensordata))
  {
    if (strncmp(sensordata, "?Cal,", 5))
    {
      char tmp[2];
      tmp[0]                = sensordata[5];
      tmp[1]                = '\0',
      nb_calibration_points = str2int(tmp);
    }
  }

  return nb_calibration_points;
}

void P103_html_color_message(const __FlashStringHelper *color, const String& message) {
  addHtml(F("&nbsp;<span style='color:"));
  addHtml(color);
  addHtml(F(";'>"));
  addHtml(message);
  addHtml(F("</span>"));
}

void P103_html_red(const String& message) {
  P103_html_color_message(F("red"), message);
}

void P103_html_orange(const String& message) {
  P103_html_color_message(F("orange"), message);
}

void P103_html_green(const String& message) {
  P103_html_color_message(F("green"), message);
}

void P103_addDisabler() {
  // disabler(): argument(s) on 'true' will set that checkbox to false! non-boolean ignores argument
  // addHtml(F("\n<script type='text/javascript'>"
  //           "function elId(e){return document.getElementById(e)}"
  //           "function chkd(e, y) { x = elId(e); if (x) x.checked = y }"
  //           "function disabler(clear,single,l,h,dry,nul,atm)"
  //           "{if (clear===true){chkd('en_cal_clear',false);}"
  //           "if (single===true){chkd('en_cal_single',false);}"
  //           "if(l===true){chkd('en_cal_L',false);}"
  //           "if(h===true){chkd('en_cal_H',false);}"
  //           "if(dry===true){chkd('en_cal_dry',false);}"
  //           "if(nul===true){chkd('en_cal_0',false);}"
  //           "if(atm===true){chkd('en_cal_atm',false);}};"
  //           "</script>"));
  // Minified:
  addHtml(F("\n<script type='text/javascript'>"
            "function elId(e){return document.getElementById(e)}"
            "function chkd(n,c){(x=elId(n))&&(x.checked=c)}"
            "function disabler(e,l,c,a,n,d,t)"
            "{!0===e&&(chkd('en_cal_clear',!1))"
            ",!0===l&&(chkd('en_cal_single',!1))"
            ",!0===c&&(chkd('en_cal_L',!1))"
            ",!0===a&&(chkd('en_cal_H',!1))"
            ",!0===n&&(chkd('en_cal_dry',!1))"
            ",!0===d&&(chkd('en_cal_0',!1))"
            ",!0===t&&(chkd('en_cal_atm',!1))};"
            "</script>"));
}

void P103_addClearCalibration()
{
  addRowLabel(F("<strong>Clear calibration</strong>"));
  addFormCheckBox(F("Clear"), F("en_cal_clear"), false);
  addHtml(F("\n<script type='text/javascript'>elId('en_cal_clear').onclick=function(){disabler(0,true,true,true,true,0,0)}</script>"));
  addFormNote(F("Attention! This will reset all calibrated data. New calibration will be needed!!!"));
}

int P103_addDOCalibration(uint8_t I2Cchoice)
{
  const int32_t nb_calibration_points = P103_getCalibrationPoints(I2Cchoice);

  addRowLabel("Calibrated Points");
  addHtmlInt(nb_calibration_points);

  if (nb_calibration_points < 1)
  {
    P103_html_red(F("   Calibration needed"));
  }

  addRowLabel(F("<strong>Calibrate to atmospheric oxygen levels</strong>"));
  addFormCheckBox(F("Enable"), F("en_cal_atm"), false);
  addHtml(F("\n<script type='text/javascript'>elId('en_cal_atm').onclick=function(){disabler(0,0,0,0,0,true,0)}</script>"));

  addRowLabel(F("<strong>Calibrate device to 0 dissolved oxygen</strong>"));
  addFormCheckBox(F("Enable"), F("en_cal_0"), false);
  addHtml(F("\n<script type='text/javascript'>elId('en_cal_0').onclick=function(){disabler(0,0,0,0,0,0,true)}</script>"));

  if (nb_calibration_points > 0)
  {
    P103_html_green(F("OK"));
  }
  else
  {
    P103_html_red(F("Not yet calibrated"));
  }

  return nb_calibration_points;
}

void P103_addCreateDryCalibration()
{
  addRowLabel(F("<strong>Dry calibration</strong>"));
  addFormCheckBox(F("Enable"), F("en_cal_dry"), false);
  addHtml(F("\n<script type='text/javascript'>elId('en_cal_dry').onclick=function(){disabler(true,true,true,true,0,0,0)}</script>"));
  addFormNote(F("Dry calibration must always be done first!"));
  addFormNote(F("Calibration for pH-Probe could be 1 (single) or 2 point (low, high)."));
}

int P103_addCreateSinglePointCalibration(AtlasEZO_Sensors_e  board_type,
                                         struct EventStruct *event,
                                         uint8_t             I2Cchoice,
                                         String              unit,
                                         float               min,
                                         float               max,
                                         uint8_t             nrDecimals,
                                         float               stepsize)
{
  const int32_t nb_calibration_points = P103_getCalibrationPoints(I2Cchoice);

  addRowLabel("Calibrated Points");
  addHtmlInt(nb_calibration_points);

  if (nb_calibration_points < 1)
  {
    P103_html_red(F("   Calibration needed"));
  }

  addRowLabel(F("<strong>Single point calibration</strong>"));
  addFormFloatNumberBox(F("Ref single point"), F("ref_cal_single'"), P103_CALIBRATION_SINGLE, min, max, nrDecimals, stepsize);
  addUnit(unit);

  if (((board_type != AtlasEZO_Sensors_e::EC) && (nb_calibration_points > 0)) ||
      ((board_type == AtlasEZO_Sensors_e::EC) && (nb_calibration_points == 1)))
  {
    P103_html_green(F("OK"));
  }
  else
  {
    if ((board_type == AtlasEZO_Sensors_e::EC) && (nb_calibration_points > 1))
    {
      P103_html_green(F("Not calibrated, because two point calibration is active."));
    }
    else
    {
      P103_html_red(F("Not yet calibrated"));
    }
  }
  addFormCheckBox(F("Enable"), F("en_cal_single"), false);
  addHtml(F("\n<script type='text/javascript'>elId('en_cal_single').onclick=function(){disabler(true,0,true,true,true,0,0)}</script>"));

  return nb_calibration_points;
}

int P103_addCreate3PointCalibration(AtlasEZO_Sensors_e  board_type,
                                    struct EventStruct *event,
                                    uint8_t             I2Cchoice,
                                    String              unit,
                                    float               min,
                                    float               max,
                                    uint8_t             nrDecimals,
                                    float               stepsize)
{
  int nb_calibration_points = P103_addCreateSinglePointCalibration(board_type, event, I2Cchoice, unit, min, max, nrDecimals, stepsize);

  addRowLabel(F("<strong>Low calibration</strong>"));
  addFormFloatNumberBox(F("Ref low point"), F("ref_cal_L"), P103_CALIBRATION_LOW, min, max, nrDecimals, stepsize);
  addUnit(unit);

  if (nb_calibration_points > 1)
  {
    P103_html_green(F("OK"));
  }
  else
  {
    P103_html_orange(F("Not yet calibrated"));
  }
  addFormCheckBox(F("Enable"), F("en_cal_L"), false);
  addHtml(F("\n<script type='text/javascript'>elId('en_cal_L').onclick=function(){disabler(true,true,0,true,true,0,0)}</script>"));

  addRowLabel(F("<strong>High calibration</strong>"));
  addFormFloatNumberBox(F("Ref high point"), F("ref_cal_H"), P103_CALIBRATION_HIGH, min, max, nrDecimals, stepsize);
  addUnit(unit);

  // pH: low, high OK with 3 calibration points (single is the first one); EC: low high OK with 2 calibration points
  if ((nb_calibration_points > 2) || ((board_type == AtlasEZO_Sensors_e::EC) && (nb_calibration_points > 1)))
  {
    P103_html_green(F("OK"));
  }
  else
  {
    P103_html_orange(F("Not yet calibrated"));
  }
  addFormCheckBox(F("Enable"), F("en_cal_H"), false);
  addHtml(F("\n<script type='text/javascript'>elId('en_cal_H').onclick=function(){disabler(true,true,true,0,true,0,0)}</script>"));

  return nb_calibration_points;
}

bool P103_getHUMOutputOptions(struct EventStruct *event,
                              bool              & _HUMhasHum,
                              bool              & _HUMhasTemp,
                              bool              & _HUMhasDew) {
  bool result = false;

  char boarddata[ATLAS_EZO_RETURN_ARRAY_SIZE]{};

  if ((result = P103_send_I2C_command(P103_I2C_ADDRESS, F("O,?"), boarddata))) {
    String outputs(boarddata);
    int    o      = 2;
    String outPar = parseString(outputs, o);

    while (!outPar.isEmpty()) {
      if (equals(outPar, F("hum"))) {
        _HUMhasHum = true;
      } else
      if (equals(outPar, F("t"))) {
        _HUMhasTemp = true;
      } else
      if (equals(outPar, F("dew"))) {
        _HUMhasDew = true;
      }
      o++;
      outPar = parseString(outputs, o);
    }
  }
  return result;
}

#endif  // ifdef USES_P103
