#ifndef DATASTRUCTS_PROVISIONINGSTRUCT_H
#define DATASTRUCTS_PROVISIONINGSTRUCT_H

#include "../../ESPEasy_common.h"

#if FEATURE_CUSTOM_PROVISIONING
# include "../CustomBuild/ESPEasyLimits.h"
# include "../DataStructs/FactoryDefaultPref.h"
# include "../DataTypes/ESPEasyFileType.h"

/*********************************************************************************************\
* ProvisioningStruct
\*********************************************************************************************/
struct ProvisioningStruct
{
  ProvisioningStruct();

  void validate();

  bool matchingFlashSize() const;

  bool setUser(const String& username);
  bool setPass(const String& password);
  bool setUrl(const String& url_str);

  bool fetchFileTypeAllowed(FileType::Enum filetype, unsigned int filenr) const;
  void setFetchFileTypeAllowed(FileType::Enum filetype, unsigned int filenr, bool checked);

  // its safe to extend this struct, up to 4096 bytes, default values in config are 0.
  uint8_t md5[16]        = { 0 };
  uint8_t ProgmemMd5[16] = { 0 }; // crc of the binary that last saved the struct to file.

  ResetFactoryDefaultPreference_struct ResetFactoryDefaultPreference;

  // Credentials + URL for fetching configuration from HTTP server
  char user[26] = { 0 };
  char pass[64] = { 0 };
  char url[128] = { 0 };

  union {
    uint16_t allowed{};
    struct {
      uint16_t allowFetchFirmware :1;
      uint16_t allowFetchConfigDat :1;
      uint16_t allowFetchSecurityDat :1;
      uint16_t allowFetchNotificationDat :1;
      uint16_t allowFetchProvisioningDat :1;
      uint16_t allowFetchRules :4;

      uint16_t unused :7;  // Add to use full 16 bit.
    } allowedFlags;
  };


};

typedef std::shared_ptr<ProvisioningStruct> ProvisioningStruct_ptr_type;
# define MakeProvisioningSettings(T) ProvisioningStruct_ptr_type ProvisioningStruct_ptr(new (std::nothrow)  ProvisioningStruct()); \
  ProvisioningStruct& T = *ProvisioningStruct_ptr;  \
  if (ProvisioningStruct_ptr) { memset(&T, 0, sizeof(ProvisioningStruct)); }

// Need to make sure every byte between the members is also zero
// Otherwise the checksum will fail and settings will be saved too often.
// The memset above is just for this.


// Check to see if MakeProvisioningSettings was successful
# define AllocatedProvisioningSettings() (ProvisioningStruct_ptr.get() != nullptr)


#endif // if FEATURE_CUSTOM_PROVISIONING

#endif // ifndef DATASTRUCTS_PROVISIONINGSTRUCT_H
