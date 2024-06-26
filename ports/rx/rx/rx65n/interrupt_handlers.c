/************************************************************************/
/*    File Version: V0.5A                                               */
/*    History: 0.5A  (2015-12-04)  [Hardware Manual Revision : 0.5A]    */
/*    Date Modified: 26/04/2016                                         */
/************************************************************************/

#include "interrupt_handlers.h"

// Exception(Supervisor Instruction)
void __attribute__ ((weak)) INT_Excep_SuperVisorInst(void) {/* brk(){  } */
}

// Exception(Access Instruction)
void __attribute__ ((weak)) INT_Excep_AccessInst(void) {/* brk(); */
}

// Exception(Undefined Instruction)
void __attribute__ ((weak)) INT_Excep_UndefinedInst(void) {/* brk(){  } */
}

// Exception(Floating Point)
void __attribute__ ((weak)) INT_Excep_FloatingPoint(void) {/* brk(){  } */
}

// NMI
void __attribute__ ((weak)) INT_NonMaskableInterrupt(void) {/* brk(){  } */
}

// Dummy
void Dummy(void) {/* brk(){  } */
}

// BRK
void __attribute__ ((weak)) INT_Excep_BRK(void) {/* wait(); */
}

// BSC BUSERR
void __attribute__ ((weak)) INT_Excep_BSC_BUSERR(void) {
}

// RAM RAMERR
void __attribute__ ((weak)) INT_Excep_RAM_RAMERR(void) {
}

// FCU FIFERR
void __attribute__ ((weak)) INT_Excep_FCU_FIFERR(void) {
}

// FCU FRDYI
void __attribute__ ((weak)) INT_Excep_FCU_FRDYI(void) {
}

// ICU SWINT2
void __attribute__ ((weak)) INT_Excep_ICU_SWINT2(void) {
}

// ICU SWINT
void __attribute__ ((weak)) INT_Excep_ICU_SWINT(void) {
}

// CMT0 CMI0
void __attribute__ ((weak)) INT_Excep_CMT0_CMI0(void) {
}

// CMT1 CMI1
void __attribute__ ((weak)) INT_Excep_CMT1_CMI1(void) {
}

// CMTW0 CMWI0
void __attribute__ ((weak)) INT_Excep_CMTW0_CMWI0(void) {
}

// CMTW1 CMWI1
void __attribute__ ((weak)) INT_Excep_CMTW1_CMWI1(void) {
}

// USB0 D0FIFO0
void __attribute__ ((weak)) INT_Excep_USB0_D0FIFO0(void) {
}

// USB0 D1FIFO0
void __attribute__ ((weak)) INT_Excep_USB0_D1FIFO0(void) {
}

// RSPI0 SPRI0
void __attribute__ ((weak)) INT_Excep_RSPI0_SPRI0(void) {
}

// RSPI0 SPTI0
void __attribute__ ((weak)) INT_Excep_RSPI0_SPTI0(void) {
}

// RSPI1 SPRI1
void __attribute__ ((weak)) INT_Excep_RSPI1_SPRI1(void) {
}

// RSPI1 SPTI1
void __attribute__ ((weak)) INT_Excep_RSPI1_SPTI1(void) {
}

// QSPI SPRI
void __attribute__ ((weak)) INT_Excep_QSPI_SPRI(void) {
}

// QSPI SPTI
void __attribute__ ((weak)) INT_Excep_QSPI_SPTI(void) {
}

// SDHI SBFAI
void __attribute__ ((weak)) INT_Excep_SDHI_SBFAI(void) {
}

// MMCIF MBFAI
void __attribute__ ((weak)) INT_Excep_MMCIF_MBFAI(void) {
}

// RIIC0 RXI0
void __attribute__ ((weak)) INT_Excep_RIIC0_RXI0(void) {
}

// RIIC0 TXI0
void __attribute__ ((weak)) INT_Excep_RIIC0_TXI0(void) {
}

// RIIC2 RXI2
void __attribute__ ((weak)) INT_Excep_RIIC2_RXI2(void) {
}

// RIIC2 TXI2
void __attribute__ ((weak)) INT_Excep_RIIC2_TXI2(void) {
}

// SCI0 RXI0
void __attribute__ ((weak)) INT_Excep_SCI0_RXI0(void) {
}

// SCI0 TXI0
void __attribute__ ((weak)) INT_Excep_SCI0_TXI0(void) {
}

// SCI1 RXI1
void __attribute__ ((weak)) INT_Excep_SCI1_RXI1(void) {
}

// SCI1 TXI1
void __attribute__ ((weak)) INT_Excep_SCI1_TXI1(void) {
}

// SCI2 RXI2
void __attribute__ ((weak)) INT_Excep_SCI2_RXI2(void) {
}

// SCI2 TXI2
void __attribute__ ((weak)) INT_Excep_SCI2_TXI2(void) {
}

// ICU IRQ0
void __attribute__ ((weak)) INT_Excep_ICU_IRQ0(void) {
}

// ICU IRQ1
void __attribute__ ((weak)) INT_Excep_ICU_IRQ1(void) {
}

// ICU IRQ2
void __attribute__ ((weak)) INT_Excep_ICU_IRQ2(void) {
}

// ICU IRQ3
void __attribute__ ((weak)) INT_Excep_ICU_IRQ3(void) {
}

// ICU IRQ4
void __attribute__ ((weak)) INT_Excep_ICU_IRQ4(void) {
}

// ICU IRQ5
void __attribute__ ((weak)) INT_Excep_ICU_IRQ5(void) {
}

// ICU IRQ6
void __attribute__ ((weak)) INT_Excep_ICU_IRQ6(void) {
}

// ICU IRQ7
void __attribute__ ((weak)) INT_Excep_ICU_IRQ7(void) {
}

// ICU IRQ8
void __attribute__ ((weak)) INT_Excep_ICU_IRQ8(void) {
}

// ICU IRQ9
void __attribute__ ((weak)) INT_Excep_ICU_IRQ9(void) {
}

// ICU IRQ10
void __attribute__ ((weak)) INT_Excep_ICU_IRQ10(void) {
}

// ICU IRQ11
void __attribute__ ((weak)) INT_Excep_ICU_IRQ11(void) {
}

// ICU IRQ12
void __attribute__ ((weak)) INT_Excep_ICU_IRQ12(void) {
}

// ICU IRQ13
void __attribute__ ((weak)) INT_Excep_ICU_IRQ13(void) {
}

// ICU IRQ14
void __attribute__ ((weak)) INT_Excep_ICU_IRQ14(void) {
}

// ICU IRQ15
void __attribute__ ((weak)) INT_Excep_ICU_IRQ15(void) {
}

// SCI3 RXI3
void __attribute__ ((weak)) INT_Excep_SCI3_RXI3(void) {
}

// SCI3 TXI3
void __attribute__ ((weak)) INT_Excep_SCI3_TXI3(void) {
}

// SCI4 RXI4
void __attribute__ ((weak)) INT_Excep_SCI4_RXI4(void) {
}

// SCI4 TXI4
void __attribute__ ((weak)) INT_Excep_SCI4_TXI4(void) {
}

// SCI5 RXI5
void __attribute__ ((weak)) INT_Excep_SCI5_RXI5(void) {
}

// SCI5 TXI5
void __attribute__ ((weak)) INT_Excep_SCI5_TXI5(void) {
}

// SCI6 RXI6
void __attribute__ ((weak)) INT_Excep_SCI6_RXI6(void) {
}

// SCI6 TXI6
void __attribute__ ((weak)) INT_Excep_SCI6_TXI6(void) {
}

// LVD1 LVD1
void __attribute__ ((weak)) INT_Excep_LVD1_LVD1(void) {
}

// LVD2 LVD2
void __attribute__ ((weak)) INT_Excep_LVD2_LVD2(void) {
}

// USB0 USBR0
void __attribute__ ((weak)) INT_Excep_USB0_USBR0(void) {
}

// RTC ALM
void __attribute__ ((weak)) INT_Excep_RTC_ALM(void) {
}

// RTC PRD
void __attribute__ ((weak)) INT_Excep_RTC_PRD(void) {
}

// USBA USBAR
void __attribute__ ((weak)) INT_Excep_USBA_USBAR(void) {
}

// IWDT IWUNI
void __attribute__ ((weak)) INT_Excep_IWDT_IWUNI(void) {
}

// WDT WUNI
void __attribute__ ((weak)) INT_Excep_WDT_WUNI(void) {
}

// PDC PCDFI
void __attribute__ ((weak)) INT_Excep_PDC_PCDFI(void) {
}

// SCI7 RXI7
void __attribute__ ((weak)) INT_Excep_SCI7_RXI7(void) {
}

// SCI7 TXI7
void __attribute__ ((weak)) INT_Excep_SCI7_TXI7(void) {
}

// SCI8 RXI8
void __attribute__ ((weak)) INT_Excep_SCI8_RXI8(void) {
}

// SCI8 TXI8
void __attribute__ ((weak)) INT_Excep_SCI8_TXI8(void) {
}

// SCI9 RXI9
void __attribute__ ((weak)) INT_Excep_SCI9_RXI9(void) {
}

// SCI9 TXI9
void __attribute__ ((weak)) INT_Excep_SCI9_TXI9(void) {
}

// SCI10 RXI10
void __attribute__ ((weak)) INT_Excep_SCI10_RXI10(void) {
}

// SCI10 TXI10
void __attribute__ ((weak)) INT_Excep_SCI10_TXI10(void) {
}

// ICU GROUPBE0
void __attribute__ ((weak)) INT_Excep_ICU_GROUPBE0(void) {
}

// ICU GROUPBL2
void __attribute__ ((weak)) INT_Excep_ICU_GROUPBL2(void) {
}

// RSPI2 SPRI2
void __attribute__ ((weak)) INT_Excep_RSPI2_SPRI2(void) {
}

// RSPI2 SPTI2
void __attribute__ ((weak)) INT_Excep_RSPI2_SPTI2(void) {
}

// ICU GROUPBL0
void __attribute__ ((weak)) INT_Excep_ICU_GROUPBL0(void) {
}

// ICU GROUPBL1
void __attribute__ ((weak)) INT_Excep_ICU_GROUPBL1(void) {
}

// ICU GROUPAL0
void __attribute__ ((weak)) INT_Excep_ICU_GROUPAL0(void) {
}

// ICU GROUPAL1
void __attribute__ ((weak)) INT_Excep_ICU_GROUPAL1(void) {
}

// SCI11 RXI11
void __attribute__ ((weak)) INT_Excep_SCI11_RXI11(void) {
}

// SCI11 TXI11
void __attribute__ ((weak)) INT_Excep_SCI11_TXI11(void) {
}

// SCI12 RXI12
void __attribute__ ((weak)) INT_Excep_SCI12_RXI12(void) {
}

// SCI12 TXI12
void __attribute__ ((weak)) INT_Excep_SCI12_TXI12(void) {
}

// DMAC DMAC0I
void __attribute__ ((weak)) INT_Excep_DMAC_DMAC0I(void) {
}

// DMAC DMAC1I
void __attribute__ ((weak)) INT_Excep_DMAC_DMAC1I(void) {
}

// DMAC DMAC2I
void __attribute__ ((weak)) INT_Excep_DMAC_DMAC2I(void) {
}

// DMAC DMAC3I
void __attribute__ ((weak)) INT_Excep_DMAC_DMAC3I(void) {
}

// DMAC DMAC74I
void __attribute__ ((weak)) INT_Excep_DMAC_DMAC74I(void) {
}

// OST OSTDI
void __attribute__ ((weak)) INT_Excep_OST_OSTDI(void) {
}

// EXDMAC EXDMAC0I
void __attribute__ ((weak)) INT_Excep_EXDMAC_EXDMAC0I(void) {
}

// EXDMAC EXDMAC1I
void __attribute__ ((weak)) INT_Excep_EXDMAC_EXDMAC1I(void) {
}

// PERIB INTB128
void __attribute__ ((weak)) INT_Excep_PERIB_INTB128(void) {
}

// PERIB INTB129
void __attribute__ ((weak)) INT_Excep_PERIB_INTB129(void) {
}

// PERIB INTB130
void __attribute__ ((weak)) INT_Excep_PERIB_INTB130(void) {
}

// PERIB INTB131
void __attribute__ ((weak)) INT_Excep_PERIB_INTB131(void) {
}

// PERIB INTB132
void __attribute__ ((weak)) INT_Excep_PERIB_INTB132(void) {
}

// PERIB INTB133
void __attribute__ ((weak)) INT_Excep_PERIB_INTB133(void) {
}

// PERIB INTB134
void __attribute__ ((weak)) INT_Excep_PERIB_INTB134(void) {
}

// PERIB INTB135
void __attribute__ ((weak)) INT_Excep_PERIB_INTB135(void) {
}

// PERIB INTB136
void __attribute__ ((weak)) INT_Excep_PERIB_INTB136(void) {
}

// PERIB INTB137
// void __attribute__ ((weak)) INT_Excep_PERIB_INTB137(void){ }

// PERIB INTB138
void __attribute__ ((weak)) INT_Excep_PERIB_INTB138(void) {
}

// PERIB INTB139
void __attribute__ ((weak)) INT_Excep_PERIB_INTB139(void) {
}

// PERIB INTB140
void __attribute__ ((weak)) INT_Excep_PERIB_INTB140(void) {
}

// PERIB INTB141
void __attribute__ ((weak)) INT_Excep_PERIB_INTB141(void) {
}

// PERIB INTB142
void __attribute__ ((weak)) INT_Excep_PERIB_INTB142(void) {
}

// PERIB INTB143
void __attribute__ ((weak)) INT_Excep_PERIB_INTB143(void) {
}

// PERIB INTB144
void __attribute__ ((weak)) INT_Excep_PERIB_INTB144(void) {
}

// PERIB INTB145
void __attribute__ ((weak)) INT_Excep_PERIB_INTB145(void) {
}

// PERIB INTB146
void __attribute__ ((weak)) INT_Excep_PERIB_INTB146(void) {
}

// PERIB INTB147
void __attribute__ ((weak)) INT_Excep_PERIB_INTB147(void) {
}

// PERIB INTB148
void __attribute__ ((weak)) INT_Excep_PERIB_INTB148(void) {
}

// PERIB INTB149
void __attribute__ ((weak)) INT_Excep_PERIB_INTB149(void) {
}

// PERIB INTB150
void __attribute__ ((weak)) INT_Excep_PERIB_INTB150(void) {
}

// PERIB INTB151
void __attribute__ ((weak)) INT_Excep_PERIB_INTB151(void) {
}

// PERIB INTB152
void __attribute__ ((weak)) INT_Excep_PERIB_INTB152(void) {
}

// PERIB INTB153
void __attribute__ ((weak)) INT_Excep_PERIB_INTB153(void) {
}

// PERIB INTB154
void __attribute__ ((weak)) INT_Excep_PERIB_INTB154(void) {
}

// PERIB INTB155
void __attribute__ ((weak)) INT_Excep_PERIB_INTB155(void) {
}

// PERIB INTB156
void __attribute__ ((weak)) INT_Excep_PERIB_INTB156(void) {
}

// PERIB INTB157
void __attribute__ ((weak)) INT_Excep_PERIB_INTB157(void) {
}

// PERIB INTB158
void __attribute__ ((weak)) INT_Excep_PERIB_INTB158(void) {
}

// PERIB INTB159
void __attribute__ ((weak)) INT_Excep_PERIB_INTB159(void) {
}

// PERIB INTB160
void __attribute__ ((weak)) INT_Excep_PERIB_INTB160(void) {
}

// PERIB INTB161
void __attribute__ ((weak)) INT_Excep_PERIB_INTB161(void) {
}

// PERIB INTB162
void __attribute__ ((weak)) INT_Excep_PERIB_INTB162(void) {
}

// PERIB INTB163
void __attribute__ ((weak)) INT_Excep_PERIB_INTB163(void) {
}

// PERIB INTB164
void __attribute__ ((weak)) INT_Excep_PERIB_INTB164(void) {
}

// PERIB INTB165
void __attribute__ ((weak)) INT_Excep_PERIB_INTB165(void) {
}

// PERIB INTB166
void __attribute__ ((weak)) INT_Excep_PERIB_INTB166(void) {
}

// PERIB INTB167
void __attribute__ ((weak)) INT_Excep_PERIB_INTB167(void) {
}

// PERIB INTB168
void __attribute__ ((weak)) INT_Excep_PERIB_INTB168(void) {
}

// PERIB INTB169
void __attribute__ ((weak)) INT_Excep_PERIB_INTB169(void) {
}

// PERIB INTB170
void __attribute__ ((weak)) INT_Excep_PERIB_INTB170(void) {
}

// PERIB INTB171
void __attribute__ ((weak)) INT_Excep_PERIB_INTB171(void) {
}

// PERIB INTB172
void __attribute__ ((weak)) INT_Excep_PERIB_INTB172(void) {
}

// PERIB INTB173
void __attribute__ ((weak)) INT_Excep_PERIB_INTB173(void) {
}

// PERIB INTB174
void __attribute__ ((weak)) INT_Excep_PERIB_INTB174(void) {
}

// PERIB INTB175
void __attribute__ ((weak)) INT_Excep_PERIB_INTB175(void) {
}

// PERIB INTB176
void __attribute__ ((weak)) INT_Excep_PERIB_INTB176(void) {
}

// PERIB INTB177
void __attribute__ ((weak)) INT_Excep_PERIB_INTB177(void) {
}

// PERIB INTB178
void __attribute__ ((weak)) INT_Excep_PERIB_INTB178(void) {
}

// PERIB INTB179
void __attribute__ ((weak)) INT_Excep_PERIB_INTB179(void) {
}

// PERIB INTB180
void __attribute__ ((weak)) INT_Excep_PERIB_INTB180(void) {
}

// PERIB INTB181
void __attribute__ ((weak)) INT_Excep_PERIB_INTB181(void) {
}

// PERIB INTB182
void __attribute__ ((weak)) INT_Excep_PERIB_INTB182(void) {
}

// PERIB INTB183
void __attribute__ ((weak)) INT_Excep_PERIB_INTB183(void) {
}

// PERIB INTB184
void __attribute__ ((weak)) INT_Excep_PERIB_INTB184(void) {
}

// PERIB INTB185
void __attribute__ ((weak)) INT_Excep_PERIB_INTB185(void) {
}

// PERIB INTB186
void __attribute__ ((weak)) INT_Excep_PERIB_INTB186(void) {
}

// PERIB INTB187
void __attribute__ ((weak)) INT_Excep_PERIB_INTB187(void) {
}

// PERIB INTB188
void __attribute__ ((weak)) INT_Excep_PERIB_INTB188(void) {
}

// PERIB INTB189
void __attribute__ ((weak)) INT_Excep_PERIB_INTB189(void) {
}

// PERIB INTB190
void __attribute__ ((weak)) INT_Excep_PERIB_INTB190(void) {
}

// PERIB INTB191
void __attribute__ ((weak)) INT_Excep_PERIB_INTB191(void) {
}

// PERIB INTB192
void __attribute__ ((weak)) INT_Excep_PERIB_INTB192(void) {
}

// PERIB INTB193
void __attribute__ ((weak)) INT_Excep_PERIB_INTB193(void) {
}

// PERIB INTB194
void __attribute__ ((weak)) INT_Excep_PERIB_INTB194(void) {
}

// PERIB INTB195
void __attribute__ ((weak)) INT_Excep_PERIB_INTB195(void) {
}

// PERIB INTB196
void __attribute__ ((weak)) INT_Excep_PERIB_INTB196(void) {
}

// PERIB INTB197
void __attribute__ ((weak)) INT_Excep_PERIB_INTB197(void) {
}

// PERIB INTB198
void __attribute__ ((weak)) INT_Excep_PERIB_INTB198(void) {
}

// PERIB INTB199
void __attribute__ ((weak)) INT_Excep_PERIB_INTB199(void) {
}

// PERIB INTB200
void __attribute__ ((weak)) INT_Excep_PERIB_INTB200(void) {
}

// PERIB INTB201
void __attribute__ ((weak)) INT_Excep_PERIB_INTB201(void) {
}

// PERIB INTB202
void __attribute__ ((weak)) INT_Excep_PERIB_INTB202(void) {
}

// PERIB INTB203
void __attribute__ ((weak)) INT_Excep_PERIB_INTB203(void) {
}

// PERIB INTB204
void __attribute__ ((weak)) INT_Excep_PERIB_INTB204(void) {
}

// PERIB INTB205
void __attribute__ ((weak)) INT_Excep_PERIB_INTB205(void) {
}

// PERIB INTB206
void __attribute__ ((weak)) INT_Excep_PERIB_INTB206(void) {
}

// PERIB INTB207
void __attribute__ ((weak)) INT_Excep_PERIB_INTB207(void) {
}

// PERIA INTA208
void __attribute__ ((weak)) INT_Excep_PERIA_INTA208(void) {
}

// PERIA INTA209
void __attribute__ ((weak)) INT_Excep_PERIA_INTA209(void) {
}

// PERIA INTA210
void __attribute__ ((weak)) INT_Excep_PERIA_INTA210(void) {
}

// PERIA INTA211
void __attribute__ ((weak)) INT_Excep_PERIA_INTA211(void) {
}

// PERIA INTA212
void __attribute__ ((weak)) INT_Excep_PERIA_INTA212(void) {
}

// PERIA INTA213
void __attribute__ ((weak)) INT_Excep_PERIA_INTA213(void) {
}

// PERIA INTA214
void __attribute__ ((weak)) INT_Excep_PERIA_INTA214(void) {
}

// PERIA INTA215
void __attribute__ ((weak)) INT_Excep_PERIA_INTA215(void) {
}

// PERIA INTA216
void __attribute__ ((weak)) INT_Excep_PERIA_INTA216(void) {
}

// PERIA INTA217
void __attribute__ ((weak)) INT_Excep_PERIA_INTA217(void) {
}

// PERIA INTA218
void __attribute__ ((weak)) INT_Excep_PERIA_INTA218(void) {
}

// PERIA INTA219
void __attribute__ ((weak)) INT_Excep_PERIA_INTA219(void) {
}

// PERIA INTA220
void __attribute__ ((weak)) INT_Excep_PERIA_INTA220(void) {
}

// PERIA INTA221
void __attribute__ ((weak)) INT_Excep_PERIA_INTA221(void) {
}

// PERIA INTA222
void __attribute__ ((weak)) INT_Excep_PERIA_INTA222(void) {
}

// PERIA INTA223
void __attribute__ ((weak)) INT_Excep_PERIA_INTA223(void) {
}

// PERIA INTA224
void __attribute__ ((weak)) INT_Excep_PERIA_INTA224(void) {
}

// PERIA INTA225
void __attribute__ ((weak)) INT_Excep_PERIA_INTA225(void) {
}

// PERIA INTA226
void __attribute__ ((weak)) INT_Excep_PERIA_INTA226(void) {
}

// PERIA INTA227
void __attribute__ ((weak)) INT_Excep_PERIA_INTA227(void) {
}

// PERIA INTA228
void __attribute__ ((weak)) INT_Excep_PERIA_INTA228(void) {
}

// PERIA INTA229
void __attribute__ ((weak)) INT_Excep_PERIA_INTA229(void) {
}

// PERIA INTA230
void __attribute__ ((weak)) INT_Excep_PERIA_INTA230(void) {
}

// PERIA INTA231
void __attribute__ ((weak)) INT_Excep_PERIA_INTA231(void) {
}

// PERIA INTA232
void __attribute__ ((weak)) INT_Excep_PERIA_INTA232(void) {
}

// PERIA INTA233
void __attribute__ ((weak)) INT_Excep_PERIA_INTA233(void) {
}

// PERIA INTA234
void __attribute__ ((weak)) INT_Excep_PERIA_INTA234(void) {
}

// PERIA INTA235
void __attribute__ ((weak)) INT_Excep_PERIA_INTA235(void) {
}

// PERIA INTA236
void __attribute__ ((weak)) INT_Excep_PERIA_INTA236(void) {
}

// PERIA INTA237
void __attribute__ ((weak)) INT_Excep_PERIA_INTA237(void) {
}

// PERIA INTA238
void __attribute__ ((weak)) INT_Excep_PERIA_INTA238(void) {
}

// PERIA INTA239
void __attribute__ ((weak)) INT_Excep_PERIA_INTA239(void) {
}

// PERIA INTA240
void __attribute__ ((weak)) INT_Excep_PERIA_INTA240(void) {
}

// PERIA INTA241
void __attribute__ ((weak)) INT_Excep_PERIA_INTA241(void) {
}

// PERIA INTA242
void __attribute__ ((weak)) INT_Excep_PERIA_INTA242(void) {
}

// PERIA INTA243
void __attribute__ ((weak)) INT_Excep_PERIA_INTA243(void) {
}

// PERIA INTA244
void __attribute__ ((weak)) INT_Excep_PERIA_INTA244(void) {
}

// PERIA INTA245
void __attribute__ ((weak)) INT_Excep_PERIA_INTA245(void) {
}

// PERIA INTA246
void __attribute__ ((weak)) INT_Excep_PERIA_INTA246(void) {
}

// PERIA INTA247
void __attribute__ ((weak)) INT_Excep_PERIA_INTA247(void) {
}

// PERIA INTA248
void __attribute__ ((weak)) INT_Excep_PERIA_INTA248(void) {
}

// PERIA INTA249
void __attribute__ ((weak)) INT_Excep_PERIA_INTA249(void) {
}

// PERIA INTA250
void __attribute__ ((weak)) INT_Excep_PERIA_INTA250(void) {
}

// PERIA INTA251
void __attribute__ ((weak)) INT_Excep_PERIA_INTA251(void) {
}

// PERIA INTA252
void __attribute__ ((weak)) INT_Excep_PERIA_INTA252(void) {
}

// PERIA INTA253
void __attribute__ ((weak)) INT_Excep_PERIA_INTA253(void) {
}

// PERIA INTA254
void __attribute__ ((weak)) INT_Excep_PERIA_INTA254(void) {
}

// PERIA INTA255
void __attribute__ ((weak)) INT_Excep_PERIA_INTA255(void) {
}
