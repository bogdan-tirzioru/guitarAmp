#include "console_board.h"
#include "console.h"
#include <stddef.h>

HAL_StatusTypeDef Console_Board_Init(UART_HandleTypeDef *huart)
{
    /* CubeMX owns DMA initialization, UART linkage and both IRQ handlers. */
    if (huart == NULL || huart->Instance != USART2 ||
        huart->gState != HAL_UART_STATE_READY || huart->hdmatx == NULL ||
        huart->hdmatx->Instance != GPDMA1_Channel0 ||
        huart->hdmatx->State != HAL_DMA_STATE_READY)
        return HAL_ERROR;

    Console_Init(huart);
    return HAL_OK;
}
