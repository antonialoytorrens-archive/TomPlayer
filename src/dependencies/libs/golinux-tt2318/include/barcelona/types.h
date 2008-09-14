/* include/barcelona/types.h
 *
 * Backwards compatibility Barcelona typedefs.
 *
 * Copyright (C) 2004,2005 TomTom BV <http://www.tomtom.com/>
 * Author: Dimitry Andric <dimitry.andric@tomtom.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#ifndef __INCLUDE_BARCELONA_TYPES_H
#define __INCLUDE_BARCELONA_TYPES_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/*
 * NOTE: Please don't use these, use the types from linux/types.h instead.
 */

typedef unsigned int	UINT32;
typedef signed int		INT32;
typedef unsigned short	UINT16;
typedef signed short	SINT16;
typedef unsigned char	UINT8;
typedef signed char		SINT8;
typedef unsigned long	ULONG;

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __INCLUDE_BARCELONA_TYPES_H */

/* EOF */
