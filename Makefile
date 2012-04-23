ASM = nasm
A_FLAGS = -f elf
LDFLAGS = -Tsrc/kernel/link.ld
CC = cc
SOURCES = src/kernel/
CFLAGS = -nostdlib -nostdinc -fno-builtin -ffreestanding -I./src/kernel/includes

GEN = $(patsubst %.c,%.o,$(wildcard src/kernel/*.c))
FILESYSTEMS = $(patsubst %.c,%.o,$(wildcard src/kernel/fs/*.c))
DATASTRUCTURES = $(patsubst %.c,%.o,$(wildcard src/kernel/ds/*.c))
VIDEODRIVERS = $(patsubst %.c,%.o,$(wildcard src/kernel/video/*.c))
VIRTUALMEM = $(patsubst %.c,%.o,$(wildcard src/kernel/mem/*.c))
MISCMODS = $(patsubst %.c,%.o,$(wildcard src/kernel/misc/*.c))
SYSTEM = $(patsubst %.c,%.o,$(wildcard src/kernel/sys/*.c))
CPUBITS = $(patsubst %.c,%.o,$(wildcard src/kernel/cpu/*.c))
SERVICES = $(patsubst %.c,%.o,$(wildcard src/kernel/services/*.c))

GENs = $(patsubst %.asm,%.o,$(wildcard src/kernel/*.asm))
FILESYSTEMSs = $(patsubst %.asm,%.o,$(wildcard src/kernel/fs/*.asm))
VIDEODRIVERSs = $(patsubst %.asm,%.o,$(wildcard src/kernel/video/*.asm))
VIRTUALMEMs = $(patsubst %.asm,%.o,$(wildcard src/kernel/mem/*.asm))
MISCMODSs = $(patsubst %.asm,%.o,$(wildcard src/kernel/misc/*.asm))
SYSTEMs = $(patsubst %.asm,%.o,$(wildcard src/kernel/sys/*.asm))
CPUBITSs = $(patsubst %.asm,%.o,$(wildcard src/kernel/cpu/*.asm))

OBJ_FILES_f = $(GEN) $(DATASTRUCTURES) $(SERVICES) $(FILESYSTEMS) $(VIDEODRIVERS) $(VIRTUALMEM) $(MISCMODS) $(SYSTEM) $(CPUBITS) $(FILESYSTEMSs) $(VIDEODRIVERSs) $(VIRTUALMEMs) $(GENs) $(MISCMODSs) $(SYSTEMs) $(CPUBITSs)
OBJ_FILES = $(patsubst src/kernel/%,bin/obj/%,$(OBJ_FILES_f))

.PHONY: all clean programs initrd directories qemu bochs hdd_image cd_image gitclean

all: bin/kernel.bin

bin/obj/%.o: src/kernel/%.c Makefile bin/obj
	@echo CC      $<       $@
	@$(CC) $(CFLAGS) -c $< -o $@
bin/obj/%.o: src/kernel/%.asm Makefile bin/obj
	@echo NASM    $<  $@
	@$(ASM) $(A_FLAGS) $< -o $@
bin/kernel.bin: $(OBJ_FILES)
	@echo LINKING bin/kernel.bin
	@ld $(LDFLAGS) -o bin/kernel.bin $(OBJ_FILES)

bin/obj:
	@mkdir bin/obj
	@mkdir bin/obj/fs bin/obj/mem bin/obj/misc bin/obj/sys bin/obj/cpu bin/obj/ds bin/obj/services bin/obj/video
	
programs:
	@sh ./scripts/programs.sh
initrd:
	@sh ./scripts/initrd.sh

cd_image:
	@sh ./scripts/create_cd.sh

hdd_image:
	@sh ./scripts/create_hda.sh
	
qemu:
	@sh ./scripts/create_hda.sh
	@qemu -hda bin/hdd.img -monitor stdio -m 128
bochs:
	@sh ./scripts/create_hda.sh
	@sh ./scripts/run hda.sh
	
gitclean:
	@sh ./scripts/git_clean.sh

clean:
	@rm -rf bin/obj
	@rm -rf bin/*.bin
