/* include/barcelona/Barc_Battery.h
 *
 * Public interface for the battery driver.
 *
 * Copyright (C) 2004,2005 TomTom BV <http://www.tomtom.com/>
 * Author: Dimitry Andric <dimitry.andric@tomtom.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#ifndef __INCLUDE_BARCELONA_BARC_BATTERY_H
#define __INCLUDE_BARCELONA_BARC_BATTERY_H

#ifndef __INCLUDE_BARCELONA_TYPES_H
#include <barcelona/types.h>
#endif /* __INCLUDE_BARCELONA_TYPES_H */

#include <linux/ioctl.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#define BATTERY_DEVNAME				"battery"
#define BATTERY_MAJOR				241

typedef struct {
	UINT16 u16BatteryVoltage;		/* battery voltage */
	UINT16 u16ChargeCurrent;		/* charge current */
	UINT8 u8ChargeStatus;
	UINT8 u8VoltageSource;
} BATTERY_STATUS;

#define CHARGE_STATE_ACPWR_OFF		0		/* AC power is off   */
#define CHARGE_STATE_FAULT		1		/* Charge fault      */
#define CHARGE_STATE_COMPLETE		2		/* Charging complete */
#define CHARGE_STATE_FAST_CHARGING	3		/* Fast charging     */
#define CHARGE_STATE_TOPPINGUP		4		/* Topping up        */

#define VOLTAGE_SOURCE_BATTERY		0		/* Standard battery  */
#define VOLTAGE_SOURCE_BATTERY_PACK	1		/* AA batter pack    */

#define BATTERY_DRIVER_MAGIC	'B' /* Battery driver magic number */
#define IOR_BATTERY_STATUS	_IOR(BATTERY_DRIVER_MAGIC, 3, BATTERY_STATUS)
#define IO_ENABLE_CHARGING	_IO(BATTERY_DRIVER_MAGIC, 4)
#define IO_DISABLE_CHARGING	_IO(BATTERY_DRIVER_MAGIC, 5)

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __INCLUDE_BARCELONA_BARC_BATTERY_H */

/* EOF */
