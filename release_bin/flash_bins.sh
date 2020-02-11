#!/bin/bash

# $1 : serial port 

python $IDF_PATH/components/esptool_py/esptool/esptool.py --chip esp32 --port $1 --baud 921600 --before default_reset --after hard_reset write_flash -z --flash_mode dio --flash_freq 40m --flash_size detect 0x1000 ./bootloader.bin 0x10000 ./family_mruby.bin 0x8000 ./partitions.bin


python $IDF_PATH/components/esptool_py/esptool/esptool.py --chip esp32 --port $1 --baud 921600 write_flash -z 0x210000 ./spiffs_image
