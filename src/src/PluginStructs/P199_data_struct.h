#ifndef PLUGINSTRUCTS_P199_DATA_STRUCT_H
#define PLUGINSTRUCTS_P199_DATA_STRUCT_H

#include "../../ESPEasy_common.h"

#ifdef USES_P199
# include "../../_Plugin_Helper.h"
# include <ESPeasySerial.h>


# define P199_DEV_ID          PCONFIG(0)
# define P199_DEV_ID_LABEL    PCONFIG_LABEL(0)
# define P199_MODEL           PCONFIG(1)
# define P199_MODEL_LABEL     PCONFIG_LABEL(1)
# define P199_BAUDRATE        PCONFIG(2)
# define P199_BAUDRATE_LABEL  PCONFIG_LABEL(2)

# define P199_GET_FLAG_COLL_DETECT bitRead(PCONFIG(7), 0)
# define P199_SET_FLAG_COLL_DETECT(x) bitWrite(PCONFIG(7), 0, x)
# define P199_FLAG_COLL_DETECT_LABEL "colldet"

# define P199_QUERY1_CONFIG_POS  3

# define P199_DEPIN           CONFIG_PIN3

# define P199_DEV_ID_DFLT     1
# define P199_MODEL_DFLT      0 // SDM120C
# define P199_BAUDRATE_DFLT   3 // 9600 baud



#endif // ifdef USES_P199

#endif // ifndef PLUGINSTRUCTS_P199_DATA_STRUCT_H
