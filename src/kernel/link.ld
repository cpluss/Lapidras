ENTRY (_start)
SECTIONS{
    . = 0x00100000;

    .mbHeader : {
	*(.mbHeader)
    }

    .text :{
        *(.text)
    }

    .rodata ALIGN (0x1000) : {
        *(.rodata)
    }

    .data ALIGN (0x1000) : {
        *(.data)
    }

    .bss : {
        sbss = .;
        *(COMMON)
        *(.bss)
        ebss = .;
    }
    
    .entryother : {
		*(.entryother)
	}
    
    end = .; _end = .; __end = .;
}
