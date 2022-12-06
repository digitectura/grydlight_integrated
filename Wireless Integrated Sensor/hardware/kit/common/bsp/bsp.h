/***************************************************************************//**
 * @file
 * @brief Board support package API definitions.
 *******************************************************************************
 * # License
 * <b>Copyright 2018 Silicon Laboratories Inc. www.silabs.com</b>
 *******************************************************************************
 *
 * SPDX-License-Identifier: Zlib
 *
 * The licensor of this software is Silicon Laboratories Inc.
 *
 * This software is provided 'as-is', without any express or implied
 * warranty. In no event will the authors be held liable for any damages
 * arising from the use of this software.
 *
 * Permission is granted to anyone to use this software for any purpose,
 * including commercial applications, and to alter it and redistribute it
 * freely, subject to the following restrictions:
 *
 * 1. The origin of this software must not be misrepresented; you must not
 *    claim that you wrote the original software. If you use this software
 *    in a product, an acknowledgment in the product documentation would be
 *    appreciated but is not required.
 * 2. Altered source versions must be plainly marked as such, and must not be
 *    misrepresented as being the original software.
 * 3. This notice may not be removed or altered from any source distribution.
 *
 ******************************************************************************/

#ifndef BSP_H
#define BSP_H

#include <stdbool.h>
#if defined(HAL_CONFIG)
#include "bsphalconfig.h"
#else
#include "bspconfig.h"
#endif
#if defined(BSP_STK) || defined(BSP_WSTK)
#include "em_usart.h"
#endif

#ifdef __cplusplus
extern "C" {
#endif


/***************************************************************************//**
 * @addtogroup BSPCOMMON Common BSP for all kits
 * @{
 ******************************************************************************/

int             BSP_Disable                 (void);
int             BSP_Init                    (uint32_t flags);

#ifdef __cplusplus
}
#endif

#endif /* BSP_H */