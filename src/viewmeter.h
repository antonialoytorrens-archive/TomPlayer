/**
 * \file viewmeter.h 
 * \brief This module provides viewmeter creation and handling facilities
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

#ifndef __VIEWMETER_H__
#define __VIEWMETER_H__

#include <stdbool.h>

struct _vm_state;
typedef  struct _vm_state* vm_handle;

struct vm_config{
    IDirectFB  * dfb;     /**< DirectFB instance */
    IDirectFBWindow * win;/**< Parent window that holds the object */
    DFBRectangle pos;     /**< Postion of the viewmeter in the window */
    char * name;          /**< True Type Font filename  */
    DFBColor font_color;  /**< Font color */
    int height;           /**< Font height */
    char * format;        /**< Format of the view meter */    
    double inc;           /**< Value to increment / decrement */
    double min;           /**< Min possible value */
    double max;           /**< Max possible value */
};

vm_handle vm_create(const struct vm_config * config);
bool vm_set_value(vm_handle hdl, double val);
bool vm_inc(vm_handle hdl);
bool vm_dec(vm_handle hdl);
double vm_get_value(vm_handle hdl);
void vm_release(vm_handle hdl);

#endif

