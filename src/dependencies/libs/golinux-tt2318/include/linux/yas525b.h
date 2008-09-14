/*
 * Driver for the Yamaha YASS525B-MC1 Magnetic Field Sensor.
 *
 *              This program is free software; you can redistribute it and/or
 *              modify it under the terms of the GNU General Public License
 *              as published by the Free Software Foundation; either version
 *              2 of the License, or (at your option) any later version.
 *
 * Author:      Ithamar R. Adema, <ithamar.adema@tomtom.com>
 */

#ifndef __INCLUDE_LINUX_YAS525B_H
#define __INCLUDE_LINUX_YAS525B_H	__FILE__

#include <linux/ioctl.h>
#include <linux/major.h>

#define YAS525B_MAJOR   MISC_MAJOR
#define YAS525B_MINOR   240

#define YAS525B_DRIVER_MAGIC		'Y'

#define YAS525B_COILINIT		_IO(YAS525B_DRIVER_MAGIC,1)
#define YAS525B_GETROUGHOFFSET		_IOR(YAS525B_DRIVER_MAGIC,2,__u32)
#define YAS525B_SETROUGHOFFSET		_IOW(YAS525B_DRIVER_MAGIC,3,__u32)
#define YAS525B_GETCALIBDATA		_IOR(YAS525B_DRIVER_MAGIC,4,__u32)

#endif /* __INCLUDE_LINUX_YAS525B_H */

