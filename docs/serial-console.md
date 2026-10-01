# USART2 DMA console

Ported from BA2 (`bogdan-tirzioru/BusAnalizerII`, commit
`90d8e84094533c9ec175bb13e15b4c279520b821`,
`firmBoard735/Core/Inc/console.h` and `Core/Src/console.c`). The public
`Console_Write`, `Console_Printf`, idle/flush and counter APIs are retained.
The port targets the guitarAmp STM32H5E5 firmware, not BA2's STM32H7 DMA layout.

| Resource | guitarAmp assignment |
| --- | --- |
| Serial console | USART2, 115200 baud, 8 data bits, no parity, 1 stop bit |
| TX / RX | PA2 / PA3, as already allocated in CubeMX |
| TX DMA | GPDMA1 channel 0, normal mode, USART2_TX request, byte transfers |
| IRQs | GPDMA1_Channel0 and USART2, priority 15 |
| Queue | 64 messages × 320 bytes (20,480 bytes of payload storage) |
| Buffer storage | Internal SRAM `.bss`, 32-byte aligned; no H7 `.console_ram` section |
| USART3 | Remains reserved for BM83; callbacks ignore other UART handles |

CubeMX owns the peripheral configuration in `firmwaredsp.ioc`: GPDMA1 channel 0
with the USART2_TX request, byte widths, source increment, destination fixed,
single-byte bursts and low priority. It generates `MX_GPDMA1_Init`, the DMA
initialization and `__HAL_LINKDMA` in USART2's MSP initialization, and both DMA
and USART2 interrupt handlers. The DMA controller clock is initialized before
USART2. Both interrupts use priority 15.

`Console_Board_Init(&huart2)` only validates the generated UART/TX DMA binding and
attaches the message queue. It does not configure DMA or NVIC and owns no IRQ
handlers. USART2 completion is needed as well as DMA completion: the queue slot
is released only after the final byte finishes transmitting. `main()` calls
`Console_Process()` to service pending messages after a failed start. There is
no per-message wait in startup or main-loop logging.

Reserve GPDMA1 channel 0 for the console when assigning future audio/storage DMA
channels in CubeMX. Application calls stay in USER CODE blocks; regenerate from
the `.ioc` for future peripheral changes. The generated DMA initialization and
handlers are committed along with their configuration.

## Usage

```c
#include "console.h"

Console_Write("audio: starting\r\n");
Console_Printf("codec result: %u\r\n", (unsigned)codec_app_result);
```

Messages are copied into the queue; callers may reuse their buffers after return.
Use `\r\n` for terminal line endings. The logger emits the startup banner, CPU
clock and codec enabled/disabled status. With the codec enabled it also reports
startup results and later state changes. No periodic LED log is emitted.

`Console_GetDroppedCount()` reports queue-full and TX-start/transfer failures;
`Console_GetUartErrorCount()` reports UART/DMA errors. `Console_Write` truncates
at 320 bytes and `Console_Printf` at 319 bytes; truncation is not counted as a
whole-message drop. Queue admission and publication use short critical sections,
so ISR-origin messages do not overwrite partially filled slots. Formatting uses
`vsnprintf` and a 320-byte stack buffer; keep formatted diagnostics out of the
future audio ISR and avoid floating-point formatting with the default nano libc.

`Console_Flush()` retains BA2's blocking API for deliberate main-context use. It
requires interrupts and a working peripheral, and can wait indefinitely on a
hardware fault. The firmware does not call it in startup or the main/audio loop.
Ordinary `printf` is not retargeted; use `Console_Printf` for queued logging.

Compared with BA2, failed starts no longer recurse through the queue. The
main-loop service retries remaining messages one at a time. RX-only UART errors
do not release a still-active TX DMA buffer. The console exclusively owns USART2
TX DMA; do not also transmit on it directly through HAL or bind an RX DMA channel
without extending the error/callback routing. Its HAL TX/error callbacks are
shared with USART3, whose future BM83 driver will need explicit callback dispatch.

Inspect `console_startup_status` in the debugger if adapter initialization fails.
Failure does not stop the LED heartbeat. No simulated test code is included.
Physical UART/GPDMA validation remains pending until the board is available:
connect a 3.3 V UART adapter (adapter RX to PA2, common ground), confirm startup
text at 115200 8N1, then check queue saturation and DMA/UART recovery.
