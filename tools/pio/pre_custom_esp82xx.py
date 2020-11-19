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
  # ,"NO_HTTP_UPDATER"
  # ,("WEBSERVER_RULES_DEBUG", "0")
])
if os.path.isfile('src/Custom.h'):
  env.Append(CPPDEFINES=["USE_CUSTOM_H"])
else:
  env.Append(CPPDEFINES=[
    "CONTROLLER_SET_ALL",
    "NOTIFIER_SET_NONE",
#    "PLUGIN_BUILD_NORMAL",
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

    "USES_C016",  # Cache Controller
    "USES_C018",  # TTN/RN2483
#    "USES_C015",  # TTN/RN2483

    "FEATURE_MDNS",
    "FEATURE_SD",
    "FEATURE_I2CMULTIPLEXER",

    "USE_SETTINGS_ARCHIVE"
  ])



my_flags = env.ParseFlags(env['BUILD_FLAGS'])
my_defines = my_flags.get("CPPDEFINES")
#defines = {k: v for (k, v) in my_defines}

print("\u001b[32m Custom PIO configuration check \u001b[0m")
# print the defines
print("\u001b[33m CPPDEFINES: \u001b[0m  {}".format(my_defines))
print("\u001b[33m Custom CPPDEFINES: \u001b[0m  {}".format(env['CPPDEFINES']))
print("\u001b[32m ------------------------------- \u001b[0m")


if (len(my_defines) == 0):
  print("\u001b[31m No defines are set, probably configuration error. \u001b[0m")
  raise ValueError



