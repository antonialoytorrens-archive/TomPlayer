/**
 * \file skin_display.h
 * \brief Handle skin display and the update of the controls
 *
 * $URL:$
 * $Rev:$
 * $Author:$
 * $Date:$
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

#ifndef __SKIN_DISPLAY_H__
#define __SKIN_DISPLAY_H__

enum skin_display_update{  
    SKIN_DISPLAY_NEW_TRACK, 
    SKIN_DISPLAY_PERIODIC    
};

void skin_display_refresh(enum skin_display_update type);

#endif
