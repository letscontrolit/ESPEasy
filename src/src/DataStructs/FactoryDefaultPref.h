#ifndef DATASTRUCTS_FACTORY_DEFAULT_PREF_H
#define DATASTRUCTS_FACTORY_DEFAULT_PREF_H

#include "../DataTypes/DeviceModel.h"

#include <stdint.h>

#include "../../ESPEasy_common.h"

#ifdef ESP32
# include "../Helpers/ESPEasy_NVS_Helper.h"
#endif // ifdef ESP32

struct ResetFactoryDefaultPreference_struct {
  ResetFactoryDefaultPreference_struct();
  ResetFactoryDefaultPreference_struct(uint32_t preference);

  void set(uint32_t preference);

#ifdef ESP32
  bool init(ESPEasy_NVS_Helper& preferences);
  bool from_NVS(ESPEasy_NVS_Helper& preferences);

  void to_NVS(ESPEasy_NVS_Helper& preferences) const;
#endif // ifdef ESP32

private:

  union {
    struct {
      uint32_t deviceModel            : 8;
      uint32_t unused_bit8            : 1;
      uint32_t keepWiFi               : 1;
      uint32_t keepNTP                : 1;
      uint32_t keepNetwork            : 1;
      uint32_t keepLogConsoleSettings : 1;
      uint32_t keepUnitName           : 1;
      uint32_t fetchRulesFile         : 4;
      uint32_t fetchNotificationDat   : 1;
      uint32_t fetchSecurityDat       : 1;
      uint32_t fetchConfigDat         : 1;
      uint32_t deleteFirst            : 1;
      uint32_t saveURL                : 1;
      uint32_t delete_Bak_Files       : 1;
      uint32_t storeCredentials       : 1;
      uint32_t fetchProvisioningDat   : 1;
      uint32_t keepCustomCdnUrl       : 1;

      uint32_t unused : 5;
    }        bits;
    uint32_t _preference{};
  };

public:

  DeviceModel getDeviceModel() const {
    return static_cast<DeviceModel>(bits.deviceModel);
  }

  void setDeviceModel(DeviceModel model) {
    bits.deviceModel = static_cast<uint32_t>(model);
  }

  bool keepWiFi() const {
    return bits.keepWiFi;
  }

  void keepWiFi(bool keep) {
    bits.keepWiFi = keep;
  }

  bool keepNTP() const {
    return bits.keepNTP;
  }

  void keepNTP(bool keep) {
    bits.keepNTP = keep;
  }

  bool keepNetwork() const  {
    return bits.keepNetwork;
  }

  void keepNetwork(bool keep) {
    bits.keepNetwork = keep;
  }

  bool keepLogConsoleSettings() const  {
    return bits.keepLogConsoleSettings;
  }

  void keepLogConsoleSettings(bool keep) {
    bits.keepLogConsoleSettings = keep;
  }

  bool keepUnitName() const  {
    return bits.keepUnitName;
  }

  void keepUnitName(bool keep) {
    bits.keepUnitName = keep;
  }

  bool keepCustomCdnUrl() const  {
    return bits.keepCustomCdnUrl;
  }

  void keepCustomCdnUrl(bool keep) {
    bits.keepCustomCdnUrl = keep;
  }

  // filenr = 0...3 for files rules1.txt ... rules4.txt
  bool fetchRulesTXT(int filenr) const {
    return bitRead(bits.fetchRulesFile, filenr);
  }

  void fetchRulesTXT(int filenr, bool fetch) {
    bitWrite(bits.fetchRulesFile, filenr, fetch);
  }

  bool fetchNotificationDat() const {
    return bits.fetchNotificationDat;
  }

  void fetchNotificationDat(bool fetch) {
    bits.fetchNotificationDat = fetch;
  }

  bool fetchSecurityDat() const {
    return bits.fetchSecurityDat;
  }

  void fetchSecurityDat(bool fetch) {
    bits.fetchSecurityDat = fetch;
  }

  bool fetchConfigDat() const {
    return bits.fetchConfigDat;
  }

  void fetchConfigDat(bool fetch) {
    bits.fetchConfigDat = fetch;
  }

  bool fetchProvisioningDat() const {
    return bits.fetchProvisioningDat;
  }

  void fetchProvisioningDat(bool fetch) {
    bits.fetchProvisioningDat = fetch;
  }

  bool deleteFirst() const {
    return bits.deleteFirst;
  }

  void deleteFirst(bool checked) {
    bits.deleteFirst = checked;
  }

  bool delete_Bak_Files() const {
    return bits.delete_Bak_Files;
  }

  void delete_Bak_Files(bool checked) {
    bits.delete_Bak_Files = checked;
  }

  bool saveURL() const {
    return bits.saveURL;
  }

  void saveURL(bool checked) {
    bits.saveURL = checked;
  }

  bool storeCredentials() const {
    return bits.storeCredentials;
  }

  void storeCredentials(bool checked) {
    bits.storeCredentials = checked;
  }

  uint32_t getPreference() const {
    return _preference;
  }

  // TODO TD-er: Add extra flags for settings to keep/set when reset to default.
};


#endif // DATASTRUCTS_FACTORY_DEFAULT_PREF_H
