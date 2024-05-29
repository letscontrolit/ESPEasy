Import("env")
import os

# Check for IR and IRext builds and use a Custom-IR.h file if available, uses Custom-IR-sample.h for defaults
pioenv = env['PIOENV']
# print("***** PIOENV: ",pioenv)
if "_IR_" in pioenv or "_IRext_" in pioenv:
  if os.path.isfile('src/CustomIR.h'):
    print("\n\u001b[35m *** CustomIR.h build detected. Create a clean build after changes to CustomIR.h *** \u001b[0m\n")
    ir_defines=["-include src/CustomIR.h"]
    env.Append(BUILD_FLAGS=ir_defines)
  else:
    # For all ESP32 IR builds enable all IR protocols, except when CustomIR.h is available (above)
    if "_ESP32" in pioenv:
      print("\n ESP32 IR build. See CustomIR-sample.h how to customize included IR protocols.\n Default: All protocols enabled.\n")
    else:
      if os.path.isfile('src/CustomIR-sample.h'):
        print(" IR build detected. Using CustomIR-sample.h for defaults.")
        ir_defines=["-include src/CustomIR-sample.h"] # Use as default settings
        env.Append(BUILD_FLAGS=ir_defines)

