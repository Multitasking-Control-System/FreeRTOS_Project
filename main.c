/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2026 STMicroelectronics.
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
#include "cmsis_os.h"

#include "FreeRTOS.h"
#include "queue.h"
#include "semphr.h"

#include "lcd.h"
#include "MAX30102.h"
#include "mpu6050.h"
#include "bmp280.h"
#include "bh1750.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include <stdio.h>
#include <string.h>
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
I2C_HandleTypeDef hi2c1;

UART_HandleTypeDef huart2;

/* Definitions for defaultTask */
osThreadId_t defaultTaskHandle;
const osThreadAttr_t defaultTask_attributes = {
  .name = "defaultTask",
  .stack_size = 128 * 4,
  .priority = (osPriority_t) osPriorityNormal,
};
/* USER CODE BEGIN PV */

QueueHandle_t LCDQueue;
SemaphoreHandle_t I2CMutex;
SemaphoreHandle_t UARTMutex;

//char uartBuff[100];

typedef struct
{
    char line1[22];
    char line2[22];

}LCD_Message_t;

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_I2C1_Init(void);
static void MX_USART2_UART_Init(void);
void StartDefaultTask(void *argument);

/* USER CODE BEGIN PFP */
void UART_Print(char *str);

void vLCD_Task(void *argument);

void vMPU_Task(void *argument);

void vBMP_Task(void *argument);

void vBH1750_Task(void *argument);

void vMAX30102_Task(void *argument);
/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
void UART_Print(char *str)
{
    xSemaphoreTake(UARTMutex, portMAX_DELAY);

    HAL_UART_Transmit(&huart2,(uint8_t*)str,strlen(str),HAL_MAX_DELAY);

    xSemaphoreGive(UARTMutex);
}


void vLCD_Task(void *argument)
{
    LCD_Message_t msg;

    lcd_init();

    while(1)
    {
        if(xQueueReceive(LCDQueue,&msg,portMAX_DELAY)==pdPASS)
        {
            xSemaphoreTake(I2CMutex,portMAX_DELAY);

            lcd_clear();

            lcd_puts(LCD_LINE1,msg.line1);

            lcd_puts(LCD_LINE2,msg.line2);

            xSemaphoreGive(I2CMutex);
        }
    }
}

void vMPU_Task(void *argument)
{
	char uartBuff[60];
    MPU6050_Accel_t accel;

    LCD_Message_t msg;

    xSemaphoreTake(I2CMutex,portMAX_DELAY);

    if(MPU6050_Init(&hi2c1)==HAL_OK)
        UART_Print("MPU6050 OK\r\n");
    else
        UART_Print("MPU6050 FAIL\r\n");

    xSemaphoreGive(I2CMutex);

    while(1)
    {
        xSemaphoreTake(I2CMutex,portMAX_DELAY);

        if(MPU6050_ReadAccel(&hi2c1,&accel)==HAL_OK)
        {
            xSemaphoreGive(I2CMutex);

            sprintf(uartBuff,"AX=%d AY=%d AZ=%d\r\n",accel.ax,accel.ay,accel.az);

            UART_Print(uartBuff);

            sprintf(msg.line1,"AX:%4d, AY:%4d",accel.ax,accel.ay);

            sprintf(msg.line2,"AY:%4d",accel.ay);

            xQueueSend(LCDQueue,&msg,portMAX_DELAY);
        }
        else
        {
            xSemaphoreGive(I2CMutex);

            UART_Print("MPU Read Error\r\n");
        }

        vTaskDelay(pdMS_TO_TICKS(2000));
    }
}

void vBMP_Task(void *argument)
{
	char uartBuff[60];
    float temp,press;

    LCD_Message_t msg;

    xSemaphoreTake(I2CMutex,portMAX_DELAY);

    if(BMP280_Init())
        UART_Print("BMP280 OK\r\n");
    else
        UART_Print("BMP280 FAIL\r\n");

    xSemaphoreGive(I2CMutex);

    while(1)
    {
        xSemaphoreTake(I2CMutex,portMAX_DELAY);

        temp = BMP280_ReadTemperature();

        press = BMP280_ReadPressure();

        xSemaphoreGive(I2CMutex);

        sprintf(uartBuff,"Temp=%.2f Pressure=%.2f\r\n", temp, press/100.0f);
        //sprintf(uartBuff,"Temp=%2d Pressure=%4d\r\n",(int)temp,(int)press/100);

        UART_Print(uartBuff);

        sprintf(msg.line1,"Temp:%2d C",(int)temp);

		sprintf(msg.line2,"Pres:%4d",(int)(press/100));

        xQueueSend(LCDQueue,&msg,portMAX_DELAY);

        vTaskDelay(pdMS_TO_TICKS(2000));
    }
}

void vBH1750_Task(void *argument)
{
	char uartBuff[60];
    float lux;

    LCD_Message_t msg;

    xSemaphoreTake(I2CMutex, portMAX_DELAY);

    if(BH1750_CheckConnection(&hi2c1))
    {
        UART_Print("BH1750 OK\r\n");

        BH1750_Init(&hi2c1);
    }
    else
    {
        UART_Print("BH1750 FAIL\r\n");
    }

    xSemaphoreGive(I2CMutex);

    while(1)
    {
        xSemaphoreTake(I2CMutex, portMAX_DELAY);

        if(BH1750_Read(&hi2c1,&lux)==HAL_OK)
        {
            xSemaphoreGive(I2CMutex);

            sprintf(msg.line1,"Light");

            sprintf(msg.line2,"%4d Lux",(int)lux);

            xQueueSend(LCDQueue,&msg,portMAX_DELAY);

            sprintf(uartBuff,"Light=%d Lux\r\n",(int)lux);

            UART_Print(uartBuff);
        }
        else
        {
            xSemaphoreGive(I2CMutex);

            UART_Print("BH1750 Read Error\r\n");
        }

        vTaskDelay(pdMS_TO_TICKS(2000));
    }
}

void vMAX30102_Task(void *argument)
{
	char uartBuff[60];
    uint32_t red, ir;
    int32_t bpm = 0, spo2 = 0;
    LCD_Message_t msg;
    uint32_t lastPrint = 0;

    xSemaphoreTake(I2CMutex, portMAX_DELAY);
    if(MAX30102_CheckConnection()) { UART_Print("MAX30102 OK\r\n"); MAX30102_Init(); }
    else UART_Print("MAX30102 FAIL\r\n");
    xSemaphoreGive(I2CMutex);

    while(1)
    {
        xSemaphoreTake(I2CMutex, portMAX_DELAY);
        MAX30102_ReadFIFO(&red, &ir);
        xSemaphoreGive(I2CMutex);

        if(ir < 50000)
        {
            bpm = 0; spo2 = 0;
            MAX30102_ResetFilters();
        }
        else
        {
            bpm  = MAX30102_EstimateBPM(ir);
            spo2 = MAX30102_EstimateSpO2(red, ir);
        }

        if(HAL_GetTick() - lastPrint >= 2000)
        {
            lastPrint = HAL_GetTick();
            sprintf(msg.line1,"HR:%3ld BPM", (long)bpm);
            sprintf(msg.line2, bpm==0 ? "No Finger" : "SpO2:%3ld%%", (long)spo2);
            xQueueSend(LCDQueue,&msg,portMAX_DELAY);
            sprintf(uartBuff,"Heart Rate = %ld BPM  SpO2=%ld%%\r\n",(long)bpm,(long)spo2);
            UART_Print(uartBuff);
        }
        vTaskDelay(pdMS_TO_TICKS(20));
    }
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
  MX_I2C1_Init();
  MX_USART2_UART_Init();
  /* USER CODE BEGIN 2 */



  /* USER CODE END 2 */

  /* Init scheduler */
  osKernelInitialize();

  /* USER CODE BEGIN RTOS_MUTEX */
  /* add mutexes, ... */
  I2CMutex = xSemaphoreCreateMutex();
  UARTMutex = xSemaphoreCreateMutex();

  if((I2CMutex == NULL) || (UARTMutex == NULL))
  {
      Error_Handler();
  }

  /* USER CODE END RTOS_MUTEX */

  /* USER CODE BEGIN RTOS_SEMAPHORES */
  /* add semaphores, ... */
  /* USER CODE END RTOS_SEMAPHORES */

  /* USER CODE BEGIN RTOS_TIMERS */
  /* start timers, add new ones, ... */
  /* USER CODE END RTOS_TIMERS */

  /* USER CODE BEGIN RTOS_QUEUES */
  /* add queues, ... */
  LCDQueue = xQueueCreate(10,sizeof(LCD_Message_t));
  if((LCDQueue == NULL))
  {
	  Error_Handler();
  }

  /* USER CODE END RTOS_QUEUES */

  /* Create the thread(s) */
  /* creation of defaultTask */
  defaultTaskHandle = osThreadNew(StartDefaultTask, NULL, &defaultTask_attributes);

  /* USER CODE BEGIN RTOS_THREADS */
  /* add threads, ... */
  xTaskCreate(vLCD_Task,"LCD",512,NULL,1,NULL);

    xTaskCreate(vMPU_Task,"MPU",512,NULL,2,NULL);

    xTaskCreate(vBMP_Task,"BMP",512,NULL,3,NULL);

    xTaskCreate(vBH1750_Task,"BH1750",512,NULL,2,NULL);

    xTaskCreate(vMAX30102_Task,"MAX30102",768,NULL,2,NULL);


  /* USER CODE END RTOS_THREADS */

  /* USER CODE BEGIN RTOS_EVENTS */
  /* add events, ... */
  /* USER CODE END RTOS_EVENTS */

  /* Start scheduler */
  osKernelStart();

  /* We should never get here as control is now taken by the scheduler */

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

  /** Configure the main internal regulator output voltage
  */
  __HAL_RCC_PWR_CLK_ENABLE();
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI;
  RCC_OscInitStruct.PLL.PLLM = 8;
  RCC_OscInitStruct.PLL.PLLN = 168;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLQ = 7;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
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
  * @brief I2C1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_I2C1_Init(void)
{

  /* USER CODE BEGIN I2C1_Init 0 */

  /* USER CODE END I2C1_Init 0 */

  /* USER CODE BEGIN I2C1_Init 1 */

  /* USER CODE END I2C1_Init 1 */
  hi2c1.Instance = I2C1;
  hi2c1.Init.ClockSpeed = 100000;
  hi2c1.Init.DutyCycle = I2C_DUTYCYCLE_2;
  hi2c1.Init.OwnAddress1 = 0;
  hi2c1.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
  hi2c1.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
  hi2c1.Init.OwnAddress2 = 0;
  hi2c1.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
  hi2c1.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;
  if (HAL_I2C_Init(&hi2c1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN I2C1_Init 2 */

  /* USER CODE END I2C1_Init 2 */

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
  /* USER CODE BEGIN MX_GPIO_Init_1 */

  /* USER CODE END MX_GPIO_Init_1 */

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOH_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();

  /* USER CODE BEGIN MX_GPIO_Init_2 */

  /* USER CODE END MX_GPIO_Init_2 */
}

/* USER CODE BEGIN 4 */

/* USER CODE END 4 */

/* USER CODE BEGIN Header_StartDefaultTask */
/**
  * @brief  Function implementing the defaultTask thread.
  * @param  argument: Not used
  * @retval None
  */
/* USER CODE END Header_StartDefaultTask */
void StartDefaultTask(void *argument)
{
  /* USER CODE BEGIN 5 */
  /* Infinite loop */
  for(;;)
  {
    osDelay(1);
  }
  /* USER CODE END 5 */
}

/**
  * @brief  Period elapsed callback in non blocking mode
  * @note   This function is called  when TIM6 interrupt took place, inside
  * HAL_TIM_IRQHandler(). It makes a direct call to HAL_IncTick() to increment
  * a global variable "uwTick" used as application time base.
  * @param  htim : TIM handle
  * @retval None
  */
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
  /* USER CODE BEGIN Callback 0 */

  /* USER CODE END Callback 0 */
  if (htim->Instance == TIM6)
  {
    HAL_IncTick();
  }
  /* USER CODE BEGIN Callback 1 */

  /* USER CODE END Callback 1 */
}

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
#ifdef USE_FULL_ASSERT
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
