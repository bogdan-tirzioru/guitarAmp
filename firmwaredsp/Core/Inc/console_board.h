#ifndef CONSOLE_BOARD_H
#define CONSOLE_BOARD_H
#include "main.h"

/* Call after CubeMX-generated MX_GPDMA1_Init and MX_USART2_UART_Init.
 * Validates generated TX DMA binding and attaches the message queue.
 * Peripheral configuration and IRQ handlers are owned by CubeMX. */
HAL_StatusTypeDef Console_Board_Init(UART_HandleTypeDef *huart);
#endif
