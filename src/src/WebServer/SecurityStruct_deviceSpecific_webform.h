#pragma once

#if FEATURE_STORE_CREDENTIALS_SEPARATE_FILE

# include "../WebServer/ESPEasy_key_value_store_webform.h"
# include "../DataStructs/SecurityStruct_deviceSpecific.h"

void show_SecurityStruct_deviceSpecific_WebFormItem(
  SecurityStruct_deviceSpecific::KeyType keytype,
  uint16_t                               index,
  bool                                   suppressIndex_displayString = false);

void store_SecurityStruct_deviceSpecific_WebFormItem(
  SecurityStruct_deviceSpecific::KeyType keytype,
  uint16_t                               index);


#endif // if FEATURE_STORE_CREDENTIALS_SEPARATE_FILE
