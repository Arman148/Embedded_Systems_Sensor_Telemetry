/*
 * hts221.c
 *
 *  Created on: Nov 11, 2026
 *      Author: Arman
 */


#include "hts221.h"

static HTS221_ReadCallback sensor_read = 0;
static HTS221_WriteCallback sensor_write = 0;
static float h0_rh;
static float h1_rh;
static float t0_deg;
static float t1_deg;

static int16_t h0_out;
static int16_t h1_out;
static int16_t t0_out;
static int16_t t1_out;

static int calibration_ready = 0;

void HTS221_RegisterCallbacks(HTS221_ReadCallback read_callback,
                              HTS221_WriteCallback write_callback)
{
    sensor_read = read_callback;
    sensor_write = write_callback;
}

int HTS221_ReadRegisters(uint8_t reg, uint8_t *data, uint16_t length)
{
    if (sensor_read == 0 || data == 0 ||
        length == 0 || length > 255)
    {
        return -1;
    }

    if (length > 1)
    {
        reg |= HTS221_AUTO_INCREMENT;
    }

    return sensor_read(HTS221_ADDRESS, reg, data, length);
}

int HTS221_WriteRegisters(uint8_t reg, const uint8_t *data,
                          uint16_t length)
{
    if (sensor_write == 0 || data == 0 ||
        length == 0 || length > 254)
    {
        return -1;
    }

    if (length > 1)
    {
        reg |= HTS221_AUTO_INCREMENT;
    }

    return sensor_write(HTS221_ADDRESS, reg, data, length);
}

static int HTS221_LoadCalibration(void)
{
    uint8_t data[16];
    uint16_t t0_x8;
    uint16_t t1_x8;

    calibration_ready = 0;

    if (HTS221_ReadRegisters(HTS221_CALIB_START, data, 16) != 0)
    {
        return -1;
    }

    h0_rh = data[0] / 2.0f;
    h1_rh = data[1] / 2.0f;

    t0_x8 = (uint16_t)data[2] |
            ((uint16_t)(data[5] & 0x03) << 8);

    t1_x8 = (uint16_t)data[3] |
            ((uint16_t)(data[5] & 0x0C) << 6);

    t0_deg = t0_x8 / 8.0f;
    t1_deg = t1_x8 / 8.0f;

    h0_out = (int16_t)((uint16_t)data[6] |
                      ((uint16_t)data[7] << 8));

    h1_out = (int16_t)((uint16_t)data[10] |
                      ((uint16_t)data[11] << 8));

    t0_out = (int16_t)((uint16_t)data[12] |
                      ((uint16_t)data[13] << 8));

    t1_out = (int16_t)((uint16_t)data[14] |
                      ((uint16_t)data[15] << 8));

    if (h0_out == h1_out || t0_out == t1_out)
    {
        return -1;
    }

    calibration_ready = 1;

    return 0;
}

int HTS221_Init(void)
{
    uint8_t device_id;
    uint8_t config;
    uint8_t readback;

    calibration_ready = 0;

    if (HTS221_ReadRegisters(HTS221_WHO_AM_I, &device_id, 1) != 0)
    {
        return -1;
    }

    if (device_id != HTS221_DEVICE_ID)
    {
        return -1;
    }

    config = HTS221_POWER_ON |
             HTS221_BDU_ENABLE |
             HTS221_ODR_1HZ;

    if (HTS221_WriteRegisters(HTS221_CTRL_REG1, &config, 1) != 0)
    {
        return -1;
    }

    if (HTS221_ReadRegisters(HTS221_CTRL_REG1, &readback, 1) != 0)
    {
        return -1;
    }

    if (readback != config)
    {
        return -1;
    }

    if (HTS221_LoadCalibration() != 0)
    {
        return -1;
    }

    return 0;
}

int HTS221_ReadTemperature(float *temperature)
{
    uint8_t status;
    uint8_t data[2];
    int16_t raw;

    if (temperature == 0 || calibration_ready == 0)
    {
        return -1;
    }

    if (HTS221_ReadRegisters(HTS221_STATUS_REG, &status, 1) != 0)
    {
        return -1;
    }

    if ((status & HTS221_TEMP_READY) == 0)
    {
        return 1;
    }

    if (HTS221_ReadRegisters(HTS221_TEMP_OUT_L, data, 2) != 0)
    {
        return -1;
    }

    raw = (int16_t)((uint16_t)data[0] |
                    ((uint16_t)data[1] << 8));

    *temperature = t0_deg +
                   ((float)raw - t0_out) *
                   (t1_deg - t0_deg) /
                   ((float)t1_out - t0_out);

    return 0;
}


int HTS221_ReadHumidity(float *humidity)
{
    uint8_t status;
    uint8_t data[2];
    int16_t raw;
    float value;

    if (humidity == 0 || calibration_ready == 0)
    {
        return -1;
    }

    if (HTS221_ReadRegisters(HTS221_STATUS_REG, &status, 1) != 0)
    {
        return -1;
    }

    if ((status & HTS221_HUMIDITY_READY) == 0)
    {
        return 1;
    }

    if (HTS221_ReadRegisters(HTS221_HUMIDITY_OUT_L, data, 2) != 0)
    {
        return -1;
    }

    raw = (int16_t)((uint16_t)data[0] |
                    ((uint16_t)data[1] << 8));

    value = h0_rh +
            ((float)raw - h0_out) *
            (h1_rh - h0_rh) /
            ((float)h1_out - h0_out);

    if (value < 0.0f)
    {
        value = 0.0f;
    }
    else if (value > 100.0f)
    {
        value = 100.0f;
    }

    *humidity = value;

    return 0;
}


