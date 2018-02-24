#!/usr/bin/env python3

from esptest import *

# hardware requirements:
# - all nodes

# tests:
# - builds, flashes and resets to factory defaults

# prevent confusing messages during build
controller.log_enabled=False

# @step()
# def clean():
#     subprocess.check_call("cd ..;platformio run --target clean -s", shell=True)

@step()
def patch():
    subprocess.check_call("cd ../patches; ./check_puya_patch", shell=True)


@step()
def build():
    for n in node:
        n.build()


@step()
def erase():
    for n in node:
        n.erase()

@step()
def flash():
    for n in node:
        n.flashserial()

@step()
def wificonfig():
    for n in node:
        n.wificonfig()

controller.log_enabled=True







if __name__=='__main__':
    completed()
