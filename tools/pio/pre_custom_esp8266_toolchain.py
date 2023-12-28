
Import("env")

env.Replace(
    AR="xtensa-lx106-elf-gcc-ar",
    RANLIB="xtensa-lx106-elf-gcc-ranlib"
)

# May need to manually change in platform of the esp8266 in folder 
# .platformio/../platforms/esp8266xxx in python script builder/main.py 
