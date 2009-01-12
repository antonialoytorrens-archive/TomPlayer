/**
 * \file label.h 
 * \brief This module provides label creation and handling facilities
 *
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
#ifndef __LABEL_H__
#define __LABEL_H__

#include <stdbool.h>
#include <directfb.h>

struct _label_state;
typedef  struct _label_state* label_handle;

struct label_config{
    IDirectFB  * dfb;     /**< DirectFB instance */
    IDirectFBWindow * win;/**< Parent window that holds the label */
    DFBRectangle pos;     /**< Postion of the label in the window */
    char * name;          /**< True Type Font filename   */
    DFBColor font_color;  /**< Font color */
    int height;           /**< Font height */
};

label_handle label_create(const struct label_config * config);
bool label_set_text(label_handle hdl, const char * msg);
void label_release(label_handle hdl);
IDirectFBFont * label_get_font(label_handle hdl);
#endif