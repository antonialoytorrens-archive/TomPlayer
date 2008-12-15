/**
 * \file file_list.h
 * \author Stephan Rafin
 *
 * This module implements a file list object 
 *
 * $URL: $
 * $Rev: $
 * $Author: $
 * $Date: $
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

#ifndef __FILE_LIST_H__
#define __FILE_LIST_H__

#include <stdio.h>
#include <regex.h>
#include <stdbool.h> 

typedef struct file_list* file_list ;
typedef struct _fl_handle * flenum ;

file_list fl_create(const char *, regex_t *, bool);
void fl_release(file_list);
bool fl_select_by_pos(file_list, int, bool *);
bool fl_select_all(file_list);
bool fl_unselect_by_pos(file_list, int);
bool fl_is_selected(file_list, int);  
const char * fl_get_single_selection(file_list);
bool fl_is_folder(file_list, int);  
int fl_get_entries_nb(file_list);
const char * fl_get_filename(file_list, int );
const char * fl_get_basename(file_list);
int fl_get_selected_number(file_list fl);

flenum   fl_get_selection(file_list);
const char * flenum_get_next_file(flenum, bool );
bool flenum_release(flenum);

#endif
