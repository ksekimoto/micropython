/***********************************************************************************************************************
* DISCLAIMER
* This software is supplied by Renesas Electronics Corporation and is only intended for use with Renesas products. No 
* other uses are authorized. This software is owned by Renesas Electronics Corporation and is protected under all 
* applicable laws, including copyright laws. 
* THIS SOFTWARE IS PROVIDED "AS IS" AND RENESAS MAKES NO WARRANTIES REGARDING
* THIS SOFTWARE, WHETHER EXPRESS, IMPLIED OR STATUTORY, INCLUDING BUT NOT LIMITED TO WARRANTIES OF MERCHANTABILITY, 
* FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT. ALL SUCH WARRANTIES ARE EXPRESSLY DISCLAIMED. TO THE MAXIMUM 
* EXTENT PERMITTED NOT PROHIBITED BY LAW, NEITHER RENESAS ELECTRONICS CORPORATION NOR ANY OF ITS AFFILIATED COMPANIES 
* SHALL BE LIABLE FOR ANY DIRECT, INDIRECT, SPECIAL, INCIDENTAL OR CONSEQUENTIAL DAMAGES FOR ANY REASON RELATED TO THIS 
* SOFTWARE, EVEN IF RENESAS OR ITS AFFILIATES HAVE BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES.
* Renesas reserves the right, without notice, to make changes to this software and to discontinue the availability of 
* this software. By using this software, you agree to the additional terms and conditions found by accessing the 
* following link:
* http://www.renesas.com/disclaimer 
*
* Copyright (C) 2018 Renesas Electronics Corporation. All rights reserved.    
***********************************************************************************************************************/
/***********************************************************************************************************************
* File Name    : r_ether_setting_rza2m.c
* Version      : 1.00
* Device       : RZA2M
* Description  : Ethernet module device driver
***********************************************************************************************************************/

/***********************************************************************************************************************
Includes   <System Includes> , "Project Includes"
***********************************************************************************************************************/
#include "iodefine.h"
#include "iobitmask.h"
#include "cmsis.h"
#include "pinmap.h"

#include "r_ether_rza2_if.h"
#include "src/r_ether_rza2_private.h"

/***********************************************************************************************************************
Macro definitions
***********************************************************************************************************************/

/***********************************************************************************************************************
Typedef definitions
***********************************************************************************************************************/

/***********************************************************************************************************************
Imported global variables and functions (from other files)
***********************************************************************************************************************/

/***********************************************************************************************************************
Exported global variables (to be accessed by other files)
***********************************************************************************************************************/

/***********************************************************************************************************************
Private global variables and functions
***********************************************************************************************************************/

/***********************************************************************************************************************
* Function Name: ether_set_phy_mode
* Description  :
* Arguments    : connect -
*                    Ethernet channel number
* Return Value : none
***********************************************************************************************************************/
void ether_set_phy_mode(uint8_t connect)
{
#if (ETHER_CH0_EN == 1)
    if (PORT_CONNECT_ET0 == (connect & PORT_CONNECT_ET0)) {
    #if (ETHER_CFG_MODE_SEL == 0)
        /* CH0 MII */
        GPIO.PFENET.BIT.PHYMODE0 = 1;
        pin_function(PE_5, 1); // ET0_MDC
        pin_function(PE_6, 1); // ET0_MDIO
        pin_function(PG_0, 1); // ET0_TXCLK
        pin_function(PG_4, 1); // ET0_TXER
        pin_function(P6_1, 1); // ET0_TXEN
        pin_function(P6_2, 1); // ET0_TXD0
        pin_function(P6_3, 1); // ET0_TXD1
        pin_function(PG_1, 1); // ET0_TXD2
        pin_function(PG_2, 1); // ET0_TXD3
        pin_function(PE_0, 1); // ET0_RXCLK
        pin_function(PE_3, 1); // ET0_RXER
        pin_function(PG_5, 1); // ET0_RXDV
        pin_function(PE_1, 1); // ET0_RXD0
        pin_function(PE_2, 1); // ET0_RXD1
        pin_function(PG_6, 1); // ET0_RXD2
        pin_function(PG_7, 1); // ET0_RXD3
        pin_function(PE_4, 1); // ET0_CRS
        pin_function(PG_3, 1); // ET0_COL
    #elif (ETHER_CFG_MODE_SEL == 1)
        /* CH0 RMII */
        GPIO.PFENET.BIT.PHYMODE0 = 0;
        GPIO.PMODEPFS.BIT.ET0_EXOUT_SEL = 0;
        #error "Not support in this board."
    #endif
    }
#endif

#if (ETHER_CH1_EN == 1)
    if (PORT_CONNECT_ET1 == (connect & PORT_CONNECT_ET1)) {
    #if (ETHER_CFG_MODE_SEL == 0)
        /* CH1 MII */
        GPIO.PFENET.BIT.PHYMODE1 = 1;
        pin_function(P3_3, 1); // ET1_MDC
        pin_function(P3_4, 1); // ET1_MDIO
        pin_function(PC_0, 3); // ET1_TXCLK
        pin_function(PC_4, 3); // ET1_TXER
        pin_function(PK_0, 1); // ET1_TXEN
        pin_function(PK_1, 1); // ET1_TXD0
        pin_function(PK_2, 1); // ET1_TXD1
        pin_function(PC_1, 3); // ET1_TXD2
        pin_function(PC_2, 3); // ET1_TXD3
        pin_function(PK_3, 1); // ET1_RXCLK
        pin_function(P3_1, 1); // ET1_RXER
        pin_function(PC_5, 3); // ET1_RXDV
        pin_function(PK_4, 1); // ET1_RXD0
        pin_function(P3_5, 1); // ET1_RXD1
        pin_function(PC_6, 3); // ET1_RXD2
        pin_function(PC_7, 3); // ET1_RXD3
        pin_function(P3_2, 1); // ET1_CRS
        pin_function(PC_3, 3); // ET1_COL
    #elif (ETHER_CFG_MODE_SEL == 1)
        /* CH1 RMII */
        GPIO.PFENET.BIT.PHYMODE1 = 0;
        GPIO.PMODEPFS.BIT.ET1_EXOUT_SEL = 0;
        #error "Not support in this board."
    #endif
    }
#endif

} /* End of function ether_set_phy_mode() */

/* End of File */
