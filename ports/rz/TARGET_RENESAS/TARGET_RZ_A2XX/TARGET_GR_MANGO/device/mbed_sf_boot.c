/*******************************************************************************
* DISCLAIMER
* This software is supplied by Renesas Electronics Corporation and is only
* intended for use with Renesas products. No other uses are authorized. This
* software is owned by Renesas Electronics Corporation and is protected under
* all applicable laws, including copyright laws.
* THIS SOFTWARE IS PROVIDED "AS IS" AND RENESAS MAKES NO WARRANTIES REGARDING
* THIS SOFTWARE, WHETHER EXPRESS, IMPLIED OR STATUTORY, INCLUDING BUT NOT
* LIMITED TO WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE
* AND NON-INFRINGEMENT. ALL SUCH WARRANTIES ARE EXPRESSLY DISCLAIMED.
* TO THE MAXIMUM EXTENT PERMITTED NOT PROHIBITED BY LAW, NEITHER RENESAS
* ELECTRONICS CORPORATION NOR ANY OF ITS AFFILIATED COMPANIES SHALL BE LIABLE
* FOR ANY DIRECT, INDIRECT, SPECIAL, INCIDENTAL OR CONSEQUENTIAL DAMAGES FOR
* ANY REASON RELATED TO THIS SOFTWARE, EVEN IF RENESAS OR ITS AFFILIATES HAVE
* BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES.
* Renesas reserves the right, without notice, to make changes to this software
* and to discontinue the availability of this software. By using this software,
* you agree to the additional terms and conditions found by accessing the
* following link:
* http://www.renesas.com/disclaimer
* Copyright (C) 2018 Renesas Electronics Corporation. All rights reserved.
*******************************************************************************/
/**************************************************************************//**
* @file         mbed_sf_boot.c
* $Rev: $
* $Date::                           $
* @brief        RZ_A2 HyperFlash boot loader
******************************************************************************/
#if(1) /* No boot loader is used */
const char  * boot_loader = (char  *)0x50000000;
#else

#if !defined(APPLICATION_ADDR)
    #define APPLICATION_ADDR       0x50000000
#endif

#if (APPLICATION_ADDR != 0x50000000)
const char  * boot_loader = (char  *)0x50000000;

#else /* (APPLICATION_ADDR == 0x50000000) */

#if defined  (__CC_ARM)
#pragma arm section rodata = "BOOT_LOADER"
const char boot_loader[]  __attribute__((used)) =
#elif (defined(__ARMCC_VERSION) && (__ARMCC_VERSION >= 6010050))
const char boot_loader[]  __attribute__ ((section("BOOT_LOADER"), used)) =
#elif defined (__ICCARM__)
__root const char boot_loader[] @ 0x50000000 =
#else
const char boot_loader[]  __attribute__ ((section(".boot_loader"), used)) =
#endif
{
};
#if defined  (__CC_ARM)
#pragma arm section
#endif

#endif /* APPLICATION_ADDR */
#endif
