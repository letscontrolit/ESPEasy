#ifndef NOTIFIERSTRUCTS_N001_DATA_STRUCT_H
#define NOTIFIERSTRUCTS_N001_DATA_STRUCT_H

#include "../../_Plugin_Helper.h"

#ifdef USES_N001

// FIXME TD-er: Must we add a complete inheritance structure like done with pluginTaskData?
// We do not have multiple instances of these notifiers, so let's keep it simple.


# define NPLUGIN_001_PKT_SZ     256

# include <base64.h>

# include "../DataStructs/NotificationSettingsStruct.h"
# include "../Globals/NPlugins.h"


bool NPlugin_001_send(const NotificationSettingsStruct& notificationsettings,
                      const String                    & aSub,
                      String                          & aMesg);
bool NPlugin_001_Auth(WiFiClient  & client,
                      const String& user,
                      const String& pass,
                      uint16_t      timeout);
bool NPlugin_001_MTA(WiFiClient  & client,
                     const String& aStr,
                     uint16_t      aWaitForPattern,
                     uint16_t      timeout);
bool getNextMailAddress(const String& data,
                        String      & address,
                        int           index);


#endif // ifdef USES_N001

#endif // ifndef NOTIFIERSTRUCTS_N001_DATA_STRUCT_H
