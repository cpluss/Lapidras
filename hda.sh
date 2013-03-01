#!/bin/bash

# Different options depending on platform
platform=`uname`

if [ $platform == 'Darwin' ]; then
    hdiutil attach bin/hdd.img -mountpoint bin/mnt > /dev/null
    cp bin/kernel.bin bin/mnt/boot/ > /dev/null
    sync > /dev/null
    hdiutil detach bin/mnt > /dev/null
else #Probably linux ...
    sudo losetup -o 1048576 /dev/loop1 bin/hdd.img
    sudo mount /dev/loop1 bin/mnt
    sudo cp bin/kernel.bin bin/mnt/boot/
    sudo sync
    sudo umount bin/mnt
    sudo losetup -d /dev/loop1
fi
