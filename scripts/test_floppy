#!/bin/bash

#Simulate 
#qemu -hda bin/hdd.img -m 32 -monitor stdio

sudo losetup /dev/loop1 bin/hdd.img
sudo bochs -f bin/bochsrc.txt -q
sudo losetup -d /dev/loop1
