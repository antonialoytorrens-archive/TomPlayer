/* include/barcelona/Barc_adc.h
 *
 * Public interface for the ADC driver.
 *
 * Copyright (C) 2004,2005 TomTom BV <http://www.tomtom.com/>
 * Author: Koen Martens <kmartens@sonologic.nl>
 * Author: Jeroen Taverne <jeroen.taverne@tomtom.com>
 * Author: Dimitry Andric <dimitry.andric@tomtom.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#ifndef __INCLUDE_BARCELONA_BARC_ADC_H
#define __INCLUDE_BARCELONA_BARC_ADC_H

#include <linux/config.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#define ADC_DEVNAME		"adc"
#define ADC_MAJOR		121

#define ADC_CHANNELS	9
#define ADC_BUFSIZE		(ADC_CHANNELS * sizeof(short))
#define ADC_RATE		100

#define ADC_BAT        0
#define ADC_ALCALINE   1
#define ADC_CHRG       7
#define ADC_REF        6
#define ADC_ACC_X      4
#define ADC_ACC_Y      3
#define ADC_ACC_Z_TEMP 1
#define ADC_GYRO       3
#define ADC_TS_DOWN    2
#define ADC_TS_X       5
#define ADC_TS_Y       8
#define ADC_LX_OUT     3
#define ADC_DOCK       4

#ifdef __KERNEL__
extern void adc_get_buffer(short *buf);
extern short adc_get_channel(int channel);
typedef void (*adc_pollfunc_t)(short buf[ADC_CHANNELS], void* arg);
extern int adc_register_poll(adc_pollfunc_t func, void* arg);
extern int adc_unregister_poll(adc_pollfunc_t func);
#endif /* __KERNEL__ */

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __INCLUDE_BARCELONA_BARC_ADC_H */

/* EOF */
