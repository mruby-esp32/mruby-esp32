#!/bin/bash

~/esp/mkspiffs/mkspiffs -c ./spiffs_data -b 4096 -p 256 -s 0x100000 spiffs_image

#python ~/esp/esp-idf/components/esptool_py/esptool/esptool.py --chip esp32 --port /dev/tty.usbserial-DN066N52 --baud 921600 write_flash -z 0x210000 spiffs_image


