/*
 * MAX30102.h
 *
 *  Created on: 22-Jul-2026
 *      Author: sunbeam
 */

#ifndef __MAX30102_H
#define __MAX30102_H

#include "main.h"
#include <stdio.h>
#include <string.h>

/* I2C Handle */
extern I2C_HandleTypeDef hi2c1;
extern UART_HandleTypeDef huart2;

/* MAX30102 I2C Address */
#define MAX30102_ADDR           (0x57 << 1)

/* Registers */
#define MAX30102_REG_INT_STATUS1    0x00
#define MAX30102_REG_INT_STATUS2    0x01
#define MAX30102_REG_INT_ENABLE1    0x02
#define MAX30102_REG_INT_ENABLE2    0x03

#define MAX30102_REG_FIFO_WR_PTR    0x04
#define MAX30102_REG_OVF_COUNTER    0x05
#define MAX30102_REG_FIFO_RD_PTR    0x06
#define MAX30102_REG_FIFO_DATA      0x07
#define MAX30102_REG_FIFO_CONFIG    0x08

#define MAX30102_REG_MODE_CONFIG    0x09
#define MAX30102_REG_SPO2_CONFIG    0x0A

#define MAX30102_REG_LED1_PA        0x0C
#define MAX30102_REG_LED2_PA        0x0D

#define MAX30102_REG_MULTI_LED1     0x11
#define MAX30102_REG_MULTI_LED2     0x12

#define MAX30102_REG_TEMP_INT       0x1F
#define MAX30102_REG_TEMP_FRAC      0x20
#define MAX30102_REG_REV_ID         0xFE
#define MAX30102_REG_PART_ID        0xFF

/* Function  */

void MAX30102_Init(void);

uint8_t MAX30102_CheckConnection(void);

void MAX30102_ReadFIFO(uint32_t *red, uint32_t *ir);

int32_t MAX30102_EstimateBPM(uint32_t ir);

int32_t MAX30102_EstimateSpO2(uint32_t red,
                              uint32_t ir);

void UART_Print(char *str);
uint8_t MAX30102_ReadReg(uint8_t reg);

#endif
