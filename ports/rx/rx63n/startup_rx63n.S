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
/* initialise user stack pointer */
    mvtc    #0x00018000-4,USP
/* initialise interrupt stack pointer */
    mvtc    #0x00018000-4,ISP
/* setup intb */
    mvtc    #_RelocatableVectors,intb    /* INTERRUPT VECTOR ADDRESS  definition    */
/* setup FPSW */
    mvtc    #100h, fpsw
/* setup PSW */
    mvtc    #10000h, psw            /* Set Ubit & Ibit for PSW */
/* start user program */

    mov #_sidata,r2     /* src ROM address of data section in R2 */
    mov #_sdata,r1      /* dest start RAM address of data section in R1 */
    mov #_edata,r3      /* end RAM address of data section in R3 */
    sub r1,r3           /* size of data section in R3 (R3=R3-R1) */
    smovf               /* block copy R3 bytes from R2 to R1 */

    mov #00h,r2         /* load R2 reg with zero */
    mov #_ebss, r3      /* store the end address of bss in R3 */
    mov #_sbss, r1      /* store the start address of bss in R1 */
    sub   r1,r3         /* size of bss section in R3 (R3=R3-R1) */
    sstr.b

    bsr.a    _main
    bsr.a     _exit
/* call to exit*/
_exit:
    bra  _loop_here
_loop_here:
    bra _loop_here

    .text
    .end
