echo Starting QEMU...
qemu\qemu.exe -L .\qemu -M pc -m 64 -boot d -std-vga -soundhw pcspk -cdrom bin/Lapidras.iso