/**
 * @file i2c_inst.h
 * 
 * @brief I2c instance for the app
 * 
 * @copyright Copyright © 2026 Purple Monkey Software. All rights reserved.
 * 
 */

#include "app_config.h"
#include "i2c_inst.h"

static I2c i2c_inst = 0;

void I2c_Init(void) {
    I2cConfig config = {
        .i2c_bus = I2C_PORT,
    };
    mHalI2c_Init(&i2c_inst, &config);
}

/**
 * @brief Accessor function to get I2c instance
 */ 
I2c I2c_GetInstance(void) {
    return i2c_inst;
}
