/* include/barcelona/s3c24xx_gpio_i2c.h
 *
 * S3C24XX GPIO pin I2C driver header file.
 *
 * Copyright (C) 2006 TomTom BV <http://www.tomtom.com/>
 * Author: Rogier Stam <rogier.stam@tomtom.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#include <barcelona/gopins.h>

#ifndef __INCLUDE_BARCELONA_S3C24XX_GPIO_I2C_H
#define __INCLUDE_BARCELONA_S3C24XX_GPIO_I2C_H
struct s3c24xx_gpio_i2c_pins
{
        gopin_t scl_pin;
        gopin_t sda_pin;
};
#endif
