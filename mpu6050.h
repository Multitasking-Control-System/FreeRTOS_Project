/*
 * mpu6050.h
 *
 *  Created on: 21-Jul-2026
 *      Author: sunbeam
 */

#ifndef INC_MPU6050_H_
#define INC_MPU6050_H_



#include "main.h"

/************ I2C ADDRESS ************/
#define MPU6050_I2C_ADDR        (0x68 << 1)


/*********************REGISTERS ************/
#define MPU6050_REG_PWR_MGMT_1    0x6B
#define MPU6050_REG_PWR_MGMT_2    0x6C
#define MPU6050_REG_WHO_AM_I      0x75

#define MPU6050_REG_ACCEL_CONFIG  0x1C

#define MPU6050_REG_ACCEL_XOUT_H  0x3B

#define MPU6050_WHO_AM_I_VAL      0x68

#define MPU6050_TIMEOUT           100

typedef struct
{
    int16_t ax;
    int16_t ay;
    int16_t az;

}MPU6050_Accel_t;

HAL_StatusTypeDef MPU6050_Init(I2C_HandleTypeDef *hi2c);

HAL_StatusTypeDef MPU6050_IsConnected(I2C_HandleTypeDef *hi2c);

HAL_StatusTypeDef MPU6050_ReadAccel(I2C_HandleTypeDef *hi2c,MPU6050_Accel_t *accel);



#endif /* INC_MPU6050_H_ */
