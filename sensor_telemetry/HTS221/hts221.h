/*
 * hts221.h
 *
 *  Created on: Nov 11, 2026
 *      Author: Arman
 */

#ifndef HTS221_H
#define HTS221_H

#include <stdint.h>

#define HTS221_ADDRESS          0x5F
#define HTS221_WHO_AM_I          0x0F
#define HTS221_DEVICE_ID         0xBC
#define HTS221_CTRL_REG1         0x20
#define HTS221_AUTO_INCREMENT    0x80
#define HTS221_POWER_ON          0x80
#define HTS221_BDU_ENABLE        0x04
#define HTS221_ODR_1HZ           0x01
#define HTS221_CALIB_START       0x30

#define HTS221_STATUS_REG        0x27
#define HTS221_HUMIDITY_OUT_L    0x28
#define HTS221_TEMP_OUT_L        0x2A

#define HTS221_TEMP_READY        0x01
#define HTS221_HUMIDITY_READY    0x02

typedef int (*HTS221_ReadCallback)(uint8_t address, uint8_t reg,
                                   uint8_t *data, uint16_t length);

typedef int (*HTS221_WriteCallback)(uint8_t address, uint8_t reg,
                                    const uint8_t *data, uint16_t length);

void HTS221_RegisterCallbacks(HTS221_ReadCallback read_callback,
                              HTS221_WriteCallback write_callback);

int HTS221_Init(void);


int HTS221_ReadTemperature(float *temperature);
int HTS221_ReadHumidity(float *humidity);

int HTS221_ReadRegisters(uint8_t reg, uint8_t *data, uint16_t length);

int HTS221_WriteRegisters(uint8_t reg, const uint8_t *data,
                          uint16_t length);

#endif
