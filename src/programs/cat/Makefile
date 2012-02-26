ASM = nasm
A_FLAGS = -f elf
LDFLAGS = -T../clib/link.ld
CC = cc -I../inc
#CFLAGS = -nostdlib -nostdinc -fno-builtin -fno-stack-protector -I./src/kernel
CFLAGS = -nostdlib -nostdinc -fno-builtin -ffreestanding

C_FILES = $(shell find *.c)
OBJ_FILES = $(patsubst %.c,%.o,$(C_FILES)) ../clib/system.o ../clib/string.o ../clib/start.o

OUT_NAME = cat

.PHONY: all clean

all: ../bin/$(OUT_NAME)
	
%.o: %.c Makefile
	@echo CC      $<       $@
	@$(CC) $(CFLAGS) -c $< -o $@
	
../bin/$(OUT_NAME): $(OBJ_FILES)
	@echo LINKING $(OUT_NAME)
	@ld $(LDFLAGS) -o ../bin/$(OUT_NAME) $(OBJ_FILES)
#@objcopy -O binary $(OUT_NAME).elf ../bin/$(OUT_NAME).bin

clean:
	@rm *.o ../bin/$(OUT_NAME)
	
