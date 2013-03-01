#!/bin/bash

bochs_cmd='bochs'
qemu_cmd='/Applications/Q.app/Contents/MacOS/i386-softmmu.app/Contents/MacOS/i386-softmmu'
platform=`uname`

if [ $platform == "Darwin" ]; then
    $qemu_cmd -hda bin/hdd.img -smp 2 -m 128
else
    sudo losetup /dev/loop0 bin/hdd.img
    sudo bochs -f bin/bochsrc.txt -q
    sudo losetup -d /dev/loop0
fi
