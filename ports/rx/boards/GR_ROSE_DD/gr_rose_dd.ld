MEMORY
{
	RAM : ORIGIN = 0x00000000, LENGTH = 262144
	RAMHI : ORIGIN = 0x800000, LENGTH = 393216
	ROM : ORIGIN = 0xFFE00000, LENGTH = 2097152
}

SECTIONS
{
	.text 0xFFE00000 : AT (0xFFE00000)
	{
		_stext = . ;
		KEEP(*(i.EntryPoint))
		*(.text)
		*(.text.*)
		*(P)
		_etext = .;
		etext = .;
	} > ROM
	.rvectors : 
	{
		_rvectors_start = .;
		KEEP(*(.rvectors))
		_rvectors_end = .;
	} > ROM
	.init : 
	{
		KEEP(*(.init))
		__preinit_array_start = .;
		KEEP(*(.preinit_array))
		__preinit_array_end = .;
		__init_array_start = (. + 3) & ~ 3;
		KEEP(*(.init_array))
		KEEP(*(SORT(.init_array.*)))
		__init_array_end = .;
		__fini_array_start = .;
		KEEP(*(.fini_array))
		KEEP(*(SORT(.fini_array.*)))
		__fini_array_end = .;
	} > ROM
	.fini : 
	{
		KEEP(*(.fini))
	} > ROM
	.got : 
	{
		*(.got)
		*(.got.plt)
	} > ROM
	.rodata : 
	{
		*(.rodata)
		*(.rodata.*)
		*(C_1)
		*(C_2)
		*(C)
		_erodata = .;
	} > ROM
	.exvectors : 
	{
		KEEP(*(.exvectors))
	} > ROM
	.fvectors : 
	{
		KEEP(*(.fvectors))
	} > ROM
	.eh_frame_hdr : 
	{
		*(.eh_frame_hdr)
	} > ROM
	.eh_frame : 
	{
		*(.eh_frame)
	} > ROM
	.jcr : 
	{
		*(.jcr)
	} > ROM
	.tors : 
	{
		__CTOR_LIST__ = .;
		. = ALIGN(2);
		___ctors = .;
		*(.ctors)
		___ctors_end = .;
		__CTOR_END__ = .;
		__DTOR_LIST__ = .;
		___dtors = .;
		*(.dtors)
		___dtors_end = .;
		__DTOR_END__ = .;
		. = ALIGN(2);
		_mdata = .;
	} > ROM

	. = ALIGN(4);
	__end_of_text__ = .;
	_sidata = __end_of_text__;
	_sdataonrom = .; 
	
	.istack 0x860000 : AT (0x860000)
	{
		_istack = .;
	} > RAMHI
	.ustack 0x85FC00 : AT (0x85FC00)
	{
		_ustack = .;
	} > RAMHI
	.heapmax 0x85C000 : AT (0x85C000)
	{
		_heapmax = .;
	} > RAMHI

	.data 0x1000 : AT (_mdata)
	{
		_sdata = .;
		_data = .;
		*(FLASH_OP)
		*(.data)
		*(.data.*)
		*(D)
		*(D_1)
		*(D_2)
		_edata = .;
	} > RAM

	._RX_DESC : 
	{
		. = ALIGN(32);
		*(._RX_DESC)
	} > RAM
	._TX_DESC : 
	{
		. = ALIGN(32);
		*(._TX_DESC)
	} > RAM
	._ETH_BUF : 
	{
		. = ALIGN(32);
		*(._ETH_BUF)
	} > RAM

	.bss : 
	{
		_sbss = .;
		_bss = .;
		*(.bss)
		*(.bss.**)
		*(COMMON)
		*(B)
		*(B_1)
		*(B_2)
		_ebss = .;
		_end = .;
	} > RAMHI
	.gcc_exc : 
	{
		*(.gcc_exc)
	} > RAMHI
}
/* produce a link error if there is not this amount of RAM for these sections */
/* _minimum_stack_size = 2K; */
/* _minimum_heap_size = 16K; */

/* Define tho top end of the stack.  The stack is full descending so begins just
   above last byte of RAM.  Note that EABI requires the stack to be 8-byte
   aligned for a call. */
_estack = ORIGIN(RAMHI) + LENGTH(RAMHI);

/* RAM extents for the garbage collector */
_ram_start = ORIGIN(RAMHI);
_ram_end = ORIGIN(RAMHI) + LENGTH(RAMHI);
_heap_start = _ebss;	/* heap starts just after statically allocated memory */
_heap_end = 0x85A000;	/* tunable */

