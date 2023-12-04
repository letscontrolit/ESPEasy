#ifndef DATASTRUCTS_PINMODE_H
#define DATASTRUCTS_PINMODE_H

#include "../DataTypes/PluginID.h"

// FIXME TD-er: Move this to DataTypes when GPIO PR is merged.

#define PIN_MODE_UNDEFINED                  0
#define PIN_MODE_INPUT                      1
#define PIN_MODE_OUTPUT                     2
#define PIN_MODE_PWM                        3
#define PIN_MODE_SERVO                      4
#define PIN_MODE_INPUT_PULLUP               5
#define PIN_MODE_OFFLINE                    6
#define PIN_MODE_INPUT_PULLDOWN             7


#define PLUGIN_GPIO_INT  1
#define PLUGIN_MCP_INT  9
#define PLUGIN_PCF_INT  19


extern const pluginID_t PLUGIN_GPIO;
extern const pluginID_t PLUGIN_MCP;
extern const pluginID_t PLUGIN_PCF;



#define GPIO_TYPE_INVALID   0
#define GPIO_TYPE_INTERNAL  1
#define GPIO_TYPE_MCP       2
#define GPIO_TYPE_PCF       3


#define SEARCH_PIN_STATE                 true
#define NO_SEARCH_PIN_STATE             false

#endif // DATASTRUCTS_PINMODE_H