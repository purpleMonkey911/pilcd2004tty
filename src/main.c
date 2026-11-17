/**
 * @file           : main.c
 * @brief          : Entry point for lcd2004tty application
 * 
 * @copyright Copyright © 2026 Purple Monkey Software. All rights reserved.
 * 
 */

#include <signal.h>
#include <stdio.h>
#include <string.h>

#include "app_config.h"
#include "i2c_inst.h"
#include "lcd2004-inst.h"
#include "mhal_i2c.h"
#include "mhal_timer.h"
#include "tty.h"
#include "uart_inst.h"

// Set by the signal handler, polled by the main loop.  sig_atomic_t is the only
// type guaranteed safe to write from a signal handler.
static volatile sig_atomic_t shutdown_requested = 0;

static void Main_SignalHandler(int signum) {
    (void)signum;
    shutdown_requested = 1;
}

/**
  * @brief Install handlers so Ctrl-C (SIGINT) and kill/systemd stop (SIGTERM)
  *        end the main loop instead of killing the process outright.
  */
static void Main_InstallSignalHandlers(void) {
    struct sigaction sa;
    memset(&sa, 0, sizeof(sa));
    sa.sa_handler = Main_SignalHandler;
    sigemptyset(&sa.sa_mask);
    sigaction(SIGINT, &sa, NULL);
    sigaction(SIGTERM, &sa, NULL);
}

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{
    Main_InstallSignalHandlers();

    // create and init I/O instances
    Uart_Init();
    I2c_Init();

    // create and init LCD
    Lcd2004Config lcd_config = {
        .i2c = I2c_GetInstance(),
        .slave_addr = LCD2004_I2C_SLAVE_ADDR,
    };
    Lcd_Init(&lcd_config);

    // create and init TTY module
    TtyConfig tty_config = {
        .uart_in = Uart_GetInstance(),
        .lcd_out = Lcd_GetInstance(),
    };
    Tty_Init(&tty_config);

    while (!shutdown_requested)
    {
        // exercise state machines
        if (mHalUart_StateMachine(Uart_GetInstance()) < MHAL_STATUS_OK) {
            // set off howling alarm!
        }
        if (mHalI2c_StateMachine(I2c_GetInstance()) < MHAL_STATUS_OK) {
            // set off howling alarm!
        }
        if (Lcd2004_StateMachine(Lcd_GetInstance()) < LCD2004_STATUS_OK) {
            // set off howling alarm
        }

        Tty_Process();
    }

    // shut down in reverse order of creation: the LCD uses the I2C bus, so it
    // goes before the bus.  TTY only holds handles, nothing to release.
    printf("\nShutting down...\n");
    Lcd_Deinit();
    I2c_Deinit();
    Uart_Deinit();
    printf("Shutdown complete\n");

    return 0;
}
