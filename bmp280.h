/*
 * bmp280.h
 *
 *  Created on: 22-Jul-2026
 *      Author: sunbeam
 */
#ifndef __BMP280_H
#define __BMP280_H

#include "stm32f4xx_hal.h"

/* I2C Handle */
extern I2C_HandleTypeDef hi2c1;

/* BMP280 I2C Address */
#define BMP280_ADDR            (0x76 << 1)

/* Registers */
#define BMP280_REG_CHIP_ID     0xD0
#define BMP280_REG_RESET       0xE0
#define BMP280_REG_STATUS      0xF3
#define BMP280_REG_CTRL_MEAS   0xF4
#define BMP280_REG_CONFIG      0xF5

#define BMP280_REG_PRESS_MSB   0xF7
#define BMP280_REG_PRESS_LSB   0xF8
#define BMP280_REG_PRESS_XLSB  0xF9

#define BMP280_REG_TEMP_MSB    0xFA
#define BMP280_REG_TEMP_LSB    0xFB
#define BMP280_REG_TEMP_XLSB   0xFC

/* Calibration Registers */
#define BMP280_CALIB_START     0x88

/* Chip ID */
#define BMP280_CHIP_ID         0x58

/* Function Prototypes */
uint8_t BMP280_Init(void);
HAL_StatusTypeDef BMP280_ReadRegister(uint8_t reg, uint8_t *data, uint8_t len);
HAL_StatusTypeDef BMP280_WriteRegister(uint8_t reg, uint8_t data);
void BMP280_ReadCalibrationData(void);
void BMP280_ReadRawData(int32_t *temperature,int32_t *pressure);
float BMP280_ReadTemperature(void);
float BMP280_ReadPressure(void);


typedef struct
{
    uint16_t dig_T1;
    int16_t  dig_T2;
    int16_t  dig_T3;

    uint16_t dig_P1;
    int16_t  dig_P2;
    int16_t  dig_P3;
    int16_t  dig_P4;
    int16_t  dig_P5;
    int16_t  dig_P6;
    int16_t  dig_P7;
    int16_t  dig_P8;
    int16_t  dig_P9;

} BMP280_CalibData;

#endif

