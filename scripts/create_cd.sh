#!/bin/bash
cp bin/kernel.bin isofiles/boot/
cp src/initrd/initrd isofiles/
genisoimage -R -b boot/grub/stage2_eltorito -no-emul-boot -boot-load-size 4 -boot-info-table -o bin/Lapidras.iso -input-charset utf-8 -A "Lapidras" isofiles
