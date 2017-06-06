import serial
import sys
import time

def log(txt):
    print(txt, end="", flush=True)


class Esp():
    def __init__(self, config):
        print("Using unit {unit} ({type}) with ip {ip}".format(**config))
        self.config=config
        self.serial=serial.Serial(port=config['port'], baudrate=115200, timeout=1, write_timeout=1)



    def pingserial(self, timeout=60):
        """send enters until espeasy responds"""
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
