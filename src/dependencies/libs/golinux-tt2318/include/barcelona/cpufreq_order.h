/* include/barcelona/cpufreq_order.h
 *
 * Order of the CPUFrequency routines (which get changed first, which last).
 *
 * Copyright (C) 2007 TomTom BV <http://www.tomtom.com/>
 * Author: Rogier Stam <rogier.stam@tomtom.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#ifndef __INCLUDE_CPUFREQ_ORDER_H
#define __INCLUDE_CPUFREQ_ORDER_H

/* Defines denoting the order. The higher the number, the lower the time between the PRECHANGE and POSTCHANGE. */
#define CPUFREQ_ORDER_S3C24XX_SERIAL_PRIO	255
#define CPUFREQ_ORDER_S3C24XX_TTGFB_PRIO	235
#define CPUFREQ_ORDER_S3C24XX_PWM_PRIO		215
#define CPUFREQ_ORDER_S3C24XX_SOUND_PRIO	195
#define CPUFREQ_ORDER_S3C24XX_TIME_PRIO		175
#define CPUFREQ_ORDER_S3C24XX_ADC_PRIO		155
#define CPUFREQ_ORDER_S3C24XX_WDT_PRIO		135
#define CPUFREQ_ORDER_S3C24XX_BUZ_PRIO		115
#define CPUFREQ_ORDER_S3C24XX_OHCI_HOST_PRIO	 95
#define CPUFREQ_ORDER_S3C24XX_I2C_PRIO		 75
#define CPUFREQ_ORDER_S3C24XX_SPI_PRIO		 55
#define CPUFREQ_ORDER_S3C24XX_CAMERA_PRIO	 35

#endif /* __INCLUDE_CPUFREQ_ORDER_H */
/* EOF */
