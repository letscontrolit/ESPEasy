#include "../PluginStructs/P003_data_struct.h"


#ifdef USES_P003
P003_data_struct::P003_data_struct(const Internal_GPIO_pulseHelper::pulseCounterConfig& config)
  :
  pulseHelper(config)
{}

P003_data_struct::~P003_data_struct()
{}
#endif