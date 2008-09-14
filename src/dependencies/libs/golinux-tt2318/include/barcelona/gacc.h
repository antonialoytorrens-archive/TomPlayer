/* include/barcelona/Barc_gacc.h
 *
 * Public interface for the accelerometer driver.
 *
 * Copyright (C) 2006 TomTom BV <http://www.tomtom.com/>
 * Author: Mark Vels <mark.vels@tomtom.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#ifndef __INCLUDE_BARCELONA_BARC_GACC_H
#define __INCLUDE_BARCELONA_BARC_GACC_H

#include <linux/ioctl.h>

#ifdef __KERNEL__
#include <linux/time.h>
#endif /* __KERNEL__	*/

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#define GACC_DEVPREFIX			"gacc"
#define GACC_MAJOR			(168) /* Supplied to drivers through the platform device definition	*/

#define GACC_DEBUG			(1)

#define GACC_DEFAULT_SAMPLERATE		(10)
#define GACC_MAX_SAMPLERATE		(25)
#define GACC_READING_QUEUE_SIZE		( GACC_MAX_SAMPLERATE * 2 )

typedef struct accelerometer_reading_struct {
	struct timeval	timestamp;
	short		x_axis;			/* signed two complements	*/
	short		y_axis;
	short		z_axis;
	
#ifdef GACC_DEBUG
	unsigned short	x_raw;	/* raw ADC value, hardware dependant!	*/
	unsigned short	y_raw;	/* raw ADC value, hardware dependant!	*/
	unsigned short	z_raw;	/* raw ADC value, hardware dependant!	*/
	unsigned int	seq;
#endif	/*	GACC_DEBUG	*/

} accelerometer_reading_t;

typedef struct acc_readings_desc_struct {
	
	unsigned int			count;
	accelerometer_reading_t*	head;
} acc_readings_desc_t;

typedef struct accelerometer_settings_struct {

	unsigned short sample_rate;			/* Nr of samples taken per second	*/
	unsigned short sensitivity;			/* nr of LSB to have 1 m/s²	*/
	unsigned short range;				/* indicates max value for acceleration so that:*/
										/* Max positive acc.  = range-1 / sensitivity ( m/s² )	*/
										/* Max negative	acc.  = -range / sensitivity ( m/s² )	*/
										/* Example: for an 8 bit, +/- 2 g accelerometer the range would be	*/
										/* 	range = 0x80 and sensitivity would be 0x40		*/ 
										/* Note: this assumes that measurement ranges for positive and negative x, y and z axis are identical	*/
} accelerometer_settings_t;

#define GACC_MAGIC				('6')
#define GACC_IOCTL_RESET			_IO (  GACC_MAGIC, 1)
#define GACC_IOCTL_BIST				_IOR( GACC_MAGIC, 2, int )
#define GACC_IOCTL_SET_SAMPLE_RATE	_IOW( GACC_MAGIC, 3, int )
#define GACC_IOCTL_GET_SETTINGS		_IOR( GACC_MAGIC, 4, accelerometer_settings_t )
#define GACC_IOCTL_TEST				_IO ( GACC_MAGIC, 5 )

#ifdef __cplusplus
}
#endif /* __cplusplus */



#ifdef __KERNEL__

#include <linux/types.h>
#include <linux/spinlock.h>
#include <linux/timer.h>
#include <linux/workqueue.h>

#include <barcelona/gopins.h>
/**
 * DON'T TOUCH THIS FROM USER SPACE
 * DOING SO MAY INVOLVE SEVERE HEALTH RISKS :-)
 */
 

struct gacc_platform_data {
  	dev_t 		device_nr;	/* defined by platform definition, use MKDEV( major, minor) */
  	gopin_t		irq_pin;		/* pin of the reset line	*/
};

#define GACC_FLGS_EXIT			(1<<0)			/* is set when it is time to die	*/
#define GACC_FLGS_QOVERFLOW		(1<<1)			/* is set when old data is overwritten	*/
#define GACC_FLGS_CONGESTION	(1<<2)			/* is set when sample must start while last one is not finished yet	*/

struct gacc_driver_private_data {
	spinlock_t					lock;				/* for synchronization	*/
	volatile unsigned int 		flags;				
	
	unsigned 					readings_size;		/* Total number of elements @head	*/
	volatile unsigned			readings_count;		/* The actual number of readings on store	*/
	volatile unsigned			readings_writeindex;/* The index of the next element to write	*/
	volatile unsigned			readings_readindex;	/* The index of the next element to read	*/
	accelerometer_reading_t* 	head;				/* pointer to start of store	*/
	accelerometer_settings_t* 	settings;			/* settings of this instance	*/
			
	struct timer_list	timer;
	struct workqueue_struct*	work_queue;	/* we explicitly choose a local workqueue to reduce latencies as much as possible	*/
	struct work_struct			work;
	wait_queue_head_t 			wait_queue;

	struct spi_device 	*spi_dev;			/* pointer to spi device structure	*/
	void*				private;			/* Every driver that implements the GACC interface can place driver private data here	*/
	
#ifdef GACC_DEBUG
	unsigned int				seq;
#endif	/*	GACC_DEBUG	*/
};


#endif //__KERNEL__

#endif /* __INCLUDE_BARCELONA_BARC_GACC_H */

/* EOF */
