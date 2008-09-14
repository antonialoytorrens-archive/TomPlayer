/*
 * Driver for the ns73 FM tranmitter
 *
 *	This program is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU General Public License
 *  as published by the Free Software Foundation; either version
 *  2 of the License, or (at your option) any later version.
 *
 * Author:      Rogier Stam, <Rogier.Stamtomtom.com>
 */

#ifndef __INCLUDE_LINUX_NS73_H
#define __INCLUDE_LINUX_NS73_H       __FILE__

#include <linux/ioctl.h>
#include <linux/major.h>
#include <linux/fmtransmitter.h>

#define NS73_DEVNAME  		"NS73"
#define NS73_POWERON_FREQUENCY	100700000

#define NS73_MAJOR    		FMTRANSMITTER_MAJOR
#define NS73_MINOR    		FMTRANSMITTER_MINOR

#ifdef __KERNEL__
#define NS73_I2C_SLAVE_ADDR         (0x67)
#endif /* __KERNEL__ */

#endif /* __INCLUDE_LINUX_SI4710_H */


