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
* Copyright (C) 2017 Renesas Electronics Corporation. All rights reserved.
*******************************************************************************/
/*******************************************************************************
* File Name : r_typedefs.h
* $Rev: 149 $
* $Date:: 2017-12-15 16:34:38 +0900#$
* Description : basic type definition
******************************************************************************/
#ifndef R_TYPEDEFS_H
#define R_TYPEDEFS_H

/******************************************************************************
Includes <System Includes> , "Project Includes"
******************************************************************************/
#include <stddef.h>
#include <stdbool.h>
#include <stdint.h>

#if defined(__ARM_NEON__)
#include <arm_neon.h>
#endif /* __ARM_NEON__ */

#ifndef float32_t
typedef float               float32_t;
#endif
#ifndef float64_t
typedef double              float64_t;
#endif

/******************************************************************************
Typedef definitions
******************************************************************************/
typedef char                char_t;
#ifndef bool_t
typedef bool                bool_t;
#endif
typedef int                 int_t;
typedef long double         float128_t;
typedef signed long         long_t;
typedef unsigned long       ulong_t;


#endif /* R_TYPEDEFS_H */
