/***************************************************************************
 *
 *  14.02.08 : wolfgar - Widescreen Handling 
 ****************************************************************************/
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
#define WS_YMAX 272
#define WS_XMAX 480

/* Resolution of "normal" screen */
#define WS_NOXL_YMAX 240
#define WS_NOXL_XMAX 320

/*Prefix for widescreen filenames*/
#define WS_FILENAME_PREFIX "ws_"

extern int ws_probe(void);
extern void ws_translate(struct tomplayer_config *);

#endif
