/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2025 STMicroelectronics.
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
#include <stdio.h>
#include <string.h>
#include "lcd.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
#define MAIN_MENU  0
#define ADC_MENU   1
#define LED_MENU   2
#define WATCH_MENU 3


#define ROW  0
#define COLUMN 1 
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
ADC_HandleTypeDef hadc1;

RTC_HandleTypeDef hrtc;

UART_HandleTypeDef huart2;

/* USER CODE BEGIN PV */


volatile int8_t cursorPosition[2] ;
volatile char uart2Command[3];

RTC_TimeTypeDef time;
RTC_DateTypeDef date;



volatile uint8_t menu;
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_ADC1_Init(void);
static void MX_RTC_Init(void);
static void MX_USART2_UART_Init(void);
/* USER CODE BEGIN PFP */

void TimeStringHanldler(char *data){

   

    HAL_RTC_GetTime(&hrtc, &time, RTC_FORMAT_BCD);
    HAL_RTC_GetDate(&hrtc, &date, RTC_FORMAT_BCD);

    sprintf(data,"%02d:%02d:%02d",time.Hours, time.Minutes, time.Seconds);


}


void MenuHandler(){
  

  if(strcmp(uart2Command, "ba") == 0){ menu = MAIN_MENU; }

  if(cursorPosition[ROW] == 0 && cursorPosition[COLUMN] == 0 && strcmp(uart2Command, "en") == 0) {menu = ADC_MENU;}

  if(cursorPosition[ROW] == 0 && cursorPosition[COLUMN] == 7 && strcmp(uart2Command, "en") == 0) {menu = LED_MENU;}

  if(cursorPosition[ROW] == 1 && cursorPosition[COLUMN] == 0 && strcmp(uart2Command, "en") == 0) {menu = WATCH_MENU;}

}



void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart){

    if (huart->Instance == USART2)
    {
        // Retrieve the context associated with the UART

        // char data[50];
    
     

        uart2Command[3] = '\0';

       CursorPositionHandler(cursorPosition);
       MenuHandler();

        // Lcd_clear(&lcd);

        // Restart UART reception
        HAL_UART_Receive_IT(huart, (uint8_t *)uart2Command, 2);
    }

}

void ADCStringHanldler( uint16_t *adcValue){
 

    HAL_ADC_Start(&hadc1);
    for(int i = 0; i<2 ; i++){
      HAL_ADC_PollForConversion(&hadc1, HAL_MAX_DELAY);
      adcValue[i] =  HAL_ADC_GetValue(&hadc1);
    }

    if(adcValue[0] < 2000 && adcValue[1] < 2010){HAL_GPIO_WritePin(GPIOA,GPIO_PIN_4,SET); HAL_GPIO_WritePin(GPIOB,GPIO_PIN_0,SET); }
    if(adcValue[0] < 2000 && adcValue[1] > 2020){HAL_GPIO_WritePin(GPIOA,GPIO_PIN_4,SET); HAL_GPIO_WritePin(GPIOB,GPIO_PIN_0,RESET); }
    if(adcValue[0] > 2050 && adcValue[1] < 2010){HAL_GPIO_WritePin(GPIOA,GPIO_PIN_4,RESET); HAL_GPIO_WritePin(GPIOB,GPIO_PIN_0,SET); }
    if(adcValue[0] > 2050 && adcValue[1] > 2020){HAL_GPIO_WritePin(GPIOA,GPIO_PIN_4,RESET); HAL_GPIO_WritePin(GPIOB,GPIO_PIN_0,RESET); }
   
  // return adcValue;

}


void LEDStateStringHandler(char * data){
  uint8_t ledState[2];

  

  ledState[0] = !HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_4);
  ledState[1] = !HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_0);


  sprintf(data, "LED0 %d, LED1 %d\r\n",ledState[0], ledState[1]);
}


Lcd_HandleTypeDef LCDInitial(){


  Lcd_HandleTypeDef lcd;
    // Lcd_PortType ports[] = { D4_GPIO_Port, D5_GPIO_Port, D6_GPIO_Port, D7_GPIO_Port };
  Lcd_PortType ports[] = { GPIOC, GPIOB, GPIOA, GPIOA };
  // Lcd_PinType pins[] = {D4_Pin, D5_Pin, D6_Pin, D7_Pin};
  Lcd_PinType pins[] = {GPIO_PIN_7, GPIO_PIN_6, GPIO_PIN_7, GPIO_PIN_6};
  
  // Lcd_create(ports, pins, RS_GPIO_Port, RS_Pin, EN_GPIO_Port, EN_Pin, LCD_4_BIT_MODE);
  lcd = Lcd_create(ports, pins, GPIOB, GPIO_PIN_5, GPIOB, GPIO_PIN_4, LCD_4_BIT_MODE);
  Lcd_cursor(&lcd, 0,0);
  Lcd_string(&lcd, "Hello Mahsa <3");
  Lcd_cursor(&lcd, 1,0);
  Lcd_string(&lcd, "Select an option");
  HAL_Delay (2000);



  return lcd;
}

void CursorInitial(uint8_t *cursorPosition){
  cursorPosition[ROW] = 0;
  cursorPosition[COLUMN] = 0;

}

void CursorPositionHandler(uint8_t *cursorPosition){
  if(strcmp(uart2Command , "up") == 0) {
        
          if(cursorPosition[ROW] == 0) 
                cursorPosition[ROW] = 1;
          else cursorPosition[ROW] -= 1;
        cursorPosition[COLUMN] = 0; // cursor should not be at (1,7). so the culumn is reset to zero

        sprintf(uart2Command, "w8");

        
      }


      if(strcmp(uart2Command , "dw") == 0 ){

         cursorPosition[ROW] += 1;
        if(cursorPosition[ROW] > 1) 
                cursorPosition[ROW] = 0;
        cursorPosition[COLUMN] = 0; // cursor should not be at (1,7). so the culumn is reset to zero

        sprintf(uart2Command, "w8");

        
      }

      if(strcmp(uart2Command , "ri") == 0 ){
        cursorPosition[COLUMN] += 7;
        if(cursorPosition[COLUMN] > 13) 
                cursorPosition[COLUMN] = 0;

        if(cursorPosition[ROW] == 1) cursorPosition[COLUMN] = 0; // cursor should not be at (1,7). so the culumn is reset to zero

        sprintf(uart2Command, "w8");

        
      }

        if(strcmp(uart2Command , "le") == 0 ){
              if(cursorPosition[COLUMN] == 0) 
                  cursorPosition[COLUMN] = 7;
              else   
                  cursorPosition[COLUMN] -= 7;


        if(cursorPosition[ROW] == 1) cursorPosition[COLUMN] = 0; // cursor should not be at (1,7). so the culumn is reset to zero

        
        sprintf(uart2Command, "w8");

        
      }



}


void MenuInitial(){
  menu = MAIN_MENU;
  
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
  char lcdData[16];
  uint16_t adcValue[2];
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
  MX_ADC1_Init();
  MX_RTC_Init();
  MX_USART2_UART_Init();
  /* USER CODE BEGIN 2 */

    
 
  HAL_UART_Receive_IT(&huart2,(uint8_t *)uart2Command,2);
  
  Lcd_PortType ports[] = { GPIOC, GPIOB, GPIOA, GPIOA };
  // Lcd_PinType pins[] = {D4_Pin, D5_Pin, D6_Pin, D7_Pin};
  Lcd_PinType pins[] = {GPIO_PIN_7, GPIO_PIN_6, GPIO_PIN_7, GPIO_PIN_6};
  Lcd_HandleTypeDef lcd;
  lcd = Lcd_create(ports, pins, GPIOB, GPIO_PIN_5, GPIOB, GPIO_PIN_4, LCD_4_BIT_MODE);

  MenuInitial();
  CursorInitial(cursorPosition);



  // Lcd_create(ports, pins, RS_GPIO_Port, RS_Pin, EN_GPIO_Port, EN_Pin, LCD_4_BIT_MODE);
  
  Lcd_cursor(&lcd, 0,0);
  Lcd_string(&lcd, "Hello Mahsa <3");
  Lcd_cursor(&lcd,1,0);
  Lcd_string(&lcd, "Select Option");

    HAL_Delay(2000);
  
    Lcd_clear(&lcd);


  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */


    TimeStringHanldler(lcdData);

    

    // HAL_UART_Transmit(&huart2, (uint8_t *)lcdData, strlen(lcdData), HAL_MAX_DELAY);


    // HAL_GPIO_TogglePin(GPIOA,GPIO_PIN_4);
    if(menu == MAIN_MENU){
      
      Lcd_clear(&lcd);

      Lcd_cursor(&lcd,cursorPosition[ROW], cursorPosition[COLUMN]);
      Lcd_string(&lcd, ">");

      Lcd_cursor(&lcd,0,1);
      Lcd_string(&lcd, "ADC");

      Lcd_cursor(&lcd,0,8);
      Lcd_string(&lcd, "LED");

      Lcd_cursor(&lcd,1,1);
      Lcd_string(&lcd, "Watch");
    }

    ADCStringHanldler(adcValue);
    if(menu == ADC_MENU){
      Lcd_clear(&lcd);
      



      Lcd_cursor(&lcd,0,0);
      sprintf(lcdData,"ADC0:%d",adcValue[0]);
      Lcd_string(&lcd, lcdData);



      Lcd_cursor(&lcd,1,0);
      sprintf(lcdData,"ADC1:%d",adcValue[1]);
      Lcd_string(&lcd, lcdData);
    }

    if(menu == LED_MENU){
      Lcd_clear(&lcd);
      



      Lcd_cursor(&lcd,0,0);
      sprintf(lcdData,"RED:%d",!HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_4));
      Lcd_string(&lcd, lcdData);



      Lcd_cursor(&lcd,1,0);
      sprintf(lcdData,"BLUE:%d",!HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_0));
      Lcd_string(&lcd, lcdData);
    }

    if(menu == WATCH_MENU){
      Lcd_clear(&lcd);
      

      TimeStringHanldler(lcdData);

      Lcd_cursor(&lcd,0,0);
      
      

      Lcd_string(&lcd, lcdData);




    }
    




    HAL_Delay(100);

  /* USER CODE END 3 */
  }
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Configure the main internal regulator output voltage
  */
  __HAL_RCC_PWR_CLK_ENABLE();
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE2);

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE|RCC_OSCILLATORTYPE_LSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_BYPASS;
  RCC_OscInitStruct.LSEState = RCC_LSE_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_NONE;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_HSE;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_0) != HAL_OK)
  {
    Error_Handler();
  }

  /** Enables the Clock Security System
  */
  HAL_RCC_EnableCSS();
}

/**
  * @brief ADC1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_ADC1_Init(void)
{

  /* USER CODE BEGIN ADC1_Init 0 */

  /* USER CODE END ADC1_Init 0 */

  ADC_ChannelConfTypeDef sConfig = {0};

  /* USER CODE BEGIN ADC1_Init 1 */

  /* USER CODE END ADC1_Init 1 */

  /** Configure the global features of the ADC (Clock, Resolution, Data Alignment and number of conversion)
  */
  hadc1.Instance = ADC1;
  hadc1.Init.ClockPrescaler = ADC_CLOCK_SYNC_PCLK_DIV2;
  hadc1.Init.Resolution = ADC_RESOLUTION_12B;
  hadc1.Init.ScanConvMode = ENABLE;
  hadc1.Init.ContinuousConvMode = ENABLE;
  hadc1.Init.DiscontinuousConvMode = DISABLE;
  hadc1.Init.ExternalTrigConvEdge = ADC_EXTERNALTRIGCONVEDGE_NONE;
  hadc1.Init.ExternalTrigConv = ADC_SOFTWARE_START;
  hadc1.Init.DataAlign = ADC_DATAALIGN_RIGHT;
  hadc1.Init.NbrOfConversion = 2;
  hadc1.Init.DMAContinuousRequests = DISABLE;
  hadc1.Init.EOCSelection = ADC_EOC_SINGLE_CONV;
  if (HAL_ADC_Init(&hadc1) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure for the selected ADC regular channel its corresponding rank in the sequencer and its sample time.
  */
  sConfig.Channel = ADC_CHANNEL_0;
  sConfig.Rank = 1;
  sConfig.SamplingTime = ADC_SAMPLETIME_15CYCLES;
  if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure for the selected ADC regular channel its corresponding rank in the sequencer and its sample time.
  */
  sConfig.Channel = ADC_CHANNEL_1;
  sConfig.Rank = 2;
  if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN ADC1_Init 2 */

  /* USER CODE END ADC1_Init 2 */

}

/**
  * @brief RTC Initialization Function
  * @param None
  * @retval None
  */
static void MX_RTC_Init(void)
{

  /* USER CODE BEGIN RTC_Init 0 */

  /* USER CODE END RTC_Init 0 */

  RTC_TimeTypeDef sTime = {0};
  RTC_DateTypeDef sDate = {0};

  /* USER CODE BEGIN RTC_Init 1 */

  /* USER CODE END RTC_Init 1 */

  /** Initialize RTC Only
  */
  hrtc.Instance = RTC;
  hrtc.Init.HourFormat = RTC_HOURFORMAT_24;
  hrtc.Init.AsynchPrediv = 127;
  hrtc.Init.SynchPrediv = 255;
  hrtc.Init.OutPut = RTC_OUTPUT_DISABLE;
  hrtc.Init.OutPutPolarity = RTC_OUTPUT_POLARITY_HIGH;
  hrtc.Init.OutPutType = RTC_OUTPUT_TYPE_OPENDRAIN;
  if (HAL_RTC_Init(&hrtc) != HAL_OK)
  {
    Error_Handler();
  }

  /* USER CODE BEGIN Check_RTC_BKUP */

  /* USER CODE END Check_RTC_BKUP */

  /** Initialize RTC and set the Time and Date
  */
  sTime.Hours = 0x0;
  sTime.Minutes = 0x0;
  sTime.Seconds = 0x0;
  sTime.DayLightSaving = RTC_DAYLIGHTSAVING_NONE;
  sTime.StoreOperation = RTC_STOREOPERATION_RESET;
  if (HAL_RTC_SetTime(&hrtc, &sTime, RTC_FORMAT_BCD) != HAL_OK)
  {
    Error_Handler();
  }
  sDate.WeekDay = RTC_WEEKDAY_MONDAY;
  sDate.Month = RTC_MONTH_JANUARY;
  sDate.Date = 0x1;
  sDate.Year = 0x0;

  if (HAL_RTC_SetDate(&hrtc, &sDate, RTC_FORMAT_BCD) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN RTC_Init 2 */

  /* USER CODE END RTC_Init 2 */

}

/**
  * @brief USART2 Initialization Function
  * @param None
  * @retval None
  */
static void MX_USART2_UART_Init(void)
{

  /* USER CODE BEGIN USART2_Init 0 */

  /* USER CODE END USART2_Init 0 */

  /* USER CODE BEGIN USART2_Init 1 */

  /* USER CODE END USART2_Init 1 */
  huart2.Instance = USART2;
  huart2.Init.BaudRate = 115200;
  huart2.Init.WordLength = UART_WORDLENGTH_8B;
  huart2.Init.StopBits = UART_STOPBITS_1;
  huart2.Init.Parity = UART_PARITY_NONE;
  huart2.Init.Mode = UART_MODE_TX_RX;
  huart2.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart2.Init.OverSampling = UART_OVERSAMPLING_16;
  if (HAL_UART_Init(&huart2) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USART2_Init 2 */

  /* USER CODE END USART2_Init 2 */

}

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};
/* USER CODE BEGIN MX_GPIO_Init_1 */
/* USER CODE END MX_GPIO_Init_1 */

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOH_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOA, GPIO_PIN_4|GPIO_PIN_6|GPIO_PIN_7, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_0|GPIO_PIN_4|GPIO_PIN_5|GPIO_PIN_6, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOC, GPIO_PIN_7, GPIO_PIN_RESET);

  /*Configure GPIO pins : PA4 PA6 PA7 */
  GPIO_InitStruct.Pin = GPIO_PIN_4|GPIO_PIN_6|GPIO_PIN_7;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /*Configure GPIO pins : PB0 PB4 PB5 PB6 */
  GPIO_InitStruct.Pin = GPIO_PIN_0|GPIO_PIN_4|GPIO_PIN_5|GPIO_PIN_6;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /*Configure GPIO pin : PC7 */
  GPIO_InitStruct.Pin = GPIO_PIN_7;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

/* USER CODE BEGIN MX_GPIO_Init_2 */
/* USER CODE END MX_GPIO_Init_2 */
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
  while (1)
  {
  }
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
