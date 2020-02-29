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
* Copyright (C) 2019 Renesas Electronics Corporation. All rights reserved.
*******************************************************************************/
/*******************************************************************************
* File Name   : hyperram_init.c
*******************************************************************************/


/******************************************************************************
Includes   <System Includes> , "Project Includes"
******************************************************************************/
#include "r_typedefs.h"
#include "iodefine.h"

/******************************************************************************
Typedef definitions
******************************************************************************/


/******************************************************************************
Macro definitions
******************************************************************************/


/******************************************************************************
Imported global variables and functions (from other files)
******************************************************************************/


/******************************************************************************
Exported global variables and functions (to be accessed by other files)
******************************************************************************/


/******************************************************************************
Private global variables and functions
******************************************************************************/
static uint16_t HyperRAM_ReadConfig0(void);
static void HyperRAM_WriteConfig0(uint16_t config0);


/******************************************************************************
* Function Name: HyperRAM_Init
* Description  :
* Arguments    : none
* Return Value : none
******************************************************************************/
void HyperRAM_Init(void)
{
    uint16_t config0;

    /* STBCR9 Setting */
    CPG.STBCR9.BIT.MSTP93 = 0;

    /* SCLKSEL Setting */
    CPG.SCLKSEL.BIT.HYMCR = 3; /* Hyper Clock = G/2phy */


    /* MCR1 Setting */
    HYPER.MCR1.BIT.MAXEN = 0;
    HYPER.MCR1.BIT.MAXLEN = 0;

    HYPER.MCR1.BIT.CRT = 0;      /* memory space */
    HYPER.MCR1.BIT.DEVTYPE = 1;  /* HyerRAM */

    /* MTR1 Setting */
    HYPER.MTR1.BIT.RCSHI = 0; /* 1.5 clock */
    HYPER.MTR1.BIT.WCSHI = 0; /* 1.5 clock */
    HYPER.MTR1.BIT.RCSS = 0;  /* 1 clock */
    HYPER.MTR1.BIT.WCSS = 0;  /* 1 clock */
    HYPER.MTR1.BIT.RCSH = 0;  /* 1 clock */
    HYPER.MTR1.BIT.WCSH = 0;  /* 1 clock */
    HYPER.MTR1.BIT.LTCY = 1;  /* 6 clock Latency */

    /* PHMOM0 Setting */
    GPIO.PHMOM0.BIT.HOSEL = 0; /* select Hyper Bus */

    config0 = HyperRAM_ReadConfig0();

    config0 = (config0 & 0xff07) ; /* bit[7:4]= b'0000 :Latency:5 cycle */
                                   /* bit[3] = 0 : Variable Latency */

    HyperRAM_WriteConfig0(config0);

    HYPER.MTR1.BIT.LTCY = 0;  /* 5 clock Latency */
    HYPER.MTR1.LONG;          /* dummy read */

    config0 = HyperRAM_ReadConfig0();

}

static uint16_t HyperRAM_ReadConfig0(void)
{
    uint16_t config0;
    uint32_t dummy;

    HYPER.MCR1.BIT.CRT = 1;      /* io space */
    dummy = HYPER.MCR1.LONG;

    /* Read Configuration0 */
   config0 = *(volatile uint16_t *) (0x40000000 + (0x800 << 1) );

    HYPER.MCR1.BIT.CRT = 0;      /* memory space */
    dummy = HYPER.MCR1.LONG;
    (void)dummy;

    return config0;
}

static void HyperRAM_WriteConfig0(uint16_t config0)
{
    uint32_t dummy;

    HYPER.MCR1.BIT.CRT = 1;      /* io space */
    dummy = HYPER.MCR1.LONG;

    *(volatile uint16_t *)(0x40000000 + (0x800 << 1) ) = config0;

    HYPER.MCR1.BIT.CRT = 0;      /* memory space */
    dummy = HYPER.MCR1.LONG;
    (void)dummy;

}

/* End of File */
