#ifndef CODEC_BOARD_H
#define CODEC_BOARD_H
#include "stm32h5xx_hal.h"
#include "tlv320aic3104.h"
#ifdef __cplusplus
extern "C" {
#endif
/* Binds current board I2C1 and PE0 /RESET; no hardware operations.
 * Handle must stay valid. HAL init/GPIO init precede codec init.
 * Call aic3104_init only once SAI is generating MCLK/BCLK/WCLK. */
aic3104_result_t codec_board_attach(aic3104_t *device, I2C_HandleTypeDef *i2c);
#ifdef __cplusplus
}
#endif
#endif
