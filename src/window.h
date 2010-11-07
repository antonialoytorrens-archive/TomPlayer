/**
 * \file window.c
 * \author nullpointer & wolfgar
 * \brief This module provides window creation and handling facilities
 *
 * This is the base object used to create and handle windows in Tomplayer.
 * The underlying objet is a DirectFB window.
 * This module enables :
 *    \li To create the window from a configuration file
 *    \li To retrieve a control in the window given its name
 *    \li To handle mouses events for every created windows
 *    \li To attach callback to any control  
 *
 * It keeps track of the created windows in a very basic stack model and delivers events only to the last created window
 *
 * \todo enable to create this object without regaular file configuration (have a configuration object)
 * \warning This module is not thread-safe (coz we dont need it at this time) :
 * \warning Especially :gui_window_load(), :gui_window_release() and :gui_window_handle_click() have to be called from the same thread...
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


#ifndef __WINDOW_H__
#define __WINDOW_H__

#include <stdbool.h>
#include <directfb.h>


/* FIFO Where key inputs can be read from */
#define KEY_INPUT_FIFO "/tmp/key_fifo"

/** define the type of control */
enum gui_type_ctrl{
	GUI_TYPE_CTRL_TEXT = 1,	      /**< a static text */
	GUI_TYPE_CTRL_BUTTON,	      /**< an icon button */
    GUI_TYPE_CTRL_CLICKABLE_ZONE, /**< a clickable zone */ 
	GUI_TYPE_CTRL_FILESELECTOR,   /**< a file selector */    
    GUI_TYPE_CTRL_VIEWMETER,      /**< A viewmeter */
    GUI_TYPE_CTRL_MAX_NB
};

/** Handle on the window object */
typedef struct _gui_window * gui_window;

struct gui_control;

union gui_event{
    struct{ 
	int x;
	int y;
    } ts;
    DFBInputDeviceKeyIdentifier key;
};

enum gui_event_type {GUI_EVT_TS, GUI_EVT_KEY, GUI_EVT_SELECT, GUI_EVT_UNSELECT};

/** Callback on a control click */
typedef void (*gui_control_cb)(struct gui_control *, enum gui_event_type, union gui_event*);

/** define a single GUI control */
struct gui_control{
	enum gui_type_ctrl type;		/**< type of control */
        char * name ;                           /**< Name of the control */
        void * obj;                             /**< pointer to the underlying object (depends on type) 
                                                  GUI_TYPE_CTRL_TEXT =>  label_handle 
                                                  GUI_TYPE_CTRL_BUTTON => IDirectFBSurface * 
                                                  GUI_TYPE_CTRL_CLICKABLE_ZONE => None
                                                  GUI_TYPE_CTRL_FILESELECTOR => fs_handle
                                                */                                                
        DFBRectangle zone ;                     /**< Zone occupied by the control in the window */
        gui_window   win;                       /**< Handle to the window that holds the control */
        gui_control_cb cb;                      /**< Callback attached to the control */
        int            cb_param;                /**< Additionnal param passed to the callback */
};


gui_window  gui_window_load(IDirectFB  *,  IDirectFBDisplayLayer *, const char * filename);
bool   gui_window_release(gui_window );
struct gui_control * gui_window_get_control(gui_window, const char *);
void   gui_window_attach_cb(gui_window, const char *, gui_control_cb);
void   gui_window_handle_click(int  x, int y);
IDirectFBSurface * gui_window_get_surface(gui_window);
void gui_window_release_all(void);
gui_window gui_window_get_top(void);
/*void   gui_window_get_pos(gui_window, int *, int* );*/

#endif /* __WINDOW_H__ */
