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


    def __init__(self, config, id, skip_power=False):
        self.log=logging.getLogger(id)
        self.log.debug("{type} has ip {ip}".format(id=id, **config))
        self._config=config
        self._id=id
        self._url="http://{ip}/".format(**self._config)
        self._serial_initialized=False
        self._skip_power=True

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
        self.log.debug("Rebooting")

        self.serial_needed()
        self._serial.setDTR(0)
        time.sleep(0.1)
        self._serial.setDTR(1)

    def powercycle(self):
        """powercycle the device"""
        if self._skip_power:
            self.log.warning("Skipping power cycle "+self._id)
            return False


        self.poweroff()
        self.poweron()
        return True

    def poweroff(self):
        """power off device"""

        if self._skip_power:
            self.log.warning("Skipping power off "+self._id)
            return False

        #cant yet be done automaticly unfortunatly
        self.log.info("Please power off node "+self._id)

        done=False
        while not done:
            try:
                self.serial_needed()
                self._serial.readline()
            except serial.SerialException:
                done=True
                if hasattr(self, '_serial'):
                    del self._serial

        self.log.debug("Detected power off")
        return True

    def poweron(self):
        """power on device"""

        if self._skip_power:
            self.log.warning("Skipping power on"+self._id)
            return False

        self.log.info("Please power on node "+self._id)

        done=False
        while not done:
            try:
                self.serial_needed()
                self._serial.readline()
                done=True
            except serial.SerialException:
                time.sleep(0.1)

        self.log.debug("Detected power on")
        return True

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

        serial_str="wifissid {ssid}\nwifikey {password}\nip {ip}\nsubnet {subnet}\ngateway {gateway}\nsave\nreboot\n".format(ssid=wificonfig.ssid, password=wificonfig.password, ip=self._config['ip'], subnet=self._config['subnet'], gateway=self._config['gateway'])
        self._serial.write(bytes(serial_str, 'ascii'));

        self.pingwifi(timeout=timeout)


    def serialcmd(self, command):
        """send command via serial"""

        self.serial_needed()
        self.log.debug("Send serial command: "+command)
        serial_str=command+"\n"
        self._serial.write(bytes(serial_str, 'ascii'));


    def build(self):
        """compile binary"""

        self.log.debug("Building...")
        subprocess.check_call(self._config['build_cmd'].format(**self._config), shell=True, cwd='..')


    def flashserial(self):
        """flash binary to esp via serial"""

        self.serial_needed()

        self.log.debug("Flashing...")
        subprocess.check_call(self._config['flash_cmd'].format(**self._config), shell=True, cwd='..')

        time.sleep(1)
        #to prevent hangs when ESPEasy tries to reboot via ESP.reboot (due to an ESP/lib bug)
        self.reboot()


    def serial(self):
        """open serial terminal to esp"""
        self.serial_needed()
        self.log.debug("Opening serial terminal")
        subprocess.check_call("platformio serialports monitor --baud 115200 --port {port} --echo".format(**self._config), shell=True, cwd='..')
        # print("JA")
        # term=serial.tools.miniterm.Miniterm(self._serial)
        # term.start()
        # term.join()
        # print("kk")


    def erase(self):
        """erase flash via serial"""
        self.serial_needed()
        self.log.debug("Erasing...")
        subprocess.check_call("esptool.py --port {port} -b 1500000  erase_flash".format(**self._config), shell=True, cwd='..')


    def bfs(self):
        """build + flashserial + serial, all in one"""
        self.build()
        self.flashserial()
        self.serial()

    def bf(self):
        """build + flashserial"""
        self.build()
        self.flashserial()


    def http_post(self, page, params=None,  data=None, twice=False):
        """http post to espeasy webinterface. (GET if data is None)"""

        # transform easy copy/pastable chromium data into a dict

        if params:
            params_dict={}
            for line in params.split("\n"):
                m=re.match(" *(.*?):(.*)",line)
                if (m):
                    params_dict[m.group(1)]=m.group(2)
        else:
            params_dict=None

        if data:
            data_dict={}
            for line in data.split("\n"):
                m=re.match(" *(.*?):(.*)",line)
                if (m):
                    data_dict[m.group(1)]=m.group(2)
        else:
            data_dict=None


        url=self._url+page
        self.log.debug("HTTP POST {url} with params {params} and data {data}".format(url=url,params=params,data=data))

        r=requests.post(
            url,
            params=params_dict,
            data=data_dict
        )
        r.raise_for_status()

        if twice:
            r=requests.post(
                self._url+page,
                params=params_dict,
                data=data_dict
            )
            r.raise_for_status()
