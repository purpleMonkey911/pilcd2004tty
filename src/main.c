/**
 * @file           : main.c
 * @brief          : Entry point for lcd2004tty application
 * 
 * @copyright Copyright © 2026 Purple Monkey Software. All rights reserved.
 * 
 */

#define _GNU_SOURCE   // ppoll()
#include <poll.h>
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
  * @brief Sleep until there is something to do: UART data received, an I2C
  *        transfer completed, an LCD timing delay expired, or a signal.
  *
  *        Tty_Process() drains the UART RX buffer completely on every pass,
  *        so only the LCD can have work left over.
  *
  * @param sigmask   signal mask to use while sleeping (SIGINT/SIGTERM
  *                  unblocked)
  */
static void Main_WaitForWork(const sigset_t* sigmask) {
    int32_t wait_us = Lcd2004_GetWaitTimeUs(Lcd_GetInstance());

    // fds of -1 (instance failed to init) are ignored by ppoll()
    struct pollfd fds[] = {
        { .fd = mHalUart_GetFd(Uart_GetInstance()),     .events = POLLIN },
        { .fd = mHalI2c_GetEventFd(I2c_GetInstance()),  .events = POLLIN },
    };

    // wait_us < 0: no timeout, sleep until an fd is ready or a signal arrives
    struct timespec timeout;
    struct timespec* ptimeout = NULL;
    if (wait_us >= 0) {
        timeout.tv_sec  = wait_us / 1000000;
        timeout.tv_nsec = (wait_us % 1000000) * 1000L;
        ptimeout = &timeout;
    }

    // ppoll() is called even when wait_us == 0: it's the only point where
    // SIGINT/SIGTERM are unblocked, so a busy loop must still pass through it
    ppoll(fds, sizeof(fds) / sizeof(fds[0]), ptimeout, sigmask);
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

    // Keep SIGINT/SIGTERM blocked except while sleeping in ppoll().  Without
    // this, a Ctrl-C landing between the shutdown_requested check and the
    // sleep would be missed, and the app would sleep until the next event.
    sigset_t block_mask, wait_mask;
    sigemptyset(&block_mask);
    sigaddset(&block_mask, SIGINT);
    sigaddset(&block_mask, SIGTERM);
    sigprocmask(SIG_BLOCK, &block_mask, &wait_mask);  // wait_mask = previous mask

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

        Main_WaitForWork(&wait_mask);
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
