#include "../PluginStructs/P078_data_struct.h"

#include "../../_Plugin_Helper.h"

# include <SDM.h> // Requires SDM library from Reaper7 - https://github.com/reaper7/SDM_Energy_Meter/

constexpr p078_register_description register_description_list[] = {
//------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
//      REGISTERS LIST FOR SDM DEVICES                                                                                                                                                           |
//------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
//      REGISTER NAME                          REGISTER ADDRESS              UNIT                 | phase | dir | SDM630  | SDM230  | SDM220  | SDM120CT| SDM120  | SDM72D  | SDM72 V2 | DDM18SD |
//------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
{ SDM_PHASE_1_VOLTAGE                           /* 0x0000 */              ,  SDM_UOM::V           ,   1   ,  0  ,    1    ,    1    ,    1    ,    1    ,    1    ,    0    ,    1     ,    0    },         
{ SDM_PHASE_2_VOLTAGE                           /* 0x0002 */              ,  SDM_UOM::V           ,   2   ,  0  ,    1    ,    0    ,    0    ,    0    ,    0    ,    0    ,    1     ,    0    },
{ SDM_PHASE_3_VOLTAGE                           /* 0x0004 */              ,  SDM_UOM::V           ,   3   ,  0  ,    1    ,    0    ,    0    ,    0    ,    0    ,    0    ,    1     ,    0    },
{ SDM_PHASE_1_CURRENT                           /* 0x0006 */              ,  SDM_UOM::A           ,   1   ,  0  ,    1    ,    1    ,    1    ,    1    ,    1    ,    0    ,    1     ,    0    },
{ SDM_PHASE_2_CURRENT                           /* 0x0008 */              ,  SDM_UOM::A           ,   2   ,  0  ,    1    ,    0    ,    0    ,    0    ,    0    ,    0    ,    1     ,    0    },
{ SDM_PHASE_3_CURRENT                           /* 0x000A */              ,  SDM_UOM::A           ,   3   ,  0  ,    1    ,    0    ,    0    ,    0    ,    0    ,    0    ,    1     ,    0    },
{ SDM_PHASE_1_POWER                             /* 0x000C */              ,  SDM_UOM::W           ,   1   ,  0  ,    1    ,    1    ,    1    ,    1    ,    1    ,    0    ,    1     ,    0    },
{ SDM_PHASE_2_POWER                             /* 0x000E */              ,  SDM_UOM::W           ,   2   ,  0  ,    1    ,    0    ,    0    ,    0    ,    0    ,    0    ,    1     ,    0    },
{ SDM_PHASE_3_POWER                             /* 0x0010 */              ,  SDM_UOM::W           ,   3   ,  0  ,    1    ,    0    ,    0    ,    0    ,    0    ,    0    ,    1     ,    0    },
{ SDM_PHASE_1_APPARENT_POWER                    /* 0x0012 */              ,  SDM_UOM::VA          ,   1   ,  0  ,    1    ,    1    ,    1    ,    1    ,    1    ,    0    ,    1     ,    0    },
{ SDM_PHASE_2_APPARENT_POWER                    /* 0x0014 */              ,  SDM_UOM::VA          ,   2   ,  0  ,    1    ,    0    ,    0    ,    0    ,    0    ,    0    ,    1     ,    0    },
{ SDM_PHASE_3_APPARENT_POWER                    /* 0x0016 */              ,  SDM_UOM::VA          ,   3   ,  0  ,    1    ,    0    ,    0    ,    0    ,    0    ,    0    ,    1     ,    0    },
{ SDM_PHASE_1_REACTIVE_POWER                    /* 0x0018 */              ,  SDM_UOM::VAr         ,   1   ,  0  ,    1    ,    1    ,    1    ,    1    ,    1    ,    0    ,    1     ,    0    },
{ SDM_PHASE_2_REACTIVE_POWER                    /* 0x001A */              ,  SDM_UOM::VAr         ,   2   ,  0  ,    1    ,    0    ,    0    ,    0    ,    0    ,    0    ,    1     ,    0    },
{ SDM_PHASE_3_REACTIVE_POWER                    /* 0x001C */              ,  SDM_UOM::VAr         ,   3   ,  0  ,    1    ,    0    ,    0    ,    0    ,    0    ,    0    ,    1     ,    0    },
{ SDM_PHASE_1_POWER_FACTOR                      /* 0x001E */              ,  SDM_UOM::cos_phi     ,   1   ,  0  ,    1    ,    1    ,    1    ,    1    ,    1    ,    0    ,    1     ,    0    },
{ SDM_PHASE_2_POWER_FACTOR                      /* 0x0020 */              ,  SDM_UOM::cos_phi     ,   2   ,  0  ,    1    ,    0    ,    0    ,    0    ,    0    ,    0    ,    1     ,    0    },
{ SDM_PHASE_3_POWER_FACTOR                      /* 0x0022 */              ,  SDM_UOM::cos_phi     ,   3   ,  0  ,    1    ,    0    ,    0    ,    0    ,    0    ,    0    ,    1     ,    0    },
{ SDM_PHASE_1_ANGLE                             /* 0x0024 */              ,  SDM_UOM::degrees     ,   1   ,  0  ,    1    ,    1    ,    1    ,    1    ,    0    ,    0    ,    0     ,    0    },
{ SDM_PHASE_2_ANGLE                             /* 0x0026 */              ,  SDM_UOM::degrees     ,   2   ,  0  ,    1    ,    0    ,    0    ,    0    ,    0    ,    0    ,    0     ,    0    },
{ SDM_PHASE_3_ANGLE                             /* 0x0028 */              ,  SDM_UOM::degrees     ,   3   ,  0  ,    1    ,    0    ,    0    ,    0    ,    0    ,    0    ,    0     ,    0    },
{ SDM_AVERAGE_L_TO_N_VOLTS                      /* 0x002A */              ,  SDM_UOM::V           ,   0   ,  0  ,    1    ,    0    ,    0    ,    0    ,    0    ,    0    ,    1     ,    0    },
{ SDM_AVERAGE_LINE_CURRENT                      /* 0x002E */              ,  SDM_UOM::A           ,   0   ,  0  ,    1    ,    0    ,    0    ,    0    ,    0    ,    0    ,    1     ,    0    },
{ SDM_SUM_LINE_CURRENT                          /* 0x0030 */              ,  SDM_UOM::A           ,   0   ,  0  ,    1    ,    0    ,    0    ,    0    ,    0    ,    0    ,    1     ,    0    },
{ SDM_TOTAL_SYSTEM_POWER                        /* 0x0034 */              ,  SDM_UOM::W           ,   0   ,  3  ,    1    ,    0    ,    0    ,    0    ,    0    ,    1    ,    1     ,    0    },
{ SDM_TOTAL_SYSTEM_APPARENT_POWER               /* 0x0038 */              ,  SDM_UOM::VA          ,   0   ,  3  ,    1    ,    0    ,    0    ,    0    ,    0    ,    0    ,    1     ,    0    },
{ SDM_TOTAL_SYSTEM_REACTIVE_POWER               /* 0x003C */              ,  SDM_UOM::VAr         ,   0   ,  3  ,    1    ,    0    ,    0    ,    0    ,    0    ,    0    ,    1     ,    0    },
{ SDM_TOTAL_SYSTEM_POWER_FACTOR                 /* 0x003E */              ,  SDM_UOM::cos_phi     ,   0   ,  3  ,    1    ,    0    ,    0    ,    0    ,    0    ,    0    ,    1     ,    0    },
{ SDM_TOTAL_SYSTEM_PHASE_ANGLE                  /* 0x0042 */              ,  SDM_UOM::degrees     ,   0   ,  3  ,    1    ,    0    ,    0    ,    0    ,    0    ,    0    ,    0     ,    0    },
{ SDM_FREQUENCY                                 /* 0x0046 */              ,  SDM_UOM::Hz          ,   0   ,  0  ,    1    ,    1    ,    1    ,    1    ,    1    ,    0    ,    1     ,    0    },
{ SDM_IMPORT_ACTIVE_ENERGY                      /* 0x0048 */              ,  SDM_UOM::kWh         ,   0   ,  1  ,    1    ,    1    ,    1    ,    1    ,    1    ,    1    ,    1     ,    0    },
{ SDM_EXPORT_ACTIVE_ENERGY                      /* 0x004A */              ,  SDM_UOM::kWh         ,   0   ,  2  ,    1    ,    1    ,    1    ,    1    ,    1    ,    1    ,    1     ,    0    },
{ SDM_IMPORT_REACTIVE_ENERGY                    /* 0x004C */              ,  SDM_UOM::kVArh       ,   0   ,  1  ,    1    ,    1    ,    1    ,    1    ,    1    ,    0    ,    0     ,    0    },
{ SDM_EXPORT_REACTIVE_ENERGY                    /* 0x004E */              ,  SDM_UOM::kVArh       ,   0   ,  2  ,    1    ,    1    ,    1    ,    1    ,    1    ,    0    ,    0     ,    0    },
{ SDM_VAH_SINCE_LAST_RESET                      /* 0x0050 */              ,  SDM_UOM::kVAh        ,   0   ,  0  ,    1    ,    0    ,    0    ,    0    ,    0    ,    0    ,    0     ,    0    },
{ SDM_AH_SINCE_LAST_RESET                       /* 0x0052 */              ,  SDM_UOM::Ah          ,   0   ,  0  ,    1    ,    0    ,    0    ,    0    ,    0    ,    0    ,    0     ,    0    },
{ SDM_TOTAL_SYSTEM_POWER_DEMAND                 /* 0x0054 */              ,  SDM_UOM::W           ,   0   ,  3  ,    1    ,    1    ,    0    ,    0    ,    0    ,    0    ,    0     ,    0    },
{ SDM_MAXIMUM_TOTAL_SYSTEM_POWER_DEMAND         /* 0x0056 */              ,  SDM_UOM::W           ,   0   ,  0  ,    1    ,    1    ,    0    ,    0    ,    0    ,    0    ,    0     ,    0    },
{ SDM_CURRENT_SYSTEM_POSITIVE_POWER_DEMAND      /* 0x0058 */              ,  SDM_UOM::W           ,   0   ,  1  ,    0    ,    1    ,    0    ,    0    ,    0    ,    0    ,    0     ,    0    },
{ SDM_MAXIMUM_SYSTEM_POSITIVE_POWER_DEMAND      /* 0x005A */              ,  SDM_UOM::W           ,   0   ,  1  ,    0    ,    1    ,    0    ,    0    ,    0    ,    0    ,    0     ,    0    },
{ SDM_CURRENT_SYSTEM_REVERSE_POWER_DEMAND       /* 0x005C */              ,  SDM_UOM::W           ,   0   ,  2  ,    0    ,    1    ,    0    ,    0    ,    0    ,    0    ,    0     ,    0    },
{ SDM_MAXIMUM_SYSTEM_REVERSE_POWER_DEMAND       /* 0x005E */              ,  SDM_UOM::W           ,   0   ,  2  ,    0    ,    1    ,    0    ,    0    ,    0    ,    0    ,    0     ,    0    },
{ SDM_TOTAL_SYSTEM_VA_DEMAND                    /* 0x0064 */              ,  SDM_UOM::VA          ,   0   ,  3  ,    1    ,    0    ,    0    ,    0    ,    0    ,    0    ,    0     ,    0    },
{ SDM_MAXIMUM_TOTAL_SYSTEM_VA_DEMAND            /* 0x0066 */              ,  SDM_UOM::VA          ,   0   ,  0  ,    1    ,    0    ,    0    ,    0    ,    0    ,    0    ,    0     ,    0    },
{ SDM_NEUTRAL_CURRENT_DEMAND                    /* 0x0068 */              ,  SDM_UOM::A           ,   0   ,  0  ,    1    ,    0    ,    0    ,    0    ,    0    ,    0    ,    0     ,    0    },
{ SDM_MAXIMUM_NEUTRAL_CURRENT                   /* 0x006A */              ,  SDM_UOM::A           ,   0   ,  0  ,    1    ,    0    ,    0    ,    0    ,    0    ,    0    ,    0     ,    0    },
{ SDM_LINE_1_TO_LINE_2_VOLTS                    /* 0x00C8 */              ,  SDM_UOM::V           ,   0   ,  0  ,    1    ,    0    ,    0    ,    0    ,    0    ,    0    ,    1     ,    0    },
{ SDM_LINE_2_TO_LINE_3_VOLTS                    /* 0x00CA */              ,  SDM_UOM::V           ,   0   ,  0  ,    1    ,    0    ,    0    ,    0    ,    0    ,    0    ,    1     ,    0    },
{ SDM_LINE_3_TO_LINE_1_VOLTS                    /* 0x00CC */              ,  SDM_UOM::V           ,   0   ,  0  ,    1    ,    0    ,    0    ,    0    ,    0    ,    0    ,    1     ,    0    },
{ SDM_AVERAGE_LINE_TO_LINE_VOLTS                /* 0x00CE */              ,  SDM_UOM::V           ,   0   ,  0  ,    1    ,    0    ,    0    ,    0    ,    0    ,    0    ,    1     ,    0    },
{ SDM_NEUTRAL_CURRENT                           /* 0x00E0 */              ,  SDM_UOM::A           ,   0   ,  0  ,    1    ,    0    ,    0    ,    0    ,    0    ,    0    ,    1     ,    0    },
{ SDM_PHASE_1_LN_VOLTS_THD                      /* 0x00EA */              ,  SDM_UOM::percent     ,   1   ,  0  ,    1    ,    0    ,    0    ,    0    ,    0    ,    0    ,    0     ,    0    },
{ SDM_PHASE_2_LN_VOLTS_THD                      /* 0x00EC */              ,  SDM_UOM::percent     ,   2   ,  0  ,    1    ,    0    ,    0    ,    0    ,    0    ,    0    ,    0     ,    0    },
{ SDM_PHASE_3_LN_VOLTS_THD                      /* 0x00EE */              ,  SDM_UOM::percent     ,   3   ,  0  ,    1    ,    0    ,    0    ,    0    ,    0    ,    0    ,    0     ,    0    },
{ SDM_PHASE_1_CURRENT_THD                       /* 0x00F0 */              ,  SDM_UOM::percent     ,   1   ,  0  ,    1    ,    0    ,    0    ,    0    ,    0    ,    0    ,    0     ,    0    },
{ SDM_PHASE_2_CURRENT_THD                       /* 0x00F2 */              ,  SDM_UOM::percent     ,   2   ,  0  ,    1    ,    0    ,    0    ,    0    ,    0    ,    0    ,    0     ,    0    },
{ SDM_PHASE_3_CURRENT_THD                       /* 0x00F4 */              ,  SDM_UOM::percent     ,   3   ,  0  ,    1    ,    0    ,    0    ,    0    ,    0    ,    0    ,    0     ,    0    },
{ SDM_AVERAGE_LINE_TO_NEUTRAL_VOLTS_THD         /* 0x00F8 */              ,  SDM_UOM::percent     ,   0   ,  0  ,    1    ,    0    ,    0    ,    0    ,    0    ,    0    ,    0     ,    0    },
{ SDM_AVERAGE_LINE_CURRENT_THD                  /* 0x00FA */              ,  SDM_UOM::percent     ,   0   ,  0  ,    1    ,    0    ,    0    ,    0    ,    0    ,    0    ,    0     ,    0    },
{ SDM_TOTAL_SYSTEM_POWER_FACTOR_INV             /* 0x00FE */              ,  SDM_UOM::cos_phi     ,   0   ,  3  ,    1    ,    0    ,    0    ,    0    ,    0    ,    0    ,    0     ,    0    },
{ SDM_PHASE_1_CURRENT_DEMAND                    /* 0x0102 */              ,  SDM_UOM::A           ,   1   ,  0  ,    1    ,    1    ,    0    ,    0    ,    0    ,    0    ,    0     ,    0    },
{ SDM_PHASE_2_CURRENT_DEMAND                    /* 0x0104 */              ,  SDM_UOM::A           ,   2   ,  0  ,    1    ,    0    ,    0    ,    0    ,    0    ,    0    ,    0     ,    0    },
{ SDM_PHASE_3_CURRENT_DEMAND                    /* 0x0106 */              ,  SDM_UOM::A           ,   3   ,  0  ,    1    ,    0    ,    0    ,    0    ,    0    ,    0    ,    0     ,    0    },
{ SDM_MAXIMUM_PHASE_1_CURRENT_DEMAND            /* 0x0108 */              ,  SDM_UOM::A           ,   1   ,  0  ,    1    ,    1    ,    0    ,    0    ,    0    ,    0    ,    0     ,    0    },
{ SDM_MAXIMUM_PHASE_2_CURRENT_DEMAND            /* 0x010A */              ,  SDM_UOM::A           ,   2   ,  0  ,    1    ,    0    ,    0    ,    0    ,    0    ,    0    ,    0     ,    0    },
{ SDM_MAXIMUM_PHASE_3_CURRENT_DEMAND            /* 0x010C */              ,  SDM_UOM::A           ,   3   ,  0  ,    1    ,    0    ,    0    ,    0    ,    0    ,    0    ,    0     ,    0    },
{ SDM_LINE_1_TO_LINE_2_VOLTS_THD                /* 0x014E */              ,  SDM_UOM::percent     ,   0   ,  0  ,    1    ,    0    ,    0    ,    0    ,    0    ,    0    ,    0     ,    0    },
{ SDM_LINE_2_TO_LINE_3_VOLTS_THD                /* 0x0150 */              ,  SDM_UOM::percent     ,   0   ,  0  ,    1    ,    0    ,    0    ,    0    ,    0    ,    0    ,    0     ,    0    },
{ SDM_LINE_3_TO_LINE_1_VOLTS_THD                /* 0x0152 */              ,  SDM_UOM::percent     ,   0   ,  0  ,    1    ,    0    ,    0    ,    0    ,    0    ,    0    ,    0     ,    0    },
{ SDM_AVERAGE_LINE_TO_LINE_VOLTS_THD            /* 0x0154 */              ,  SDM_UOM::percent     ,   0   ,  0  ,    1    ,    0    ,    0    ,    0    ,    0    ,    0    ,    0     ,    0    },
{ SDM_TOTAL_ACTIVE_ENERGY                       /* 0x0156 */              ,  SDM_UOM::kWh         ,   0   ,  3  ,    1    ,    1    ,    1    ,    1    ,    1    ,    1    ,    1     ,    0    },
{ SDM_TOTAL_REACTIVE_ENERGY                     /* 0x0158 */              ,  SDM_UOM::kVArh       ,   0   ,  3  ,    1    ,    1    ,    1    ,    1    ,    1    ,    0    ,    1     ,    0    },
{ SDM_L1_IMPORT_ACTIVE_ENERGY                   /* 0x015A */              ,  SDM_UOM::kWh         ,   1   ,  1  ,    1    ,    0    ,    0    ,    0    ,    0    ,    0    ,    0     ,    0    },
{ SDM_L2_IMPORT_ACTIVE_ENERGY                   /* 0x015C */              ,  SDM_UOM::kWh         ,   2   ,  1  ,    1    ,    0    ,    0    ,    0    ,    0    ,    0    ,    0     ,    0    },
{ SDM_L3_IMPORT_ACTIVE_ENERGY                   /* 0x015E */              ,  SDM_UOM::kWh         ,   3   ,  1  ,    1    ,    0    ,    0    ,    0    ,    0    ,    0    ,    0     ,    0    },
{ SDM_L1_EXPORT_ACTIVE_ENERGY                   /* 0x0160 */              ,  SDM_UOM::kWh         ,   1   ,  2  ,    1    ,    0    ,    0    ,    0    ,    0    ,    0    ,    0     ,    0    },
{ SDM_L2_EXPORT_ACTIVE_ENERGY                   /* 0x0162 */              ,  SDM_UOM::kWh         ,   2   ,  2  ,    1    ,    0    ,    0    ,    0    ,    0    ,    0    ,    0     ,    0    },
{ SDM_L3_EXPORT_ACTIVE_ENERGY                   /* 0x0164 */              ,  SDM_UOM::kWh         ,   3   ,  2  ,    1    ,    0    ,    0    ,    0    ,    0    ,    0    ,    0     ,    0    },
{ SDM_L1_TOTAL_ACTIVE_ENERGY                    /* 0x0166 */              ,  SDM_UOM::kWh         ,   1   ,  3  ,    1    ,    0    ,    0    ,    0    ,    0    ,    0    ,    0     ,    0    },
{ SDM_L2_TOTAL_ACTIVE_ENERGY                    /* 0x0168 */              ,  SDM_UOM::kWh         ,   2   ,  3  ,    1    ,    0    ,    0    ,    0    ,    0    ,    0    ,    0     ,    0    },
{ SDM_L3_TOTAL_ACTIVE_ENERGY                    /* 0x016a */              ,  SDM_UOM::kWh         ,   3   ,  3  ,    1    ,    0    ,    0    ,    0    ,    0    ,    0    ,    0     ,    0    },
{ SDM_L1_IMPORT_REACTIVE_ENERGY                 /* 0x016C */              ,  SDM_UOM::kVArh       ,   1   ,  1  ,    1    ,    0    ,    0    ,    0    ,    0    ,    0    ,    0     ,    0    },
{ SDM_L2_IMPORT_REACTIVE_ENERGY                 /* 0x016E */              ,  SDM_UOM::kVArh       ,   2   ,  1  ,    1    ,    0    ,    0    ,    0    ,    0    ,    0    ,    0     ,    0    },
{ SDM_L3_IMPORT_REACTIVE_ENERGY                 /* 0x0170 */              ,  SDM_UOM::kVArh       ,   3   ,  1  ,    1    ,    0    ,    0    ,    0    ,    0    ,    0    ,    0     ,    0    },
{ SDM_L1_EXPORT_REACTIVE_ENERGY                 /* 0x0172 */              ,  SDM_UOM::kVArh       ,   1   ,  2  ,    1    ,    0    ,    0    ,    0    ,    0    ,    0    ,    0     ,    0    },
{ SDM_L2_EXPORT_REACTIVE_ENERGY                 /* 0x0174 */              ,  SDM_UOM::kVArh       ,   2   ,  2  ,    1    ,    0    ,    0    ,    0    ,    0    ,    0    ,    0     ,    0    },
{ SDM_L3_EXPORT_REACTIVE_ENERGY                 /* 0x0176 */              ,  SDM_UOM::kVArh       ,   3   ,  2  ,    1    ,    0    ,    0    ,    0    ,    0    ,    0    ,    0     ,    0    },
{ SDM_L1_TOTAL_REACTIVE_ENERGY                  /* 0x0178 */              ,  SDM_UOM::kVArh       ,   1   ,  3  ,    1    ,    0    ,    0    ,    0    ,    0    ,    0    ,    0     ,    0    },
{ SDM_L2_TOTAL_REACTIVE_ENERGY                  /* 0x017A */              ,  SDM_UOM::kVArh       ,   2   ,  3  ,    1    ,    0    ,    0    ,    0    ,    0    ,    0    ,    0     ,    0    },
{ SDM_L3_TOTAL_REACTIVE_ENERGY                  /* 0x017C */              ,  SDM_UOM::kVArh       ,   3   ,  3  ,    1    ,    0    ,    0    ,    0    ,    0    ,    0    ,    0     ,    0    },
{ SDM_CURRENT_RESETTABLE_TOTAL_ACTIVE_ENERGY    /* 0x0180 */              ,  SDM_UOM::kWh         ,   0   ,  3  ,    0    ,    1    ,    0    ,    0    ,    0    ,    1    ,    1     ,    0    },
{ SDM_CURRENT_RESETTABLE_TOTAL_REACTIVE_ENERGY  /* 0x0182 */              ,  SDM_UOM::kVArh       ,   0   ,  3  ,    0    ,    1    ,    0    ,    0    ,    0    ,    0    ,    0     ,    0    },
{ SDM_CURRENT_RESETTABLE_IMPORT_ENERGY          /* 0x0184 */              ,  SDM_UOM::kWh         ,   0   ,  1  ,    0    ,    0    ,    0    ,    0    ,    0    ,    1    ,    1     ,    0    },
{ SDM_CURRENT_RESETTABLE_EXPORT_ENERGY          /* 0x0186 */              ,  SDM_UOM::kWh         ,   0   ,  2  ,    0    ,    0    ,    0    ,    0    ,    0    ,    1    ,    1     ,    0    },
{ SDM_NET_KWH                                   /* 0x018C */              ,  SDM_UOM::kWh         ,   0   ,  0  ,    0    ,    0    ,    0    ,    0    ,    0    ,    0    ,    1     ,    0    },
{ SDM_IMPORT_POWER                              /* 0x0500 */              ,  SDM_UOM::W           ,   0   ,  1  ,    0    ,    0    ,    0    ,    0    ,    0    ,    1    ,    1     ,    0    },
{ SDM_EXPORT_POWER                              /* 0x0502 */              ,  SDM_UOM::W           ,   0   ,  2  ,    0    ,    0    ,    0    ,    0    ,    0    ,    1    ,    1     ,    0    },

{ DDM_PHASE_1_VOLTAGE                           /* 0x0000 */              ,  SDM_UOM::V           ,   1   ,  0  ,    0    ,    0    ,    0    ,    0    ,    0    ,    0    ,    0     ,    1    },
{ DDM_PHASE_1_CURRENT                           /* 0x0008 */              ,  SDM_UOM::A           ,   1   ,  0  ,    0    ,    0    ,    0    ,    0    ,    0    ,    0    ,    0     ,    1    },
{ DDM_PHASE_1_POWER                             /* 0x0012 */              ,  SDM_UOM::W           ,   1   ,  0  ,    0    ,    0    ,    0    ,    0    ,    0    ,    0    ,    0     ,    1    },
{ DDM_PHASE_1_REACTIVE_POWER                    /* 0x001A */              ,  SDM_UOM::VAr         ,   1   ,  0  ,    0    ,    0    ,    0    ,    0    ,    0    ,    0    ,    0     ,    1    },
{ DDM_PHASE_1_POWER_FACTOR                      /* 0x002A */              ,  SDM_UOM::cos_phi     ,   1   ,  0  ,    0    ,    0    ,    0    ,    0    ,    0    ,    0    ,    0     ,    1    },
{ DDM_FREQUENCY                                 /* 0x0036 */              ,  SDM_UOM::Hz          ,   0   ,  0  ,    0    ,    0    ,    0    ,    0    ,    0    ,    0    ,    0     ,    1    },
{ DDM_IMPORT_ACTIVE_ENERGY                      /* 0x0100 */              ,  SDM_UOM::kWh         ,   0   ,  1  ,    0    ,    0    ,    0    ,    0    ,    0    ,    0    ,    0     ,    1    },
{ DDM_IMPORT_REACTIVE_ENERGY                    /* 0x0400 */              ,  SDM_UOM::kVArh       ,   0   ,  1  ,    0    ,    0    ,    0    ,    0    ,    0    ,    0    ,    0     ,    1    }
};

constexpr size_t register_description_list_size = sizeof(register_description_list) / sizeof(register_description_list[0]);

const __FlashStringHelper* SDM_UOMtoString(SDM_UOM uom, bool display) {
  switch (uom) {
    case SDM_UOM::percent: return (display) ? F("THD")             :  F("%");
    case SDM_UOM::V:       return (display) ? F("Voltage")         :  F("V");
    case SDM_UOM::A:       return (display) ? F("Current")         :  F("A");
    case SDM_UOM::W:       return (display) ? F("Active Power")    :  F("W");
    case SDM_UOM::kWh:     return (display) ? F("Active Energy")   :  F("kWh");
    case SDM_UOM::Ah:      return (display) ? F("Ah")              :  F("Ah");
    case SDM_UOM::Hz:      return (display) ? F("Frequency")       :  F("Hz");
    case SDM_UOM::degrees: return (display) ? F("Phase Angle")     :  F("Degrees");
    case SDM_UOM::cos_phi: return (display) ? F("Power Factor")    :  F("cosphi");
    case SDM_UOM::VA:      return (display) ? F("Apparent Power")  :  F("VA");
    case SDM_UOM::VAr:     return (display) ? F("Reactive Power")  :  F("VAr");
    case SDM_UOM::kVAh:    return (display) ? F("Apparent Energy") :  F("kVAh");
    case SDM_UOM::kVArh:   return (display) ? F("Reactive Energy") :  F("kVArh");
  }
  return F("");
}

const __FlashStringHelper* SDM_directionToString(SDM_DIRECTION dir) {
    if (dir == SDM_DIRECTION::Import) return F("Import");
    if (dir == SDM_DIRECTION::Export) return F("Export");
    if (dir == SDM_DIRECTION::Total)  return F("Total");
    return F("");
}

bool SDM_getRegisterDescriptionForModel(SDM_MODEL model, int x, const p078_register_description* reg)
{
  int count = -1;
  for (int index = 0; index < register_description_list_size; ++index)
  {
    if (register_description_list[index].match_SDM_model(model)) {
        ++count;
        if (x == count) {
            reg = &(register_description_list[index]);
            return true;
        }
    }
  }
  reg = nullptr;
  return false;
}

uint16_t SDM_getRegisterForModel(SDM_MODEL model, int choice)
{
  const p078_register_description* reg = nullptr;          
  if (SDM_getRegisterDescriptionForModel(model, choice, reg) && reg != nullptr) {
    return reg->getRegister();
  }
  return 0u;
}

String SDM_getValueNameForModel(SDM_MODEL model, int choice)
{
  const p078_register_description* reg = nullptr;          
  if (SDM_getRegisterDescriptionForModel(model, choice, reg) && reg != nullptr) {
    const SDM_UOM uom = reg->getUnitOfMeasure();
    return SDM_UOMtoString(uom, false);
  }
  return EMPTY_STRING;
}

void SDM_loadOutputSelector(struct EventStruct *event, uint8_t pconfigIndex, uint8_t valuenr)
{
  const SDM_MODEL model = static_cast<SDM_MODEL>(P078_MODEL);
  const String label = concat(F("Value "), valuenr + 1);
  const String id = PCONFIG_LABEL(pconfigIndex);
  addRowLabel_tr_id(label, id);
  do_addSelector_Head(id, F("wide"), EMPTY_STRING, false);
  const int selectedIndex = PCONFIG(pconfigIndex);

  uint8_t x{};
  for (int index = 0; index < register_description_list_size; ++index)
  {
    if (register_description_list[index].match_SDM_model(model)) {
        const String option = register_description_list[index].getDescription();
        addSelector_Item(option, x, selectedIndex == x, false, EMPTY_STRING);
        ++x;    
    }
    if (x % 10 == 0) delay(0);
  }
  addSelector_Foot();
}


uint16_t p078_register_description::getRegister() const
{
    return static_cast<uint16_t>((val >> 16) & 0xFFFF);
}

SDM_UOM p078_register_description::getUnitOfMeasure() const
{
    return static_cast<SDM_UOM>(val & 0xF);
}

uint8_t p078_register_description::getPhase() const
{
    return static_cast<uint8_t>((val >> 4) & 0x3);
}

SDM_DIRECTION p078_register_description::getDirection() const
{
    return static_cast<SDM_DIRECTION>((val >> 6) & 0x3);
}

bool p078_register_description::match_SDM_model(SDM_MODEL model) const
{
    switch (model) {
        case SDM_MODEL::SDM630: return bitRead(val, 8);
        case SDM_MODEL::SDM230: return bitRead(val, 9);
        case SDM_MODEL::SDM220_SDM120CT_SDM120: return bitRead(val, 10);
        case SDM_MODEL::SDM72D: return bitRead(val, 13);
        case SDM_MODEL::SDM72_V2: return bitRead(val, 14);
        case SDM_MODEL::DDM18SD: return bitRead(val, 15);
    }
    return false;
}

String p078_register_description::getDescription() const
{
    String res;
    const SDM_DIRECTION direction = getDirection();
    const SDM_UOM uom = getUnitOfMeasure();
    const int phase = getPhase();
    bool showFullUnitOfMeasure = true;
    // TODO TD-er: Check first for specific strings not generated using the description bitmap

    switch (getRegister())
    {
      case SDM_MAXIMUM_TOTAL_SYSTEM_POWER_DEMAND:
      case SDM_MAXIMUM_TOTAL_SYSTEM_VA_DEMAND:
      case SDM_MAXIMUM_NEUTRAL_CURRENT:
      case SDM_MAXIMUM_SYSTEM_POSITIVE_POWER_DEMAND:
      case SDM_MAXIMUM_SYSTEM_REVERSE_POWER_DEMAND:
      case SDM_MAXIMUM_PHASE_1_CURRENT_DEMAND:
      case SDM_MAXIMUM_PHASE_2_CURRENT_DEMAND:
      case SDM_MAXIMUM_PHASE_3_CURRENT_DEMAND:

        res = F("Maximum ");
        break;
    
      case SDM_CURRENT_RESETTABLE_TOTAL_ACTIVE_ENERGY:
      case SDM_CURRENT_RESETTABLE_TOTAL_REACTIVE_ENERGY:
      case SDM_CURRENT_RESETTABLE_IMPORT_ENERGY:
      case SDM_CURRENT_RESETTABLE_EXPORT_ENERGY:

        res = F("Resetable ");
        break;

      case SDM_AVERAGE_LINE_TO_LINE_VOLTS:
      case SDM_AVERAGE_L_TO_N_VOLTS:
      case SDM_AVERAGE_LINE_CURRENT:
      case SDM_AVERAGE_LINE_TO_NEUTRAL_VOLTS_THD:
      case SDM_AVERAGE_LINE_CURRENT_THD:
      case SDM_AVERAGE_LINE_TO_LINE_VOLTS_THD:
        res = F("Average ");
        break;

      case SDM_SUM_LINE_CURRENT:
        res = F("Sum ");
        break;

      default:
        break;
    }

    switch (getRegister())
    {
      case SDM_NEUTRAL_CURRENT_DEMAND:
      case SDM_MAXIMUM_NEUTRAL_CURRENT:
      case SDM_NEUTRAL_CURRENT:

        res += F("Neutral ");
        break;
      
      case SDM_LINE_1_TO_LINE_2_VOLTS:
      case SDM_LINE_1_TO_LINE_2_VOLTS_THD:
        res += F("L1 to L2 ");
        break;
      case SDM_LINE_2_TO_LINE_3_VOLTS:
      case SDM_LINE_2_TO_LINE_3_VOLTS_THD:
        res += F("L2 to L3 ");
        break;
      case SDM_LINE_3_TO_LINE_1_VOLTS:
      case SDM_LINE_3_TO_LINE_1_VOLTS_THD:
        res += F("L3 to L1 ");
        break;
      case SDM_AVERAGE_LINE_TO_LINE_VOLTS:
      case SDM_AVERAGE_LINE_TO_LINE_VOLTS_THD:
        res += F("Line to Line ");
        break;
      case SDM_AVERAGE_L_TO_N_VOLTS:
      case SDM_AVERAGE_LINE_TO_NEUTRAL_VOLTS_THD:
      case SDM_PHASE_1_LN_VOLTS_THD:
      case SDM_PHASE_2_LN_VOLTS_THD:
      case SDM_PHASE_3_LN_VOLTS_THD:
        res += F("L to N ");
        break;
      case SDM_AVERAGE_LINE_CURRENT:
      case SDM_AVERAGE_LINE_CURRENT_THD:
      case SDM_SUM_LINE_CURRENT:
        res += F("Line ");
        break;
    }
    
    if (direction != SDM_DIRECTION::NotSpecified) {
        res += SDM_directionToString(direction);
        res += ' ';
    }
    
    if (showFullUnitOfMeasure) {
      res += SDM_UOMtoString(uom, true);
    }

    switch (getRegister())
    {
      case SDM_VAH_SINCE_LAST_RESET:
      case SDM_AH_SINCE_LAST_RESET:

        res += F(" since last reset");
        break;
    }

    res += ' ';
    res += '(';
    res += SDM_UOMtoString(uom, false);
    res += ')';

    
    if (phase != 0) {
        res += ' ';
        res += 'L';
        res += phase;
    }
    return res;
}