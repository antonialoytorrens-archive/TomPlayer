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
 * \file list.h
 * \author nullpointer
 * \brief basic list manipulation
 */

#ifndef __LIST_H__
#define __LIST_H__

/**
 * \struct list_object
 * \brief basic structure to store element list
 */
struct list_object{
	void * object;		/*!< object stored */
	struct list_object * next; /*!<adress of the following object */
};

struct list_object * new_element_list( void );
void release_list( struct list_object *, void (*release_object)(void *)  );
void add_to_list( struct list_object **, void * );
void add_to_list_sorted( struct list_object **, void *, int (*compare)( void *, void * ) );
int get_list_count( struct list_object * );
void * get_object_from_list( struct list_object *, int );

#endif /* __LIST_H__ */
