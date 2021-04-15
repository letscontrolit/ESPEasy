nodes=[
    # node 0
    {
        'type'      : 'wemos d1 mini v2.2.0',
        'port'      : '/dev/serial/by-path/pci-0000:00:14.0-usb-0:3.1:1.0-port0',
        'ip'        : '192.168.13.91',
        'subnet'    : '255.255.255.0',
        'gateway'   : '192.168.13.1',
        'flash_cmd' : 'esptool.py --port {port} -b 1500000  write_flash 0x0 .pioenvs/dev_ESP8266_4096/firmware.bin --flash_size=32m -p',
        'build_cmd' : 'platformio run --environment dev_ESP8266_4096 -s'
    },

    # node 1
    {
        'type'      : 'nodemcu geekcreit ESP12E devkit v2',
        'port'      : '/dev/serial/by-path/pci-0000:00:14.0-usb-0:3.2:1.0-port0',
        'ip'        : '192.168.13.92',
        'subnet'    : '255.255.255.0',
        'gateway'   : '192.168.13.1',
        'flash_cmd' : 'esptool.py --port {port} -b 1500000  write_flash 0x0 .pioenvs/dev_ESP8266_4096/firmware.bin --flash_size=32m -p',
        'build_cmd' : 'platformio run --environment dev_ESP8266_4096 -s'
    },

    # node 2
    {
        'type'      : 'ESP-01S 1Mb PUYA flash',
        'port'      : '/dev/ttyUSB0',
        'ip'        : '192.168.13.55',
        'subnet'    : '255.255.255.0',
        'gateway'   : '192.168.13.1',
        'flash_cmd' : 'esptool.py --port {port} -b 115200  write_flash 0x0 .pioenvs/dev_ESP8266PUYA_1024/firmware.bin --flash_size=1MB -p',
        'build_cmd' : 'platformio run --environment dev_ESP8266PUYA_1024 -s'
    },

    # {
    #     'node'      : 3,
    #     'type'      : 'wemos d1 mini v2.2.0',
    #     'port'      : '/dev/ttyUSB0',
    #     'ip'        : '192.168.13.91',
    #     'flash_cmd' : 'esptool.py --port {port} -b 1500000  write_flash 0x0 .pioenvs/dev_ESP8266_4096/firmware.bin --flash_size=32m -p',
    #     'build_cmd' : 'platformio run --environment dev_ESP8266_4096'
    # },

]


#an mqtt broker that both ESPEasy and the test suite are connecting to
mqtt_broker="192.168.13.159"
mqtt_port=1883

#ip of the server running this script
test_server="192.168.13.159"
http_port=8080
linebased_port=8181
