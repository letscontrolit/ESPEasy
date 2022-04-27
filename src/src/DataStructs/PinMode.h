#ifndef DATASTRUCTS_PINMODE_H
#define DATASTRUCTS_PINMODE_H

// FIXME TD-er: Move this to DataTypes when GPIO PR is merged.

#define PIN_MODE_UNDEFINED                  0
#define PIN_MODE_INPUT                      1
#define PIN_MODE_OUTPUT                     2
#define PIN_MODE_PWM                        3
#define PIN_MODE_SERVO                      4
#define PIN_MODE_INPUT_PULLUP               5
#define PIN_MODE_OFFLINE                    6
#define PIN_MODE_INPUT_PULLDOWN             7


#define PLUGIN_GPIO          1
#define PLUGIN_MCP           9
#define PLUGIN_PCF          19

#define GPIO_TYPE_INVALID   0
#define GPIO_TYPE_INTERNAL  1
#define GPIO_TYPE_MCP       2
#define GPIO_TYPE_PCF       3


#define SEARCH_PIN_STATE                 true
#define NO_SEARCH_PIN_STATE             false

#endif // DATASTRUCTS_PINMODE_H