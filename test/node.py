### Generic lowlevel ESP and espeasy per-node stuff.
### Used for things like flashing, serial communication, resetting, wificonfig, http communication

import serial
import sys
import time
import subprocess
import wificonfig
import requests
import serial.tools.miniterm
import re

from espcore import *


class Node():


    def __init__(self, config, id):
        self.log=logging.getLogger(id)
        self.log.debug("{type} has ip {ip}".format(id=id, **config))
        self._config=config
        self._id=id
        self._url="http://{ip}/".format(**self._config)
        self._serial_initialized=False

    def serial_needed(self):
        """call this at least once if you need serial stuff"""
        if not hasattr(self,"_serial"):
            self._serial=serial.Serial(port=self._config['port'], baudrate=115200, timeout=1, write_timeout=1)


    def pingserial(self, timeout=60):
        """waits until espeasy reponds via serial"""

        self.serial_needed()
        self._serial.reset_input_buffer();
        self.log.debug("Waiting for serial response")
        start_time=time.time()

        while (time.time()-start_time)< int(timeout):
                self._serial.write(bytes('.\n', 'ascii'));
                a=True
                while a!=b'':
                    a=self._serial.readline()
                    if a==b"Unknown command!\r\n":
                        self.log.debug("Got serial response")
                        self._serial.reset_input_buffer();
                        return


        raise(Exception("Timeout!"))


    def reboot(self):
        '''reboot the esp via the serial DTR line'''
        self.serial_needed()
        self._serial.setDTR(0)
        time.sleep(0.1)
        self._serial.setDTR(1)


    def pingwifi(self, timeout=60):
        """waits until espeasy reponds via wifi"""

        self.log.debug("Waiting for ping reply")
        start_time=time.time()

        while (time.time()-start_time)< int(timeout):
                # log(".")
                if not subprocess.call(["ping", "-w", "1", "-c", "1", self._config['ip']], stdout=subprocess.DEVNULL):
                    self.log.debug("Got ping reply")
                    return

        raise(Exception("Timeout!"))


    def wificonfig(self, timeout=60):
        """configure wifi via serial and make sure esp is online and pingable."""

        self.serial_needed()

        self.reboot()
        self.pingserial(timeout=timeout)

        self.log.info("Configuring wifi")

        serial_str="wifissid {ssid}\nwifikey {password}\nip {ip}\nsave\nreboot\n".format(ssid=wificonfig.ssid, password=wificonfig.password, ip=self._config['ip'])
        self._serial.write(bytes(serial_str, 'ascii'));

        self.pingwifi(timeout=timeout)


    def build(self):
        """compile binary"""

        subprocess.check_call(self._config['build_cmd'].format(**self._config), shell=True, cwd='..')


    def flashserial(self):
        """flash binary to esp via serial"""

        self.serial_needed()

        subprocess.check_call(self._config['flash_cmd'].format(**self._config), shell=True, cwd='..')

        time.sleep(1)
        #to prevent hangs when ESPEasy tries to reboot via ESP.reboot (due to an ESP/lib bug)
        self.reboot()


    def serial(self):
        """open serial terminal to esp"""
        self.serial_needed()
        subprocess.check_call("platformio serialports monitor --baud 115200 --port {port} --echo".format(**self._config), shell=True, cwd='..')
        # print("JA")
        # term=serial.tools.miniterm.Miniterm(self._serial)
        # term.start()
        # term.join()
        # print("kk")


    def erase(self):
        """erase flash via serial"""
        self.serial_needed()
        subprocess.check_call("esptool.py --port {port} -b 1500000  erase_flash".format(**self._config), shell=True, cwd='..')


    def bfs(self):
        """build + flashserial + serial, all in one"""
        self.build()
        self.flashserial()
        self.serial()



    def http_post(self, page, params,  data=None):
        """http post to espeasy webinterface. (GET if data is None)"""

        # transform easy copy/pastable chromium data into a dict

        params_dict={}
        for line in params.split("\n"):
            m=re.match(" *(.*?):(.*)",line)
            if (m):
                params_dict[m.group(1)]=m.group(2)

        if data:
            data_dict={}
            for line in data.split("\n"):
                m=re.match(" *(.*?):(.*)",line)
                if (m):
                    data_dict[m.group(1)]=m.group(2)
        else:
            data_dict=None



        r=requests.post(
            self._url+page,
            params=params_dict,
            data=data_dict
        )
        r.raise_for_status()
