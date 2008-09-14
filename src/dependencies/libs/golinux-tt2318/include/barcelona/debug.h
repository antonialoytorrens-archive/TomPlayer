/* include/barcelona/debug.h
 *
 * Barcelona debug helper definitions.
 *
 * Copyright (C) 2004,2005 TomTom BV <http://www.tomtom.com/>
 * Author: Dimitry Andric <dimitry.andric@tomtom.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#ifndef __INCLUDE_BARCELONA_DEBUG_H
#define __INCLUDE_BARCELONA_DEBUG_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#ifdef __KERNEL__

/* Note: for compatibility with gcc 2.x, the spacing of the arguments
 * in the variadic macros should NOT be modified.  In particular, there
 * MUST be a space before the last comma. */
#define PK_DBG_NONE(fmt, arg...)	do {} while (0)
#ifdef BARCELONA_DEBUG
#define PK_DBG_PLAIN(fmt, arg...)	printk(KERN_DEBUG fmt ,##arg)
#define PK_DBG_PREFIX(fmt, arg...)	printk(KERN_DEBUG PFX fmt ,##arg)
#define PK_DBG_FUNC(fmt, arg...)	printk(KERN_DEBUG PFX "%s: " fmt, __FUNCTION__ ,##arg)
#else /* BARCELONA_DEBUG */
#define PK_DBG_PLAIN				PK_DBG_NONE
#define PK_DBG_PREFIX				PK_DBG_NONE
#define PK_DBG_FUNC					PK_DBG_NONE
#endif /* BARCELONA_DEBUG */

#define PK_ERR(fmt, arg...)			printk(KERN_ERR PFX "%s: " fmt, __FUNCTION__ ,##arg)
#define PK_WARN(fmt, arg...)		printk(KERN_WARNING PFX "%s: " fmt, __FUNCTION__ ,##arg)
#define PK_NOTICE(fmt, arg...)		printk(KERN_NOTICE PFX "%s: " fmt, __FUNCTION__ ,##arg)
#define PK_INFO(fmt, arg...)		printk(KERN_INFO PFX "%s: " fmt, __FUNCTION__ ,##arg)

#endif /* __KERNEL__ */

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __INCLUDE_BARCELONA_DEBUG_H */

/* EOF */
