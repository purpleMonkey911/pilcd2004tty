/**
 * @file uart_inst.c
 * 
 * @brief UART instance for the app
 * 
 * @copyright Copyright © 2026 Purple Monkey Software. All rights reserved.
 * 
 */

#include "app_config.h"

static Uart uart_inst = 0;

void Uart_Init(void) {
    UartConfig uart_config = {
        .port = UART_PORT,
        .baud = UART_BAUD,
        .data_bits = UART_DATA_BITS,
        .stop_bits = UART_STOP_BITS,
        .parity_bits = UART_PARITY,
        .ring_buffer_size = UART_RING_BUFFER_SIZE_BYTES,
    };
    mHalUart_Init(&uart_inst, &uart_config);
}

void Uart_Deinit(void) {
    mHalUart_Deinit(&uart_inst);
}

Uart Uart_GetInstance(void) {
    return uart_inst;
}
