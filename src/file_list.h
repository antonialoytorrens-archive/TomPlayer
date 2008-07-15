/***************************************************************************
 *  19/06/2008
 *  Copyright  2008  nullpointer
 *  Email nullpointer[at]lavabit[dot]com
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

/*!
 * \file file_list.h
 * \brief file list utilities and structures
 * \author nullpointer
 */

#ifndef __FILE_LIST_H__
#define __FILE_LIST_H__
#include <stdbool.h>
#include "list.h"

/**
 * \enum E_FILE_ELT_TYPE
 */
enum E_FILE_ELT_TYPE{
	TYPE_DIR = 0,	/*!< directory */
	TYPE_FILE		/*!<regular file */
};

/**
 * \struct file_elt
 */
struct file_elt{
	char * name;	/*!<filename or foldername */
	enum E_FILE_ELT_TYPE type;	/*!<type of elt */
};

int compare_file_elt( void * e1, void * e2 );
void release_file_elt( void * e );
bool fill_list_files( char *, char *, struct list_object ** );

#endif /* __FILE_LIST_H__ */
