/*
 * Copyright (C) 2012-2026 Erica Synths
 * SPDX-License-Identifier: GPL-3.0-only
 *
 * This file is part of the LXR firmware. It is licensed under the GNU General
 * Public License version 3, with the additional permission and terms in
 * NOTICE.md. See LICENSE and NOTICE.md in the root of the repository.
 */



#include "delay.h"
#pragma GCC push_options
#pragma GCC optimize ("O0")
/**
 * @brief Initializes DWT_Clock_Cycle_Count for DWT_Delay_us function
 * @return Error DWT counter
 * 1: clock cycle counter not started
 * 0: clock cycle counter works
 */
uint32_t DWT_Delay_Init(void) {
	 /* Disable TRC */
	 CoreDebug->DEMCR &= ~CoreDebug_DEMCR_TRCENA_Msk; // ~0x01000000;
	 /* Enable TRC */
	 CoreDebug->DEMCR |= CoreDebug_DEMCR_TRCENA_Msk; // 0x01000000;
	 //unlock the dbg regs
     DWT->LAR = 0xC5ACCE55;
	 /* Disable clock cycle counter */
	 DWT->CTRL &= ~DWT_CTRL_CYCCNTENA_Msk; //~0x00000001;
	 /* Enable clock cycle counter */
	 DWT->CTRL |= DWT_CTRL_CYCCNTENA_Msk; //0x00000001;
	 /* Reset the clock cycle counter value */
	 DWT->CYCCNT = 0;
	/* 3 NO OPERATION instructions */
	__ASM volatile ("NOP");
	__ASM volatile ("NOP");
	__ASM volatile ("NOP");
	 /* Check if clock cycle counter has started */
	if(DWT->CYCCNT)
	{
	 return 0; /*clock cycle counter started*/
	}
	else
	 {
		  //---------------------------#
		  while(1)
		  {
			  GPIOB->BSRR = GPIO_PIN_8;

			  uint32_t time = HAL_GetTick();
			  while(HAL_GetTick() - time < 50);

			  GPIOB->BSRR = GPIO_PIN_8<<16;

			  time = HAL_GetTick();
			  while(HAL_GetTick() - time < 50);
		  }
		  //------------------------------------------
	 return 1; /*clock cycle counter not started*/
	 }
#pragma GCC pop_options
}
