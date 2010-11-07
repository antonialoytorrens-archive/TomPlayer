/**
 * \file file_selector.h
 * \author Stephan Rafin
 *
 * This module implements a directfb based file selector object.
 *
 * This file selector aims at being used with a touchscreen.
 *
 * It has the following features : 
 *    \li Enable navigation (of course)
 *    \li Filtering facilities
 *    \li One or multiple file(s) selection (Multiple selection possible only in the same folder)
 *    \li Preview facility 
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


#ifndef __FILE_SELECTOR_H__
#include <directfb.h>
#include <stdint.h>
#include "file_list.h"

/** Different kind of icons used by the file selector object */
enum fs_icon_ids{
    FS_ICON_UP,
    FS_ICON_DOWN,
    FS_ICON_CHECK,
    FS_ICON_FOLDER,
    FS_MAX_ICON_ID 
};

enum  fs_events_type{
  FS_EVT_UNSELECT,
  FS_EVT_SELECT,
  FS_EVT_RELEASE
};


/** File selector configuration */
struct fs_config{
  struct {
    char * filename[FS_MAX_ICON_ID]; /**< Icons filenames */
    char * font;                     /**< True Type Font filename   */
    DFBColor font_color;                   /**< Font color */
  } graphics;

  struct {
    DFBRectangle pos;                      /**< Postion of the file selector in the window */
    int preview_width_ratio;              /**< preview zone ratio (as a percent of the file selector width)*/ 
  } geometry;

  struct {  
    bool preview_box ;                    /**< Is there a preview zone */
    bool multiple_selection;              /**< Are multiple file selection enabled*/
    bool events_thread;                   /**< Is there an autonomous thread to handle events 
                                              (if no :fs_handle_click() has to be called on mouse click on the file selector) */
  } options;
  
  struct {
    char * filter;                  /**< Extended posix regular expression to apply as a filter on files. Null if no filter needed */
    char * pathname;                /**< Directory path to display */
  } folder;
} ;


/** Handle on a file selector */
typedef struct fs_data * fs_handle;
/** Handle on a file selector list */
typedef struct _fl_handle * fslist;
/** Callback prototype on select/unselect file */
typedef void (select_cb)(fs_handle, const char *,enum  fs_events_type);

fs_handle fs_create (IDirectFB  *, IDirectFBWindow *,const struct fs_config *);
bool fs_release(fs_handle);
bool fs_set_select_cb(fs_handle, select_cb * );
void fs_handle_click(fs_handle,int , int );
void  fs_handle_key(fs_handle, DFBInputDeviceKeyIdentifier);
bool fs_new_path(fs_handle hdl, const char * path, const char * filter);
bool fs_select(fs_handle, int);
bool fs_select_all(fs_handle);
bool fs_unselect_all(fs_handle);
int  fs_get_selected_number(fs_handle);
int  fs_select_filename(fs_handle, const char * );
bool fs_set_first_displayed_item(fs_handle, int );
IDirectFBSurface * fs_get_preview_surface(fs_handle);
IDirectFBWindow *  fs_get_window(fs_handle);
const struct fs_config * fs_get_config(fs_handle);
const char * fs_get_single_selection(fs_handle);
const char * fs_get_folder(fs_handle hdl);

flenum  fs_get_selection(fs_handle);

#endif /* __FILE_SELECTOR_H__ */
