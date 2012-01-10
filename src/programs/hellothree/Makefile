ASM = nasm
A_FLAGS = -f elf
LDFLAGS = -Tlink.ld
CC = cc -I../inc
#CFLAGS = -nostdlib -nostdinc -fno-builtin -fno-stack-protector -I./src/kernel
CFLAGS = -nostdlib -nostdinc -fno-builtin -ffreestanding

C_FILES = $(shell find *.c)
HDR_FILES = $(shell find *.h)
ASM_FILES = $(shell find *.asm)
OBJ_FILES = $(patsubst %.c,%.o,$(C_FILES)) $(patsubst %.asm,%.o,$(ASM_FILES)) ../clib/system.o ../clib/string.o

.PHONY: all clean

all: ../bin/hellothree.bin
	
%.o: %.c Makefile
	@echo CC      $<       $@
	@$(CC) $(CFLAGS) -c $< -o $@
%.o: %.asm Makefile
	@echo NASM    $<  $@
	@$(ASM) $(A_FLAGS) $< -o $@
	
../bin/hellothree.bin: $(OBJ_FILES)
	@echo LINKING hellothree.bin
	@ld $(LDFLAGS) -o hellothree.elf $(OBJ_FILES)
	@objcopy -O binary hellothree.elf ../bin/hellothree.bin

clean:
	@rm *.o ../bin/hellothree.bin hellothree.elf
	
