/*
 * MAX30102.c
 *
 *  Created on: 21-Jul-2026
 *      Author: sunbeam
 */
/*
 * MAX30102.c
 */



#include "MAX30102.h"
#include <string.h>
#include <stdio.h>

/*-------------------------------------------------------
  UART Print
-------------------------------------------------------*/
void UART_Print(char *str)
{
    HAL_UART_Transmit(&huart2,
                      (uint8_t *)str,
                      strlen(str),
                      HAL_MAX_DELAY);
}

/*-------------------------------------------------------
  Write Register
-------------------------------------------------------*/
static void MAX30102_WriteReg(uint8_t reg, uint8_t value)
{
    HAL_I2C_Mem_Write(&hi2c1,
                      MAX30102_ADDR,
                      reg,
                      I2C_MEMADD_SIZE_8BIT,
                      &value,
                      1,
                      100);
}

/*-------------------------------------------------------
  Read Register
-------------------------------------------------------*/
uint8_t MAX30102_ReadReg(uint8_t reg)
{
    uint8_t data = 0;

    HAL_I2C_Mem_Read(&hi2c1,
                     MAX30102_ADDR,
                     reg,
                     I2C_MEMADD_SIZE_8BIT,
                     &data,
                     1,
                     100);

    return data;
}

/*-------------------------------------------------------
  Check Sensor
-------------------------------------------------------*/
uint8_t MAX30102_CheckConnection(void)
{
    uint8_t partID;

    partID = MAX30102_ReadReg(MAX30102_REG_PART_ID);

    if(partID == 0x15)
    {
        return 1;
    }

    return 0;
}

/*-------------------------------------------------------
  Sensor Initialization
-------------------------------------------------------*/
void MAX30102_Init(void)
{
    /* Reset Sensor */
    MAX30102_WriteReg(MAX30102_REG_MODE_CONFIG, 0x40);
    HAL_Delay(100);

    /* Reset FIFO Pointers */
    MAX30102_WriteReg(MAX30102_REG_FIFO_WR_PTR, 0x00);
    MAX30102_WriteReg(MAX30102_REG_OVF_COUNTER, 0x00);
    MAX30102_WriteReg(MAX30102_REG_FIFO_RD_PTR, 0x00);

    /* FIFO Configuration
       Sample Average = 4
       FIFO Rollover = Enable
       Almost Full = 15
    */
    MAX30102_WriteReg(MAX30102_REG_FIFO_CONFIG, 0x4F);

    /* SpO2 Mode (RED + IR) */
    MAX30102_WriteReg(MAX30102_REG_MODE_CONFIG, 0x03);

    /* SpO2 Configuration
       ADC Range = 4096 nA
       Sample Rate = 100 SPS
       Pulse Width = 411 us (18-bit)
    */
    MAX30102_WriteReg(MAX30102_REG_SPO2_CONFIG, 0x27);

    /* RED LED Current (adjust if required) */
    MAX30102_WriteReg(MAX30102_REG_LED1_PA, 0x24);

    /* IR LED Current (adjust if required) */
    MAX30102_WriteReg(MAX30102_REG_LED2_PA, 0x24);

    /* Multi-LED Slot Configuration
       Slot1 = RED LED
       Slot2 = IR LED
    */
    MAX30102_WriteReg(MAX30102_REG_MULTI_LED1, 0x21);

    /* Slot3 = None
       Slot4 = None
    */
    MAX30102_WriteReg(MAX30102_REG_MULTI_LED2, 0x00);

    HAL_Delay(100);
}
/*-------------------------------------------------------
  Read FIFO
-------------------------------------------------------*/
void MAX30102_ReadFIFO(uint32_t *red, uint32_t *ir)
{
    uint8_t fifo[6];

    if(HAL_I2C_Mem_Read(&hi2c1,
                        MAX30102_ADDR,
                        MAX30102_REG_FIFO_DATA,
                        I2C_MEMADD_SIZE_8BIT,
                        fifo,
                        6,
                        100) != HAL_OK)
    {
        *red = 0;
        *ir = 0;
        return;
    }

    *red = ((uint32_t)fifo[0] << 16) |
           ((uint32_t)fifo[1] << 8)  |
            fifo[2];

    *ir = ((uint32_t)fifo[3] << 16) |
          ((uint32_t)fifo[4] << 8)  |
           fifo[5];

    /* 18-bit ADC data */
    *red &= 0x03FFFF;
    *ir  &= 0x03FFFF;
}



/*-------------------------------------------------------
  Heart Rate Calculation
-------------------------------------------------------*/
int32_t MAX30102_EstimateBPM(uint32_t ir)
{
    static float dc = 0;
    static int32_t previous = 0;

    static uint32_t lastBeat = 0;
    static int32_t bpm = 0;


    uint32_t now = HAL_GetTick();


    // Remove DC component
    dc = (0.95f * dc) + (0.05f * ir);


    // Heartbeat AC signal
    int32_t signal = (int32_t)ir - (int32_t)dc;


    // Peak detection
    if(signal > 80 && previous <= 80)
    {
        uint32_t interval = now - lastBeat;


        if(interval > 300 && interval < 1500)
        {
            bpm = 60000 / interval;
        }


        lastBeat = now;
    }


    previous = signal;


    return bpm;
}
/*-------------------------------------------------------
  SpO2 Calculation
-------------------------------------------------------*/
int32_t MAX30102_EstimateSpO2(uint32_t red,uint32_t ir)
{

    static float red_dc = 0;
    static float ir_dc = 0;


    red_dc = (0.95f * red_dc) + (0.05f * red);
    ir_dc  = (0.95f * ir_dc)  + (0.05f * ir);



    float red_ac;
    float ir_ac;


    red_ac = ((float)red - red_dc);

    ir_ac = ((float)ir - ir_dc);



    if(ir_dc == 0 || ir_ac == 0)
        return 0;



    float R;


    R = (red_ac/red_dc) / (ir_ac/ir_dc);



    int32_t spo2;


    spo2 = 110 - (25 * R);



    if(spo2 > 100)
        spo2 = 100;


    if(spo2 < 70)
        spo2 = 70;


    return spo2;
}

