/*
 * Driver for the mcp23008 IO expander
 *
 *	This program is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU General Public License
 *  as published by the Free Software Foundation; either version
 *  2 of the License, or (at your option) any later version.
 *
 * Author:      Rogier Stam, <Rogier.Stam@tomtom.com>
 */

#ifndef __INCLUDE_LINUX_MCP23008_H
#define __INCLUDE_LINUX_MCP23008_H	__FILE__

#include <linux/ioctl.h>
#include <linux/major.h>

#define MCP23008_DEVNAME  		"MCP23008-IOExp"

#ifdef __KERNEL__
#define MCP23008_I2C_SLAVE_ADDR			(0x20)
#define MCP23008_IOPIN_OUTPUT			(1 << 0)
#define MCP23008_IOPIN_INPUT			(0 << 0)
#define MCP23008_IOPIN_NONINVERTED		(1 << 1)
#define MCP23008_IOPIN_INVERTED			(0 << 1)
#define MCP23008_IOPIN_INTERRUPT		(1 << 2)
#define MCP23008_IOPIN_NOINTERRUPT		(0 << 2)
#define MCP23008_IOPIN_DEFVAL_HIGH		(1 << 3)
#define MCP23008_IOPIN_DEFVAL_LOW		(0 << 3)
#define MCP23008_IOPIN_IRQ_CMPDEFVAL		(1 << 4)
#define MCP23008_IOPIN_IRQ_CMPPREVVAL		(0 << 4)
#define MCP23008_IOPIN_INTPULLUP		(1 << 5)
#define MCP23008_IOPIN_NOINTPULLUP		(0 << 5)

#define MCP23008_IOEXPCFG_SEQOP_DISABLE		(1 << 5)
#define MCP23008_IOEXPCFG_SEQOP_ENABLE		(0 << 5)
#define MCP23008_IOEXPCFG_SDA_DISSLW_DISABLE	(1 << 4)
#define MCP23008_IOEXPCFG_SDA_DISSLW_ENABLE	(0 << 4)
#define MCP23008_IOEXPCFG_HAEN_ENABLE		(1 << 3)
#define MCP23008_IOEXPCFG_HAEN_DISABLE		(0 << 3)
#define MCP23008_IOEXPCFG_INT_OPENDRAIN		(1 << 2)
#define MCP23008_IOEXPCFG_INT_ACTIVEDRIVE	(0 << 2)
#define MCP23008_IOEXPCFG_INTPOL_ACTHIGH	(1 << 1)
#define MCP23008_IOEXPCFG_INTPOL_ACTLOW		(0 << 1)

struct ioexp_info {
                unsigned char           slave_address;
};

struct ioexp_handler
{
	atomic_t is_valid;
	atomic_t use_count;
	struct i2c_client *client;
	int (*set_ioexp_config)( struct i2c_client *, unsigned char );
	int (*get_intflag_pinmask)( struct i2c_client * );
	int (*get_pinval)( struct i2c_client *, int );
	int (*set_pinval)( struct i2c_client *, int, int );
	int (*set_pincfg)( struct i2c_client *, int, int );
	int (*get_pincfg)( struct i2c_client *, int );
};

#define IOEXP_SetConfig( handler, cfg_flags )		( ((struct ioexp_handler *) (handler))->set_ioexp_config( ((struct ioexp_handler *) (handler))->client, cfg_flags ) )
#define IOEXP_GetIntFlagPinMask( handler )		( ((struct ioexp_handler *) (handler))->get_intflag_pinmask( ((struct ioexp_handler *) (handler))->client ) )
#define IOEXP_GetPinval( handler, pin )			( ((struct ioexp_handler *) (handler))->get_pinval( ((struct ioexp_handler *) (handler))->client, pin ) )
#define IOEXP_SetPinval( handler, pin, val )		( ((struct ioexp_handler *) (handler))->set_pinval( ((struct ioexp_handler *) (handler))->client, pin, val ) )
#define IOEXP_SetPincfg( handler, pin_mask, cfg_mask )	( ((struct ioexp_handler *) (handler))->set_pincfg( ((struct ioexp_handler *) (handler))->client, pin_mask, cfg_mask ) )
#define IOEXP_GetPincfg( handler, pin )			( ((struct ioexp_handler *) (handler))->get_pincfg( ((struct ioexp_handler *) (handler))->client, pin ) )

#define IOEXP_GetHandle( handler_symbol )               ({struct ioexp_handler  *ioexp_handle; \
                                                          unsigned long int     flags; \
                                                          local_irq_save( flags ); \
                                                          ioexp_handle=(struct ioexp_handler *) symbol_get( handler_symbol ); \
                                                          if( ioexp_handle != NULL ) \
                                                          { \
                                                                if( atomic_read( &(ioexp_handle->is_valid) ) > 0 ) \
                                                                        atomic_inc( &(ioexp_handle->use_count) ); \
                                                                else \
                                                                { \
                                                                        symbol_put( handler_symbol ); \
                                                                        ioexp_handle=NULL; \
                                                                } \
                                                          } \
                                                          local_irq_restore( flags ); \
                                                          ioexp_handle;})
#define IOEXP_PutHandle( handler_symbol )               ({struct ioexp_handler  *ioexp_handle; \
                                                          unsigned long int     flags; \
                                                          local_irq_save( flags ); \
                                                          ioexp_handle=(struct ioexp_handler *) symbol_get( handler_symbol ); \
                                                          if( ioexp_handle ) \
                                                          { \
                                                                if( atomic_read( &ioexp_handle->use_count ) ) \
                                                                { \
                                                                        atomic_dec( &(ioexp_handle->use_count) ); \
                                                                        symbol_put( handler_symbol ); \
                                                                } \
                                                                symbol_put( handler_symbol ); \
                                                          } \
                                                          local_irq_restore( flags ); \
                                                         })
#endif /* __KERNEL__ */

#endif /* __INCLUDE_LINUX_MCP23008_H */


