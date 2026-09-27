/*
* miscfunc.h
*
*  Created on: Oct 24, 2025
*      Author: alvin
*/
#ifndef INC_MISCFUNC_H_
#define INC_MISCFUNC_H_

typedef struct {
   float roll_deg;
   float pitch_deg;
   float yaw_deg;
   float heading_deg;
   float temp_c;
   float hum_pct;
   float pres_hpa;
   uint8_t mode;
   uint8_t alive;
   uint8_t lastGame;
   uint32_t duration_ms;
} LiveIMU_t;

extern LiveIMU_t liveIMU;


void WiFi(void);
void WiFi_Process(void);

HAL_StatusTypeDef uart_sendDMA(const uint8_t *data, uint16_t len);
void HAL_UART_TxCpltCallback(UART_HandleTypeDef *huart);
#endif /* INC_MISCFUNC_H_ */

