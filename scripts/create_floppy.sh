#!/bin/bash

#for the ext2 system yes..
#sudo losetup -o 32256 /dev/loop0 bin/hdd.img

#fat12/16
sudo losetup -o 1048576 /dev/loop0 bin/hdd.img
sudo mount /dev/loop0 bin/mnt
sudo cp bin/kernel.bin bin/mnt/boot/
sudo cp src/programs/initrd bin/mnt/
sudo sync
sudo umount bin/mnt
sudo losetup -d /dev/loop0
