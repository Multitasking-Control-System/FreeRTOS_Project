/*
 * mpu6050.c
 *
 *  Created on: 21-Jul-2026
 *      Author: sunbeam
 */
#include "mpu6050.h"

HAL_StatusTypeDef MPU6050_IsConnected(I2C_HandleTypeDef *hi2c)
{
    uint8_t whoami = 0;

    if(HAL_I2C_Mem_Read(hi2c,MPU6050_I2C_ADDR,MPU6050_REG_WHO_AM_I,I2C_MEMADD_SIZE_8BIT,&whoami,1,MPU6050_TIMEOUT) != HAL_OK)
    {
        return HAL_ERROR;
    }

    if(whoami == MPU6050_WHO_AM_I_VAL || whoami == 0x70)
    {
        return HAL_OK;
    }

    return HAL_ERROR;
}

HAL_StatusTypeDef MPU6050_Init(I2C_HandleTypeDef *hi2c)
{
    uint8_t data;
    uint8_t check;

    /* Wake up MPU6050 */
    data = 0x00;

    if(HAL_I2C_Mem_Write(hi2c,MPU6050_I2C_ADDR,MPU6050_REG_PWR_MGMT_1,I2C_MEMADD_SIZE_8BIT,&data,1,MPU6050_TIMEOUT)!=HAL_OK)
    {
    	return HAL_ERROR;
    }
    HAL_Delay(100);
    /* Disable Gyroscope */
    data = 0x07;
    if(HAL_I2C_Mem_Write(hi2c,MPU6050_I2C_ADDR,MPU6050_REG_PWR_MGMT_2,I2C_MEMADD_SIZE_8BIT,&data,1,MPU6050_TIMEOUT)!=HAL_OK)
    {
        	return HAL_ERROR;
    }
    if(HAL_I2C_Mem_Read(hi2c,MPU6050_I2C_ADDR,MPU6050_REG_PWR_MGMT_2,I2C_MEMADD_SIZE_8BIT,&check,1,MPU6050_TIMEOUT) != HAL_OK)
	{
		return HAL_ERROR;
	}

	if(check != 0x07)
	{
		return HAL_ERROR;
	}
    /* Set Accelerometer ±2g */
    data = 0x00;
    if(HAL_I2C_Mem_Write(hi2c,MPU6050_I2C_ADDR,MPU6050_REG_ACCEL_CONFIG,I2C_MEMADD_SIZE_8BIT,&data,1,MPU6050_TIMEOUT)!=HAL_OK)
    {
            	return HAL_ERROR;
    }

    return HAL_OK;
}

HAL_StatusTypeDef MPU6050_ReadAccel(I2C_HandleTypeDef *hi2c,MPU6050_Accel_t *accel)
{
	uint8_t buffer[6];

	    /* Read 6 bytes starting from ACCEL_XOUT_H (0x3B) */
	    if (HAL_I2C_Mem_Read(hi2c,MPU6050_I2C_ADDR,MPU6050_REG_ACCEL_XOUT_H,I2C_MEMADD_SIZE_8BIT,buffer,6,MPU6050_TIMEOUT) != HAL_OK)
	    {
	        return HAL_ERROR;
	    }

	    /* Combine High and Low bytes */
	    accel->ax = (int16_t)((buffer[0] << 8) | buffer[1]);
	    accel->ay = (int16_t)((buffer[2] << 8) | buffer[3]);
	    accel->az = (int16_t)((buffer[4] << 8) | buffer[5]);

	    return HAL_OK;
}
