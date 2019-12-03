Import("env")
import os

# Simple check for the configuration of an environment.

# access to global construction environment
#print(env)

# Dump construction environment (for debug purpose)
#print(env.Dump())


my_flags = env.ParseFlags(env['BUILD_FLAGS'])
my_defines = my_flags.get("CPPDEFINES")
#defines = {k: v for (k, v) in my_defines}

print("\u001b[32m Default PIO configuration check \u001b[0m", flush=True)
# print the defines
print("\u001b[33m CPPDEFINES: \u001b[0m  {}".format(my_defines), flush=True)
print("\u001b[32m ------------------------------- \u001b[0m", flush=True)


if (len(my_defines) == 0):
  print("\u001b[31m No defines are set, probably configuration error. \u001b[0m", flush=True)
  raise ValueError

