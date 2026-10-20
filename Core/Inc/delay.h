/*
 * Copyright (C) 2012-2026 Erica Synths
 * SPDX-License-Identifier: GPL-3.0-only
 *
 * This file is part of the LXR firmware. It is licensed under the GNU General
 * Public License version 3, with the additional permission and terms in
 * NOTICE.md. See LICENSE and NOTICE.md in the root of the repository.
 */

    #ifndef DWT_STM32_DELAY_H
    #define DWT_STM32_DELAY_H
    #ifdef __cplusplus
    extern 'C' {
    #endif

#include "stm32f7xx_hal.h"
    /**
     * @brief Initializes DWT_Cycle_Count for DWT_Delay_us function
     * @return Error DWT counter
     * 1: DWT counter Error
     * 0: DWT counter works
     */
    uint32_t DWT_Delay_Init(void);

    /**
     * @brief This function provides a delay (in microseconds)
     * @param microseconds: delay in microseconds
     */
#pragma GCC push_options
#pragma GCC optimize ("O0")
    static volatile uint32_t clk_cycle_start_nonblock=0;
    static volatile uint32_t microseconds_nonblock=0;

    static inline void DWT_init_nonblock(volatile uint32_t microseconds)
    {
    	clk_cycle_start_nonblock = DWT->CYCCNT;
    	microseconds_nonblock = microseconds * (HAL_RCC_GetHCLKFreq() / 1000000L);
    }

    static inline uint8_t DWT_check_nonblock()
    {
    	if( (DWT->CYCCNT - clk_cycle_start_nonblock) < microseconds_nonblock )
    	{
    		return 0;
    	}
    	else return 1;
    }


    __STATIC_INLINE void DWT_Delay_us(volatile uint32_t microseconds)
    {
     uint32_t clk_cycle_start = DWT->CYCCNT;
     /* Go to number of cycles for system */
     microseconds *= (HAL_RCC_GetHCLKFreq() / 1000000);
     /* Delay till end */
     while ((DWT->CYCCNT - clk_cycle_start) < microseconds);
    }
#pragma GCC pop_options

    #ifdef __cplusplus
    }
    #endif
    #endif

