#ifndef DATASTRUCTS_PLUGINSTATS_CONFIG_H
#define DATASTRUCTS_PLUGINSTATS_CONFIG_H

#include "../../ESPEasy_common.h"

#if FEATURE_PLUGIN_STATS

// Configuration of the plugin stats per task value
struct PluginStats_Config_t {
  enum class AxisPosition {
    Left,
    Right
  };

  PluginStats_Config_t() : stored(0) {}

  PluginStats_Config_t(uint8_t stored_value) : stored(stored_value) {}

  PluginStats_Config_t& operator=(const PluginStats_Config_t& other);

  AxisPosition          getAxisPosition() const {
    return static_cast<AxisPosition>(bits.chartAxisPosition);
  }

  bool isLeft() const { return AxisPosition::Left == getAxisPosition(); }

  void setAxisPosition(AxisPosition position) {
    bits.chartAxisPosition = static_cast<uint8_t>(position);
  }

  uint8_t getAxisIndex() const {
    return bits.chartAxisIndex;
  }

  void setAxisIndex(uint8_t index) {
    bits.chartAxisIndex = index;
  }

  uint8_t getStoredBits() const {
    return stored & ~0x02; // Mask unused_01
  }

  bool isEnabled() const {
    return bits.enabled;
  }

  void setEnabled(bool enable) {
    bits.enabled = enable;
  }

  bool showHidden() const {
    return bits.hidden;
  }

  void setHidden(bool enable) {
    bits.hidden = enable;
  }

private:

  union {
    struct {
      uint8_t enabled           : 1; // Bit 00
      uint8_t unused_01         : 1; // Bit 01  Used by isDefaultTaskVarName in ExtraTaskSettingsStruct
      uint8_t hidden            : 1; // Bit 02  Hidden/Displayed state on initial showing of the chart
      uint8_t chartAxisIndex    : 2; // Bit 03 ... 04
      uint8_t chartAxisPosition : 1; // Bit 05
      uint8_t unused_06         : 1; // Bit 06
      uint8_t unused_07         : 1; // Bit 07
    }       bits;
    uint8_t stored{};
  };
};

#endif // if FEATURE_PLUGIN_STATS
#endif // ifndef DATASTRUCTS_PLUGINSTATS_CONFIG_H
