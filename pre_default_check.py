# Simple check for the configuration of an environment.


Import("env")
import os

# access to global construction environment
#print env

# Dump construction environment (for debug purpose)
#print env.Dump()

my_flags = env.ParseFlags(env['BUILD_FLAGS'])


# print the defines
print("CPPDEFINES: {}".format(my_flags.get("CPPDEFINES")))
print("LINKFLAGS: {}".format(my_flags.get("LINKFLAGS")))
#print(my_flags)


if (len(my_flags.get("CPPDEFINES")) == 0):
  raise ValueError
