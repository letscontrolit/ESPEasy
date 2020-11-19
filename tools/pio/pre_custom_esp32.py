Import("env")
import os

# access to global construction environment
#print env

# Dump construction environment (for debug purpose)
#print env.Dump()

# append extra flags to global build environment
# which later will be used to build:
# - project source code
# - frameworks
# - dependent libraries
env.Append(CPPDEFINES=[
#  ("WEBSERVER_RULES_DEBUG", "1")
])
if os.path.isfile('src/Custom.h'):
  env.Append(CPPDEFINES=["USE_CUSTOM_H"])
else:
  env.Append(CPPDEFINES=[
    "CONTROLLER_SET_ALL",
    "NOTIFIER_SET_NONE",
    "PLUGIN_SET_ONLY_SWITCH",
    "USES_P001",  # Switch
    "USES_P002",  # ADC
    "USES_P004",  # Dallas DS18b20
    "USES_P026",  # System info
    "USES_P027",  # INA219
    "USES_P028",  # BME280
    "USES_P036",  # FrameOLED
    "USES_P045",  # MPU6050
    "USES_P049",  # MHZ19
    "USES_P052",  # SenseAir
    "USES_P056",  # SDS011-Dust
    "USES_P059",  # Encoder
    "USES_P080",  # Dallas iButton
    "USES_P081",  # Cron
    "USES_P082",  # GPS
    "USES_P085",  # AcuDC24x
    "USES_P100",  # Pulse Counter - DS2423
#   "USES_P087",  # Serial Proxy
#   "USES_P094",  # CUL Reader
#   "USES_P095",  # TFT ILI9341
    "USES_P097",  # Touch (ESP32)

    "USES_C016",  # Cache Controller
    "USES_C018",  # TTN/RN2483
#    "USES_C015",  # TTN/RN2483

    "USE_SETTINGS_ARCHIVE",
    "FEATURE_I2CMULTIPLEXER",
    "FEATURE_ARDUINO_OTA"
  ])

print(env['CPPDEFINES'])
