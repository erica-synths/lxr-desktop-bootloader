/**
  ******************************************************************************
  * @file    LwIP/LwIP_IAP/Inc/flash_if.h
  * @author  MCD Application Team
  * @brief   Header for flash_if.c module
  ******************************************************************************
  * @attention
  *
  * THE PRESENT FIRMWARE WHICH IS FOR GUIDANCE ONLY AIMS AT PROVIDING CUSTOMERS
  * WITH CODING INFORMATION REGARDING THEIR PRODUCTS IN ORDER FOR THEM TO SAVE
  * TIME. AS A RESULT, STMICROELECTRONICS SHALL NOT BE HELD LIABLE FOR ANY
  * DIRECT, INDIRECT OR CONSEQUENTIAL DAMAGES WITH RESPECT TO ANY CLAIMS ARISING
  * FROM THE CONTENT OF SUCH FIRMWARE AND/OR THE USE MADE BY CUSTOMERS OF THE
  * CODING INFORMATION CONTAINED HEREIN IN CONNECTION WITH THEIR PRODUCTS.
  *
  * <h2><center>&copy; COPYRIGHT(c) 2017 STMicroelectronics</center></h2>>
  ******************************************************************************
  */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __FLASH_IF_H
#define __FLASH_IF_H

/* Includes ------------------------------------------------------------------*/
#include "main.h"

/* Exported types ------------------------------------------------------------*/
/* Exported constants --------------------------------------------------------*/

/* Base address of the Flash sectors */
//  1 Mbyte Flash memory single bank organization (256 bits read width) for STM32F765VG
#define ADDR_FLASH_SECTOR_0     ((uint32_t)0x08000000) /* Base @ of Sector 0, 32 Kbyte */
#define ADDR_FLASH_SECTOR_1     ((uint32_t)0x08008000) /* Base @ of Sector 1, 32 Kbyte */
#define ADDR_FLASH_SECTOR_2     ((uint32_t)0x08010000) /* Base @ of Sector 2, 32 Kbyte */
#define ADDR_FLASH_SECTOR_3     ((uint32_t)0x08018000) /* Base @ of Sector 3, 32 Kbyte */
#define ADDR_FLASH_SECTOR_4     ((uint32_t)0x08020000) /* Base @ of Sector 4, 128 Kbyte */
#define ADDR_FLASH_SECTOR_5     ((uint32_t)0x08040000) /* Base @ of Sector 5, 256 Kbyte */
#define ADDR_FLASH_SECTOR_6     ((uint32_t)0x08080000) /* Base @ of Sector 6, 256 Kbyte */
#define ADDR_FLASH_SECTOR_7     ((uint32_t)0x080C0000) /* Base @ of Sector 7, 256 Kbyte */
/*#define ADDR_FLASH_SECTOR_8     ((uint32_t)0x08080000)  Base @ of Sector 8, 128 Kbyte
#define ADDR_FLASH_SECTOR_9     ((uint32_t)0x080A0000)  Base @ of Sector 9, 128 Kbyte
#define ADDR_FLASH_SECTOR_10    ((uint32_t)0x080C0000)  Base @ of Sector 10, 128 Kbyte
#define ADDR_FLASH_SECTOR_11    ((uint32_t)0x080E0000)  Base @ of Sector 11, 128 Kbyte */

/* End of the Flash address */
#define USER_FLASH_END_ADDRESS        0x080FFFFF

#define USER_FLASH_SIZE   (USER_FLASH_END_ADDRESS - USER_FLASH_FIRST_PAGE_ADDRESS)

/* Define the address from where user application will be loaded.
   Note: the 1st sector 0x08000000-0x08003FFF is reserved for the IAP code */
#define APPLICATION_ADDRESS   ADDR_FLASH_SECTOR_1
#define USER_FLASH_LAST_PAGE_ADDRESS ADDR_FLASH_SECTOR_7

/* Exported macro ------------------------------------------------------------*/
/* Exported functions ------------------------------------------------------- */
uint32_t FLASH_If_Write(__IO uint32_t* Address, uint32_t* Data, uint16_t DataLength);
int8_t FLASH_If_Erase(uint32_t StartSector);
void FLASH_If_Init(void);

#if 0
uint16_t FLASH_If_GetWriteProtectionStatus(void);
uint32_t FLASH_If_DisableWriteProtection(void);
#endif

#endif /* __FLASH_IF_H */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
