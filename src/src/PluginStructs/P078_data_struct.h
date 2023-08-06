#ifndef PLUGINSTRUCTS_P078_DATA_STRUCT_H
#define PLUGINSTRUCTS_P078_DATA_STRUCT_H

#include "../../ESPEasy_common.h"

#ifdef USES_P078
# include "../../_Plugin_Helper.h"
# include <ESPeasySerial.h>

# include <SDM.h> // Requires SDM library from Reaper7 - https://github.com/reaper7/SDM_Energy_Meter/


# define P078_DEV_ID          PCONFIG(0)
# define P078_DEV_ID_LABEL    PCONFIG_LABEL(0)
# define P078_MODEL           PCONFIG(1)
# define P078_MODEL_LABEL     PCONFIG_LABEL(1)
# define P078_BAUDRATE        PCONFIG(2)
# define P078_BAUDRATE_LABEL  PCONFIG_LABEL(2)

# define P078_GET_FLAG_COLL_DETECT bitRead(PCONFIG(7), 0)
# define P078_SET_FLAG_COLL_DETECT(x) bitWrite(PCONFIG(7), 0, x)
# define P078_FLAG_COLL_DETECT_LABEL "colldet"

# define P078_QUERY1_CONFIG_POS  3

# define P078_QUERY1          PCONFIG(P078_QUERY1_CONFIG_POS)
# define P078_QUERY2          PCONFIG((P078_QUERY1_CONFIG_POS)+1)
# define P078_QUERY3          PCONFIG((P078_QUERY1_CONFIG_POS)+2)
# define P078_QUERY4          PCONFIG((P078_QUERY1_CONFIG_POS)+3)
# define P078_DEPIN           CONFIG_PIN3

# define P078_DEV_ID_DFLT     1
# define P078_MODEL_DFLT      0 // SDM120C
# define P078_BAUDRATE_DFLT   3 // 9600 baud
# define P078_QUERY1_DFLT     0 // Voltage (V)
# define P078_QUERY2_DFLT     1 // Current (A)
# define P078_QUERY3_DFLT     2 // Power (W)
# define P078_QUERY4_DFLT     5 // Power Factor (cos-phi)


enum class SDM_UOM {
  percent,
  V,
  A,
  W,
  kWh,
  Ah,
  Hz,
  degrees,
  cos_phi,
  VA,
  VAr,
  kVAh,
  kVArh
};

const __FlashStringHelper* SDM_UOMtoString(SDM_UOM uom,
                                           bool    display);

// Value being stored, do not change order
enum class SDM_MODEL {
  SDM220_SDM120CT_SDM120 = 0,
  SDM230                 = 1,
  SDM72D                 = 2,
  DDM18SD                = 3,
  SDM630                 = 4,
  SDM72_V2               = 5,
  SDM320C                = 6
};

enum class SDM_DIRECTION {
  NotSpecified = 0,
  Import       = 1,
  Export       = 2,
  Total        = 3
};

const __FlashStringHelper *SDM_directionToString(SDM_DIRECTION);

void SDM_loadOutputSelector(struct EventStruct *event,
                            uint8_t             pconfigIndex,
                            uint8_t             valuenr);

struct p078_register_description {
  // Using special constexpr constructor so the bitflags can be stored as compact as possible
  // The compiler will compute the integer values.
  constexpr p078_register_description(
    uint16_t reg,
    SDM_UOM  uom,
    uint8_t  phase,
    uint8_t  direction,
    uint8_t  SDM630,
    uint8_t  SDM320C,
    uint8_t  SDM230,
    uint8_t  SDM220,
    uint8_t  SDM120CT,
    uint8_t  SDM120,
    uint8_t  SDM72D,
    uint8_t  SDM72_V2,
    uint8_t  DDM18SD
    ) : val(
      (static_cast<uint32_t>(uom) & 0xF) |
      static_cast<uint32_t>(phase & 0x3) << 4 |
        static_cast<uint32_t>(direction & 0x3) << 6 |
        static_cast<uint32_t>(SDM630 & 0x1)    << 8 |
        static_cast<uint32_t>(SDM320C & 0x1)   << 9 |
        static_cast<uint32_t>(SDM230 & 0x1)    << 10 |
        static_cast<uint32_t>(SDM220 & 0x1)    << 11 |
        static_cast<uint32_t>(SDM120CT & 0x1)  << 11 | // Same set of registers as SDM220
        static_cast<uint32_t>(SDM120 & 0x1)    << 11 | // Nearly same set of registers as SDM220
        static_cast<uint32_t>(SDM72D & 0x1)    << 12 |
        static_cast<uint32_t>(SDM72_V2 & 0x1)  << 13 |
        static_cast<uint32_t>(DDM18SD & 0x1)   << 14 |
        static_cast<uint32_t>(reg & 0xFFFF)    << 16)
  {}

  uint16_t      getRegister() const;

  SDM_UOM       getUnitOfMeasure() const;

  uint8_t       getPhase() const;

  SDM_DIRECTION getDirection() const;

  bool          match_SDM_model(SDM_MODEL model) const;

  String        getDescription(SDM_MODEL model) const;

  String        getPhaseDescription(SDM_MODEL model,
                                    char      separator) const;

  uint32_t val{};
};

int      SDM_getRegisterDescriptionIndexForModel(SDM_MODEL model,
                                                 int       x);

uint16_t SDM_getRegisterForModel(SDM_MODEL model,
                                 int       choice);

String   SDM_getValueNameForModel(SDM_MODEL model,
                                  int       choice);

struct SDM_RegisterReadQueueElement {
  SDM_RegisterReadQueueElement(taskIndex_t TaskIndex, taskVarIndex_t TaskVarIndex, uint16_t reg, uint8_t dev_id)
    : taskIndex(TaskIndex),
    taskVarIndex(TaskVarIndex),
    _reg(reg),
    _dev_id(dev_id)
  {}

  taskIndex_t    taskIndex    = INVALID_TASK_INDEX;
  taskVarIndex_t taskVarIndex = INVALID_TASKVAR_INDEX;
  uint16_t       _reg         = std::numeric_limits<uint16_t>::max(); // Modbus register
  uint8_t        _dev_id      = 0;                                    // Modbus address
  uint8_t        _state       = 0;
};

/*
   bool compare_SDM_RegisterReadQueueElement(const SDM_RegisterReadQueueElement& first, const SDM_RegisterReadQueueElement& second) const {
    return first._userVarIndex < second._userVarIndex;
   }
 */


typedef std::list<SDM_RegisterReadQueueElement> SDM_RegisterReadQueue;

void SDM_removeRegisterReadQueueElement(taskIndex_t    TaskIndex,
                                        taskVarIndex_t TaskVarIndex);
void SDM_addRegisterReadQueueElement(taskIndex_t    TaskIndex,
                                     taskVarIndex_t TaskVarIndex,
                                     uint16_t       reg,
                                     uint8_t        dev_id);

void SDM_loopRegisterReadQueue(SDM *sdm);

void SDM_pause_loopRegisterReadQueue();

void SDM_resume_loopRegisterReadQueue();

#endif // ifdef USES_P078

#endif // ifndef PLUGINSTRUCTS_P078_DATA_STRUCT_H
