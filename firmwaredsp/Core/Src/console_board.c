#include "console_board.h"
#include "console.h"
#include <stddef.h>

static DMA_HandleTypeDef console_dma;
static UART_HandleTypeDef *board_uart;

HAL_StatusTypeDef Console_Board_Init(UART_HandleTypeDef *huart)
{
    if (huart == NULL || huart->Instance != USART2 ||
        huart->gState != HAL_UART_STATE_READY || huart->hdmatx != NULL)
        return HAL_ERROR;

    __HAL_RCC_GPDMA1_CLK_ENABLE();
    console_dma.Instance = GPDMA1_Channel0;
    console_dma.Init.Request = GPDMA1_REQUEST_USART2_TX;
    console_dma.Init.BlkHWRequest = DMA_BREQ_SINGLE_BURST;
    console_dma.Init.Direction = DMA_MEMORY_TO_PERIPH;
    console_dma.Init.SrcInc = DMA_SINC_INCREMENTED;
    console_dma.Init.DestInc = DMA_DINC_FIXED;
    console_dma.Init.SrcDataWidth = DMA_SRC_DATAWIDTH_BYTE;
    console_dma.Init.DestDataWidth = DMA_DEST_DATAWIDTH_BYTE;
    console_dma.Init.Priority = DMA_LOW_PRIORITY_LOW_WEIGHT;
    console_dma.Init.SrcBurstLength = 1;
    console_dma.Init.DestBurstLength = 1;
    console_dma.Init.TransferAllocatedPort = DMA_SRC_ALLOCATED_PORT0 | DMA_DEST_ALLOCATED_PORT0;
    console_dma.Init.TransferEventMode = DMA_TCEM_BLOCK_TRANSFER;
    console_dma.Init.Mode = DMA_NORMAL;
    if (HAL_DMA_Init(&console_dma) != HAL_OK) return HAL_ERROR;
    if (HAL_DMA_ConfigChannelAttributes(&console_dma, DMA_CHANNEL_NPRIV) != HAL_OK)
    {
        (void)HAL_DMA_DeInit(&console_dma);
        return HAL_ERROR;
    }
    __HAL_LINKDMA(huart, hdmatx, console_dma);
    board_uart = huart;
    Console_Init(huart);

    /* DMA completion enables UART TC; both IRQs are needed to finish a message. */
    HAL_NVIC_ClearPendingIRQ(GPDMA1_Channel0_IRQn);
    HAL_NVIC_ClearPendingIRQ(USART2_IRQn);
    HAL_NVIC_SetPriority(GPDMA1_Channel0_IRQn, 15, 0);
    HAL_NVIC_SetPriority(USART2_IRQn, 15, 0);
    HAL_NVIC_EnableIRQ(GPDMA1_Channel0_IRQn);
    HAL_NVIC_EnableIRQ(USART2_IRQn);
    return HAL_OK;
}

void Console_Board_DMA_IRQHandler(void)
{
    if (board_uart != NULL) HAL_DMA_IRQHandler(&console_dma);
}

void Console_Board_UART_IRQHandler(void)
{
    if (board_uart != NULL) HAL_UART_IRQHandler(board_uart);
}
