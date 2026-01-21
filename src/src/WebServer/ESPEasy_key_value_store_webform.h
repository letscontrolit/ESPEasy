#ifndef WEBSERVER_ESPEASY_KEY_VALUE_STORE_WEBFORM_H
#define WEBSERVER_ESPEASY_KEY_VALUE_STORE_WEBFORM_H

#include "../WebServer/common.h"

#if FEATURE_ESPEASY_KEY_VALUE_STORE

# include "../DataTypes/FormSelectorOptions.h"
# include "../Helpers/_ESPEasy_key_value_store.h"

struct WebFormItemParams {
  WebFormItemParams(const String                       & label,
                    const String                       & id,
                    KVS_StorageType::Enum storageType,
                    uint32_t                             key = 0);

  WebFormItemParams(const __FlashStringHelper           *label,
                    const __FlashStringHelper           *id,
                    KVS_StorageType::Enum storageType,
                    uint32_t                             key = 0);


  void checkRanges();

  String _label;
  String _id;
# if FEATURE_TOOLTIPS
  String _tooltip;
# endif // if FEATURE_TOOLTIPS
  float                                _min = INT_MIN;
  float                                _max = INT_MAX;
  uint8_t                              _nrDecimals{};
  float                                _stepsize    = 1;
  KVS_StorageType::Enum _storageType = KVS_StorageType::Enum::not_set;
  uint32_t                             _key{};
  int                                  _maxLength{};
  bool                                 _disabled{};
  bool                                 _readOnly{};
  bool                                 _required{};
  bool                                 _password{};
  String                               _pattern;

  int64_t _defaultIntValue{};
  double  _defaultFloatValue{};
  String  _defaultStringValue;


};

bool showWebformItem(const ESPEasy_key_value_store& store,
                     WebFormItemParams              params);

void showFormSelector(const ESPEasy_key_value_store& store,
                      const FormSelectorOptions          & selector,
                      const WebFormItemParams      & params);

void storeWebformItem(ESPEasy_key_value_store            & store,
                      uint32_t                             key,
                      KVS_StorageType::Enum storageType,
                      const __FlashStringHelper           *id);

void storeWebformItem(ESPEasy_key_value_store            & store,
                      uint32_t                             key,
                      KVS_StorageType::Enum storageType,
                      const String                       & id = EMPTY_STRING);

#endif // if FEATURE_ESPEASY_KEY_VALUE_STORE
#endif // ifndef WEBSERVER_ESPEASY_KEY_VALUE_STORE_WEBFORM_H
