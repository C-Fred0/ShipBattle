/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2022 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "main.h"
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */
/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
// Image settings
#define IMG_ROWS 320
#define IMG_COLUMNS 240
#define IMG_TOTAL_BUF32 IMG_ROWS*IMG_COLUMNS/2 //(total de datos 32 bits)= 38400
#define IMG_TOTAL_BUF16 IMG_ROWS*IMG_COLUMNS //(total de datos 16 bits)= 76800
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */
/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
I2C_HandleTypeDef hi2c3;

SPI_HandleTypeDef hspi5;

TIM_HandleTypeDef htim3;

/* USER CODE BEGIN PV */
/* USER CODE END PV */

void delayx(uint32_t nTime);

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_I2C3_Init(void);
static void MX_SPI5_Init(void);
static void MX_TIM3_Init(void);
/* USER CODE BEGIN PFP */
/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
#include "lcd_Touch_F4DISCO.c" //incluir librerias del LCD para Touch
static volatile uint32_t frame_buffer[IMG_TOTAL_BUF32]; //Bufer de imagen
uint16_t Xant=0, Yant=0, TP_X=0, TP_Y=0;				//Valores de touch
uint16_t Y_nave=120, Y_nave_ant=120, vel=500;			//Valores de nave
uint16_t Y_enemy=15, Y_enemy_ant=16, dir_enemy=0;		//Valores de enemigo
int16_t LaserX, LaserY,LaserX_ant, LaserY_ant, Laser_ON;//Valores del laser
uint16_t dir=0, win=5;									//Dirección y vidas de enemigo
//TP_STATE TP_StateOK;/*------------- INTERRUPCION DEL TIMER 3 -------------------------------------*/
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim){
	HAL_GPIO_TogglePin(GPIOG, LED_GREEN_Pin);
	if(win!=0){ //Si el enemigo todavia tiene vidas...
		if(dir_enemy == 0){	//Si va para abajo...
			if(Y_enemy_ant < 225){
				Y_enemy = Y_enemy + 1; //Baja 1
				//Borra nave enemiga
				LCD_ILI9341_DrawFilledRectangle_ImageBuffer(0, Y_enemy_ant-15, 20, Y_enemy_ant-5, ILI9341_COLOR_BLACK, (uint32_t*)frame_buffer); //dibujar nuevo
				LCD_ILI9341_DrawFilledRectangle_ImageBuffer(10, Y_enemy_ant-5, 30, Y_enemy_ant+5, ILI9341_COLOR_BLACK, (uint32_t*)frame_buffer); //dibujar nuevo
				LCD_ILI9341_DrawFilledRectangle_ImageBuffer(0, Y_enemy_ant+5, 20, Y_enemy_ant+15, ILI9341_COLOR_BLACK, (uint32_t*)frame_buffer); //dibujar nuevo
				//Pinta nueva nave enemiga
				LCD_ILI9341_DrawFilledRectangle_ImageBuffer(0, Y_enemy-15, 20, Y_enemy-5, ILI9341_COLOR_RED, (uint32_t*)frame_buffer); //dibujar nuevo
				LCD_ILI9341_DrawFilledRectangle_ImageBuffer(10, Y_enemy-5, 30, Y_enemy+5, ILI9341_COLOR_RED, (uint32_t*)frame_buffer); //dibujar nuevo
				LCD_ILI9341_DrawFilledRectangle_ImageBuffer(0, Y_enemy-5, 10,Y_enemy+5, ILI9341_COLOR_BLACK, (uint32_t*)frame_buffer); //dibujar nuevo
				LCD_ILI9341_DrawFilledRectangle_ImageBuffer(0, Y_enemy+5, 20, Y_enemy+15, ILI9341_COLOR_RED, (uint32_t*)frame_buffer); //dibujar nuevo
				Y_enemy_ant = Y_enemy;

			}
			else{dir_enemy=1;} //Llegó al fondo, ahora va para arriba
		}
		if(dir_enemy == 1){ //Si va para arriba...
			if(Y_enemy_ant > 15){
				Y_enemy = Y_enemy - 1; //Sube 1
				//Borra nave enemiga
				LCD_ILI9341_DrawFilledRectangle_ImageBuffer(0, Y_enemy_ant-15, 20, Y_enemy_ant-5, ILI9341_COLOR_BLACK, (uint32_t*)frame_buffer); //dibujar nuevo
				LCD_ILI9341_DrawFilledRectangle_ImageBuffer(10, Y_enemy_ant-5, 30, Y_enemy_ant+5, ILI9341_COLOR_BLACK, (uint32_t*)frame_buffer); //dibujar nuevo
				LCD_ILI9341_DrawFilledRectangle_ImageBuffer(0, Y_enemy_ant+5, 20, Y_enemy_ant+15, ILI9341_COLOR_BLACK, (uint32_t*)frame_buffer); //dibujar nuevo
				//Pinta nueva nave
				LCD_ILI9341_DrawFilledRectangle_ImageBuffer(0, Y_enemy-15, 20, Y_enemy-5, ILI9341_COLOR_RED, (uint32_t*)frame_buffer); //dibujar nuevo
				LCD_ILI9341_DrawFilledRectangle_ImageBuffer(10, Y_enemy-5, 30, Y_enemy+5, ILI9341_COLOR_RED, (uint32_t*)frame_buffer); //dibujar nuevo
				LCD_ILI9341_DrawFilledRectangle_ImageBuffer(0, Y_enemy-5, 10,Y_enemy+5, ILI9341_COLOR_BLACK, (uint32_t*)frame_buffer); //dibujar nuevo
				LCD_ILI9341_DrawFilledRectangle_ImageBuffer(0, Y_enemy+5, 20, Y_enemy+15, ILI9341_COLOR_RED, (uint32_t*)frame_buffer); //dibujar nuevo
				Y_enemy_ant = Y_enemy;

			}
			else{dir_enemy=0;} //Llegó al tope, va para abajo
		}

		if(Laser_ON == 1){ //Si se picó al botón..
			LaserX = LaserX-1;	//El laser avanza a la izquierda...
			if (LaserX>1){		//Cuando no ha llegado al borde...
				//Borrar y pintar nuevo laser
				LCD_ILI9341_DrawFilledRectangle_ImageBuffer(LaserX_ant, LaserY-5, LaserX_ant+20,LaserY+5, ILI9341_COLOR_BLACK, (uint32_t*)frame_buffer);
				LCD_ILI9341_DrawFilledRectangle_ImageBuffer(LaserX, LaserY-5, LaserX+20,LaserY+5, ILI9341_COLOR_YELLOW, (uint32_t*)frame_buffer);
				LaserX_ant=LaserX;
				//Si el laser llega al enemigo y están en coordenadas parecidas...
				if(LaserX==15 && LaserY>Y_enemy-20 && LaserY<Y_enemy+20){
					win=win-1;	//Se quita una vida
					//Se borra un cuadro de vida
					LCD_ILI9341_DrawFilledRectangle_ImageBuffer(285-50*(5-win), 0, 285-1, 14, ILI9341_COLOR_BLACK, (uint32_t*)frame_buffer);
				}
			}
			else{ //Ya llegó al borde y se borra
				LCD_ILI9341_DrawFilledRectangle_ImageBuffer(0, LaserY-5, 21, LaserY+5, ILI9341_COLOR_BLACK, (uint32_t*)frame_buffer);
				Laser_ON=0;
			}
		}
	}
	else{}
}
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
  MX_I2C3_Init();
  MX_SPI5_Init();
  MX_TIM3_Init();
  /* USER CODE BEGIN 2 */
	HAL_GPIO_WritePin(GPIOG, LED_RED_Pin, GPIO_PIN_RESET); //LED1 apagado
	HAL_GPIO_WritePin(GPIOG, LED_GREEN_Pin, GPIO_PIN_RESET); //LED2 apagado
	LCD_ILI9341_Init(&hspi5); //inicializar LCD
	LCD_ILI9341_Rotate(LCD_ILI9341_Orientation_Landscape_2);
	if (TP_Config() != 0){ //Inicializar Touch, ERROR?
		LCD_ILI9341_DrawFilledRectangle_ImageBuffer(0,0,320,240, ILI9341_COLOR_WHITE,(uint32_t*)frame_buffer);
		LCD_ILI9341_Puts_ImageBuffer(10, 10, "ID o CONFIGURACION ERROR", &LCD_Font_7x10,ILI9341_COLOR_RED, (uint32_t*)frame_buffer);
		//LCD_ILI9341_Puts(10, 40, "ID o CONFIGURACION ERROR", &LCD_Font_7x10, ILI9341_COLOR_RED,ILI9341_COLOR_BLACK);
		while(1){}
	}
	/////////////////////////////////Preparaciones//////////////////////////////////////
	//Pintar fondo, botón y barra de vida
	LCD_ILI9341_DrawFilledRectangle_ImageBuffer(0,0,320,240, ILI9341_COLOR_BLACK,(uint32_t*)frame_buffer);
	LCD_ILI9341_DrawFilledRectangle_ImageBuffer(140,220,180,240, ILI9341_COLOR_WHITE,(uint32_t*)frame_buffer);
	LCD_ILI9341_DrawFilledRectangle_ImageBuffer(35,0,285,15, ILI9341_COLOR_RED,(uint32_t*)frame_buffer);
	//Pintar nave de personaje
	LCD_ILI9341_DrawFilledRectangle_ImageBuffer(310, Y_nave-25, 320,Y_nave-15, ILI9341_COLOR_BLUE, (uint32_t*)frame_buffer); //dibujar nuevo
	LCD_ILI9341_DrawFilledRectangle_ImageBuffer(300, Y_nave-15, 320,Y_nave-5, ILI9341_COLOR_BLUE, (uint32_t*)frame_buffer); //dibujar nuevo
	LCD_ILI9341_DrawFilledRectangle_ImageBuffer(290, Y_nave-5, 310,Y_nave+5, ILI9341_COLOR_BLUE, (uint32_t*)frame_buffer); //dibujar nuevo
	LCD_ILI9341_DrawFilledRectangle_ImageBuffer(300, Y_nave+5, 320,Y_nave+15, ILI9341_COLOR_BLUE, (uint32_t*)frame_buffer); //dibujar nuevo
	LCD_ILI9341_DrawFilledRectangle_ImageBuffer(310, Y_nave+15, 320,Y_nave+25, ILI9341_COLOR_BLUE, (uint32_t*)frame_buffer); //dibujar nuevo

	LCD_ILI9341_DisplayImage((uint32_t*)frame_buffer);
	HAL_TIM_Base_Start_IT(&htim3);//inicializa interrupciones de overflow del timer3
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
	while (1){
		TP_GetState();
		//>1 para evitar falsas lecturas del touch
		if((TP_State.TouchDetected)&&(TP_State.X<239)&&(TP_State.Y>1)){
			TP_X=TP_State.Y;//Coordenada del touch en X Porque el display esta girado
			TP_Y=240-TP_State.X; //Coordenada del touch en Y
			//Dibujar posicion del touch
			//LCD_ILI9341_DrawLine_ImageBuffer(Xant-10, Yant, Xant+10, Yant,ILI9341_COLOR_GREEN2, (uint32_t*)frame_buffer);
			//LCD_ILI9341_DrawLine_ImageBuffer(Xant, Yant-10, Xant, Yant+10,ILI9341_COLOR_GREEN2, (uint32_t*)frame_buffer);
			//LCD_ILI9341_DrawLine_ImageBuffer(TP_X-10, TP_Y, TP_X+10, TP_Y,ILI9341_COLOR_BLACK, (uint32_t*)frame_buffer);
			//LCD_ILI9341_DrawLine_ImageBuffer(TP_X, TP_Y-10, TP_X, TP_Y+10,ILI9341_COLOR_BLACK, (uint32_t*)frame_buffer);
			if(TP_X>200){
				if((TP_Y>25)&&(TP_Y<215)){
					Y_nave=TP_Y;
					//Borrar nave de jugador
					LCD_ILI9341_DrawFilledRectangle_ImageBuffer(310, Y_nave_ant-25, 320,Y_nave_ant-15, ILI9341_COLOR_BLACK, (uint32_t*)frame_buffer); //dibujar nuevo
					LCD_ILI9341_DrawFilledRectangle_ImageBuffer(300, Y_nave_ant-15, 320,Y_nave_ant-5, ILI9341_COLOR_BLACK, (uint32_t*)frame_buffer); //dibujar nuevo
					LCD_ILI9341_DrawFilledRectangle_ImageBuffer(290, Y_nave_ant-5, 310,Y_nave_ant+5, ILI9341_COLOR_BLACK, (uint32_t*)frame_buffer); //dibujar nuevo
					LCD_ILI9341_DrawFilledRectangle_ImageBuffer(300, Y_nave_ant+5, 320,Y_nave_ant+15, ILI9341_COLOR_BLACK, (uint32_t*)frame_buffer); //dibujar nuevo
					LCD_ILI9341_DrawFilledRectangle_ImageBuffer(310, Y_nave_ant+15, 320,Y_nave_ant+25, ILI9341_COLOR_BLACK, (uint32_t*)frame_buffer); //dibujar nuevo
					//Imprimir nueva nave de jugador
					LCD_ILI9341_DrawFilledRectangle_ImageBuffer(310, Y_nave-25, 320,Y_nave-15, ILI9341_COLOR_BLUE, (uint32_t*)frame_buffer); //dibujar nuevo
					LCD_ILI9341_DrawFilledRectangle_ImageBuffer(300, Y_nave-15, 320,Y_nave-5, ILI9341_COLOR_BLUE, (uint32_t*)frame_buffer); //dibujar nuevo
					LCD_ILI9341_DrawFilledRectangle_ImageBuffer(290, Y_nave-5, 310,Y_nave+5, ILI9341_COLOR_BLUE, (uint32_t*)frame_buffer); //dibujar nuevo
					LCD_ILI9341_DrawFilledRectangle_ImageBuffer(310, Y_nave-5, 320,Y_nave+5, ILI9341_COLOR_BLACK, (uint32_t*)frame_buffer); //dibujar nuevo
					LCD_ILI9341_DrawFilledRectangle_ImageBuffer(300, Y_nave+5, 320,Y_nave+15, ILI9341_COLOR_BLUE, (uint32_t*)frame_buffer); //dibujar nuevo
					LCD_ILI9341_DrawFilledRectangle_ImageBuffer(310, Y_nave+15, 320,Y_nave+25, ILI9341_COLOR_BLUE, (uint32_t*)frame_buffer); //dibujar nuevo
					Y_nave_ant=Y_nave;
				}
			}
			if((TP_X>140)&&(TP_X<180)&&(TP_Y>220)&&(TP_Y<240)){ //Presión del boton de disparo
				Laser_ON=1;
				LaserY = Y_nave_ant;	//Captura eje Y para el laser
				LaserX = 280;			//Desde jugador...
				LaserX_ant=LaserX+1;
			}

			Xant=TP_X;
			Yant=TP_Y;

		}
		if(win==0){ //Si acabaste con el enemigo...
			//Borrar enemigo e imprimir mensaje de victoria
			LCD_ILI9341_DrawFilledRectangle_ImageBuffer(0, 0, 30, 30, ILI9341_COLOR_BLACK, (uint32_t*)frame_buffer);
			LCD_ILI9341_DrawFilledRectangle_ImageBuffer(0, 16, 140, 220, ILI9341_COLOR_BLACK, (uint32_t*)frame_buffer);
			LCD_ILI9341_DisplayImage((uint32_t*)frame_buffer);
			delayx(5);
			LCD_ILI9341_Puts_ImageBuffer(138 , 120, "YOU WIN", &LCD_Font_7x10,ILI9341_COLOR_GREEN, (uint32_t*)frame_buffer);
			LCD_ILI9341_DisplayImage((uint32_t*)frame_buffer);
			while(1){}	//Fin del juego
		}
		LCD_ILI9341_DisplayImage((uint32_t*)frame_buffer);
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
	}
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void delayx(uint32_t nTime) {while(nTime--);}

void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

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
  RCC_OscInitStruct.PLL.PLLM = 4;
  RCC_OscInitStruct.PLL.PLLN = 180;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLQ = 4;
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

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_5) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief I2C3 Initialization Function
  * @param None
  * @retval None
  */
static void MX_I2C3_Init(void)
{

  /* USER CODE BEGIN I2C3_Init 0 */

  /* USER CODE END I2C3_Init 0 */

  /* USER CODE BEGIN I2C3_Init 1 */

  /* USER CODE END I2C3_Init 1 */
  hi2c3.Instance = I2C3;
  hi2c3.Init.ClockSpeed = 100000;
  hi2c3.Init.DutyCycle = I2C_DUTYCYCLE_2;
  hi2c3.Init.OwnAddress1 = 0;
  hi2c3.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
  hi2c3.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
  hi2c3.Init.OwnAddress2 = 0;
  hi2c3.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
  hi2c3.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;
  if (HAL_I2C_Init(&hi2c3) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure Analogue filter
  */
  if (HAL_I2CEx_ConfigAnalogFilter(&hi2c3, I2C_ANALOGFILTER_ENABLE) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure Digital filter
  */
  if (HAL_I2CEx_ConfigDigitalFilter(&hi2c3, 0) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN I2C3_Init 2 */

  /* USER CODE END I2C3_Init 2 */

}

/**
  * @brief SPI5 Initialization Function
  * @param None
  * @retval None
  */
static void MX_SPI5_Init(void)
{

  /* USER CODE BEGIN SPI5_Init 0 */

  /* USER CODE END SPI5_Init 0 */

  /* USER CODE BEGIN SPI5_Init 1 */

  /* USER CODE END SPI5_Init 1 */
  /* SPI5 parameter configuration*/
  hspi5.Instance = SPI5;
  hspi5.Init.Mode = SPI_MODE_MASTER;
  hspi5.Init.Direction = SPI_DIRECTION_2LINES;
  hspi5.Init.DataSize = SPI_DATASIZE_8BIT;
  hspi5.Init.CLKPolarity = SPI_POLARITY_LOW;
  hspi5.Init.CLKPhase = SPI_PHASE_1EDGE;
  hspi5.Init.NSS = SPI_NSS_SOFT;
  hspi5.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_2;
  hspi5.Init.FirstBit = SPI_FIRSTBIT_MSB;
  hspi5.Init.TIMode = SPI_TIMODE_DISABLE;
  hspi5.Init.CRCCalculation = SPI_CRCCALCULATION_DISABLE;
  hspi5.Init.CRCPolynomial = 10;
  if (HAL_SPI_Init(&hspi5) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN SPI5_Init 2 */

  /* USER CODE END SPI5_Init 2 */

}

/**
  * @brief TIM3 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM3_Init(void)
{

  /* USER CODE BEGIN TIM3_Init 0 */

  /* USER CODE END TIM3_Init 0 */

  TIM_ClockConfigTypeDef sClockSourceConfig = {0};
  TIM_MasterConfigTypeDef sMasterConfig = {0};

  /* USER CODE BEGIN TIM3_Init 1 */

  /* USER CODE END TIM3_Init 1 */
  htim3.Instance = TIM3;
  htim3.Init.Prescaler = 99;
  htim3.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim3.Init.Period = 2699;
  htim3.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim3.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_Base_Init(&htim3) != HAL_OK)
  {
    Error_Handler();
  }
  sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
  if (HAL_TIM_ConfigClockSource(&htim3, &sClockSourceConfig) != HAL_OK)
  {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim3, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM3_Init 2 */

  /* USER CODE END TIM3_Init 2 */

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
  __HAL_RCC_GPIOF_CLK_ENABLE();
  __HAL_RCC_GPIOH_CLK_ENABLE();
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();
  __HAL_RCC_GPIOD_CLK_ENABLE();
  __HAL_RCC_GPIOG_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(LCD_CS_GPIO_Port, LCD_CS_Pin, GPIO_PIN_SET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOD, LCD_RDX_Pin|LCD_WRX_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOG, LED_GREEN_Pin|LED_RED_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin : LCD_CS_Pin */
  GPIO_InitStruct.Pin = LCD_CS_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
  HAL_GPIO_Init(LCD_CS_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pin : BOTN_Pin */
  GPIO_InitStruct.Pin = BOTN_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING;
  GPIO_InitStruct.Pull = GPIO_PULLDOWN;
  HAL_GPIO_Init(BOTN_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pin : BOOT1_Pin */
  GPIO_InitStruct.Pin = BOOT1_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  HAL_GPIO_Init(BOOT1_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pins : LCD_RDX_Pin LCD_WRX_Pin */
  GPIO_InitStruct.Pin = LCD_RDX_Pin|LCD_WRX_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
  HAL_GPIO_Init(GPIOD, &GPIO_InitStruct);

  /*Configure GPIO pins : LED_GREEN_Pin LED_RED_Pin */
  GPIO_InitStruct.Pin = LED_GREEN_Pin|LED_RED_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOG, &GPIO_InitStruct);

  /* EXTI interrupt init*/
  HAL_NVIC_SetPriority(EXTI0_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(EXTI0_IRQn);

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
  __disable_irq();
  while (1){}
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
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
