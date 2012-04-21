#!/bin/bash

qemu -hda bin/hdd_32.img -cdrom bin/Lapidras.iso -boot d -m 32 -monitor stdio

