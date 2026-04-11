# Part of ESPEasy build toolchain.
#
# Combines separate bin files with their respective offsets into a single file
# This single file must then be flashed to an ESP32 node with 0 offset.
#
# Original implementation: Bartłomiej Zimoń (@uzi18)
# Maintainer: Gijs Noorlander (@TD-er)
#
# Special thanks to @Jason2866 (Tasmota) for helping debug flashing to >4MB flash
# Thanks @jesserockz (esphome) for adapting to use esptool.py with merge_bin
#
# Typical layout of the generated file:
#    Offset | File
# -  0x1000 | ~\.platformio\packages\framework-arduinoespressif32\tools\sdk\esp32\bin\bootloader_dout_40m.bin
# -  0x8000 | ~\ESPEasy\.pio\build\<env name>\partitions.bin
# -  0xe000 | ~\.platformio\packages\framework-arduinoespressif32\tools\partitions\boot_app0.bin
# - 0x10000 | ~\ESPEasy\.pio\build\<env name>/<built binary>.bin

Import("env")

platform = env.PioPlatform()

import sys
import os
import subprocess
from os.path import join

def normalize_paths(cmd):
    for i, arg in enumerate(cmd):
        if isinstance(arg, str) and '/' in arg:
            cmd[i] = os.path.normpath(arg)
    return cmd

def esp32_create_combined_bin(source, target, env):
    print("Generating combined binary for serial flashing")

    # The offset from begin of the file where the app0 partition starts
    # This is defined in the partition .csv file
    app_offset = 0x10000

    new_file_name = env.subst("$BUILD_DIR/${PROGNAME}.factory.bin")
    sections = env.subst(env.get("FLASH_EXTRA_IMAGES"))
    firmware_name = env.subst("$BUILD_DIR/${PROGNAME}.bin")
    chip = env.get("BOARD_MCU")
    flash_size = env.BoardConfig().get("upload.flash_size", "4MB")
    flash_mode = env["__get_board_flash_mode"](env)
    flash_freq = env["__get_board_f_flash"](env)

    cmd = [
        "--chip",
        chip,
        "merge-bin",
        "-o",
        new_file_name,
        "--flash-mode",
        flash_mode,
        "--flash-freq",
        flash_freq,
        "--flash-size",
        flash_size,
    ]

    print("    Offset | File")
    for section in sections:
        sect_adr, sect_file = section.split(" ", 1)
        print(f" -  {sect_adr} | {sect_file}")
        cmd += [sect_adr, sect_file]

    print(f" - {hex(app_offset)} | {firmware_name}")
    cmd += [hex(app_offset), firmware_name]

    # print('Using esptool.py arguments: %s' % ' '.join(cmd))
    cmdline = [env.subst("$OBJCOPY")] + normalize_paths(cmd)
    print('Command Line: %s' % cmdline)
    result = subprocess.run(cmdline, text=True, check=False, stdout=subprocess.DEVNULL)
    if result.returncode != 0:
        print(f"esptool create firmware failed with exit code: {result.returncode}")


env.AddPostAction("$BUILD_DIR/${PROGNAME}.bin", esp32_create_combined_bin)
