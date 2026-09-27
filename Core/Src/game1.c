/*
* game1.c
*
*  Created on: Oct 16, 2025
*      Author: Pavan
*/
#include "main.h"
#include "game1.h"
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
#include <stdio.h>
#include <math.h>
#include <stdbool.h>
//extern//
extern int game_mode;
extern uint8_t animate;
extern uint8_t dead_animate;
extern uint8_t sensor_req;
extern TIM_HandleTypeDef htim6;
//funccall//
extern void uart_printf(const char *fmt, ...);
extern void ssd1306_Game1(void);
static void mag_calibrate_ut(float h_ut[3], float h_cal[3]);
static float tanS(float x);
//define
#define CF_ALPHA 0.02f // trust factor
#define YAW_ALPHA  0.06f
#define COS_MIN 0.0871557427f  // cos(85°)
#define G_MPS2 9.81f
#define RAD_TO_DEG (180.0f / 3.14159265358979323846f)
#define DEG_TO_RAD (3.14159265358979323846f / 180.0f)
float dt = 10.0f/1000.0f;
float bias[3] = { 2.587563f, -208.357871f, 1.766620f };
static const float Ainv[3][3] = {
    { 4.158046f, 0.192781f, 0.029771f },
    { 0.192781f, 3.542023f, 0.048736f },
    { 0.029771f, 0.048736f, 3.954912f }
};
void game1(void)
{
	uint8_t mode = 0;// 0 = Green , 1 = Red
	//time related stuff
	uint32_t phase_start = HAL_GetTick(); // to set the initial time first
	uint32_t now = HAL_GetTick();
	uint32_t last_led_toggle = now;
	uint32_t print_2s = now;
	uint32_t one_sec = now;
	uint32_t time_animate = now;
	// data collection / math
	int16_t accel_i16[3];
	int16_t magneto_data_i16[3];
	float gyro_dps[3];
	float magneto[3];
	float magC[3];
	float accel[3];
	float gyro[3];
	float yaw= 0.0f;
	float phiHat_rad= 0.0f;
	float thetaHat_rad= 0.0f;
	static float startupPhi= 0.0f;
	static float startupTheta= 0.0f;
	static float startupYaw= 0.0f;
	static float yaw_filtered = 0.0f;
	//truth//
	static bool startup;
	static bool yaw_setup;
	//--//
	BSP_LED_On(LED2);
	while (game_mode == 1){
		  liveIMU.alive = 1;
		  liveIMU.lastGame = 1;
		  now=HAL_GetTick();
		  if ((now-time_animate) >= 17){
		  ssd1306_Game1();
		  time_animate = now;
		  }
	   if ((now-phase_start)>=10000){
			mode=!mode;
			liveIMU.mode = mode;
			startup = false;
			yaw_setup = false;
			yaw = 0.0f;
			phiHat_rad = 0.0f;
			thetaHat_rad = 0.0f;
			startupPhi = 0.0f;
			startupTheta= 0.0f;
			startupYaw= 0.0f;
			yaw_filtered=0.0f;
			if (mode == 0) {
				uart_printf("\nGreen Light!\r\n");
				BSP_LED_On(LED2);
				animate = 0;
				print_2s =now;
			}
			if (mode == 1) {
				uart_printf("\nRed Light!\r\n");
				animate = 1;
				BSP_LED_Off(LED2);
				one_sec=now;
			}
			phase_start=now;
			}
	   if(mode==0){
			if(now-print_2s >=2000){
				float temp=BSP_TSENSOR_ReadTemp();
				float pres=BSP_PSENSOR_ReadPressure();
				float hum=BSP_HSENSOR_ReadHumidity();
				liveIMU.temp_c = temp;
				liveIMU.pres_hpa = pres;
				liveIMU.hum_pct = hum;
				uart_printf("Temp: %.2f , Pressure: %.2f , Humidity: %.2f\r\n",temp,pres,hum);
				WiFi_Process();
				print_2s =now;
			}
		}
	   if (mode==1){
				if ((now-last_led_toggle)>=500){
					BSP_LED_Toggle(LED2);
					last_led_toggle=now;
				}
				if (sensor_req == 1){
				  sensor_req = 0;
					MY_ACCELERO_AccGetXYZ(accel_i16);
					accel[0] = (float)accel_i16[0] * (G_MPS2/1000.0f);
					accel[1] = (float)accel_i16[1]  * (G_MPS2/1000.0f);
					accel[2] = (float)accel_i16[2] * (G_MPS2/1000.0f);
					float phiHat_acc_rad = atan2f(accel[1],accel[2]);
					float denominator = sqrtf(accel[1]*accel[1] + accel[2]*accel[2]);
					if (denominator < 1e-6f) {
						denominator = 1e-6f;   // clamp
					}
					float thetaHat_acc_rad = atan2f(-accel[0], denominator);
					MY_GYRO_GetXYZ(gyro_dps);
					gyro[0] = gyro_dps[0]/1000.0f  * DEG_TO_RAD;
					gyro[1] = gyro_dps[1]/1000.0f * DEG_TO_RAD;
					gyro[2] = gyro_dps[2]/1000.0f * DEG_TO_RAD;
					  MY_MAGNETO_GetXYZ(magneto_data_i16);
					  magneto[0] =((float)magneto_data_i16[0])*0.1f;
					  magneto[1] =((float)magneto_data_i16[1])*0.1f;
					  magneto[2] =((float)magneto_data_i16[2])*0.1f;
					  mag_calibrate_ut(magneto,magneto);
					if (startup == false){ // to initialise when paused
						phiHat_rad   = phiHat_acc_rad;
						thetaHat_rad = thetaHat_acc_rad;
						startupPhi   = phiHat_rad   * RAD_TO_DEG;  // take baseline now (not at zero)
						startupTheta = thetaHat_rad * RAD_TO_DEG;
						startup = true;
				   }
					// roll is PHI pitch is THETA
					float phiDot_rps = gyro[0] + tanS(thetaHat_rad)* (sinf(phiHat_rad) * gyro[1] + cosf(phiHat_rad)*gyro[2]); // courtesy of philslab
					float thetaDot_rps = cosf(phiHat_rad)* gyro[1] - sinf(phiHat_rad) * gyro[2];
					phiHat_rad = CF_ALPHA * phiHat_acc_rad+ (1.0f - CF_ALPHA) * (phiHat_rad + dt * phiDot_rps);
					thetaHat_rad = CF_ALPHA * thetaHat_acc_rad + (1.0f - CF_ALPHA) * (thetaHat_rad + dt * thetaDot_rps);
					//ST DT0058 Design Tips
					magC[0] = magneto[0] * cosf(thetaHat_rad) + (magneto[1] * sinf(phiHat_rad) + magneto[2] * cosf(phiHat_rad)) * sinf(thetaHat_rad);
					magC[1] = magneto[1] * cosf(phiHat_rad) - magneto[2] * sinf(phiHat_rad);
					yaw = atan2f(magC[1],magC[0]);
					yaw_filtered = atan2f(
					                    (1.0f - YAW_ALPHA) * sinf(yaw_filtered) + YAW_ALPHA * sinf(yaw),
					                    (1.0f - YAW_ALPHA) * cosf(yaw_filtered) + YAW_ALPHA * cosf(yaw)
					                );
					if (yaw_setup == false){ // to initialise when paused
						startupYaw = yaw*RAD_TO_DEG;
						yaw_filtered = yaw;
						yaw_setup = true;
						}
					 if (startup == true) {
						 if (fabsf(phiHat_rad*RAD_TO_DEG - startupPhi) > 5.0f || fabsf(thetaHat_rad*RAD_TO_DEG - startupTheta) > 5.0f || fabsf(yaw_filtered*RAD_TO_DEG - startupYaw) > 10.0f) {
						  // 10 both if on the table
						   HAL_TIM_Base_Stop_IT(&htim6);
							 uart_printf("Game Over\r\n");
							 BSP_LED_Off(LED2);
							 game_mode = 0;
							 liveIMU.alive = 0;
							 dead_animate = 1;
							 ssd1306_Game1();
							 WiFi_Process();
						 }
					 }
				   }
		if ((now-one_sec) >= 2000){
					liveIMU.roll_deg = fabsf(phiHat_rad * RAD_TO_DEG - startupPhi);
					liveIMU.pitch_deg = fabsf(thetaHat_rad * RAD_TO_DEG - startupTheta);
					liveIMU.yaw_deg = fabsf(yaw_filtered * RAD_TO_DEG - startupYaw);
					liveIMU.heading_deg = yaw_filtered * RAD_TO_DEG;
					liveIMU.alive = 1;
					uart_printf("Roll: %.2f, Pitch: %.2f, Yaw: %.2f\r\n",
								liveIMU.roll_deg,
								liveIMU.pitch_deg,
								liveIMU.yaw_deg);
					uart_printf("heading: %.2f\r\n", liveIMU.heading_deg);
					WiFi_Process();
					one_sec = now;
				   }
			 }
	}
	// Game ended not because of death — player survived or switched game
	liveIMU.duration_ms = HAL_GetTick() - phase_start;
}
static float tanS(float x) { // to stop explosion TanSafe
  float s = sinf(x), c = cosf(x);
  if (fabsf(c) < COS_MIN) c = copysignf(COS_MIN, c);
  return s / c;
}
static void mag_calibrate_ut(float magneto[3], float cal[3]) {
  float v0 = magneto[0] - bias[0];
  float v1 = magneto[1] - bias[1];
  float v2 = magneto[2] - bias[2];
  cal[0] = Ainv[0][0]*v0 + Ainv[0][1]*v1 + Ainv[0][2]*v2;
  cal[1] = Ainv[1][0]*v0 + Ainv[1][1]*v1 + Ainv[1][2]*v2;
  cal[2] = Ainv[2][0]*v0 + Ainv[2][1]*v1 + Ainv[2][2]*v2;
}



