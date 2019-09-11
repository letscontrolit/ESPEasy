#include "../../ESPEasy_common.h"
#include "../DataStructs/FactoryDefaultPref.h"

ResetFactoryDefaultPreference_struct::ResetFactoryDefaultPreference_struct(uint32_t preference) : _preference(preference) {}

DeviceModel ResetFactoryDefaultPreference_struct::getDeviceModel() const {
  return static_cast<DeviceModel>(_preference & 0xFF);
}

void ResetFactoryDefaultPreference_struct::setDeviceModel(DeviceModel model) {
  _preference &= ~(0xFF); // set DeviceModel bits to 0
  _preference |= model;
}

bool ResetFactoryDefaultPreference_struct::keepWiFi() const {
  return getBitFromUL(_preference, 9);
}

void ResetFactoryDefaultPreference_struct::keepWiFi(bool keep) {
  setBitToUL(_preference, 9, keep);
}

bool ResetFactoryDefaultPreference_struct::keepNTP() const {
  return getBitFromUL(_preference, 10);
}

void ResetFactoryDefaultPreference_struct::keepNTP(bool keep) {
  setBitToUL(_preference, 10, keep);
}

bool ResetFactoryDefaultPreference_struct::keepNetwork() const {
  return getBitFromUL(_preference, 11);
}

void ResetFactoryDefaultPreference_struct::keepNetwork(bool keep) {
  setBitToUL(_preference, 11, keep);
}

bool ResetFactoryDefaultPreference_struct::keepLogSettings() const {
  return getBitFromUL(_preference, 12);
}

void ResetFactoryDefaultPreference_struct::keepLogSettings(bool keep) {
  setBitToUL(_preference, 12, keep);
}

bool ResetFactoryDefaultPreference_struct::keepUnitName() const {
  return getBitFromUL(_preference, 13);
}

void ResetFactoryDefaultPreference_struct::keepUnitName(bool keep) {
  setBitToUL(_preference, 13, keep);
}

// filenr = 0...3 for files rules1.txt ... rules4.txt
bool ResetFactoryDefaultPreference_struct::fetchRulesTXT(int filenr) const {
  return getBitFromUL(_preference, 14 + filenr);
}

void ResetFactoryDefaultPreference_struct::fetchRulesTXT(int filenr, bool fetch) {
  setBitToUL(_preference, 14 + filenr, fetch);
}

bool ResetFactoryDefaultPreference_struct::fetchNotificationDat() const {
  return getBitFromUL(_preference, 18);
}

void ResetFactoryDefaultPreference_struct::fetchNotificationDat(bool fetch) {
  setBitToUL(_preference, 18, fetch);
}

bool ResetFactoryDefaultPreference_struct::fetchSecurityDat() const {
  return getBitFromUL(_preference, 19);
}

void ResetFactoryDefaultPreference_struct::fetchSecurityDat(bool fetch) {
  setBitToUL(_preference, 19, fetch);
}

bool ResetFactoryDefaultPreference_struct::fetchConfigDat() const {
  return getBitFromUL(_preference, 20);
}

void ResetFactoryDefaultPreference_struct::fetchConfigDat(bool fetch) {
  setBitToUL(_preference, 20, fetch);
}

bool ResetFactoryDefaultPreference_struct::deleteFirst() const {
  return getBitFromUL(_preference, 21);
}

void ResetFactoryDefaultPreference_struct::deleteFirst(bool checked) {
  setBitToUL(_preference, 21, checked);
}

uint32_t ResetFactoryDefaultPreference_struct::getPreference() {
  return _preference;
}
