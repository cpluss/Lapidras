ASM = /opt/local/bin/yasm --arch=x86
A_FLAGS = -f elf
LDFLAGS = -Tsrc/kernel/link.ld -m elf_i386
#CC = i686-pc-linux-gnu-gcc
#LD = i686-pc-linux-gnu-ld
CC = /usr/local/cross/bin/i686-elf-gcc
LD = /usr/local/cross/bin/i686-elf-ld
CFLAGS = -nostdlib -nostdinc -fno-builtin -ffreestanding -I./include/kernel

KERNEL_OBJ = $(patsubst %.asm,%.o,$(wildcard src/kernel/*.asm)) $(patsubst %.c,%.o,$(wildcard src/kernel/*.c))
KERNEL_OBJECTS = $(patsubst src/kernel/%,bin/obj/%,$(KERNEL_OBJ))

.PHONY: all clean test
all: bin/kernel.bin

bin/obj/%.o: src/kernel/%.c Makefile
	@echo CC      $<       $@
	@$(CC) $(CFLAGS) -c $< -o $@
bin/obj/%.o: src/kernel/%.asm Makefile
	@echo YASM    $<  $@
	@$(ASM) $(A_FLAGS) $< -o $@
bin/kernel.bin: $(KERNEL_OBJECTS)
	@echo LINKING bin/kernel.bin
	@$(LD) $(LDFLAGS) -o bin/kernel.bin $(KERNEL_OBJECTS)

test:
	@./hda.sh
	@./run_hda.sh
   
clean:
	@rm -rf $(KERNEL_OBJECTS)
	@rm -rf bin/*.bin
