#!/bin/bash
KERNEL_SIZE=800


cp bin/kernel.bin isofiles/boot/
cp src/programs/initrd isofiles/
genisoimage -R -b boot/grub/stage2_eltorito -no-emul-boot -boot-load-size $KERNEL_SIZE -boot-info-table -o bin/cdrom.iso -input-charset utf-8 isofiles
