#include "../Helpers/_CPlugin_Helper_LoRa.h"

#if defined(USES_C018) || defined(USES_C023)

# include "../DataTypes/FormSelectorOptions.h"
# include "../Helpers/StringConverter.h"
# include "../WebServer/HTML_wrappers.h"

/*************************************************
* LoRaWAN Join Method
*************************************************/
const __FlashStringHelper * LoRa_Helper::toString(LoRaWAN_JoinMethod joinMethod)
{
  switch (joinMethod)
  {
    case LoRaWAN_JoinMethod::OTAA: return F("OTAA");
    case LoRaWAN_JoinMethod::ABP:  return F("ABP");
  }
  return F("");
}

void LoRa_Helper::addLoRaWAN_JoinMethod_FormSelector(
  const __FlashStringHelper *label,
  const __FlashStringHelper *id,
  LoRaWAN_JoinMethod         selectedIndex)
{
  const __FlashStringHelper *options[] = {
    toString(LoRaWAN_JoinMethod::OTAA),
    toString(LoRaWAN_JoinMethod::ABP)
  };
  constexpr int values[] {
    static_cast<int>(LoRaWAN_JoinMethod::OTAA),
    static_cast<int>(LoRaWAN_JoinMethod::ABP)
  };
  FormSelectorOptions selector(NR_ELEMENTS(options), options, values);

  // Script to toggle OTAA/ABP fields visibility when changing selection.
  selector.onChangeCall = F("joinChanged(this)");

  selector.addFormSelector(label, id, static_cast<int>(selectedIndex));
  html_add_script(
    strformat(
      F("document.getElementById('%s').onchange();"),
      FsP(id)),
    false);
}

void LoRa_Helper::add_joinChanged_script_element_line(const String& id, LoRaWAN_JoinMethod joinMethod)
{
  addHtml(F("document.getElementById('tr_"));
  addHtml(id);
  addHtml(F("').style.display = style"));
  addHtml(toString(joinMethod));
  addHtml(';');
}

/*************************************************
* LoRaWAN Class
*************************************************/
void LoRa_Helper::addLoRaWANclass_FormSelector(
  const __FlashStringHelper *label,
  const __FlashStringHelper *id,
  LoRaWANclass_e             selectedIndex)
{
  const __FlashStringHelper *options[] = { F("A"), F("C") };
  constexpr int values[] {
    0,
    2
  };
  const FormSelectorOptions selector(NR_ELEMENTS(options), options, values);

  selector.addFormSelector(label, id, static_cast<int>(selectedIndex));
}

/*************************************************
* LoRaWAN Downlink Event Format
*************************************************/
void LoRa_Helper::addEventFormatStructure_FormSelector(
  const __FlashStringHelper *label,
  const __FlashStringHelper *id,
  DownlinkEventFormat_e      selectedIndex)
{
  const __FlashStringHelper *options[] = {
    F("PortNr in EventPar"),
    F("PortNr as 1st EventValue"),
    F("PortNr both EventPar & 1st EventValue")
  };
  constexpr int values[] {
    static_cast<int>(DownlinkEventFormat_e::PortNr_in_eventPar),
    static_cast<int>(DownlinkEventFormat_e::PortNr_as_first_eventvalue),
    static_cast<int>(DownlinkEventFormat_e::PortNr_both_eventPar_eventvalue)
  };
  const FormSelectorOptions selector(NR_ELEMENTS(options), options, values);

  selector.addFormSelector(label, id, static_cast<int>(selectedIndex));
}

/*************************************************
* LoRaWAN DataRate
*************************************************/
const __FlashStringHelper * LoRa_Helper::toString(LoRaWAN_DR dr)
{
  switch (dr)
  {
    case LoRaWAN_DR::SF12_BW125: return F("SF12 BW125");
    case LoRaWAN_DR::SF11_BW125: return F("SF11 BW125");
    case LoRaWAN_DR::SF10_BW125: return F("SF10 BW125");
    case LoRaWAN_DR::SF9_BW125:  return F("SF9 BW125");
    case LoRaWAN_DR::SF8_BW125:  return F("SF8 BW125");
    case LoRaWAN_DR::SF7_BW125:  return F("SF7 BW125");
    case LoRaWAN_DR::SF7_BW250:  return F("SF7 BW250");
    case LoRaWAN_DR::FSK:        return F("FSK");
    case LoRaWAN_DR::ADR:        return F("Adaptive Data Rate (ADR)");
  }
  return F("");
}

uint8_t LoRa_Helper::getSF(LoRaWAN_DR dr)
{
  switch (dr)
  {
    case LoRaWAN_DR::SF12_BW125: return 12;
    case LoRaWAN_DR::SF11_BW125: return 11;
    case LoRaWAN_DR::SF10_BW125: return 10;
    case LoRaWAN_DR::SF9_BW125:  return 9;
    case LoRaWAN_DR::SF8_BW125:  return 8;
    case LoRaWAN_DR::SF7_BW125:  return 7;
    case LoRaWAN_DR::SF7_BW250:  return 7;
    case LoRaWAN_DR::FSK:        return 0; // TODO TD-er: What is the SF equivalent here?
    case LoRaWAN_DR::ADR:        return 0;
  }
  return 0;
}

uint16_t LoRa_Helper::getBW(LoRaWAN_DR dr)
{
  if (dr == LoRaWAN_DR::SF7_BW250) { return 250; }

  if (dr == LoRaWAN_DR::FSK) { return 250; } // TODO TD-er: Is this correct?
  return 125;
}

void LoRa_Helper::addLoRaWAN_DR_FormSelector(
  const __FlashStringHelper *label,
  const __FlashStringHelper *id,
  LoRaWAN_DR                 selectedIndex)
{
  const __FlashStringHelper *options[] = {
    toString(LoRaWAN_DR::ADR),
    toString(LoRaWAN_DR::SF7_BW250),
    toString(LoRaWAN_DR::SF7_BW125),
    toString(LoRaWAN_DR::SF8_BW125),
    toString(LoRaWAN_DR::SF9_BW125),
    toString(LoRaWAN_DR::SF10_BW125),
    toString(LoRaWAN_DR::SF11_BW125),
    toString(LoRaWAN_DR::SF12_BW125)
  };
  constexpr int values[] {
    static_cast<int>(LoRaWAN_DR::ADR),
    static_cast<int>(LoRaWAN_DR::SF7_BW250),
    static_cast<int>(LoRaWAN_DR::SF7_BW125),
    static_cast<int>(LoRaWAN_DR::SF8_BW125),
    static_cast<int>(LoRaWAN_DR::SF9_BW125),
    static_cast<int>(LoRaWAN_DR::SF10_BW125),
    static_cast<int>(LoRaWAN_DR::SF11_BW125),
    static_cast<int>(LoRaWAN_DR::SF12_BW125)
  };
  const FormSelectorOptions selector(NR_ELEMENTS(options), options, values);

  selector.addFormSelector(label, id, static_cast<int>(selectedIndex));
}

/*************************************************
* LoRaWAN Air Time
*************************************************/
float LoRa_Helper::getLoRaAirTime(uint8_t pl, LoRaWAN_DR dr)
{
  if (dr == LoRa_Helper::LoRaWAN_DR::ADR) { return -1.0f; }
  uint8_t sf               = getSF(dr); // Spreading factor 7 - 12
  const uint16_t bw        = getBW(dr); // Bandwidth 125 kHz default for LoRaWAN. 250 kHz also supported.
  uint8_t cr               = 1;         // Code Rate 4 / (CR + 4) = 4/5.  4/5 default for LoRaWAN
  const uint8_t n_preamble = 8;         // Preamble length Default for frame = 8, beacon = 10
  const bool    header     = true;      // Explicit header Default on for LoRaWAN
  const bool    crc        = true;      // CRC Default on for LoRaWAN

  if (sf > 12) {
    sf = 12;
  } else if (sf < 7) {
    sf = 7;
  }

  if (cr > 4) {
    cr = 4;
  } else if (cr < 1) {
    cr = 1;
  }

  // Symbols in frame
  int payload_length = 8;
  {
    int beta_offset = 28;

    if (crc) { beta_offset += 16; }

    if (!header) { beta_offset -= 20; }
    float beta_f                  = 8.0f * pl - 4.0f * sf + beta_offset;
    bool  lowDataRateOptimization = (bw == 125 && sf >= 11);

    if (lowDataRateOptimization) {
      beta_f = beta_f / (4.0f * (sf - 2));
    } else {
      beta_f = beta_f / (4.0f * sf);
    }
    int beta = static_cast<int>(beta_f + 1.0f); // ceil

    if (beta > 0) {
      payload_length += (beta * (cr + 4));
    }
  }

  // t_symbol and t_air in msec
  const float t_symbol = static_cast<float>(1 << sf) / bw;
  const float t_air    = ((n_preamble + 4.25f) + payload_length) * t_symbol;
  return t_air;
}

#endif // if defined(USES_C018) || defined(USES_C023)
