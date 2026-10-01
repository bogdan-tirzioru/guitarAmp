#ifndef CODEC_APP_H
#define CODEC_APP_H
#include "codec_board.h"
#include "app_config.h"

typedef enum {
    CODEC_APP_DISABLED = 0,
    CODEC_APP_STARTING,
    CODEC_APP_READY_MUTED,
    CODEC_APP_CODEC_ERROR,
    CODEC_APP_SAI_ERROR
} codec_app_state_t;

/* Debugger-visible result. The application continues its LED heartbeat on failure. */
extern volatile codec_app_state_t codec_app_state;
extern volatile aic3104_result_t codec_app_result;

#if GUITARAMP_CODEC_ENABLE
/* Called after MX_GPIO/I2C1/SAI1_Init. Owns SAI1 A and its TX/error callbacks.
 * Starts a silent clock stream, initializes the codec, leaves DAC muted.
 * This is a control bring-up path, not the future DSP audio engine. */
void codec_app_start(I2C_HandleTypeDef *i2c, SAI_HandleTypeDef *tx);
void codec_app_process(void);
void codec_app_irq(void);
#endif
#endif
