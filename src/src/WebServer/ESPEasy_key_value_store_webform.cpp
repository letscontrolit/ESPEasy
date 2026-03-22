#include "../WebServer/ESPEasy_key_value_store_webform.h"

#if FEATURE_ESPEASY_KEY_VALUE_STORE
# include "../Helpers/StringConverter.h"

# include "../WebServer/Markup_Forms.h"

WebFormItemParams::WebFormItemParams(
  const String                       & label,
  const String                       & id,
  KVS_StorageType::Enum storageType,
  uint32_t                             key)
  : _label(label), _id(id), _storageType(storageType), _key(key) {}

WebFormItemParams::WebFormItemParams(
  const __FlashStringHelper           *label,
  const __FlashStringHelper           *id,
  KVS_StorageType::Enum storageType,
  uint32_t                             key)
  : _label(label), _id(id), _storageType(storageType), _key(key) {}

# define CORRECT_RANGE(T, CT)                                                                 \
          case KVS_StorageType::Enum::T:                                       \
            if (_max > std::numeric_limits<CT>::max()) _max = std::numeric_limits<CT>::max(); \
            if (_min < std::numeric_limits<CT>::min()) _min = std::numeric_limits<CT>::min(); \
            break;

void WebFormItemParams::checkRanges()
{
  switch (_storageType)
  {
    CORRECT_RANGE(int8_type,   int8_t)
    CORRECT_RANGE(uint8_type,  uint8_t)
    CORRECT_RANGE(int16_type,  int16_t)
    CORRECT_RANGE(uint16_type, uint16_t)
    CORRECT_RANGE(int32_type,  int32_t)
    CORRECT_RANGE(uint32_type, uint32_t)
    CORRECT_RANGE(float_type,  float)
    CORRECT_RANGE(uint64_type, uint64_t)
    CORRECT_RANGE(int64_type,  int64_t)

    //  CORRECT_RANGE(double_type, double)
    default: break;
  }
}

bool showWebformItem(const ESPEasy_key_value_store& store,
                     WebFormItemParams              params)
{
  params.checkRanges();
  String id = params._id.isEmpty() ? concat(F("KVS_ID_"), params._key) : params._id;

  switch (params._storageType)
  {
    case KVS_StorageType::Enum::string_type:
    {
      String value;

      if (!store.getValueAsString(params._key, value))
      {
        value = params._defaultStringValue;
      }

      if (params._password) {
        addFormPasswordBox(
          params._label,
          id,
          value,
          params._maxLength
# if FEATURE_TOOLTIPS
          , params._tooltip
# endif // if FEATURE_TOOLTIPS
          );
      } else {
        addFormTextBox(
          params._label,
          id,
          value,
          params._maxLength,
          params._readOnly,
          params._required,
          params._pattern
# if FEATURE_TOOLTIPS
          , params._tooltip
# endif // if FEATURE_TOOLTIPS
          );
      }
      return true;
    }
    case KVS_StorageType::Enum::int8_type:
    case KVS_StorageType::Enum::uint8_type:
    case KVS_StorageType::Enum::int16_type:
    case KVS_StorageType::Enum::uint16_type:
    case KVS_StorageType::Enum::int32_type:
    case KVS_StorageType::Enum::uint32_type:
    {
      int64_t value;

      if (!store.getValueAsInt(params._key, value)) { value = params._defaultIntValue; }
      addFormNumericBox(
        params._label,
        id,
        value,
        params._min,
        params._max,
# if FEATURE_TOOLTIPS
        params._tooltip,
# endif // if FEATURE_TOOLTIPS
        params._disabled
        );

      return true;
    }

    case KVS_StorageType::Enum::int64_type:
    {
      break;
    }

    case KVS_StorageType::Enum::uint64_type:
    {
      break;
    }

    case KVS_StorageType::Enum::float_type:
    case KVS_StorageType::Enum::double_type:
    {
      float value{};

      if (!store.getValue(params._key, value)) { value = params._defaultFloatValue; }
      addFormFloatNumberBox(
        params._label,
        id,
        value,
        params._min,
        params._max,
        params._nrDecimals,
        params._stepsize

# if FEATURE_TOOLTIPS
        , params._tooltip
# endif // if FEATURE_TOOLTIPS
        );


      return true;
    }

    case KVS_StorageType::Enum::bool_type:
    {
      bool value{};

      if (!store.getValue(params._key, value)) { value = params._defaultIntValue != 0; }
      addFormCheckBox(
        params._label,
        id,
        value,
        params._disabled);
      return true;
    }

    default: break;
  }
  return false;
}

void showFormSelector(const ESPEasy_key_value_store& store,
                      const FormSelectorOptions    & selector,
                      const WebFormItemParams      & params)
{
  int64_t value{};

  if (!store.getValueAsInt(params._key, value)) { value = params._defaultIntValue; }

  selector.addFormSelector(params._label, params._id, value);
}

void storeWebformItem(ESPEasy_key_value_store            & store,
                      uint32_t                             key,
                      KVS_StorageType::Enum storageType,
                      const __FlashStringHelper           *id) { storeWebformItem(store, key, storageType, String(id)); }

void storeWebformItem(ESPEasy_key_value_store            & store,
                      uint32_t                             key,
                      KVS_StorageType::Enum storageType,
                      const String                       & id)
{
  String _id = id.isEmpty() ? concat(F("KVS_ID_"), key) : id;

  if (storageType == KVS_StorageType::Enum::bool_type) {
    store.setValue(key, isFormItemChecked(_id));
    return;
  }

  if (hasArg(_id)) {
    store.setValue(storageType, key, webArg(_id));
  }
}

#endif // if FEATURE_ESPEASY_KEY_VALUE_STORE
