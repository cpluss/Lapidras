ASM = nasm
A_FLAGS = -f elf
LDFLAGS = -T../clib/link.ld
CC = cc -I../inc
#CFLAGS = -nostdlib -nostdinc -fno-builtin -fno-stack-protector -I./src/kernel
CFLAGS = -nostdlib -nostdinc -fno-builtin -ffreestanding

C_FILES = $(shell find *.c)
OBJ_FILES = $(patsubst %.c,%.o,$(C_FILES)) ../clib/system.o ../clib/string.o

.PHONY: all clean

all: ../bin/hellofour.bin
	
%.o: %.c Makefile
	@echo CC      $<       $@
	@$(CC) $(CFLAGS) -c $< -o $@
	
../bin/hellofour.bin: $(OBJ_FILES)
	@echo LINKING hellofour.bin
	@ld $(LDFLAGS) -o hellofour.elf $(OBJ_FILES)
	@objcopy -O binary hellofour.elf ../bin/hellofour.bin

clean:
	@rm *.o ../bin/hellofour.bin hellofour.elf
	
