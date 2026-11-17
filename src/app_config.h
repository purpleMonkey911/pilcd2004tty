/**
 * @file app_config.h
 * @brief Application parameters
 *
 * Copyright © 2026 Purple Monkey Software. All rights reserved.
 *
 */

#ifndef __APP_CONFIG_H__
#define __APP_CONFIG_H__

#include "mhal_i2c.h"
#include "mhal_uart.h"

// UART settings
#define UART_PORT                    "/dev/ttyAMA0"
#define UART_BAUD                    UART_BAUD_RATE_115200
#define UART_DATA_BITS               UART_DATA_BITS_8
#define UART_STOP_BITS               UART_STOP_BITS_1
#define UART_PARITY                  UART_PARITY_NONE
#define UART_RING_BUFFER_SIZE_BYTES  128

// I2C settings
#define I2C_PORT                "/dev/i2c-1"
#define LCD2004_I2C_SLAVE_ADDR  0x27

#endif // __APP_CONFIG_H__
