/*
 * misc_stuff.h
 *
 *  Created on: Oct 19, 2025
 *      Author: alvin
 */
/* Define to prevent recursive inclusion -------------------------------------*/
// misc_stuff.h
#ifndef MISC_STUFF_H
#define MISC_STUFF_H

#ifdef __cplusplus
 extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32l4s5i_iot01_accelero.h"
#include "stm32l4s5i_iot01_magneto.h"
#include "../Components/lsm6dsl/lsm6dsl.h"
#include "../Components/lis3mdl/lis3mdl.h"
#include "stm32l4s5i_iot01_gyro.h"
#include "stm32l4s5i_iot01.h"



/** @addtogroup BSP
  * @{
  */

/** @addtogroup STM32L4S5I_IOT01
  * @{
  */

/** @addtogroup STM32L4S5I_IOT01_ACCELERO
  * @{
  */

/** @defgroup STM32L4S5_DISCOVERY_ACCELERO_Exported_Types ACCELERO Exported Types
  * @{
  */

/** @defgroup STM32L4S5I_IOT01_ACCELERO_Exported_Functions ACCELERO Exported Functions
  * @{
  */
/* Sensor Configuration Functions */
uint8_t MY_GYRO_Init(void);
ACCELERO_StatusTypeDef MY_ACCELERO_Init(void);
MAGNETO_StatusTypeDef MY_MAGNETO_Init(void);
void MY_MAGNETO_GetXYZ(int16_t *pDataXYZ);
void MY_GYRO_GetXYZ(float* pfData);
void MY_ACCELERO_AccGetXYZ(int16_t *pDataXYZ);
void MY_TAP_Init(void);
void MY_TAP_DeInit(void);
/**
  * @}
  */

/**
  * @}
  */

/**
  * @}
  */

/**
  * @}
  */
#ifdef __cplusplus
}
#endif

#endif // MISC_STUFF_H

