ASM = nasm
A_FLAGS = -f elf
LDFLAGS = -Tlink.ld
CC = cc -I../inc
CFLAGS = -nostdlib -nostdinc -fno-builtin -ffreestanding

all: system.o string.o

%.o: %.c Makefile
	@echo CC      $<       $@
	@$(CC) $(CFLAGS) -c $< -o $@
