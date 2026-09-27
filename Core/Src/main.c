/******************************************************************************
* @file           : main.c
* @brief          : Main program body
* (c) EE2028 Teaching Team
******************************************************************************/
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "game1.h"
#include "game2.h"
#include "media.h"
#include "miscfunc.h"
#include "../../Drivers/MY_ADDITION/misc_stuff.h"
#include "../../Drivers/MY_ADDITION/wifi/wifi_all.h"
#include "../../Drivers/MY_ADDITION/SSD1306/ssd1306_all.h"
#include "../../Drivers/BSP/B-L4S5I-IOT01/stm32l4s5i_iot01_accelero.h"
#include "../../Drivers/BSP/B-L4S5I-IOT01/stm32l4s5i_iot01_tsensor.h"
#include "../../Drivers/BSP/B-L4S5I-IOT01/stm32l4s5i_iot01_hsensor.h"
#include "../../Drivers/BSP/B-L4S5I-IOT01/stm32l4s5i_iot01_psensor.h"
#include "../../Drivers/BSP/B-L4S5I-IOT01/stm32l4s5i_iot01_magneto.h"
#include "../../Drivers/BSP/B-L4S5I-IOT01/stm32l4s5i_iot01.h"
#include "stdio.h"
#include "stdbool.h"
#include <math.h>
//
#define WIFI_ON   1

// within main.c
static volatile uint32_t start = 0;
static volatile int count = 1;
static volatile int presses = 0;
uint32_t volatile timer = 0;
volatile bool interrupt = false;
// global stuff
extern volatile uint32_t start3s;
bool volatile button_req;
uint8_t volatile sensor_req = 0;
int volatile game_mode = 1; // when entering a game just change this int
extern uint8_t dead_animate;
extern uint8_t animate;

// functions
static void MX_DMA_Init(void);
static void MX_TIM6_Init(void);
static void MX_GPIO_Init(void);
static void UART1_Init(void);
static void MX_I2C1_Init(void);
extern void initialise_monitor_handles(void);  // for semi-hosting support (printf)
void SystemClock_Config(void);
extern void uart_printf(const char *fmt, ...);
int _kill(int pid, int sig) { return -1; }
int _getpid(void) { return 1; }

//typedef
I2C_HandleTypeDef hi2c1;
UART_HandleTypeDef huart1;
SPI_HandleTypeDef hspi3;
TIM_HandleTypeDef htim6;
DMA_HandleTypeDef hdma_usart1_tx;

//I2C_HandleTypeDef hI2cHandler;
//ES_WIFIObject_t EsWifiObj;

void DMA1_Channel4_IRQHandler(void)
{

  HAL_DMA_IRQHandler(&hdma_usart1_tx);

}
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
  if (htim->Instance == TIM6)
  {
      sensor_req = 1;
  }
}

void EXTI1_IRQHandler(void) {
 HAL_GPIO_EXTI_IRQHandler(GPIO_PIN_1);
}

void SPI3_IRQHandler(void)
{
  HAL_SPI_IRQHandler(&hspi3);
}

void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
if (GPIO_Pin == GPIO_PIN_11 ){
    uint8_t src = SENSOR_IO_Read(LSM6DSL_ACC_GYRO_I2C_ADDRESS_LOW, 0x1C); // TAP_SRC
    if ((src & 0x10) && button_req == true) {
    	 timer = HAL_GetTick();
    	 if (timer - start3s <= 3000){
    		uart_printf("Player Escaped! Good Job! \r\n");
    		button_req = false;
    		game_mode = 2;
    		start3s = 0;
    	 }
     }
   }

if (GPIO_Pin == GPIO_PIN_1) {      // Wi-Fi DRDY
   SPI_WIFI_ISR();           // clears the wait flag/semaphore in es_wifi_io.c
	  }
if(GPIO_Pin == BUTTON_EXTI13_Pin)
{
 count ++;
 if (button_req == true){
	   timer = HAL_GetTick();
	   if (timer - start3s <= 3000){ // this compares with the hal in game2
		  uart_printf("Player Escaped! Good Job! \r\n");
		  button_req = false;
		  game_mode = 2;
		  start3s = 0;
	   }
	  }
 if (count % 2 == 1){
     start = HAL_GetTick(); // this is to start the timeout event
     return;
 }
 uint32_t now = HAL_GetTick();
 if(count % 2 == 0 && ((now-start) <= 400) ){
	   if (presses % 2 == 0){
	   game_mode = 2;
	   }
	   else if (presses % 2 == 1){ // game 2
	   game_mode = 1;
	   }
	   presses ++; // increment pair
     count = 0; //reset the count to find pair
 }
 else{
   start = now; // reset timeout event
   count = 1; // this is to reset the pair >= 1s, but still consider the late press as first of the pair
}
}

__HAL_GPIO_EXTI_CLEAR_IT(GPIO_PIN_11);
__HAL_GPIO_EXTI_CLEAR_IT(BUTTON_EXTI13_Pin);
__HAL_GPIO_EXTI_CLEAR_IT(GPIO_PIN_1); //PAVAN CHANGED
}
int main(void){
 	HAL_Init();
	SystemClock_Config();
	MX_DMA_Init();
	MX_TIM6_Init();
	MX_GPIO_Init();
	MX_I2C1_Init();
	UART1_Init();
	ssd1306_Init();
	MY_ACCELERO_Init();
	MY_GYRO_Init();
	BSP_TSENSOR_Init();
	BSP_LED_Init(LED2);
	BSP_HSENSOR_Init();
	BSP_PSENSOR_Init();
	MY_MAGNETO_Init();
	#if WIFI_ON
	WiFi();
	#endif
	while (1)
	{

		 if (HAL_GetTick() - start3s > 3000){
			if (button_req == true){
			ssd1306_GameOver();
			uart_printf("Game Over! \r\n");
			liveIMU.alive = 0;
			WiFi_Process();
			uart_printf("Double tap and choose next game. \r\n");
			button_req = false;
			start3s = 0;
			}
		 }
		 if (game_mode == 1){
			 MY_TAP_DeInit();
			 ssd1306_Clear(Black);
			 dead_animate = 0;
			 animate = 0;
			 uart_printf("Entering Red Light, Green Light as Player.\r\n");
			 HAL_TIM_Base_Start_IT(&htim6);
			 liveIMU.mode=0;
			 game1();
		 }
		 if (game_mode == 2){
			 uart_printf("Entering Catch And Run as Player.\r\n");
			 ssd1306_Clear(Black);
			 HAL_TIM_Base_Start_IT(&htim6);
			 game2();
		 }
	}
}
static void MX_GPIO_Init(void)
{
  __HAL_RCC_GPIOB_CLK_ENABLE();
   __HAL_RCC_GPIOC_CLK_ENABLE();
   __HAL_RCC_GPIOD_CLK_ENABLE();
   __HAL_RCC_SYSCFG_CLK_ENABLE();
   GPIO_InitTypeDef GPIO_InitStruct = {0};

   GPIO_InitStruct.Pin  = GPIO_PIN_13;      // use GPIO_PIN_13 unless you have a macro
   GPIO_InitStruct.Mode = GPIO_MODE_IT_FALLING;
   GPIO_InitStruct.Pull = GPIO_NOPULL;
   HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);
   HAL_NVIC_SetPriority(EXTI15_10_IRQn, 1, 0);
   HAL_NVIC_EnableIRQ(EXTI15_10_IRQn);

   GPIO_InitStruct.Pin  = GPIO_PIN_11;
   GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING;
   GPIO_InitStruct.Pull = GPIO_NOPULL;
   HAL_GPIO_Init(GPIOD, &GPIO_InitStruct);


}// needed for EXTI port mapping
static void UART1_Init(void)
{
  // PB6 = TX, PB7 = RX
    __HAL_RCC_USART1_CLK_ENABLE();

    __HAL_RCC_GPIOB_CLK_ENABLE();
  GPIO_InitTypeDef GPIO_InitStruct = {0};
  GPIO_InitStruct.Pin = GPIO_PIN_6 | GPIO_PIN_7;
  GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
  GPIO_InitStruct.Alternate = GPIO_AF7_USART1;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
  huart1.Instance = USART1;
  huart1.Init.BaudRate = 115200;
  huart1.Init.WordLength = UART_WORDLENGTH_8B;
  huart1.Init.StopBits = UART_STOPBITS_1;
  huart1.Init.Parity = UART_PARITY_NONE;
  huart1.Init.Mode = UART_MODE_TX_RX;
  huart1.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart1.Init.OverSampling = UART_OVERSAMPLING_16;
  huart1.Init.OneBitSampling = UART_ONE_BIT_SAMPLE_DISABLE;
  huart1.AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_NO_INIT;
  if (HAL_UART_Init(&huart1) != HAL_OK)
  	Error_Handler();
  HAL_NVIC_SetPriority(USART1_IRQn, 6, 0);
  HAL_NVIC_EnableIRQ(USART1_IRQn);


}

static void MX_I2C1_Init(void)
{
__HAL_RCC_GPIOB_CLK_ENABLE();
GPIO_InitTypeDef GPIO_InitStruct = {0};
GPIO_InitStruct.Pin = ARD_D14_Pin|ARD_D15_Pin;
GPIO_InitStruct.Mode = GPIO_MODE_AF_OD;
GPIO_InitStruct.Pull = GPIO_PULLUP;
GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
GPIO_InitStruct.Alternate = GPIO_AF4_I2C1;
HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
__HAL_RCC_I2C1_CLK_ENABLE();
hi2c1.Instance = I2C1;
hi2c1.Init.Timing = (uint32_t)0x00702681;
hi2c1.Init.OwnAddress1 = 0;
hi2c1.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
hi2c1.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
hi2c1.Init.OwnAddress2 = 0;
hi2c1.Init.OwnAddress2Masks = I2C_OA2_NOMASK;
hi2c1.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
hi2c1.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;
if (HAL_I2C_Init(&hi2c1) != HAL_OK) {
    Error_Handler();
}
/** Configure Analogue filter
*/
if (HAL_I2CEx_ConfigAnalogFilter(&hi2c1, I2C_ANALOGFILTER_ENABLE) != HAL_OK)
{
  Error_Handler();
}
/** Configure Digital filter
*/
if (HAL_I2CEx_ConfigDigitalFilter(&hi2c1, 0) != HAL_OK)
{
  Error_Handler();
}
/* USER CODE BEGIN I2C1_Init 2 */
HAL_NVIC_SetPriority(I2C1_EV_IRQn, 5, 1);
HAL_NVIC_EnableIRQ(I2C1_EV_IRQn);
HAL_NVIC_SetPriority(I2C1_ER_IRQn, 5, 1);
HAL_NVIC_EnableIRQ(I2C1_ER_IRQn);
/* USER CODE END I2C1_Init 2 */
}

static void MX_TIM6_Init(void)
{
/* USER CODE BEGIN TIM6_Init 0 */
/* USER CODE END TIM6_Init 0 */
TIM_MasterConfigTypeDef sMasterConfig = {0};
/* USER CODE BEGIN TIM6_Init 1 */
__HAL_RCC_TIM6_CLK_ENABLE();
/* USER CODE END TIM6_Init 1 */
htim6.Instance = TIM6;
htim6.Init.Prescaler = 11999;
htim6.Init.CounterMode = TIM_COUNTERMODE_UP;
htim6.Init.Period = 99;
htim6.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
if (HAL_TIM_Base_Init(&htim6) != HAL_OK)
{
  Error_Handler();
}
sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
if (HAL_TIMEx_MasterConfigSynchronization(&htim6, &sMasterConfig) != HAL_OK)
{
  Error_Handler();
}
/* USER CODE BEGIN TIM6_Init 2 */
HAL_NVIC_SetPriority(TIM6_DAC_IRQn, 1, 0);
HAL_NVIC_EnableIRQ(TIM6_DAC_IRQn);
/* USER CODE END TIM6_Init 2 */
}

void SystemClock_Config(void)
{
RCC_OscInitTypeDef RCC_OscInitStruct = {0};
RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};
/* Configure the main internal regulator output voltage*/
if (HAL_PWREx_ControlVoltageScaling(PWR_REGULATOR_VOLTAGE_SCALE1_BOOST) != HAL_OK)
{
  Error_Handler();
}
/* Configure LSE Drive Capability*/
HAL_PWR_EnableBkUpAccess();
__HAL_RCC_LSEDRIVE_CONFIG(RCC_LSEDRIVE_LOW);
/* Initializes the RCC Oscillators according to the specified parameters in the RCC_OscInitTypeDef structure.*/
RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_LSE|RCC_OSCILLATORTYPE_MSI;
RCC_OscInitStruct.LSEState = RCC_LSE_ON;
RCC_OscInitStruct.MSIState = RCC_MSI_ON;
RCC_OscInitStruct.MSICalibrationValue = 0;
RCC_OscInitStruct.MSIClockRange = RCC_MSIRANGE_6;
RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_MSI;
RCC_OscInitStruct.PLL.PLLM = 1;
RCC_OscInitStruct.PLL.PLLN = 60;
RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
RCC_OscInitStruct.PLL.PLLQ = RCC_PLLQ_DIV2;
RCC_OscInitStruct.PLL.PLLR = RCC_PLLR_DIV2;
if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
{
  Error_Handler();
}
/* Initializes the CPU, AHB and APB buses clocks*/
RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                            |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;
if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_5) != HAL_OK)
{
  Error_Handler();
}
/** Enable MSI Auto calibration
*/
HAL_RCCEx_EnableMSIPLLMode();
}

static void MX_DMA_Init(void)
{
    __HAL_RCC_DMAMUX1_CLK_ENABLE();
    __HAL_RCC_DMA1_CLK_ENABLE();

    HAL_NVIC_SetPriority(DMA1_Channel4_IRQn, 5, 0);
    HAL_NVIC_EnableIRQ(DMA1_Channel4_IRQn);

    hdma_usart1_tx.Instance = DMA1_Channel4;
    hdma_usart1_tx.Init.Request = DMA_REQUEST_USART1_TX;
    hdma_usart1_tx.Init.Direction = DMA_MEMORY_TO_PERIPH;
    hdma_usart1_tx.Init.PeriphInc = DMA_PINC_DISABLE;
    hdma_usart1_tx.Init.MemInc = DMA_MINC_ENABLE;
    hdma_usart1_tx.Init.PeriphDataAlignment = DMA_PDATAALIGN_BYTE;
    hdma_usart1_tx.Init.MemDataAlignment = DMA_MDATAALIGN_BYTE;
    hdma_usart1_tx.Init.Mode = DMA_NORMAL;
    hdma_usart1_tx.Init.Priority = DMA_PRIORITY_LOW;

    HAL_DMA_Init(&hdma_usart1_tx);
    __HAL_LINKDMA(&huart1, hdmatx, hdma_usart1_tx);
}
void Error_Handler(void)
{
__disable_irq();
while (1)
{
}
}



