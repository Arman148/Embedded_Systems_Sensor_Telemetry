/*
 * lps22hb.h
 *
 *  Created on: Nov 14, 2026
 *      Author: Arman
 */

#ifndef LPS22HB_H
#define LPS22HB_H

#include <stdint.h>

#define LPS22HB_ADDRESS_LOW       0x5C
#define LPS22HB_ADDRESS_HIGH      0x5D

#define LPS22HB_WHO_AM_I          0x0F
#define LPS22HB_DEVICE_ID         0xB1
#define LPS22HB_CTRL_REG1         0x10
#define LPS22HB_CTRL_REG2         0x11
#define LPS22HB_STATUS_REG        0x27
#define LPS22HB_PRESS_OUT_XL      0x28

#define LPS22HB_ODR_1HZ           0x10
#define LPS22HB_BDU_ENABLE        0x02
#define LPS22HB_AUTO_INCREMENT    0x10
#define LPS22HB_PRESSURE_READY    0x01

typedef int (*LPS22HB_ReadCallback)(uint8_t address, uint8_t reg,
                                    uint8_t *data, uint16_t length);

typedef int (*LPS22HB_WriteCallback)(uint8_t address, uint8_t reg,
                                     const uint8_t *data, uint16_t length);

void LPS22HB_RegisterCallbacks(LPS22HB_ReadCallback read_callback,
                               LPS22HB_WriteCallback write_callback);

int LPS22HB_ReadRegisters(uint8_t reg, uint8_t *data, uint16_t length);

int LPS22HB_WriteRegisters(uint8_t reg, const uint8_t *data,
                           uint16_t length);

int LPS22HB_Init(void);
int LPS22HB_ReadPressure(float *pressure);

#endif
