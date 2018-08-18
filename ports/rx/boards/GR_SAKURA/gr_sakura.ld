MEMORY
{
    ram(rwx)    : o = 0x00000000, l = 0x0001e000 /* RAM 128KB */
    stack(rw)   : o = 0x0001e000, l = 0x00002000 /* end of RAM */
    rom(rx)     : o = 0xfff00000, l = 0x000fff80 /* ROM 1MB */
    fvectors(rx): o = 0xffffff80, l = 0x00000080
}

ENTRY("_startup")
SECTIONS
{
    .text : {
        _stext = . ;
        *(.text)
		*(.text.*)
		*(P)
        _etext = . ;
    } > rom

    .rodata : {
        _srodata = . ;
        *(.rodata)
        *(.rodata.*)
		*(C)
		*(C_1)
		*(C_2)
		*(C$VECT)
        *(.strings)
        _erodata = . ;
    } > rom

     . = ALIGN(4);
     __end_of_text__ = .;
     _sidata = __end_of_text__;

    _sdataonrom = .; 
    .data : AT(_sdataonrom) {
        _sdata = . ;
        *(FLASH_OP)
        *(.data)
		*(.data.*)
		*(D)
		*(D_1)
		*(D_2)
        _edata = . ;
    } > ram
    _edataonrom = _sdataonrom + SIZEOF(.data);

    .bss : {
        _sbss = . ;
        *(.bss)
		*(.bss.*)
        *(COMMON)
		*(B)
		*(B_1)
		*(B_2)
        _ebss = . ;
    } > ram

    . = ALIGN(4);
    _end = . ;

	.heapmax 0x1e000 : AT (0x1e000)
	{
		_heapmax = .;
	} > ram

    .stack : {
        _stack = .;
        _user_vbr = .;
    } > stack

    .fvector_tbl  : {
        KEEP(*(.fvectors)) 
    } > fvectors AT>fvectors
}

/* produce a link error if there is not this amount of RAM for these sections */
/* _minimum_stack_size = 2K; */
/* _minimum_heap_size = 16K; */

/* Define tho top end of the stack.  The stack is full descending so begins just
   above last byte of RAM.  Note that EABI requires the stack to be 8-byte
   aligned for a call. */
_estack = ORIGIN(ram) + LENGTH(ram);

/* RAM extents for the garbage collector */
_ram_start = ORIGIN(ram);
_ram_end = ORIGIN(ram) + LENGTH(ram);
_heap_start = _ebss; /* heap starts just after statically allocated memory */
_heap_end = 0x0001e000; /* tunable */
