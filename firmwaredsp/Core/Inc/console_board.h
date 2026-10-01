#ifndef CONSOLE_BOARD_H
#define CONSOLE_BOARD_H
#include "main.h"

/* Call after MX_USART2_UART_Init. Claims GPDMA1 channel 0 and USART2 IRQ.
 * PA2 TX / PA3 RX, current CubeMX UART setting 115200 8N1.
 * No RX logging/command parser. USART3 remains free for BM83. */
HAL_StatusTypeDef Console_Board_Init(UART_HandleTypeDef *huart);
void Console_Board_DMA_IRQHandler(void);
void Console_Board_UART_IRQHandler(void);
#endif
