/*
 * bh1750.h
 *
 *  Created on: 28-Jul-2026
 *      Author: sunbeam
 */

#ifndef INC_BH1750_H_
#define INC_BH1750_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "stm32f4xx_hal.h"

/* BH1750 I2C Address */
#define BH1750_ADDR                    (0x23 << 1)

/* BH1750 Commands */
#define BH1750_CMD_POWER_ON            0x01
#define BH1750_CMD_RESET               0x07
#define BH1750_CMD_CONT_H_RES_MODE     0x10

/* Function Prototypes */
uint8_t BH1750_CheckConnection(I2C_HandleTypeDef *hi2c);

void BH1750_Init(I2C_HandleTypeDef *hi2c);

HAL_StatusTypeDef BH1750_Read(I2C_HandleTypeDef *hi2c,
                              float *lux);

#ifdef __cplusplus
}
#endif



#endif /* INC_BH1750_H_ */
