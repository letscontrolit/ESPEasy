Import("env")
import os

# access to global construction environment
#print env

# Dump construction environment (for debug purpose)
#print(env.Dump())

# append extra flags to global build environment
# which later will be used to build:
# - project source code
# - frameworks
# - dependent libraries

custom_defines = []
#env.Append(CPPDEFINES=[
  # ,"NO_HTTP_UPDATER"
  # ,("WEBSERVER_RULES_DEBUG", "0")
#])
if os.path.isfile('src/Custom.h'):
  custom_defines=["-DUSE_CUSTOM_H"]
else:
  custom_defines=[
    "-DCONTROLLER_SET_ALL",
    "-DNOTIFIER_SET_NONE",
    "-DPLUGIN_SET_NONE",
    "-DUSES_P001",  # Switch
    "-DUSES_P002",  # ADC
    "-DUSES_P003",  # Generic Pulse Counter
    "-DUSES_P004",  # Dallas DS18b20

#    "-DFEATURE_MDNS=1",
#    "-DFEATURE_SD=1",

    "-DFEATURE_SETTINGS_ARCHIVE=1"
  ]

my_flags = env.ParseFlags(env['BUILD_FLAGS'])
my_defines = my_flags.get("CPPDEFINES")
env.Append(BUILD_FLAGS=custom_defines)
#defines = {k: v for (k, v) in my_defines}

print("\u001b[32m Custom PIO configuration check \u001b[0m")
# print the defines
print("\u001b[33m CPPDEFINES: \u001b[0m  {}".format(my_defines))
print("\u001b[33m Custom CPPDEFINES: \u001b[0m  {}".format(custom_defines))
print("\u001b[32m ------------------------------- \u001b[0m")


if (len(my_defines) == 0):
  print("\u001b[31m No defines are set, probably configuration error. \u001b[0m")
  raise ValueError



