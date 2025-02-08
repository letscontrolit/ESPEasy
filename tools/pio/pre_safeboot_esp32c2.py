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
custom_defines = []
#env.Append(CPPDEFINES=[
  # ,"NO_HTTP_UPDATER"
  # ,("WEBSERVER_RULES_DEBUG", "0")
#])
if os.path.isfile('src/Custom.h'):
  custom_defines=["-DUSE_CUSTOM_H"]
else:
  custom_defines=[
    "-DCONTROLLER_SET_NONE",
    "-DNOTIFIER_SET_NONE",
    "-DPLUGIN_BUILD_NONE",
    "-DBUILD_NO_RAM_TRACKER",
    "-DLIMIT_BUILD_SIZE",
    "-DWEBSERVER_CUSTOM_BUILD_DEFINED",
    "-DWEBSERVER_ROOT",
    "-DWEBSERVER_ADVANCED",
    "-DWEBSERVER_CONFIG",
    "-DWEBSERVER_SETUP",
    "-DWEBSERVER_SYSINFO",
    "-DWEBSERVER_TOOLS",



    "-DFEATURE_SETTINGS_ARCHIVE=1",
    "-DFEATURE_DEFINE_SERIAL_CONSOLE_PORT=0",
    "-DFEATURE_ESPEASY_P2P=1",
    "-DFEATURE_CUSTOM_PROVISIONING",
    "-DDISABLE_SC16IS752_SPI"
  ]



my_flags = env.ParseFlags(env['BUILD_FLAGS'])
my_defines = my_flags.get("CPPDEFINES")
env.Append(CXXFLAGS=custom_defines)
#defines = {k: v for (k, v) in my_defines}

print("\u001b[32m Custom PIO configuration check \u001b[0m")
# print the defines
print("\u001b[33m CPPDEFINES: \u001b[0m  {}".format(my_defines))
print("\u001b[33m Custom CPPDEFINES: \u001b[0m  {}".format(custom_defines))
print("\u001b[32m ------------------------------- \u001b[0m")


if (len(my_defines) == 0):
  print("\u001b[31m No defines are set, probably configuration error. \u001b[0m")
  raise ValueError