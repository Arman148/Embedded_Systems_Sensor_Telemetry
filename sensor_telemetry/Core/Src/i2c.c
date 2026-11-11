/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    i2c.c
  * @brief   This file provides code for the configuration
  *          of the I2C instances.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2026 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "i2c.h"

/* USER CODE BEGIN 0 */

static volatile uint32_t i2c_ticks = 0;

/* USER CODE END 0 */

/* I2C1 init function */
void MX_I2C1_Init(void)
{

  /* USER CODE BEGIN I2C1_Init 0 */

  /* USER CODE END I2C1_Init 0 */

  LL_I2C_InitTypeDef I2C_InitStruct = {0};

  LL_GPIO_InitTypeDef GPIO_InitStruct = {0};

  LL_AHB1_GRP1_EnableClock(LL_AHB1_GRP1_PERIPH_GPIOB);
  /**I2C1 GPIO Configuration
  PB6   ------> I2C1_SCL
  PB7   ------> I2C1_SDA
  */
  GPIO_InitStruct.Pin = LL_GPIO_PIN_6|LL_GPIO_PIN_7;
  GPIO_InitStruct.Mode = LL_GPIO_MODE_ALTERNATE;
  GPIO_InitStruct.Speed = LL_GPIO_SPEED_FREQ_HIGH;
  GPIO_InitStruct.OutputType = LL_GPIO_OUTPUT_OPENDRAIN;
  GPIO_InitStruct.Pull = LL_GPIO_PULL_NO;
  GPIO_InitStruct.Alternate = LL_GPIO_AF_4;
  LL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /* Peripheral clock enable */
  LL_APB1_GRP1_EnableClock(LL_APB1_GRP1_PERIPH_I2C1);

  /* I2C1 interrupt Init */
  NVIC_SetPriority(I2C1_EV_IRQn, NVIC_EncodePriority(NVIC_GetPriorityGrouping(),0, 0));
  NVIC_EnableIRQ(I2C1_EV_IRQn);
  NVIC_SetPriority(I2C1_ER_IRQn, NVIC_EncodePriority(NVIC_GetPriorityGrouping(),0, 0));
  NVIC_EnableIRQ(I2C1_ER_IRQn);

  /* USER CODE BEGIN I2C1_Init 1 */

  /* USER CODE END I2C1_Init 1 */

  /** I2C Initialization
  */
  LL_I2C_EnableAutoEndMode(I2C1);
  LL_I2C_DisableOwnAddress2(I2C1);
  LL_I2C_DisableGeneralCall(I2C1);
  LL_I2C_EnableClockStretching(I2C1);
  I2C_InitStruct.PeripheralMode = LL_I2C_MODE_I2C;
  I2C_InitStruct.Timing = 0x00201D2B;
  I2C_InitStruct.AnalogFilter = LL_I2C_ANALOGFILTER_ENABLE;
  I2C_InitStruct.DigitalFilter = 0;
  I2C_InitStruct.OwnAddress1 = 0;
  I2C_InitStruct.TypeAcknowledge = LL_I2C_ACK;
  I2C_InitStruct.OwnAddrSize = LL_I2C_OWNADDRESS1_7BIT;
  LL_I2C_Init(I2C1, &I2C_InitStruct);
  LL_I2C_SetOwnAddress2(I2C1, 0, LL_I2C_OWNADDRESS2_NOMASK);
  /* USER CODE BEGIN I2C1_Init 2 */

  /* USER CODE END I2C1_Init 2 */

}

/* USER CODE BEGIN 1 */

void I2C_Tick(void)
{
    i2c_ticks++;
}

uint32_t I2C_GetTick(void)
{
    return i2c_ticks;
}

static int I2C_WaitFlag(uint32_t flag)
{
    uint32_t start = I2C_GetTick();

    while (1)
    {
        if (LL_I2C_IsActiveFlag_NACK(I2C1) ||
            LL_I2C_IsActiveFlag_BERR(I2C1) ||
            LL_I2C_IsActiveFlag_ARLO(I2C1))
        {
            return -1;
        }

        if ((I2C1->ISR & flag) != 0)
        {
            return 0;
        }

        if ((I2C_GetTick() - start) >= 100)
        {
            return -1;
        }
    }
}

static void I2C_ResetTransfer(void)
{
    LL_I2C_Disable(I2C1);

    LL_I2C_ClearFlag_NACK(I2C1);
    LL_I2C_ClearFlag_STOP(I2C1);
    LL_I2C_ClearFlag_BERR(I2C1);
    LL_I2C_ClearFlag_ARLO(I2C1);

    LL_I2C_Enable(I2C1);
}

int I2C_WriteRegisters(uint8_t address, uint8_t reg,
                       const uint8_t *data, uint16_t length)
{
    uint32_t start;

    if (address > 0x7F || data == 0 ||
        length == 0 || length > 254)
    {
        return -1;
    }

    start = I2C_GetTick();

    while (LL_I2C_IsActiveFlag_BUSY(I2C1))
    {
        if ((I2C_GetTick() - start) >= 100)
        {
            return -1;
        }
    }

    LL_I2C_ClearFlag_NACK(I2C1);
    LL_I2C_ClearFlag_STOP(I2C1);
    LL_I2C_ClearFlag_BERR(I2C1);
    LL_I2C_ClearFlag_ARLO(I2C1);

    LL_I2C_HandleTransfer(I2C1,
                          (uint32_t)address << 1,
                          LL_I2C_ADDRSLAVE_7BIT,
                          length + 1,
                          LL_I2C_MODE_AUTOEND,
                          LL_I2C_GENERATE_START_WRITE);

    if (I2C_WaitFlag(I2C_ISR_TXIS) != 0)
    {
        I2C_ResetTransfer();
        return -1;
    }

    LL_I2C_TransmitData8(I2C1, reg);

    for (uint16_t i = 0; i < length; i++)
    {
        if (I2C_WaitFlag(I2C_ISR_TXIS) != 0)
        {
            I2C_ResetTransfer();
            return -1;
        }

        LL_I2C_TransmitData8(I2C1, data[i]);
    }

    if (I2C_WaitFlag(I2C_ISR_STOPF) != 0)
    {
        I2C_ResetTransfer();
        return -1;
    }

    LL_I2C_ClearFlag_STOP(I2C1);

    return 0;
}


int I2C_ReadRegisters(uint8_t address, uint8_t reg,
                      uint8_t *data, uint16_t length)
{
    uint32_t start;

    if (address > 0x7F || data == 0 ||
        length == 0 || length > 255)
    {
        return -1;
    }

    start = I2C_GetTick();

    while (LL_I2C_IsActiveFlag_BUSY(I2C1))
    {
        if ((I2C_GetTick() - start) >= 100)
        {
            return -1;
        }
    }

    LL_I2C_ClearFlag_NACK(I2C1);
    LL_I2C_ClearFlag_STOP(I2C1);
    LL_I2C_ClearFlag_BERR(I2C1);
    LL_I2C_ClearFlag_ARLO(I2C1);

    LL_I2C_HandleTransfer(I2C1,
                          (uint32_t)address << 1,
                          LL_I2C_ADDRSLAVE_7BIT,
                          1,
                          LL_I2C_MODE_SOFTEND,
                          LL_I2C_GENERATE_START_WRITE);

    if (I2C_WaitFlag(I2C_ISR_TXIS) != 0)
    {
        I2C_ResetTransfer();
        return -1;
    }

    LL_I2C_TransmitData8(I2C1, reg);

    if (I2C_WaitFlag(I2C_ISR_TC) != 0)
    {
        I2C_ResetTransfer();
        return -1;
    }

    LL_I2C_HandleTransfer(I2C1,
                          (uint32_t)address << 1,
                          LL_I2C_ADDRSLAVE_7BIT,
                          length,
                          LL_I2C_MODE_AUTOEND,
                          LL_I2C_GENERATE_START_READ);

    for (uint16_t i = 0; i < length; i++)
    {
        if (I2C_WaitFlag(I2C_ISR_RXNE) != 0)
        {
            I2C_ResetTransfer();
            return -1;
        }

        data[i] = LL_I2C_ReceiveData8(I2C1);
    }

    if (I2C_WaitFlag(I2C_ISR_STOPF) != 0)
    {
        I2C_ResetTransfer();
        return -1;
    }

    LL_I2C_ClearFlag_STOP(I2C1);

    return 0;
}



/* USER CODE END 1 */

