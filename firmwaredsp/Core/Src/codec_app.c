#include "codec_app.h"
#include <stddef.h>

volatile codec_app_state_t codec_app_state = CODEC_APP_DISABLED;
volatile aic3104_result_t codec_app_result = AIC3104_NOT_READY;

#if GUITARAMP_CODEC_ENABLE
static aic3104_t codec;
static SAI_HandleTypeDef *clock_tx;
static uint32_t silence[64]; /* Stereo, 24-bit data in 32-bit containers. */
static volatile bool stream_failed;

static HAL_StatusTypeDef transmit_silence(void)
{
    return HAL_SAI_Transmit_IT(clock_tx, (uint8_t *)silence,
                               (uint16_t)(sizeof(silence) / sizeof(silence[0])));
}

static void stop_clock(void)
{
    HAL_NVIC_DisableIRQ(SAI1_IRQn);
    if (clock_tx != NULL) (void)HAL_SAI_Abort(clock_tx);
    HAL_NVIC_ClearPendingIRQ(SAI1_IRQn);
}

void codec_app_start(I2C_HandleTypeDef *i2c, SAI_HandleTypeDef *tx)
{
    codec_app_state = CODEC_APP_STARTING;
    codec_app_result = codec_board_attach(&codec, i2c);
    if (codec_app_result != AIC3104_OK) {
        codec_app_state = CODEC_APP_CODEC_ERROR;
        return;
    }
    aic3104_shutdown(&codec);
    /* Match the project's SAI1 master TX profile before claiming the IRQ. */
    if (tx == NULL || tx->Instance != SAI1_Block_A ||
        tx->Init.AudioMode != SAI_MODEMASTER_TX ||
        tx->Init.DataSize != SAI_DATASIZE_24 ||
        tx->Init.AudioFrequency != SAI_AUDIO_FREQUENCY_48K ||
        tx->State != HAL_SAI_STATE_READY) {
        codec_app_state = CODEC_APP_SAI_ERROR;
        return;
    }
    clock_tx = tx;
    stream_failed = false;
    HAL_NVIC_DisableIRQ(SAI1_IRQn);
    HAL_NVIC_ClearPendingIRQ(SAI1_IRQn);
    HAL_NVIC_SetPriority(SAI1_IRQn, 5, 0);
    if (transmit_silence() != HAL_OK) {
        stream_failed = true;
    } else {
        HAL_NVIC_EnableIRQ(SAI1_IRQn);
        const aic3104_config_t cfg = aic3104_default_config();
        codec_app_result = aic3104_init(&codec, &cfg);
    }
    if (stream_failed || codec_app_result != AIC3104_OK) {
        aic3104_shutdown(&codec);
        stop_clock();
        codec_app_state = stream_failed ? CODEC_APP_SAI_ERROR : CODEC_APP_CODEC_ERROR;
        return;
    }
    codec_app_state = CODEC_APP_READY_MUTED;
}

void codec_app_process(void)
{
    if (stream_failed && codec_app_state == CODEC_APP_READY_MUTED) {
        aic3104_shutdown(&codec);
        stop_clock();
        codec_app_state = CODEC_APP_SAI_ERROR;
    }
}

void codec_app_irq(void)
{
    if (clock_tx != NULL) HAL_SAI_IRQHandler(clock_tx);
}

void HAL_SAI_TxCpltCallback(SAI_HandleTypeDef *hsai)
{
    if (hsai == clock_tx && !stream_failed && transmit_silence() != HAL_OK)
        stream_failed = true;
}

void HAL_SAI_ErrorCallback(SAI_HandleTypeDef *hsai)
{
    if (hsai == clock_tx) {
        stream_failed = true;
        /* No I2C in the ISR. Stop repeat fault interrupts; main cleans up. */
        HAL_NVIC_DisableIRQ(SAI1_IRQn);
    }
}
#endif /* GUITARAMP_CODEC_ENABLE */
