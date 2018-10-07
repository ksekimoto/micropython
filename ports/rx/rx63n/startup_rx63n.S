############################################################################
## Copyright (c) 2010-2014 Kentaro Sekimoto  All rights reserved.
############################################################################

    .section    .text, "ax", %progbits

    .global  EntryPoint
    .global _startup

    .extern _main
    .extern _RelocatableVectors

    .section i.EntryPoint, "ax", %progbits

    .org    0x000
EntryPoint:
_startup:
    nop
    nop
    nop
    nop

	mvtc    #_ustack, USP
	mvtc	#_istack, ISP
	mvtc	#_rvectors_start, intb
	mvtc    #100h, fpsw

	mov     #_mdata, r2
	mov     #_data, r1
	mov     #_edata, r3
	sub    	r1,r3
	smovf

	mov		#00h, r2
	mov		#_ebss, r3
	mov		#_bss, r1
	sub   	r1,r3
	sstr.b

	mvtc	#10000h, psw

	bsr.a	_main
	bsr.a 	_exit

_exit:
    bra  _loop_here
_loop_here:
    bra _loop_here

    .text
    .end
