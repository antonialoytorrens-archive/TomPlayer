/* include/barcelona/gyro.h
 *
 * Public interface for the gyro/ angular rate sensor driver.
 *
 * Copyright (C) 2007 TomTom BV <http://www.tomtom.com/>
 * Author: Mark Vels <mark.vels@tomtom.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#ifndef __INCLUDE_BARCELONA_BARC_GYRO_H
#define __INCLUDE_BARCELONA_BARC_GYRO_H

#include <linux/ioctl.h>

#ifdef __KERNEL__
#include <linux/time.h>
#endif /* __KERNEL__	*/


#ifndef __KERNEL__
#define CONFIG_BARCELONA_GYRO_ING300
#endif /* __KERNEL__	*/


#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#define GYRO_DEVPREFIX		"gyro"
#define GYRO_MAJOR			(169) /* Supplied to drivers through the platform device definition	*/

#define GYRO_DEBUG			(1)

#define GYRO_DEFAULT_SAMPLERATE		(10)
#define GYRO_MAX_SAMPLERATE			(25)
#define GYRO_READING_QUEUE_SIZE		( GYRO_MAX_SAMPLERATE * 2 )
#define GYRO_MAX_TEMP_SIZE			(10)

typedef struct gyro_reading_struct {
	struct timeval	timestamp;

	short			x_value;	/* only filled in case of InvenSense sensor	*/
	short			y_value;	/* idem	*/

	short			value;		/* Only filled in case of Fujitsu sensor	*/
		
#ifdef GYRO_DEBUG
	//unsigned short	rawvalue;	/* raw ADC value, hardware dependant!	*/
	unsigned int 	seq;		/* seqnr for testing	*/
#endif /* DEBUG	*/
} gyro_reading_t;

typedef struct gyro_readings_desc_struct {
	
	unsigned int			count;
	gyro_reading_t*	head;
} gyro_readings_desc_t;

typedef struct gyro_settings_struct {

	unsigned short sample_rate;			/* Nr of samples taken per second	*/
	unsigned short sensitivity;			/* nr of LSB to have 1 m/s²	*/
	unsigned short range;				/* indicates max value for acceleration so that:*/
										/* Max positive acc.  = range-1 / sensitivity ( m/s² )	*/
										/* Max negative	acc.  = -range / sensitivity ( m/s² )	*/
										/* Example: for an 8 bit, +/- 2 g accelerometer the range would be	*/
										/* 	range = 0x80 and sensitivity would be 0x40		*/ 
										/* Note: this assumes that measurement ranges for positive and negative x, y and z axis are identical	*/
} gyro_settings_t;

#define GYRO_MAGIC					('&')
#define GYRO_IOCTL_RESET			_IO (  GYRO_MAGIC, 1)
#define GYRO_IOCTL_BIST				_IOR( GYRO_MAGIC, 2, int )
#define GYRO_IOCTL_SET_SAMPLE_RATE	_IOW( GYRO_MAGIC, 3, int )
#define GYRO_IOCTL_GET_SETTINGS		_IOR( GYRO_MAGIC, 4, gyro_settings_t )
#define GYRO_IOCTL_TEST				_IO ( GYRO_MAGIC, 5 )

#ifdef __cplusplus
}
#endif /* __cplusplus */



#ifdef __KERNEL__

#include <linux/types.h>
#include <linux/spinlock.h>
#include <linux/timer.h>
#include <linux/workqueue.h>

#include <barcelona/gopins.h>
#include <barcelona/Barc_adc.h>
/**
 * DON'T TOUCH THIS FROM USER SPACE
 * DOING SO MAY INVOLVE SEVERE HEALTH RISKS :-)
 */
 

#define GYRO_FLGS_EXIT			(1<<0)			/* is set when it is time to die	*/
#define GYRO_FLGS_QOVERFLOW		(1<<1)			/* is set when old data is overwritten	*/
#define GYRO_FLGS_CONGESTION	(1<<2)			/* is set when sample must start while last one is not finished yet	*/

#define GYRO_ADC_CHANNEL		(ADC_GYRO)		/* Only use to initialize driver, must become dynamic	*/

struct gyro_platform_data {
  	dev_t 		device_nr;			/* defined by platform definition, use MKDEV( major, minor) */
  	gopin_t		gyro_enable_pin;	/* pin of the reset line	*/
  	int			adc_channel[2];		/* first x, then y	*/
};

struct gyro_driver_private_data {
	spinlock_t			lock;				/* for synchronization	*/
	volatile unsigned int 		flags;				
	unsigned int				channel;			/* ADC channel number and index into gyro_private_list */
	
	unsigned					readings_size;		/* Total number of elements @head	*/
	volatile unsigned			readings_count;		/* The actual number of readings on store	*/
	volatile unsigned			readings_writeindex;/* The index of the next element to write	*/
	volatile unsigned			readings_readindex;	/* The index of the next element to read	*/
	gyro_reading_t*				head;				/* pointer to start of store	*/
	gyro_settings_t*			settings;			/* settings of this instance	*/
			
	struct timer_list	timer;
	struct workqueue_struct*	work_queue;	/* we explicitly choose a local workqueue to reduce latencies as much as possible	*/
	struct work_struct			work;
	wait_queue_head_t 			wait_queue;

	int							adc_channel[2];
	int							last_value[2];
	struct timeval				last_timestamp;
	
#ifdef GYRO_DEBUG
	unsigned int				seq;
#endif	/* GYRO_DEBUG	*/
};


#endif //__KERNEL__

#endif /* __INCLUDE_BARCELONA_BARC_GYRO_H */

/* EOF */

