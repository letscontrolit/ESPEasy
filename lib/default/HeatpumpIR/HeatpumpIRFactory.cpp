#include <HeatpumpIRFactory.h>


HeatpumpIR* HeatpumpIRFactory::create(const char *modelName) {

  if (strcmp_P(modelName, PSTR("AUX")) == 0) {
    return new AUXHeatpumpIR();
  } else if (strcmp_P(modelName, PSTR("ballu")) == 0) {
    return new BalluHeatpumpIR();
  } else if (strcmp_P(modelName, PSTR("carrier_mca")) == 0) {
    return new CarrierMCAHeatpumpIR();
  } else if (strcmp_P(modelName, PSTR("carrier_nqv")) == 0) {
    return new CarrierNQVHeatpumpIR();
  } else if (strcmp_P(modelName, PSTR("daikin_arc417")) == 0) {
    return new DaikinHeatpumpARC417IR();
  } else if (strcmp_P(modelName, PSTR("daikin_arc480")) == 0) {
    return new DaikinHeatpumpARC480A14IR();
  } else if (strcmp_P(modelName, PSTR("daikin")) == 0) {
    return new DaikinHeatpumpIR();
  } else if (strcmp_P(modelName, PSTR("fuego")) == 0) {
    return new FuegoHeatpumpIR();
  } else if (strcmp_P(modelName, PSTR("fujitsu_awyz")) == 0) {
    return new FujitsuHeatpumpIR();
  } else if (strcmp_P(modelName, PSTR("gree")) == 0) {
    return new GreeGenericHeatpumpIR();
  } else if (strcmp_P(modelName, PSTR("greeyaa")) == 0) {
    return new GreeYAAHeatpumpIR();
  } else if (strcmp_P(modelName, PSTR("greeyan")) == 0) {
    return new GreeYANHeatpumpIR();
  } else if (strcmp_P(modelName, PSTR("hisense_aud")) == 0) {
    return new HisenseHeatpumpIR();
  } else if (strcmp_P(modelName, PSTR("hitachi")) == 0) {
    return new HitachiHeatpumpIR();
  } else if (strcmp_P(modelName, PSTR("hyundai")) == 0) {
    return new HyundaiHeatpumpIR();
  } else if (strcmp_P(modelName, PSTR("ivt")) == 0) {
    return new IVTHeatpumpIR();
  } else if (strcmp_P(modelName, PSTR("midea")) == 0) {
    return new MideaHeatpumpIR();
  } else if (strcmp_P(modelName, PSTR("mitsubishi_fa")) == 0) {
    return new MitsubishiFAHeatpumpIR();
  } else if (strcmp_P(modelName, PSTR("mitsubishi_fd")) == 0) {
    return new MitsubishiFDHeatpumpIR();
  } else if (strcmp_P(modelName, PSTR("mitsubishi_fe")) == 0) {
    return new MitsubishiFEHeatpumpIR();
  } else if (strcmp_P(modelName, PSTR("mitsubishi_heavy_fdtc")) == 0) {
    return new MitsubishiHeavyFDTCHeatpumpIR();
  } else if (strcmp_P(modelName, PSTR("mitsubishi_heavy_zj")) == 0) {
    return new MitsubishiHeavyZJHeatpumpIR();
  } else if (strcmp_P(modelName, PSTR("mitsubishi_heavy_zm")) == 0) {
    return new MitsubishiHeavyZMHeatpumpIR();
  } else if (strcmp_P(modelName, PSTR("mitsubishi_kj")) == 0) {
    return new MitsubishiKJHeatpumpIR();
  } else if (strcmp_P(modelName, PSTR("mitsubishi_msc")) == 0) {
    return new MitsubishiMSCHeatpumpIR();
  } else if (strcmp_P(modelName, PSTR("mitsubishi_msy")) == 0) {
    return new MitsubishiMSYHeatpumpIR();
  } else if (strcmp_P(modelName, PSTR("mitsubishi_sez")) == 0) {
    return new MitsubishiSEZKDXXHeatpumpIR();
  } else if (strcmp_P(modelName, PSTR("panasonic_ckp")) == 0) {
    return new PanasonicCKPHeatpumpIR();
  } else if (strcmp_P(modelName, PSTR("panasonic_dke")) == 0) {
    return new PanasonicDKEHeatpumpIR();
  } else if (strcmp_P(modelName, PSTR("panasonic_jke")) == 0) {
    return new PanasonicJKEHeatpumpIR();
  } else if (strcmp_P(modelName, PSTR("panasonic_lke")) == 0) {
    return new PanasonicLKEHeatpumpIR();
  } else if (strcmp_P(modelName, PSTR("panasonic_nke")) == 0) {
    return new PanasonicNKEHeatpumpIR();
  } else if (strcmp_P(modelName, PSTR("samsung_aqv")) == 0) {
    return new SamsungAQVHeatpumpIR();
  } else if (strcmp_P(modelName, PSTR("samsung_fjm")) == 0) {
    return new SamsungFJMHeatpumpIR();
  } else if (strcmp_P(modelName, PSTR("sharp")) == 0) {
    return new SharpHeatpumpIR();
  } else if (strcmp_P(modelName, PSTR("toshiba_daiseikai")) == 0) {
    return new ToshibaDaiseikaiHeatpumpIR();
  } else if (strcmp_P(modelName, PSTR("toshiba")) == 0) {
    return new ToshibaHeatpumpIR();
  }

   return NULL;
}