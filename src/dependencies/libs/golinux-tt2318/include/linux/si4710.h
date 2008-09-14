/*
 * Driver for the si4710 FM tranmitter
 *
 *	This program is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU General Public License
 *  as published by the Free Software Foundation; either version
 *  2 of the License, or (at your option) any later version.
 *
 * Author:      Moore Mao, <Moore.Mao@tomtom.com>
 */

#ifndef __INCLUDE_LINUX_SI4710_H
#define __INCLUDE_LINUX_SI4710_H       __FILE__

#include <linux/ioctl.h>
#include <linux/major.h>
#include <linux/fmtransmitter.h>

#define SI4710_DEVNAME  		"SI4710"

#define SI4710_MAJOR    		FMTRANSMITTER_MAJOR
#define SI4710_MINOR    		FMTRANSMITTER_MINOR

#undef SI4710_SW_TEST
#ifdef SI4710_SW_TEST
#define IOW_SUSPEND				_IOW(FMTRANSMITTER_DRIVER_MAGIC,32, unsigned int)
#define IOW_RESUME				_IOW(FMTRANSMITTER_DRIVER_MAGIC,33, unsigned int)
#endif /*  SI4710_SW_TEST	*/

#ifdef __KERNEL__
#define SI4710_I2C_SLAVE_ADDR         (0x11)
#endif /* __KERNEL__ */

#endif /* __INCLUDE_LINUX_SI4710_H */


