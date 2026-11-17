/**
 * @file           : main.c
 * @brief          : Entry point for lcd2004tty application
 * 
 * @copyright Copyright © 2026 Purple Monkey Software. All rights reserved.
 * 
 */

#include "app_config.h"
#include "i2c_inst.h"
#include "lcd2004-inst.h"
#include "mhal_i2c.h"
#include "mhal_timer.h"
#include "tty.h"
#include "uart_inst.h"


/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{
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

    while (1)
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

    // we should never get here!
    return 0;
}
