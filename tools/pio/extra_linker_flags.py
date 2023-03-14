Import("env")

#
# Dump build environment (for debug)
# print(env.Dump())
#

# make sure linker knows that we need debug info (for decoding backtraces)
env.Append(
  LINKFLAGS=[
      "-ggdb",
  ]
)