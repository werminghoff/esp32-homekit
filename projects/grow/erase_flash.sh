#!/bin/sh

esptool.py \
      --chip esp32 \
      --port /dev/tty.SLAB_USBtoUART \
      --baud 115200 \
      erase_flash
