#include "src/Helpers/_CPlugin_Helper.h"

#ifdef USES_C020

// #######################################################################################################
// ########################### Controller Plugin 020: CAN - SJA1000 ######################################
// #######################################################################################################

# define CPLUGIN_020
# define CPLUGIN_ID_020         20
# define CPLUGIN_NAME_020       "CAN 2.0 - TWAI"

# include "src/DataTypes/ESPEasy_plugin_functions.h"
# include "src/Globals/CPlugins.h"
# include "src/Helpers/_Plugin_Helper_CAN.h"

bool CPlugin_020(CPlugin::Function function, struct EventStruct *event, String& string)
{
  bool success = false;

  switch (function)
  {
    case CPlugin::Function::CPLUGIN_PROTOCOL_ADD:
    {
      ProtocolStruct& proto = getProtocolStruct(event->idx); //        = CPLUGIN_ID_020;
      /*
        // These are by default already set to false
        proto.usesMQTT = false;
        proto.usesAccount = false;
        proto.usesPassword = false;
        proto.usesTemplate = false;
        proto.usesID = false;
        proto.Custom = false;
        proto.usesSampleSets = false;
        proto.usesExtCreds = false;
        proto.allowLocalSystemTime = false;
      */
      proto.usesHost = false;
      proto.usesPort = false;
      proto.usesQueue = false;
      proto.usesCheckReply = false;
      proto.usesTimeout = false;
      proto.needsNetwork = false;
      proto.allowsExpire = false;
      break;
    }

    case CPlugin::Function::CPLUGIN_GET_DEVICENAME:
    {
      string = F(CPLUGIN_NAME_020);
      break;
    }

    case CPlugin::Function::CPLUGIN_INIT:
    {
      break;
    }

    case CPlugin::Function::CPLUGIN_EXIT:
    {
      break;
    }

    case CPlugin::Function::CPLUGIN_PROTOCOL_SEND:
    {
      canHelper_sendTaskData(event);
      break;
    }

    case CPlugin::Function::CPLUGIN_FIFTY_PER_SECOND:
    {
      canHelper_recv();
      break;
    }

    // case CPlugin::Function::CPLUGIN_INTERVAL:
    // {
    //   for (taskIndex_t x = 0; x < TASKS_MAX; x++) {
    //     constexpr pluginID_t PLUGIN_ID_CAN_HELPER(155);
    //     if (Settings.TaskDeviceEnabled[x] && (Settings.getPluginID_for_task(x) == PLUGIN_ID_CAN_HELPER))
    //     {
    //       EventStruct tmp;
    //       String dummy;
    //       unsigned int val = 0;

    //       tmp.Par1 = 0;
    //       tmp.idx = 2;
    //       tmp.TaskIndex = x;
    //       PluginCall(PLUGIN_READ, &tmp, dummy);
    //     }
    //   }
      
    //   break;
    // }

    // case CPlugin::Function::CPLUGIN_FLUSH:
    // {
    //   delay(0);
    //   break;
    // }

    default:
      break;
  }
  return success;
}

#endif // ifdef USES_C020
