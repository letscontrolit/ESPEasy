#include "rn2xx3_datatypes.h"


RN2xx3_datatypes::Model RN2xx3_datatypes::intToModel(int modelId)
{
  switch (modelId) {
    case 2903: return RN2xx3_datatypes::Model::RN2903;
    case 2483: return RN2xx3_datatypes::Model::RN2483;
    default:
      break;
  }
  return RN2xx3_datatypes::Model::RN_NA;
}

RN2xx3_datatypes::Model RN2xx3_datatypes::parseVersion(const String& version, RN2xx3_datatypes::Firmware& firmware)
{
  int model_int                 = version.substring(2, 6).toInt();
  RN2xx3_datatypes::Model model = RN2xx3_datatypes::intToModel(model_int);

  String fw_rev = version.substring(7, 12);

  fw_rev.replace(".", "");
  int fw_rev_int = fw_rev.toInt();
  firmware = RN2xx3_datatypes::Firmware::unknown;

  if (fw_rev_int != 0) {
    firmware = RN2xx3_datatypes::Firmware::pre_1_0_1;

    switch (fw_rev_int) {
      case 101: firmware = RN2xx3_datatypes::Firmware::rev1_0_1; break;
      case 102: firmware = RN2xx3_datatypes::Firmware::rev1_0_2; break;
      case 103: firmware = RN2xx3_datatypes::Firmware::rev1_0_3; break;
      case 104: firmware = RN2xx3_datatypes::Firmware::rev1_0_4; break;
      case 105: firmware = RN2xx3_datatypes::Firmware::rev1_0_5; break;
      default:
        firmware = RN2xx3_datatypes::Firmware::unknown;
        break;
    }
  }
  return model;
}
