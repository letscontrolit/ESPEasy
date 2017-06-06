import serial
import sys
import time
import subprocess
import wificonfig

def log(txt):
    print(txt, end="", flush=True)


class Esp():
    def __init__(self, config):
        print("Using unit {unit} ({type}) with ip {ip}".format(**config))
        self.config=config
        self.serial=serial.Serial(port=config['port'], baudrate=115200, timeout=1, write_timeout=1)



    def pingserial(self, timeout=60):
        """waits until espeasy reponds via serial"""
        self.serial.reset_input_buffer();
        log("Waiting for serial response: ")
        start_time=time.time()

        while (time.time()-start_time)< int(timeout):
                self.serial.write(bytes('\n', 'ascii'));
                a=True
                while a!=b'':
                    a=self.serial.readline()
                    if a==b"Unknown command!\r\n":
                        log("OK\n")
                        self.serial.reset_input_buffer();
                        return
                log(".")

        raise(Exception("Timeout!"))


    def reboot(self):
        '''reboot the esp via the serial DTR line'''
        self.serial.setDTR(0)
        time.sleep(0.1)
        self.serial.setDTR(1)


    def pingwifi(self, timeout=60):
        """waits until espeasy reponds via wifi"""

        log("Waiting for wifi response: ")
        start_time=time.time()

        while (time.time()-start_time)< int(timeout):
                log(".")
                if not subprocess.call(["ping", "-w", "1", "-c", "1", self.config['ip']], stdout=subprocess.DEVNULL):
                    log("OK\n")
                    return

        raise(Exception("Timeout!"))


    def wificonfig(self, timeout=60):
        """configure wifi via serial and make sure esp is online and pingable."""
        self.pingserial(timeout=timeout)

        serial_str="wifissid {ssid}\nwifikey {password}\nip {ip}\nsave\nreboot\n".format(ssid=wificonfig.ssid, password=wificonfig.password, ip=self.config['ip'])
        self.serial.write(bytes(serial_str, 'ascii'));
        self.pingwifi(timeout=timeout)
