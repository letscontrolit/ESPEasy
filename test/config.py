nodes=[
    {
        'type'      : 'wemos d1 mini v2.2.0',
        'port'      : '/dev/serial/by-path/pci-0000:00:14.0-usb-0:3.1:1.0-port0',
        'ip'        : '192.168.13.91',
        'flash_cmd' : 'esptool.py --port {port} -b 1500000  write_flash 0x0 .pioenvs/dev_4096/firmware.bin --flash_size=32m -p',
        'build_cmd' : 'platformio run --environment dev_4096'
    },

    {
        'type'      : 'nodemcu geekcreit ESP12E devkit v2',
        'port'      : '/dev/serial/by-path/pci-0000:00:14.0-usb-0:3.2:1.0-port0',
        'ip'        : '192.168.13.92',
        'flash_cmd' : 'esptool.py --port {port} -b 1500000  write_flash 0x0 .pioenvs/dev_4096/firmware.bin --flash_size=32m -p',
        'build_cmd' : 'platformio run --environment dev_4096'
    },

    {
        'type'      : 'wemos d1 mini v2.2.0',
        'port'      : '/dev/ttyUSB0',
        'ip'        : '192.168.13.93',
        'flash_cmd' : 'esptool.py --port {port} -b 1500000  write_flash 0x0 .pioenvs/dev_4096/firmware.bin --flash_size=32m -p',
        'build_cmd' : 'platformio run --environment dev_4096'
    },
]


mqtt_broker="192.168.13.236"

# TYPE2="geekcreit ESP12E devkit v2"
# SERIAL2=/dev/serial/by-path/pci-0000:00:14.0-usb-0:3.4.3:1.0-port0
# IP2=192.168.13.92
# FLASHCMD2="esptool.py --port $SERIAL2 -b 1500000  write_flash 0x0 .pioenvs/dev_4096/firmware.bin --flash_size=32m -p"
# #FLASHCMD2="platformio run --environment dev_4096 -t upload --upload-port $SERIAL2"
# BUILDCMD2="platformio run --environment dev_4096"
#
#
# TYPE3="esp-12-f"
# SERIAL3=/dev/serial/by-path/pci-0000:00:14.0-usb-0:3.4.2:1.0-port0
# IP3=192.168.13.93
# FLASHCMD3="esptool.py --port $SERIAL3 -b 1500000  write_flash 0x0 .pioenvs/dev_4096/firmware.bin --flash_size=32m -p"
# BUILDCMD3="platformio run --environment dev_4096"
#
