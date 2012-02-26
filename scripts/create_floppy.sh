#!/bin/bash

#for the ext2 system yes..
#sudo losetup -o 32256 /dev/loop0 bin/hdd.img

#fat12/16
sudo losetup -o 1048576 /dev/loop0 bin/hdd_32.img
#sudo losetup -o 512 /dev/loop0 bin/test.img
sudo mount /dev/loop0 bin/mnt
sudo cp bin/kernel.bin bin/mnt/boot/
sudo cp src/initrd/initrd bin/mnt/boot/
sudo cp src/initrd/initrd bin/mnt/

sudo rm -r bin/mnt/bin
sudo mkdir bin/mnt/bin
#make all programs
#for f in $(cat src/programs/program_list); do
#	sudo cp src/programs/bin/$f bin/mnt/bin/
#done
sudo cp src/programs/bin/* bin/mnt/bin

sudo sync
sudo umount bin/mnt
sudo losetup -d /dev/loop0
