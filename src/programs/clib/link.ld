ENTRY (main)
SECTIONS{
    . = 0x400000;

    .text :{
        *(.text)
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
}
