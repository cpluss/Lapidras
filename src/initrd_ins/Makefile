.PHONY: all clean

all: bin/initrd_ins bin/initrd_read

bin/initrd_ins: initrd_ins.c
	cc initrd_ins.c -o bin/initrd_ins
bin/initrd_read: initrd_read.c
	cc initrd_read.c -o bin/initrd_read
	
clean:
	rm -rf bin/initrd_ins
