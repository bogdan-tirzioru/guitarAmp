#include "codec_board.h"
#include "main.h"
#include <stddef.h>

#define CODEC_I2C_TIMEOUT_MS 20U

static int board_write(void *context, uint8_t reg, uint8_t value)
{
    return HAL_I2C_Mem_Write((I2C_HandleTypeDef *)context,
                            (uint16_t)(AIC3104_I2C_ADDRESS << 1), reg,
                            I2C_MEMADD_SIZE_8BIT, &value, 1,
                            CODEC_I2C_TIMEOUT_MS) == HAL_OK ? 0 : -1;
}

static int board_read(void *context, uint8_t reg, uint8_t *value)
{
    return HAL_I2C_Mem_Read((I2C_HandleTypeDef *)context,
                           (uint16_t)(AIC3104_I2C_ADDRESS << 1), reg,
                           I2C_MEMADD_SIZE_8BIT, value, 1,
                           CODEC_I2C_TIMEOUT_MS) == HAL_OK ? 0 : -1;
}

static void board_reset(void *context, bool asserted)
{
    (void)context;
    HAL_GPIO_WritePin(SAI_reset_GPIO_Port, SAI_reset_Pin,
                      asserted ? GPIO_PIN_RESET : GPIO_PIN_SET);
}

static void board_delay(void *context, uint32_t milliseconds)
{
    (void)context;
    HAL_Delay(milliseconds);
}

aic3104_result_t codec_board_attach(aic3104_t *device, I2C_HandleTypeDef *i2c)
{
    if (i2c == NULL) return aic3104_attach(device, NULL);
    const aic3104_bus_t bus = {
        .context = i2c, .write = board_write, .read = board_read,
        .reset = board_reset, .delay_ms = board_delay
    };
    return aic3104_attach(device, &bus);
}
