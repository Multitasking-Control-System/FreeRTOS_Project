/*
 * bh1750.c
 *
 *  Created on: 28-Jul-2026
 *      Author: sunbeam
 */
#include "bh1750.h"

uint8_t BH1750_CheckConnection(I2C_HandleTypeDef *hi2c)
{
    uint8_t cmd = BH1750_CMD_POWER_ON;

    HAL_StatusTypeDef status;

    status = HAL_I2C_Master_Transmit(
                    hi2c,
                    BH1750_ADDR,
                    &cmd,
                    1,
                    100);

    return (status == HAL_OK);
}

void BH1750_Init(I2C_HandleTypeDef *hi2c)
{
    uint8_t cmd;

    cmd = BH1750_CMD_POWER_ON;
    HAL_I2C_Master_Transmit(
            hi2c,
            BH1750_ADDR,
            &cmd,
            1,
            100);

    HAL_Delay(10);

    cmd = BH1750_CMD_RESET;
    HAL_I2C_Master_Transmit(
            hi2c,
            BH1750_ADDR,
            &cmd,
            1,
            100);

    HAL_Delay(10);

    cmd = BH1750_CMD_CONT_H_RES_MODE;
    HAL_I2C_Master_Transmit(
            hi2c,
            BH1750_ADDR,
            &cmd,
            1,
            100);

    HAL_Delay(180);
}

HAL_StatusTypeDef BH1750_Read(I2C_HandleTypeDef *hi2c,
                              float *lux)
{
    uint8_t buffer[2];

    HAL_StatusTypeDef status;

    status = HAL_I2C_Master_Receive(
                    hi2c,
                    BH1750_ADDR,
                    buffer,
                    2,
                    100);

    if(status != HAL_OK)
    {
        return status;
    }

    uint16_t raw;

    raw = ((uint16_t)buffer[0] << 8) | buffer[1];

    *lux = raw / 1.2f;

    return HAL_OK;
}

