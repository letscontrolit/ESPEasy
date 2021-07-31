#ifndef DATASTRUCTS_EXTENDED_SECURITYSTRUCT_H
#define DATASTRUCTS_EXTENDED_SECURITYSTRUCT_H

#include "../../ESPEasy_common.h"
#include "../CustomBuild/ESPEasyLimits.h"
#include "../Globals/CPlugins.h"


/*********************************************************************************************\
 * Extended SecurityStruct
\*********************************************************************************************/
struct ExtendedControllerCredentialsStruct
{
  ExtendedControllerCredentialsStruct();

  String load();
  String save() const;

  String getControllerUser(controllerIndex_t controller_idx) const;
  String getControllerPass(controllerIndex_t controller_idx) const;

  void setControllerUser(controllerIndex_t controller_idx, const String& user);
  void setControllerPass(controllerIndex_t controller_idx, const String& pass);

  private:

  String _strings[CONTROLLER_MAX * 2];


  // TODO TD-er: Add extra WiFi credentials
};



#endif // DATASTRUCTS_EXTENDED_SECURITYSTRUCT_H