#!/bin/bash
#2116
sudo losetup -o 1048576 /dev/loop0 bin/hdd.img
sudo mount /dev/loop0 bin/mnt
sudo cp bin/kernel.bin bin/mnt/boot/
sudo cp src/initrd/initrd bin/mnt/boot/
sudo cp README bin/mnt/readme.txt

sudo rm -r bin/mnt/bin
sudo mkdir bin/mnt/bin
sudo cp src/programs/bin/* bin/mnt/bin

sudo sync
sudo umount bin/mnt
sudo losetup -d /dev/loop0
