#ifndef CONSOLE_H
#define CONSOLE_H

#include "main.h"

/* Call once before logging. UART and TX DMA are exclusively console-owned. */
void Console_Init(UART_HandleTypeDef *huart);

/* Bounded, non-blocking service; call from the main loop to retry after errors. */
void Console_Process(void);

/* Queue one complete diagnostic message for DMA transmission. */
void Console_Write(const char *message);

/* Format and queue one complete diagnostic message atomically. */
#if defined(__GNUC__)
__attribute__((format(printf, 1, 2)))
#endif
void Console_Printf(const char *format, ...);

/*
 * Wait until all queued console data has physically
 * finished transmitting.
 *
 * Call only from normal/main context with interrupts enabled, not from an ISR.
 * Can wait indefinitely on a hardware fault; do not use in the audio loop.
 */
void Console_Flush(void);

/*
 * Returns non-zero when no console transmission
 * is pending.
 */
int Console_IsIdle(void);

/* Number of whole messages dropped on queue-full or UART/DMA failure.
 * Write truncates at 320 bytes; Printf at 319 bytes; truncation is not counted. */
uint32_t Console_GetDroppedCount(void);

/* Number of UART/DMA start or transmission errors. */
uint32_t Console_GetUartErrorCount(void);

#endif /* CONSOLE_H */
