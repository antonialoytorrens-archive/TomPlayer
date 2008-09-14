/*
 * Driver for the Fortemedia fm2010 AEC DSP.
 *
 *              This program is free software; you can redistribute it and/or
 *              modify it under the terms of the GNU General Public License
 *              as published by the Free Software Foundation; either version
 *              2 of the License, or (at your option) any later version.
 *
 * Author:      Ithamar R. Adema, <ithamar.adema@tomtom.com>
 */

#ifndef __INCLUDE_LINUX_FM2010_H
#define __INCLUDE_LINUX_FM2010_H       __FILE__

#include <linux/ioctl.h>
#include <linux/major.h>

#define FM2010_MAJOR	MISC_MAJOR
#define FM2010_MINOR	241

struct fm2010args
{
	__u16	addr;
	__u16	data;
};


#define FM2010_DRIVER_MAGIC            'Y'

#define FM2010_MEMWRITE			_IOR(FM2010_DRIVER_MAGIC,1,struct fm2010arg*)
#define FM2010_MEMREAD			_IOW(FM2010_DRIVER_MAGIC,2,struct fm2010arg*)
#define FM2010_LONGREGWRITE		_IOW(FM2010_DRIVER_MAGIC,3,struct fm2010arg*)
#define FM2010_REGREAD			_IOW(FM2010_DRIVER_MAGIC,4,struct fm2010arg*)
#define FM2010_READADDR			_IOW(FM2010_DRIVER_MAGIC,5,struct fm2010arg*)
#define FM2010_ANALOGBYPASS_ENABLE	_IO(FM2010_DRIVER_MAGIC,6)
#define FM2010_ANALOGBYPASS_DISABLE	_IO(FM2010_DRIVER_MAGIC,7)
#define FM2010_ACTIVATE_PWRDN	_IO(FM2010_DRIVER_MAGIC,8)
#define FM2010_DEACTIVATE_PWRDN	_IO(FM2010_DRIVER_MAGIC,9)
#define FM2010_ACTIVATE_RST	_IO(FM2010_DRIVER_MAGIC,10)
#define FM2010_DEACTIVATE_RST	_IO(FM2010_DRIVER_MAGIC,11)

#endif /* __INCLUDE_LINUX_FM2010_H */

