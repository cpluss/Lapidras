#!/bin/bash

#make all programs
for f in $(cat program_list); do
	cd $f
	make
	cd ..	 
done

#make the initrd
cd bin
./initrd_ins initrd *
cd ..
mv bin/initrd ./