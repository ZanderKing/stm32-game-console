/*
* misc_stuff.h
*
*  Created on: Oct 19, 2025
*      Author: alvin
*/
#include "misc_stuff.h"
#define LSM6DSL_ACC_GYRO_TAP_CFG       0x58
#define LSM6DSL_ACC_GYRO_TAP_THS_6D    0x59
#define LSM6DSL_ACC_GYRO_INT_DUR2      0x5A
#define LSM6DSL_ACC_GYRO_WAKE_UP_THS   0x5B
#define LSM6DSL_ACC_GYRO_MD1_CFG       0x5E
static GYRO_DrvTypeDef *GyroscopeDrv;
static ACCELERO_DrvTypeDef *AccelerometerDrv;
static MAGNETO_DrvTypeDef  *MagnetoDrv;
extern MAGNETO_DrvTypeDef Lis3mdlMagDrv;
extern ACCELERO_DrvTypeDef Lsm6dslAccDrv;
extern GYRO_DrvTypeDef     Lsm6dslGyroDrv;
ACCELERO_StatusTypeDef MY_ACCELERO_Init(void)
{
 ACCELERO_StatusTypeDef ret = ACCELERO_OK;
 uint16_t ctrl = 0x0000;
 ACCELERO_InitTypeDef LSM6DSL_InitStructure;
 if(Lsm6dslAccDrv.ReadID() != LSM6DSL_ACC_GYRO_WHO_AM_I)
 {
   ret = ACCELERO_ERROR;
 }
 else
 {
   /* Initialize the ACCELERO accelerometer driver structure */
   AccelerometerDrv = &Lsm6dslAccDrv;
   /* MEMS configuration ------------------------------------------------------*/
   /* Fill the ACCELERO accelerometer structure */
   LSM6DSL_InitStructure.AccOutput_DataRate = LSM6DSL_ODR_416Hz ;
   LSM6DSL_InitStructure.Axes_Enable = 0;
   LSM6DSL_InitStructure.AccFull_Scale = LSM6DSL_ACC_FULLSCALE_2G;
   LSM6DSL_InitStructure.BlockData_Update = LSM6DSL_BDU_BLOCK_UPDATE;
   LSM6DSL_InitStructure.High_Resolution = 0;
   LSM6DSL_InitStructure.Communication_Mode = 0;
   /* Configure MEMS: data rate, full scale  */
   ctrl =  (LSM6DSL_InitStructure.AccOutput_DataRate | LSM6DSL_InitStructure.AccFull_Scale);
   /* Configure MEMS: BDU and Auto-increment for multi read/write */
   ctrl |= ((LSM6DSL_InitStructure.BlockData_Update | LSM6DSL_ACC_GYRO_IF_INC_ENABLED) << 8);
   /* Configure the ACCELERO accelerometer main parameters */
   AccelerometerDrv->Init(ctrl);
 }

 //turning ODR on for ODR/4 for LPF1
 ctrl = SENSOR_IO_Read(LSM6DSL_ACC_GYRO_I2C_ADDRESS_LOW, LSM6DSL_ACC_GYRO_CTRL8_XL);
 ctrl |= 0x08;
 SENSOR_IO_Write(LSM6DSL_ACC_GYRO_I2C_ADDRESS_LOW, LSM6DSL_ACC_GYRO_CTRL8_XL, ctrl);
 ///////////////////
 return ret;
}
uint8_t MY_GYRO_Init(void)
{
 uint8_t ret = GYRO_ERROR;
 uint16_t ctrl = 0x0000;
 GYRO_InitTypeDef LSM6DSL_InitStructure;
 if(Lsm6dslGyroDrv.ReadID() != LSM6DSL_ACC_GYRO_WHO_AM_I)
 {
   ret = GYRO_ERROR;
 }
 else
 {
   /* Initialize the gyroscope driver structure */
   GyroscopeDrv = &Lsm6dslGyroDrv;
   /* Configure Mems : data rate, power mode, full scale and axes */
   LSM6DSL_InitStructure.Power_Mode = 0;
   LSM6DSL_InitStructure.Output_DataRate = LSM6DSL_ODR_416Hz;
   LSM6DSL_InitStructure.Axes_Enable = 0;
   LSM6DSL_InitStructure.Band_Width = 0;
   LSM6DSL_InitStructure.BlockData_Update = LSM6DSL_BDU_BLOCK_UPDATE;
   LSM6DSL_InitStructure.Endianness = 0;
   LSM6DSL_InitStructure.Full_Scale = LSM6DSL_GYRO_FS_1000;
   /* Configure MEMS: data rate, full scale  */
   ctrl = (LSM6DSL_InitStructure.Full_Scale | LSM6DSL_InitStructure.Output_DataRate);
   /* Configure MEMS: BDU and Auto-increment for multi read/write */
   ctrl |= ((LSM6DSL_InitStructure.BlockData_Update | LSM6DSL_ACC_GYRO_IF_INC_ENABLED) << 8);
   /* Initialize component */
   GyroscopeDrv->Init(ctrl);
//   ///////////////////
   /* START TO REDO THE LPF BITS*/
   uint8_t ctrl = 0x00;
   uint8_t tmp;
   /* Read CTRL6_C */
   tmp = SENSOR_IO_Read(LSM6DSL_ACC_GYRO_I2C_ADDRESS_LOW, LSM6DSL_ACC_GYRO_CTRL6_C);
   /* Write value to CTRL6_C enable LPF, structure copied from BSP*/
   ctrl = 0x02;
   tmp &= ~(0x03); // clear bits 1:0
   tmp |= ctrl;
   SENSOR_IO_Write(LSM6DSL_ACC_GYRO_I2C_ADDRESS_LOW, LSM6DSL_ACC_GYRO_CTRL6_C, tmp);
   tmp = SENSOR_IO_Read(LSM6DSL_ACC_GYRO_I2C_ADDRESS_LOW, LSM6DSL_ACC_GYRO_CTRL4_C);
   tmp |= 0x02;
   SENSOR_IO_Write(LSM6DSL_ACC_GYRO_I2C_ADDRESS_LOW, LSM6DSL_ACC_GYRO_CTRL4_C, tmp);
   //////////////////////
//    tmp = SENSOR_IO_Read(LSM6DSL_ACC_GYRO_I2C_ADDRESS_LOW, LSM6DSL_ACC_GYRO_CTRL6_C);
//    tmp &= ~0x04;    // clear bit 2 -> LPF1 disabled
//    SENSOR_IO_Write(LSM6DSL_ACC_GYRO_I2C_ADDRESS_LOW, LSM6DSL_ACC_GYRO_CTRL6_C, tmp);
   ret = GYRO_OK;
 }
 return ret;
}
void MY_ACCELERO_AccGetXYZ(int16_t *pDataXYZ)
{
 if(AccelerometerDrv != NULL)
 {
   if(AccelerometerDrv->GetXYZ != NULL)
   {
     AccelerometerDrv->GetXYZ(pDataXYZ);
   }
 }
}
void MY_GYRO_GetXYZ(float* pfData)
{
 if(GyroscopeDrv != NULL)
 {
   if(GyroscopeDrv->GetXYZ!= NULL)
   {
     GyroscopeDrv->GetXYZ(pfData);
   }
 }
}
MAGNETO_StatusTypeDef MY_MAGNETO_Init(void)
{
 MAGNETO_StatusTypeDef ret = MAGNETO_OK;
 MAGNETO_InitTypeDef LIS3MDL_InitStructureMag;
 if(Lis3mdlMagDrv.ReadID() != I_AM_LIS3MDL)
 {
   ret = MAGNETO_ERROR;
 }
 else
 {
   /* Initialize the MAGNETO magnetometer driver structure */
   MagnetoDrv = &Lis3mdlMagDrv;
   /* MEMS configuration ------------------------------------------------------*/
   /* Fill the MAGNETO magnetometer structure */
   LIS3MDL_InitStructureMag.Register1 = LIS3MDL_MAG_TEMPSENSOR_DISABLE | LIS3MDL_MAG_OM_XY_HIGH | LIS3MDL_MAG_ODR_80_HZ ;
   LIS3MDL_InitStructureMag.Register2 = LIS3MDL_MAG_FS_4_GA | LIS3MDL_MAG_REBOOT_DEFAULT | LIS3MDL_MAG_SOFT_RESET_DEFAULT;
   LIS3MDL_InitStructureMag.Register3 = LIS3MDL_MAG_CONFIG_NORMAL_MODE | LIS3MDL_MAG_CONTINUOUS_MODE;
   LIS3MDL_InitStructureMag.Register4 = LIS3MDL_MAG_OM_Z_HIGH | LIS3MDL_MAG_BLE_LSB;
   LIS3MDL_InitStructureMag.Register5 = LIS3MDL_MAG_BDU_MSBLSB;
   /* Configure the MAGNETO magnetometer main parameters */
   MagnetoDrv->Init(LIS3MDL_InitStructureMag);
 }
 return ret;
}
void MY_MAGNETO_GetXYZ(int16_t *pDataXYZ)
{
 if(MagnetoDrv != NULL)
 {
   if(MagnetoDrv->GetXYZ != NULL)
   {
     MagnetoDrv->GetXYZ(pDataXYZ);
   }
 }
}
void MY_TAP_Init(void){
	// CTRL1_XL (0x10)
	 uint8_t tmp = 0x60;
	 SENSOR_IO_Write(LSM6DSL_ACC_GYRO_I2C_ADDRESS_LOW, LSM6DSL_ACC_GYRO_CTRL1_XL, tmp);
	 // TAP_CFG (0x58)
	 tmp = 0x8E;
	 SENSOR_IO_Write(LSM6DSL_ACC_GYRO_I2C_ADDRESS_LOW, LSM6DSL_ACC_GYRO_TAP_CFG, tmp);
	 // TAP_THS_6D (0x59)
	 tmp = 0x8C;
	 SENSOR_IO_Write(LSM6DSL_ACC_GYRO_I2C_ADDRESS_LOW, LSM6DSL_ACC_GYRO_TAP_THS_6D, tmp);
	 // INT_DUR2 (0x5A)
	 tmp = 0x7F;
	 SENSOR_IO_Write(LSM6DSL_ACC_GYRO_I2C_ADDRESS_LOW, LSM6DSL_ACC_GYRO_INT_DUR2, tmp);
	 // WAKE_UP_THS (0x5B)
	 tmp = 0x80;  // SINGLE_DOUBLE_TAP = 1
	 SENSOR_IO_Write(LSM6DSL_ACC_GYRO_I2C_ADDRESS_LOW, LSM6DSL_ACC_GYRO_WAKE_UP_THS, tmp);
	 // MD1_CFG (0x5E)
	 tmp = 0x08;  // INT1_DOUBLE_TAP bit
	 SENSOR_IO_Write(LSM6DSL_ACC_GYRO_I2C_ADDRESS_LOW, LSM6DSL_ACC_GYRO_MD1_CFG, tmp);
}
void MY_TAP_DeInit(void){
	 MY_ACCELERO_Init();
	 // TAP_CFG (0x58)
	 uint8_t tmp = 0x00;
	 SENSOR_IO_Write(LSM6DSL_ACC_GYRO_I2C_ADDRESS_LOW, LSM6DSL_ACC_GYRO_TAP_CFG, tmp);
	 // TAP_THS_6D (0x59)
	 tmp = 0x00;
	 SENSOR_IO_Write(LSM6DSL_ACC_GYRO_I2C_ADDRESS_LOW, LSM6DSL_ACC_GYRO_TAP_THS_6D, tmp);
	 // INT_DUR2 (0x5A)
	 tmp = 0x00;
	 SENSOR_IO_Write(LSM6DSL_ACC_GYRO_I2C_ADDRESS_LOW, LSM6DSL_ACC_GYRO_INT_DUR2, tmp);
	 // WAKE_UP_THS (0x5B)
	 tmp = 0x00;
	 SENSOR_IO_Write(LSM6DSL_ACC_GYRO_I2C_ADDRESS_LOW, LSM6DSL_ACC_GYRO_WAKE_UP_THS, tmp);
	 // MD1_CFG (0x5E)
	 tmp = 0x00;
	 SENSOR_IO_Write(LSM6DSL_ACC_GYRO_I2C_ADDRESS_LOW, LSM6DSL_ACC_GYRO_MD1_CFG, tmp);
}



