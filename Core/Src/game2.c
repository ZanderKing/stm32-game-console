/*
* game2.c
*
*  Created on: Oct 13, 2025
*      Author: alvin
*/
#include "main.h"
#include "game2.h"
#include "miscfunc.h"
#include "media.h"
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
// ---- //
#define TH1_uT       300.0f   // < TH1 => slow
#define TH2_uT       400.0f  // TH1..TH2 => medium, >= TH2 => fast
#define TH3_uT       500.0f
#define ABS_THRESHOLD 650.0f
#define BLINK_SLOW   800
#define BLINK_MED    400
#define BLINK_FAST   200
#define BLINK_FASTEST 100
// ---- //
extern volatile bool interrupt;
extern TIM_HandleTypeDef htim6;
extern uint8_t sensor_req;
extern int game_mode;
extern void uart_printf(const char *fmt, ...);
extern bool button_req;
extern uint32_t timer;
uint32_t start3s = 0;
int intensity = 0;
float temppreshum_values[3];
float temppreshum_thresholds[3] = {33.0f, 1007.0f, 50.0f};
void game2(void)
{
	liveIMU.alive = 1;
	liveIMU.lastGame = 2;
	MY_TAP_Init();
	static uint32_t led_period_ms = BLINK_SLOW;
	static uint32_t lastBlink = 0;
	static uint32_t print_time = 0;
	uint32_t temp_time = HAL_GetTick();
	int intensity = -1;
	int intensity_pair = 0; // to detect a change, cannot be the same as intensity
	static char* messages[] = {
	    "All clear.",
	    "Enforcer nearby! Be careful.",
	    "Enforcer is even closer!",
	    "Enforcer is right behind! Don't turn back!"
	};
	bool  temppreshum_triggered[3] = {false, false, false};
	static char* messages2[] = {
	    "Temperature spike detected! T:",
		"Pressure spike detected! P:",
		"Humidity spike detected! H:",
	};
	static char* messages3[] = {
	    "C",
		"hPa",
		"%" ,
	};
	while(game_mode == 2){
		   float magneto_data[3];
		   float mag_strengthsq;
		   float ans = 0;
		   int16_t magneto_data_i16[3] = { 0 };
		   uint32_t now = HAL_GetTick(); // universal get time for cycle
		   if (sensor_req == 1){         // 10ms readout - 100HZ
		   sensor_req = 0;
		   MY_MAGNETO_GetXYZ(magneto_data_i16);
		   magneto_data[0] =(float)magneto_data_i16[0]*0.1f;
		   magneto_data[1] =(float)magneto_data_i16[1]*0.1f;
		   magneto_data[2] =(float)magneto_data_i16[2]*0.1f;
		   mag_strengthsq = magneto_data[0]*magneto_data[0] + magneto_data[1]*magneto_data[1] +magneto_data[2]*magneto_data[2];
		   ans = sqrtf(mag_strengthsq);
		   // magneto blink speed
		   if (ans <= TH1_uT){
		       led_period_ms = BLINK_SLOW;                          // case 1
		       intensity_pair = 0;
		       ssd1306_Game2(1);
		   }
		   else if (ans <= TH2_uT){
		       led_period_ms = BLINK_MED;                           // case 2
		       intensity_pair = 1;
		       ssd1306_Game2(2);
		   }
		   else if (ans <= TH3_uT)    {
			   led_period_ms = BLINK_FAST;                          // case 3
			   intensity_pair = 2;
			   ssd1306_Game2(3);
		   }
		   else{
		       led_period_ms = BLINK_FASTEST;                      // case 4
		       intensity_pair = 3;
		       ssd1306_Game2(4);
		      }
		   }
		   if (intensity != intensity_pair && (now-print_time >= 400) ){
			   intensity = intensity_pair;
			   uart_printf("%s\r\n", messages[intensity]);
			   print_time = now;
		   }
		   if ((now - lastBlink) >= led_period_ms) {
		       BSP_LED_Toggle(LED2);
		       lastBlink = now;
		   }
		   if (ans >= ABS_THRESHOLD){
			   uart_printf("You have 3 seconds to react.\r\n");
			   button_req = true;
			   game_mode = 0;
			   BSP_LED_On(LED2);
			   start3s = HAL_GetTick();
       	       HAL_TIM_Base_Stop_IT(&htim6);
			   return;
		   }
		   if (now - temp_time >= 1000){
			   temppreshum_values[0] =BSP_TSENSOR_ReadTemp();
			   temppreshum_values[1] =BSP_PSENSOR_ReadPressure();
			   temppreshum_values[2] =BSP_HSENSOR_ReadHumidity();
			   liveIMU.temp_c =  temppreshum_values[0];
			   liveIMU.pres_hpa = temppreshum_values[1];
			   liveIMU.hum_pct =  temppreshum_values[2];
			   //uart_printf("Temp: %.2f , Pressure: %.2f , Humidity: %.2f\r\n",temppreshum_values[0], temppreshum_values[1], temppreshum_values[2]);
			   WiFi_Process();
			   for (int i = 0 ; i < 3; i++){
				   bool above = (temppreshum_values[i] > temppreshum_thresholds[i]);
				   if (above &&  temppreshum_triggered[i] == false ){
					   uart_printf(messages2[i]);
					   uart_printf("%.1f",temppreshum_values[i]);
					   uart_printf(messages3[i]);
					   uart_printf(". Dangerous environment!\r\n");
				   }
				   temppreshum_triggered[i] = above; // transfer to trigger check if triggered
			   }
			   temp_time =now;
		   }
	}

}



