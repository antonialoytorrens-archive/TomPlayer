/**
 * \file widescreen.h
 * \author Wolfgar 
 * \brief This module provides functions related to screens
 * 
 * $URL$
 * $Rev$
 * $Author$
 * $Date$
 *
 */

/*
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */



#ifndef __TOMPLAYER_WIDESCRREN_H__
#define __TOMPLAYER_WIDESCRREN_H__

#include "config.h"

/* Widescreen Resolution */
/**
 * \def WS_YMAX
 */
#define WS_YMAX 272

/**
 * \def WS_XMAX
 */
#define WS_XMAX 480

/* Resolution of "normal" screen */
/**
 * \def WS_NOXL_YMAX
 */
#define WS_NOXL_YMAX 240

/**
 * \def WS_NOXL_XMAX
 */
#define WS_NOXL_XMAX 320

/* Macros to switch from WS to normal and vice versa */
#define RESIZE(a,b,c) (a) = (a) * (1.0 * b) /c
#define RESIZE_X(a) RESIZE(a, ws_probe()?WS_XMAX:WS_NOXL_XMAX, ws_probe()?WS_NOXL_XMAX:WS_XMAX)
#define RESIZE_Y(a) RESIZE(a, ws_probe()?WS_YMAX:WS_NOXL_YMAX, ws_probe()?WS_NOXL_YMAX:WS_YMAX)

/**
 * \def WS_FILENAME_PREFIX
 * \brief Prefix for widescreen filenames
 */
#define WS_FILENAME_PREFIX "ws_"

extern int ws_probe(void);
extern bool ws_are_axes_inverted(void);
extern bool ws_get_size(int *, int * );
#endif
