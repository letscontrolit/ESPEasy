import serial
import sys
import time
import subprocess
import wificonfig
import requests
import serial.tools.miniterm


def log(txt):
    print(txt, end="", flush=True)


class Esp():


    def __init__(self, config):
        print("Using unit {unit} ({type}) with ip {ip}".format(**config))
        self._config=config
        self._serial=serial.Serial(port=config['port'], baudrate=115200, timeout=1, write_timeout=1)
        self._url="http://{ip}/".format(**self._config)


    def pingserial(self, timeout=60):
        """waits until espeasy reponds via serial"""
        self._serial.reset_input_buffer();
        log("Waiting for serial response: ")
        start_time=time.time()

        while (time.time()-start_time)< int(timeout):
                self._serial.write(bytes('.\n', 'ascii'));
                a=True
                while a!=b'':
                    a=self._serial.readline()
                    if a==b"Unknown command!\r\n":
                        log("OK\n")
                        self._serial.reset_input_buffer();
                        return
                log(".")

        raise(Exception("Timeout!"))


    def reboot(self):
        '''reboot the esp via the serial DTR line'''
        self._serial.setDTR(0)
        time.sleep(0.1)
        self._serial.setDTR(1)


    def pingwifi(self, timeout=60):
        """waits until espeasy reponds via wifi"""

        log("Waiting for wifi response: ")
        start_time=time.time()

        while (time.time()-start_time)< int(timeout):
                log(".")
                if not subprocess.call(["ping", "-w", "1", "-c", "1", self._config['ip']], stdout=subprocess.DEVNULL):
                    log("OK\n")
                    return

        raise(Exception("Timeout!"))


    def wificonfig(self, timeout=60):
        """configure wifi via serial and make sure esp is online and pingable."""

        self.pingserial(timeout=timeout)

        serial_str="wifissid {ssid}\nwifikey {password}\nip {ip}\nsave\nreboot\n".format(ssid=wificonfig.ssid, password=wificonfig.password, ip=self._config['ip'])
        self._serial.write(bytes(serial_str, 'ascii'));

        self.pingwifi(timeout=timeout)


    def build(self):
        """compile binary"""

        subprocess.check_call(self._config['build_cmd'].format(**self._config), shell=True, cwd='..')


    def flashserial(self):
        """flash binary to esp via serial"""


        subprocess.check_call(self._config['flash_cmd'].format(**self._config), shell=True, cwd='..')

        time.sleep(1)
        #to prevent hangs when ESPEasy tries to reboot via ESP.reboot (due to an ESP/lib bug)
        self.reboot()


    def serial(self):
        """open serial terminal to esp"""
        subprocess.check_call("platformio serialports monitor --baud 115200 --port {port} --echo".format(**self._config), shell=True, cwd='..')
        # print("JA")
        # term=serial.tools.miniterm.Miniterm(self._serial)
        # term.start()
        # term.join()
        # print("kk")


    def erase(self):
        """erase flash via serial"""
        subprocess.check_call("esptool.py --port {port} -b 1500000  erase_flash".format(**self._config), shell=True, cwd='..')


    def bfs(self):
        """build + flashserial + serial, all in one"""
        self.build()
        self.flashserial()
        self.serial()


    def config_device(self):

        r=requests.post(
            self._url+"devices",
            params={
                'index':1,
                'page':1
            },
            data={
                'TDNUM':1,
                'TDN': "",
                'TDE': 'on',
                'taskdevicepin1': 12,
                'plugin_001_type':1,
                'plugin_001_button':0,
                'TDT':0,
                'TDSD1':'on',
                'TDID1':1,
                'TDVN1':'Switch',
                'edit':1,
                'page':1
            }
        )

        print(r.url)
