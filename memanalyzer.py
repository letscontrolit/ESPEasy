#!/usr/bin/env python
########################################################
#
# ESPEasy plugin memory analyser
# edwin@datux.nl
#
###
#  Based on:
#  https://raw.githubusercontent.com/SmingHub/Sming/develop/tools/memanalyzer.py
#  Memory Analyzer
#  Author: Slavey Karadzhov <slav@attachix.com>
#  Based on https://github.com/Sermus/ESP8266_memory_analyzer
#
########################################################
from collections import OrderedDict
import os.path
import shlex
import subprocess
import sys
import glob
import os

TOTAL_IRAM = 32786;
TOTAL_DRAM = 81920;

sections = OrderedDict([
    ("data", "Initialized Data (RAM)"),
    ("rodata", "ReadOnly Data (RAM)"),
    ("bss", "Uninitialized Data (RAM)"),
    ("text", "Cached Code (IRAM)"),
    ("irom0_text", "Uncached Code (SPI)")
])


def abort(txt):
    raise(Exception("error: "+txt))

def analyse_memory(elfFile):
    command = "%s -t '%s' " % (objectDumpBin, elfFile)
    response = subprocess.check_output(shlex.split(command))
    if isinstance(response, bytes):
        response = response.decode('utf-8')
    lines = response.split('\n')

    # print("{0: >10}|{1: >30}|{2: >12}|{3: >12}|{4: >8}".format("Section", "Description", "Start (hex)", "End (hex)", "Used space"));
    # print("------------------------------------------------------------------------------");
    ret={}
    usedRAM = 0
    usedIRAM = 0

    i = 0
    for (id, descr) in list(sections.items()):
        sectionStartToken = " _%s_start" %  id
        sectionEndToken   = " _%s_end" % id
        sectionStart = -1
        sectionEnd = -1
        for line in lines:
            if sectionStartToken in line:
                data = line.split(' ')
                sectionStart = int(data[0], 16)

            if sectionEndToken in line:
                data = line.split(' ')
                sectionEnd = int(data[0], 16)

            if sectionStart != -1 and sectionEnd != -1:
                break

        sectionLength = sectionEnd - sectionStart
        # if i < 3:
        #     usedRAM += sectionLength
        # if i == 3:
        #     usedIRAM = TOTAL_IRAM - sectionLength;

        ret[id]=sectionLength
        # print("{0: >10}|{1: >30}|{2:12X}|{3:12X}|{4:8}".format(id, descr, sectionStart, sectionEnd, sectionLength))
        # i += 1

    # print("Total Used RAM : %d" % usedRAM)
    # print("Free RAM : %d" % (TOTAL_DRAM - usedRAM))
    # print("Free IRam : %d" % usedIRAM)
    return(ret)

################### start
if len(sys.argv) < 1:
    print("Usage: \n\t%s%s <path_to_objdump>" % sys.argv[0])
    sys.exit(1)

objectDumpBin = sys.argv[1]

#restore deleted plugins
subprocess.check_output("git ls-files -d | xargs git checkout --", shell=True)

#modified files?
if subprocess.check_output("git ls-files -m src", shell=True)!="":
    abort("found modified files in src directory")

#get list of all plugins
plugins=glob.glob("src/_[CPN]*.ino")
plugins.sort()


#remove all plugins and get base size
for plugin in plugins:
    os.remove(plugin)

if len(sys.argv)>2:
    test_plugins=sys.argv[2:]
else:
    test_plugins=plugins

test_plugins.sort()



output_format="{:<30}|{:<11}|{:<11}|{:<11}|{:<11}|{:<11}"
print(output_format.format(
    "plugin",
    "cache IRAM",
    "init RAM",
    "r.o. RAM",
    "uninit RAM",
    "Flash ROM",
))


#build without plugins to get base memory usage
subprocess.check_call("platformio run --silent --environment dev_4096", shell=True)
#two times, sometimes it changes a few bytes somehow
subprocess.check_call("platformio run --silent --environment dev_4096", shell=True)
base=analyse_memory(".pioenvs/dev_4096/firmware.elf")


plugin_results={}
for plugin in test_plugins:
    # print("building with {}".format(plugin))
    subprocess.check_call("git checkout {}".format(plugin), shell=True)
    subprocess.check_call("platformio run --silent --environment dev_4096", shell=True)
    plugin_results[plugin]=analyse_memory(".pioenvs/dev_4096/firmware.elf")
    os.remove(plugin)

    print(output_format.format(
        plugin,
        plugin_results[plugin]['text']-base['text'],
        plugin_results[plugin]['data']-base['data'],
        plugin_results[plugin]['rodata']-base['rodata'],
        plugin_results[plugin]['bss']-base['bss'],
        plugin_results[plugin]['irom0_text']-base['irom0_text'],
    ))
