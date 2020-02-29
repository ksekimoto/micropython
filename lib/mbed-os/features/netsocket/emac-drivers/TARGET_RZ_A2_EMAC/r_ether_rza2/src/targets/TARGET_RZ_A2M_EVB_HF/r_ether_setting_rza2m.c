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
        #error "Not support in this board."
    #elif (ETHER_CFG_MODE_SEL == 1)
        /* CH0 RMII */
        GPIO.PFENET.BIT.PHYMODE0 = 0;

        pin_function(PE_0, 7); // REF50CK0
        pin_function(PE_4, 7); // RMII0_CRS_DV
        pin_function(P6_2, 7); // RMII0_TXD0
        pin_function(P6_3, 7); // RMII0_TXD1
        pin_function(P6_1, 7); // RMII0_TXEN
        pin_function(PE_3, 7); // RMII0_RXER
        pin_function(PE_1, 7); // RMII0_RXD0
        pin_function(PE_2, 7); // RMII0_RXD1
        pin_function(PE_5, 1); // ET0_MDC
        pin_function(PE_6, 1); // ET0_MDIO
        GPIO.PMODEPFS.BIT.ET0_EXOUT_SEL = 0;
    #endif
    }
#endif

#if (ETHER_CH1_EN == 1)
    if (PORT_CONNECT_ET1 == (connect & PORT_CONNECT_ET1)) {
    #if (ETHER_CFG_MODE_SEL == 0)
        /* CH1 MII */
        GPIO.PFENET.BIT.PHYMODE1 = 1;
        #error "Not support in this board."
    #elif (ETHER_CFG_MODE_SEL == 1)
        /* CH1 RMII */
        GPIO.PFENET.BIT.PHYMODE1 = 0;

        pin_function(PK_3, 7); // REF50CK1
        pin_function(P3_2, 7); // RMII1_CRS_DV
        pin_function(PK_1, 7); // RMII1_TXD0
        pin_function(PK_2, 7); // RMII1_TXD1
        pin_function(PK_0, 7); // RMII1_TXEN
        pin_function(P3_1, 7); // RMII1_RXER
        pin_function(PK_4, 7); // RMII1_RXD0
        pin_function(P3_5, 7); // RMII1_RXD1
        pin_function(P3_3, 1); // ET1_MDC
        pin_function(P3_4, 1); // ET1_MDIO
        pin_function(P3_0, 3); // ET1_LINKSTA
        pin_function(PH_5, 3); // ET1_EXOUT / ET1_SCLKIN
        pin_function(PH_6, 3); // ET0_WOL
        GPIO.PMODEPFS.BIT.ET1_EXOUT_SEL = 0;
    #endif
    }
#endif

} /* End of function ether_set_phy_mode() */

/* End of File */
