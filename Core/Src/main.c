/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2020 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under Ultimate Liberty license
  * SLA0044, the "License"; You may not use this file except in compliance with
  * the License. You may obtain a copy of the License at:
  *                             www.st.com/SLA0044
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "fatfs.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

#define USE_NEW_FLASH_IF 0


#include "lcd.h"
#include "ImageFile.h"

#if !USE_NEW_FLASH_IF
#include "flash_if.h"
#else
#define APPLICATION_ADDRESS ((uint32_t)0x08008000)
#include "stm32f7xx_hal_flash.h"
#endif
#include <stdio.h>
#include <string.h>
#include "delay.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

SD_HandleTypeDef hsd1;
DMA_HandleTypeDef hdma_sdmmc1_rx;
DMA_HandleTypeDef hdma_sdmmc1_tx;

/* USER CODE BEGIN PV */

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_DMA_Init(void);
static void MX_SDMMC1_SD_Init(void);
/* USER CODE BEGIN PFP */

//Clock In Interrupt
void EXTI4_IRQHandler(void)
{
	/* EXTI line interrupt detected */
	if(__HAL_GPIO_EXTI_GET_IT(GPIO_PIN_4) != RESET)
	{
		__HAL_GPIO_EXTI_CLEAR_IT(GPIO_PIN_4);



		//__HAL_GPIO_EXTI_CLEAR_IT(GPIO_PIN_4);
	}
}
/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{
  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_DMA_Init();
  MX_FATFS_Init();
  MX_SDMMC1_SD_Init();
  /* USER CODE BEGIN 2 */
  DWT_Delay_Init();
  //wait for VCC to stabilize


  typedef  void (*pFunction)(void);
  pFunction Jump_To_Application;
  uint32_t JumpAddress;
  uint8_t lcdInitialized = 0;

  //FATFS firmwareFatfs;		/* File system object for the logical drive */
  FIL firmwareFile;		/* place to hold 1 file*/

  HAL_Delay(100);

  FLASH_OBProgramInitTypeDef pOBInit;
  HAL_FLASHEx_OBGetConfig(&pOBInit);

  if(OB_WRP_SECTOR_0 & pOBInit.WRPSector)
  {
	  //sector 0 not write protected -> protect it
	  pOBInit.OptionType = OPTIONBYTE_WRP | OPTIONBYTE_BOR;

	  pOBInit.WRPState = OB_WRPSTATE_ENABLE;
	  pOBInit.WRPSector = OB_WRP_SECTOR_0;

	  pOBInit.BORLevel = OB_BOR_LEVEL1;

	  // unlock FLASH in general
	  if(HAL_FLASH_Unlock() == HAL_OK) {
	     // unlock option bytes in particular
	     if(HAL_FLASH_OB_Unlock() == HAL_OK) {
	        // erase option bytes before programming

	           // program selected option byte
	            if(HAL_FLASHEx_OBProgram(&pOBInit) != HAL_OK)
	            {
	              HAL_FLASH_OB_Lock();
	              HAL_FLASH_Lock();

	            }
	            else
	            {
					HAL_FLASH_OB_Launch();
					HAL_FLASH_OB_Lock();
					HAL_FLASH_Lock();
	            }
	     }
	  }
  }
  //--------------------------------------------------------------------
#define AUTO_DETECT_MAIN_IMG 0
#if AUTO_DETECT_MAIN_IMG
	FRESULT fr;     /* Return value */
	uint8_t startBootloader = 0;
	fr = f_mount((FATFS*)&SDFatFS, (TCHAR const*)SDPath,1);
	if( fr != FR_OK)
	{

	}
	else
	{
		DIR dj;         /* Directory object */
		FILINFO fno;    /* File information */

		fr = f_findfirst(&dj, &fno, "", "REBOOT.CMD");  /* Start to search for LXR image */
		if(fr == FR_OK && fno.fname[0])
		{
			startBootloader = 1;
		}
	}

	f_mount((FATFS*)NULL, (TCHAR const*)SDPath,1);
#endif

  //check if encoder is pressed
#if AUTO_DETECT_MAIN_IMG
  if(((Enc_Switch_GPIO_Port->IDR & Enc_Switch_Pin) == 0) || (startBootloader))
#else
	  if((Enc_Switch_GPIO_Port->IDR & Enc_Switch_Pin) == 0)
#endif
  {

		lcd_init();
		lcdInitialized = 1;
		lcd_clear();
		lcd_home();

		//start bootloader
		lcd_string("LXR Updater 2 ");
		HAL_Delay(500);

		ImageFileHeader imageHeader;
		FRESULT fr;     /* Return value */

		fr = f_mount((FATFS*)&SDFatFS, (TCHAR const*)SDPath,1);
		if( fr != FR_OK)
		{
			//uint8_t error = 1;			//error
			lcd_clear();
			lcd_home();
			lcd_string("can't mount");
			lcd_setcursor(0,2);
			lcd_string("SD Card");
			while(1);
		}


#if 1

    DIR dj;         /* Directory object */
    FILINFO fno;    /* File information */
#if AUTO_DETECT_MAIN_IMG
    if (startBootloader)
    {
    	fr = f_findfirst(&dj, &fno, "", "MAIN.IMG");  /* Start to search for LXR image */
    }
    else
    {
    	fr = f_findfirst(&dj, &fno, "", "LXR*.img");  /* Start to search for LXR image */
    }
#else
#if EURORACK_MODULE
    fr = f_findfirst(&dj, &fno, "", "LXRE*.img");  /* Start to search for LXR image */
#else
    fr = f_findfirst(&dj, &fno, "", "LXR*.img");  /* Start to search for LXR image */
#endif
#endif

    if(fr == FR_OK && fno.fname[0])
    {
    	fr = f_open((FIL*)&firmwareFile,fno.fname,FA_OPEN_EXISTING | FA_READ);
		if(fr!=FR_OK)
		{
			//error
			lcd_clear();
			lcd_home();
			lcd_string("Can't open");
			lcd_setcursor(0,2);
			lcd_string("LXRE*.img!");
			while(1);
		}
    }
    else
    {
    	//error
    				lcd_clear();
    				lcd_home();
    				lcd_string("LXRE*.img");
    				lcd_setcursor(0,2);
    				lcd_string("Not Found!");
    				while(1);
    }

#else

		//check for firmware file on SD card
		fr = f_open((FIL*)&firmwareFile,"FIRMWARE.BIN",FA_OPEN_EXISTING | FA_READ);
	if(fr!=FR_OK)
	{
		//error
		lcd_clear();
		lcd_home();
		lcd_string("FIRMWARE.BIN");
		lcd_setcursor(0,2);
		lcd_string("Not Found!");
		while(1);
	}
#endif


		//get filesize in bytes
		FSIZE_t filesize = f_size((FIL*)&firmwareFile);

		lcd_clear();
		lcd_home();
		lcd_string("Checking");
		lcd_setcursor(0,2);
		lcd_string("Firmware...");

		//read file header
		UINT bytesRead;
		f_read((FIL*)&firmwareFile, (void*)&imageHeader, HEADER_SIZE, &bytesRead);

		//header sanity check
#if EURORACK_MODULE
		if(strncmp(imageHeader.fileHeader, "LXRERIMG", (unsigned int)8) != 0)
#else
		if(strncmp(imageHeader.fileHeader, "LXRV2IMG", (unsigned int)8) != 0)
#endif
		{
			//error -> filetype header tag does not match!
			lcd_clear();
			lcd_home();
			lcd_string("LXR*.img");
			lcd_setcursor(0,2);
			lcd_string("Not Valid!");
			while(1);
		}

		//Filesize check
		filesize = imageHeader.datasize;
		if( (filesize <= 0) || (filesize >= (USER_FLASH_END_ADDRESS-ADDR_FLASH_SECTOR_1)))
		{
			lcd_clear();
			lcd_home();
			lcd_string("LXR*.img");
			lcd_setcursor(0,2);
			lcd_string("Wrong Size!");
			while(1);
		}

		//checksum calculation
		uint8_t sum = 0;
		for(uint32_t i=0; i<filesize; i++)
		{
			char byte;
			f_read((FIL*)&firmwareFile, (void*)&byte, 1, &bytesRead);
			sum += byte;
			if(bytesRead==0) break;
		}
		sum += imageHeader.checksum;

		if(sum != 0xff)
		{
			lcd_clear();
			lcd_home();
			lcd_string("Wrong");
			lcd_setcursor(0,2);
			lcd_string("Checksum!");
			while(1);
		}

		//rewind to firmware datablob
		f_lseek((FIL*)&firmwareFile, (FSIZE_t)HEADER_SIZE );
		//proceed to load image to flash



#if !USE_NEW_FLASH_IF
		FLASH_If_Init();
#else
		HAL_FLASH_Unlock();
#endif

#if 0
		//erase the application flash
		FLASH_If_Erase(APPLICATION_ADDRESS);
#else
		uint32_t SectorError;
		FLASH_EraseInitTypeDef eraseInit;
		eraseInit.TypeErase 	= FLASH_TYPEERASE_SECTORS;
		eraseInit.Sector 		=  FLASH_SECTOR_1;
		eraseInit.NbSectors	 	= 6; //sectors 1-7
		eraseInit.VoltageRange 	= FLASH_VOLTAGE_RANGE_3;


		HAL_FLASHEx_Erase(&eraseInit, &SectorError);

		if(SectorError != 0xFFFFFFFF)
		{
			lcd_clear();
			lcd_home();
			lcd_string("Can't erase");
			lcd_setcursor(0,2);
			lcd_string("Flash!");
			while(1);
		}
#endif

		lcd_clear();
		lcd_home();
		lcd_string("Loading Firmware");
		lcd_setcursor(0,2);
		lcd_string("0%");


		#define DATA_BUFFER_SIZE 16

		uint32_t bytesWritten=0;
		uint32_t data[DATA_BUFFER_SIZE];
		uint8_t eof_reached = 0;

		while(!eof_reached)
		{
			f_read((FIL*)&firmwareFile, data, DATA_BUFFER_SIZE*4, &bytesRead);

			if(bytesRead)
			{
				//program flash
				uint32_t addr = APPLICATION_ADDRESS+bytesWritten;

				//check address sanity
				if( (addr < APPLICATION_ADDRESS) || (addr >=(USER_FLASH_END_ADDRESS)) )
				{
					lcd_clear();
					lcd_home();
					lcd_string("Address");
					lcd_setcursor(0,2);
					lcd_string("Error!");
					while(1);
				}
#if !USE_NEW_FLASH_IF
				if(FLASH_If_Write((__IO uint32_t*)(&addr), data, bytesRead/4) != 0)
				{
					lcd_clear();
					lcd_home();
					lcd_string("Flash write");
					lcd_setcursor(0,2);
					lcd_string("Error!");
					while(1);
				}
#else
				HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD, addr, data);
#endif


			}
			else
			{
				//file finished
				eof_reached=1;
			}

			//update Display
			bytesWritten += bytesRead;

			if(bytesWritten == filesize)
			{
				//datablob finished
				eof_reached=1;
			}
			//lcd_clear();
			//lcd_home();
			//lcd_string("Loading Firmware");
			char text[17];
			sprintf(text, "%d%%", (uint8_t)(bytesWritten/(float)filesize * 100.f));
			lcd_setcursor(0,2);
			lcd_string(text);
		}

#if AUTO_DETECT_MAIN_IMG

		if(startBootloader)
		{
			//main.img was found
			//-> delete it

			fr = f_unlink("REBOOT.CMD");

		}
#endif

		//programming finished - reboot message
		lcd_clear();
		lcd_home();
		lcd_string("Update OK!");
		//goto 2nd line
		lcd_setcursor(0,2);
		lcd_string("Please reboot!");

		NVIC_SystemReset();
		while(1);



  }
  //else
  //always start normal application

  {
	  //start normal firmware
	  /* Test if user code is programmed starting from address "APPLICATION_ADDRESS" */
		if (((*(__IO uint32_t*)APPLICATION_ADDRESS) & 0x2FFE0000 ) == 0x20080000)
		{

			  /* USER CODE BEGIN 2 */
			  DWT_Delay_Init();


			//uart_deinit();
			/* Jump to user application */
			JumpAddress = *(__IO uint32_t*) (APPLICATION_ADDRESS + 4);
			Jump_To_Application = (pFunction) JumpAddress;

			HAL_RCC_DeInit();

		//	HAL_DeInit();

		//	fr = f_mount(NULL, (TCHAR const*)SDPath,1);
		//	  FATFS_UnLinkDriver(SDPath);
		//	  HAL_SD_MspDeInit(&hsd1);

			SysTick->CTRL = 0;
			SysTick->LOAD = 0;
			SysTick->VAL  = 0;

			  /* Disable all interrupts */
								//  __disable_irq();

			//align for the new vector table
			//NVIC_SetVectorTable(NVIC_VectTab_FLASH, APPLICATION_ADDRESS);
			SCB->VTOR = APPLICATION_ADDRESS;

			/* Initialize user application's Stack Pointer */
			__set_MSP(*(__IO uint32_t*) APPLICATION_ADDRESS);

			//NVIC_SystemReset();

			//  HAL_DeInit();
			//  HAL_SuspendTick();



			Jump_To_Application();
		}
		else
		{

			if(!lcdInitialized)
			{
				lcd_init();
			}

			//error
			lcd_clear();
			lcd_home();
			lcd_string("No Firmware");
			lcd_setcursor(0,2);
			lcd_string("Installed!");
			while(1);
		}
  }





  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
  }
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};
  RCC_PeriphCLKInitTypeDef PeriphClkInitStruct = {0};

  /** Configure the main internal regulator output voltage
  */
  __HAL_RCC_PWR_CLK_ENABLE();
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);
  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLM = 8;
  RCC_OscInitStruct.PLL.PLLN = 216;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLQ = 9;
  RCC_OscInitStruct.PLL.PLLR = 2;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }
  /** Activate the Over-Drive mode
  */
  if (HAL_PWREx_EnableOverDrive() != HAL_OK)
  {
    Error_Handler();
  }
  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV4;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV2;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_7) != HAL_OK)
  {
    Error_Handler();
  }
  PeriphClkInitStruct.PeriphClockSelection = RCC_PERIPHCLK_SDMMC1|RCC_PERIPHCLK_CLK48;
  PeriphClkInitStruct.Clk48ClockSelection = RCC_CLK48SOURCE_PLL;
  PeriphClkInitStruct.Sdmmc1ClockSelection = RCC_SDMMC1CLKSOURCE_CLK48;
  if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInitStruct) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief SDMMC1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_SDMMC1_SD_Init(void)
{

  /* USER CODE BEGIN SDMMC1_Init 0 */

  /* USER CODE END SDMMC1_Init 0 */

  /* USER CODE BEGIN SDMMC1_Init 1 */

  /* USER CODE END SDMMC1_Init 1 */
  hsd1.Instance = SDMMC1;
  hsd1.Init.ClockEdge = SDMMC_CLOCK_EDGE_RISING;
  hsd1.Init.ClockBypass = SDMMC_CLOCK_BYPASS_DISABLE;
  hsd1.Init.ClockPowerSave = SDMMC_CLOCK_POWER_SAVE_DISABLE;
  hsd1.Init.BusWide = SDMMC_BUS_WIDE_1B;
  hsd1.Init.HardwareFlowControl = SDMMC_HARDWARE_FLOW_CONTROL_ENABLE;
  hsd1.Init.ClockDiv = 2;
  /* USER CODE BEGIN SDMMC1_Init 2 */

  /* USER CODE END SDMMC1_Init 2 */

}

/**
  * Enable DMA controller clock
  */
static void MX_DMA_Init(void)
{

  /* DMA controller clock enable */
  __HAL_RCC_DMA2_CLK_ENABLE();

  /* DMA interrupt init */
  /* DMA2_Stream3_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(DMA2_Stream3_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(DMA2_Stream3_IRQn);
  /* DMA2_Stream6_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(DMA2_Stream6_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(DMA2_Stream6_IRQn);

}

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOH_CLK_ENABLE();
  __HAL_RCC_GPIOE_CLK_ENABLE();
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOD_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOE, LCD_D4_Pin|LCD_D5_Pin|LCD_D6_Pin|LCD_D7_Pin
                          |LCD_E_Pin|LCD_RS_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(LED_43_GPIO_Port, LED_43_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pins : LCD_D4_Pin LCD_D5_Pin LCD_D6_Pin LCD_D7_Pin
                           LCD_E_Pin LCD_RS_Pin */
  GPIO_InitStruct.Pin = LCD_D4_Pin|LCD_D5_Pin|LCD_D6_Pin|LCD_D7_Pin
                          |LCD_E_Pin|LCD_RS_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
  HAL_GPIO_Init(GPIOE, &GPIO_InitStruct);

  /*Configure GPIO pins : Enc_A_Pin Enc_B_Pin */
  GPIO_InitStruct.Pin = Enc_A_Pin|Enc_B_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOE, &GPIO_InitStruct);

  /*Configure GPIO pin : Enc_Switch_Pin */
  GPIO_InitStruct.Pin = Enc_Switch_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  HAL_GPIO_Init(Enc_Switch_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pins : SDIO_CS_Pin SD_CD_Pin */
  GPIO_InitStruct.Pin = SDIO_CS_Pin|SD_CD_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOD, &GPIO_InitStruct);

  /*Configure GPIO pin : LED_43_Pin */
  GPIO_InitStruct.Pin = LED_43_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(LED_43_GPIO_Port, &GPIO_InitStruct);

}

/* USER CODE BEGIN 4 */

/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
		lcd_init();


	//error
	lcd_clear();
	lcd_home();
	lcd_string("Error");

	while(1);
  /* USER CODE END Error_Handler_Debug */
}

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     tex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
