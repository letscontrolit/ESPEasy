#ifndef DATASTRUCTS_EXTENDED_CONTROLLERCREDENTIALSSTRUCT_H
#define DATASTRUCTS_EXTENDED_CONTROLLERCREDENTIALSSTRUCT_H

#include "../../ESPEasy_common.h"
#include "../CustomBuild/ESPEasyLimits.h"
#include "../Globals/CPlugins.h"


/*********************************************************************************************\
 * Extended SecurityStruct
\*********************************************************************************************/
struct ExtendedControllerCredentialsStruct
{
  ExtendedControllerCredentialsStruct();

  // Compute checksum of the data.
  // @retval true when checksum matches
  bool validateChecksum() const;

  String load();
  String save() const;

  String getControllerUser(controllerIndex_t controller_idx) const;
  String getControllerPass(controllerIndex_t controller_idx) const;

  void setControllerUser(controllerIndex_t controller_idx, const String& user);
  void setControllerPass(controllerIndex_t controller_idx, const String& pass);

private:

  String _strings[CONTROLLER_MAX * 2]{};

  // TODO TD-er: Add extra WiFi credentials
};



#endif // DATASTRUCTS_EXTENDED_CONTROLLERCREDENTIALSSTRUCT_H
