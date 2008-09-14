/*
 * Driver for the dock driver
 *
 *      This program is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU General Public License
 *  as published by the Free Software Foundation; either version
 *  2 of the License, or (at your option) any later version.
 *
 * Author:      Rogier Stam, <Rogier.Stam@tomtom.com>
 */

#ifndef __INCLUDE_BARCELONA_DOCK_H
#define __INCLUDE_BARCELONA_DOCK_H      __FILE__
extern void DOCK_DisablePullResistor( unsigned pinnr );
extern void DOCK_SetPullResistor(gopin_t pin);
extern void DOCK_SetBit(gopin_t pin, unsigned value);
extern signed DOCK_GetInterruptNumber( gopin_t pin );
extern signed DOCK_SetInterrupt( gopin_t pin );
extern signed DOCK_SetInterruptOnActivation( gopin_t pin );
extern signed DOCK_SetInterruptOnDeactivation( gopin_t pin );
extern signed DOCK_SetInterruptOnToggle( gopin_t pin );
extern void DOCK_SetInput( gopin_t pin );
extern int DOCK_GetInput( gopin_t pin );
#endif
