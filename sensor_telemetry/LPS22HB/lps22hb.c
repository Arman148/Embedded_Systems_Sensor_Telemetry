#include "lps22hb.h"

static LPS22HB_ReadCallback sensor_read = 0;
static LPS22HB_WriteCallback sensor_write = 0;

static uint8_t sensor_address = LPS22HB_ADDRESS_HIGH;
static int initialized = 0;

void LPS22HB_RegisterCallbacks(LPS22HB_ReadCallback read_callback,
                               LPS22HB_WriteCallback write_callback)
{
    sensor_read = read_callback;
    sensor_write = write_callback;
    initialized = 0;
}

int LPS22HB_ReadRegisters(uint8_t reg, uint8_t *data, uint16_t length)
{
    if (sensor_read == 0 || data == 0 ||
        length == 0 || length > 255)
    {
        return -1;
    }

    return sensor_read(sensor_address, reg, data, length);
}

int LPS22HB_WriteRegisters(uint8_t reg, const uint8_t *data,
                           uint16_t length)
{
    if (sensor_write == 0 || data == 0 ||
        length == 0 || length > 254)
    {
        return -1;
    }

    return sensor_write(sensor_address, reg, data, length);
}

int LPS22HB_Init(void)
{
    uint8_t device_id;
    uint8_t config;
    uint8_t readback;

    initialized = 0;

    if (sensor_read == 0 || sensor_write == 0)
    {
        return -1;
    }

    sensor_address = LPS22HB_ADDRESS_HIGH;

    if (LPS22HB_ReadRegisters(LPS22HB_WHO_AM_I, &device_id, 1) != 0 ||
        device_id != LPS22HB_DEVICE_ID)
    {
        sensor_address = LPS22HB_ADDRESS_LOW;

        if (LPS22HB_ReadRegisters(LPS22HB_WHO_AM_I, &device_id, 1) != 0 ||
            device_id != LPS22HB_DEVICE_ID)
        {
            return -1;
        }
    }

    config = LPS22HB_AUTO_INCREMENT;

    if (LPS22HB_WriteRegisters(LPS22HB_CTRL_REG2, &config, 1) != 0)
    {
        return -1;
    }

    if (LPS22HB_ReadRegisters(LPS22HB_CTRL_REG2, &readback, 1) != 0)
    {
        return -1;
    }

    if (readback != config)
    {
        return -1;
    }

    config = LPS22HB_ODR_1HZ | LPS22HB_BDU_ENABLE;

    if (LPS22HB_WriteRegisters(LPS22HB_CTRL_REG1, &config, 1) != 0)
    {
        return -1;
    }

    if (LPS22HB_ReadRegisters(LPS22HB_CTRL_REG1, &readback, 1) != 0)
    {
        return -1;
    }

    if (readback != config)
    {
        return -1;
    }

    initialized = 1;

    return 0;
}

int LPS22HB_ReadPressure(float *pressure)
{
    uint8_t status;
    uint8_t data[3];
    int32_t raw;

    if (pressure == 0 || initialized == 0)
    {
        return -1;
    }

    if (LPS22HB_ReadRegisters(LPS22HB_STATUS_REG, &status, 1) != 0)
    {
        return -1;
    }

    if ((status & LPS22HB_PRESSURE_READY) == 0)
    {
        return 1;
    }

    if (LPS22HB_ReadRegisters(LPS22HB_PRESS_OUT_XL, data, 3) != 0)
    {
        return -1;
    }

    raw = (int32_t)data[0] |
          ((int32_t)data[1] << 8) |
          ((int32_t)data[2] << 16);

    if ((raw & 0x00800000) != 0)
    {
        raw -= 0x01000000;
    }

    *pressure = raw / 4096.0f;

    return 0;
}
