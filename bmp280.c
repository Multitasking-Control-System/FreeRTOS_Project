/*
 * bmp280.c
 *
 *  Created on: 22-Jul-2026
 *      Author: sunbeam
 */


/* USER CODE BEGIN Header */
/**
 ******************************************************************************
 * @file    bmp280.c
 * @brief   BMP280 Driver Source File
 ******************************************************************************
 */

/* Includes ------------------------------------------------------------------*/
#include "bmp280.h"

BMP280_CalibData bmp280_calib;

int32_t t_fine;

/* Private Functions ---------------------------------------------------------*/

/**
 * @brief  Read data from BMP280 register
 * @param  reg  : Register Address
 * @param  data : Pointer to receive data
 * @param  len  : Number of bytes to read
 * @retval HAL Status
 */
HAL_StatusTypeDef BMP280_ReadRegister(uint8_t reg, uint8_t *data, uint8_t len)
{
    return HAL_I2C_Mem_Read(&hi2c1,
                            BMP280_ADDR,
                            reg,
                            I2C_MEMADD_SIZE_8BIT,
                            data,
                            len,
                            HAL_MAX_DELAY);
}

/**
 * @brief  Write data to BMP280 register
 * @param  reg  : Register Address
 * @param  data : Data to write
 * @retval HAL Status
 */
HAL_StatusTypeDef BMP280_WriteRegister(uint8_t reg, uint8_t data)
{
    return HAL_I2C_Mem_Write(&hi2c1,
                             BMP280_ADDR,
                             reg,
                             I2C_MEMADD_SIZE_8BIT,
                             &data,
                             1,
                             HAL_MAX_DELAY);
}

/**
 * @brief  Initialize BMP280 Sensor
 * @retval 1 = Success
 *         0 = Failed
 */
uint8_t BMP280_Init(void)
{
    uint8_t chipID;
    uint8_t ctrl_meas = 0x27;   // Temp x1, Pressure x1, Normal Mode
    uint8_t config = 0xA0;      // Standby + Filter

    /* Read Chip ID */
    if (BMP280_ReadRegister(BMP280_REG_CHIP_ID, &chipID, 1) != HAL_OK)
    {
        return 0;
    }

    /* Verify Chip ID */
    if (chipID != BMP280_CHIP_ID)
    {
        return 0;
    }
    BMP280_ReadCalibrationData();

    /* Configure CTRL_MEAS Register */
    if (BMP280_WriteRegister(BMP280_REG_CTRL_MEAS, ctrl_meas) != HAL_OK)
    {
        return 0;
    }

    /* Configure CONFIG Register */
    if (BMP280_WriteRegister(BMP280_REG_CONFIG, config) != HAL_OK)
    {
        return 0;
    }

    return 1;
}

void BMP280_ReadRawData(int32_t *temperature,
                            int32_t *pressure)
    {
        uint8_t data[6];

        /* Read 6 bytes starting from Pressure MSB Register */
        BMP280_ReadRegister(BMP280_REG_PRESS_MSB, data, 6);

        /* Pressure Raw Data (20-bit) */
        *pressure = (int32_t)((((uint32_t)data[0]) << 12) |
                              (((uint32_t)data[1]) << 4)  |
                              (((uint32_t)data[2]) >> 4));

        /* Temperature Raw Data (20-bit) */
        *temperature = (int32_t)((((uint32_t)data[3]) << 12) |
                                 (((uint32_t)data[4]) << 4)  |
                                 (((uint32_t)data[5]) >> 4));
    }

void BMP280_ReadCalibrationData(void)
{
    uint8_t calib[24];

    BMP280_ReadRegister(BMP280_CALIB_START, calib, 24);

    bmp280_calib.dig_T1 = (uint16_t)(calib[1] << 8 | calib[0]);
    bmp280_calib.dig_T2 = (int16_t)(calib[3] << 8 | calib[2]);
    bmp280_calib.dig_T3 = (int16_t)(calib[5] << 8 | calib[4]);

    bmp280_calib.dig_P1 = (uint16_t)(calib[7] << 8 | calib[6]);
    bmp280_calib.dig_P2 = (int16_t)(calib[9] << 8 | calib[8]);
    bmp280_calib.dig_P3 = (int16_t)(calib[11] << 8 | calib[10]);
    bmp280_calib.dig_P4 = (int16_t)(calib[13] << 8 | calib[12]);
    bmp280_calib.dig_P5 = (int16_t)(calib[15] << 8 | calib[14]);
    bmp280_calib.dig_P6 = (int16_t)(calib[17] << 8 | calib[16]);
    bmp280_calib.dig_P7 = (int16_t)(calib[19] << 8 | calib[18]);
    bmp280_calib.dig_P8 = (int16_t)(calib[21] << 8 | calib[20]);
    bmp280_calib.dig_P9 = (int16_t)(calib[23] << 8 | calib[22]);
}

float BMP280_ReadTemperature(void)
{
    int32_t adc_T;
    int32_t adc_P;
    int32_t var1, var2;
    float temperature;

    /* Read Raw Data */
    BMP280_ReadRawData(&adc_T, &adc_P);

    /* Temperature Compensation Formula */
    var1 = ((((adc_T >> 3) - ((int32_t)bmp280_calib.dig_T1 << 1))) *
            ((int32_t)bmp280_calib.dig_T2)) >> 11;

    var2 = (((((adc_T >> 4) - ((int32_t)bmp280_calib.dig_T1)) *
              ((adc_T >> 4) - ((int32_t)bmp280_calib.dig_T1))) >> 12) *
            ((int32_t)bmp280_calib.dig_T3)) >> 14;

    t_fine = var1 + var2;

    temperature = (t_fine * 5 + 128) >> 8;

    return temperature / 100.0f;
}

float BMP280_ReadPressure(void)
{
    int32_t adc_T, adc_P;
    int64_t var1, var2, p;

    /* Read Raw Temperature and Pressure */
    BMP280_ReadRawData(&adc_T, &adc_P);

    /* Update t_fine by calculating temperature */
    BMP280_ReadTemperature();

    var1 = ((int64_t)t_fine) - 128000;
    var2 = var1 * var1 * (int64_t)bmp280_calib.dig_P6;
    var2 = var2 + ((var1 * (int64_t)bmp280_calib.dig_P5) << 17);
    var2 = var2 + (((int64_t)bmp280_calib.dig_P4) << 35);

    var1 = ((var1 * var1 * (int64_t)bmp280_calib.dig_P3) >> 8) +
           ((var1 * (int64_t)bmp280_calib.dig_P2) << 12);

    var1 = ((((int64_t)1) << 47) + var1) *
           ((int64_t)bmp280_calib.dig_P1) >> 33;

    if (var1 == 0)
    {
        return 0;
    }

    p = 1048576 - adc_P;

    p = (((p << 31) - var2) * 3125) / var1;

    var1 = (((int64_t)bmp280_calib.dig_P9) *
           (p >> 13) * (p >> 13)) >> 25;

    var2 = (((int64_t)bmp280_calib.dig_P8) * p) >> 19;

    p = ((p + var1 + var2) >> 8) +
        (((int64_t)bmp280_calib.dig_P7) << 4);

    return (float)p / 256.0f;
}
